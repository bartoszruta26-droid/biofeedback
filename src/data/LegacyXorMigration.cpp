#include "data/LegacyXorMigration.hpp"
#include "data/Encryption.hpp"

#include <openssl/evp.h>
#include <openssl/crypto.h>

#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <limits>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <unistd.h>

namespace {
constexpr std::size_t kMaximumLegacyFileSize = 100U * 1024U * 1024U;

bool readRegularFile(const std::string& path, std::string& data, std::string& error) {
    const int fd = ::open(path.c_str(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (fd < 0) { error = "Nie można otworzyć pliku źródłowego: " + std::string(std::strerror(errno)); return false; }
    struct stat st{};
    if (::fstat(fd, &st) != 0 || !S_ISREG(st.st_mode) || st.st_size < 0 ||
        static_cast<std::uint64_t>(st.st_size) > kMaximumLegacyFileSize) {
        error = "Źródło musi być zwykłym plikiem o rozmiarze do 100 MiB";
        ::close(fd); return false;
    }
    data.clear();
    data.resize(static_cast<std::size_t>(st.st_size));
    std::size_t offset = 0;
    while (offset < data.size()) {
        const ssize_t n = ::read(fd, &data[offset], data.size() - offset);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) { error = "Błąd odczytu pliku źródłowego"; ::close(fd); return false; }
        offset += static_cast<std::size_t>(n);
    }
    char extra;
    const ssize_t extraRead = ::read(fd, &extra, 1);
    const bool closeOk = ::close(fd) == 0;
    if (extraRead != 0 || !closeOk) { error = "Plik źródłowy zmienił się podczas odczytu lub wystąpił błąd zamknięcia"; return false; }
    return true;
}

bool strictBase64Decode(const std::string& input, std::vector<unsigned char>& output, std::string& error) {
    if (input.empty() || input.size() % 4 != 0 || input.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        error = "Nieprawidłowa długość legacy Base64"; return false;
    }
    std::size_t padding = 0;
    if (input.back() == '=') ++padding;
    if (input.size() > 1 && input[input.size()-2] == '=') ++padding;
    for (std::size_t i=0; i<input.size(); ++i) {
        const unsigned char c = static_cast<unsigned char>(input[i]);
        const bool alpha = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
                           (c >= '0' && c <= '9') || c == '+' || c == '/';
        if (i < input.size()-padding ? !alpha : c != '=') {
            error = "Legacy plik nie jest kanonicznym Base64"; return false;
        }
    }
    if (padding > 2 || (padding == 1 && input[input.size()-2] == '=')) {
        error = "Nieprawidłowe dopełnienie Base64"; return false;
    }
    output.resize((input.size()/4)*3);
    const int decoded = EVP_DecodeBlock(output.data(),
        reinterpret_cast<const unsigned char*>(input.data()), static_cast<int>(input.size()));
    if (decoded < 0 || static_cast<std::size_t>(decoded) < padding) {
        error = "Dekodowanie legacy Base64 nie powiodło się"; return false;
    }
    output.resize(static_cast<std::size_t>(decoded)-padding);
    return true;
}

bool writeExclusiveFile(const std::string& path, const std::string& data, std::string& error) {
    // Stage in the destination directory, then publish with link(2), which
    // fails rather than replacing an existing destination.
    std::string temporary = path + ".tmp.XXXXXX";
    std::vector<char> name(temporary.begin(), temporary.end());
    name.push_back('\0');
    const int fd = ::mkstemp(name.data());
    if (fd < 0) { error = "Nie można utworzyć pliku tymczasowego: " + std::string(std::strerror(errno)); return false; }
    const std::string tempPath(name.data());
    bool ok = (::fchmod(fd, 0600) == 0);
    std::size_t offset = 0;
    while (ok && offset < data.size()) {
        const ssize_t n = ::write(fd, data.data()+offset, data.size()-offset);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) { ok = false; break; }
        offset += static_cast<std::size_t>(n);
    }
    if (ok && ::fsync(fd) != 0) ok = false;
    const int closeResult = ::close(fd);
    if (!ok || closeResult != 0) {
        error = "Nie udało się zapisać i utrwalić pliku tymczasowego";
        ::unlink(tempPath.c_str());
        return false;
    }
    if (::link(tempPath.c_str(), path.c_str()) != 0) {
        error = "Nie można opublikować pliku wynikowego bez nadpisania: " + std::string(std::strerror(errno));
        ::unlink(tempPath.c_str());
        return false;
    }
    if (::unlink(tempPath.c_str()) != 0) {
        error = "Plik został opublikowany, ale nie udało się usunąć nazwy tymczasowej";
        return false;
    }
    return true;
}

bool syncParentDirectory(const std::string& path) {
    const auto slash = path.find_last_of('/');
    const std::string dir = slash == std::string::npos ? "." : (slash == 0 ? "/" : path.substr(0, slash));
    const int fd = ::open(dir.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (fd < 0) return false;
    const bool ok = ::fsync(fd) == 0;
    ::close(fd);
    return ok;
}
} // namespace

bool migrateLegacyXorFile(const std::string& sourcePath,
                          const std::string& destinationPath,
                          const std::string& backupPath,
                          const std::string& legacyXorKey,
                          const std::string& aesKey,
                          const std::string& expectedPlaintextSha256,
                          LegacyXorMigrationResult& result,
                          std::string& error) {
    result = {};
    error.clear();
    if (sourcePath.empty() || destinationPath.empty() || backupPath.empty() ||
        sourcePath == destinationPath || sourcePath == backupPath || destinationPath == backupPath) {
        error = "Ścieżki źródła, kopii i wyniku muszą być różne"; return false;
    }
    if (legacyXorKey.empty()) { error = "Brak starego klucza XOR"; return false; }
    if (aesKey.size() < 32) { error = "Nowy klucz AES musi mieć co najmniej 32 bajty"; return false; }
    if (expectedPlaintextSha256.size() != 64 ||
        expectedPlaintextSha256.find_first_not_of("0123456789abcdef") != std::string::npos) {
        error = "Wymagany jest niezależnie ustalony SHA-256 tekstu jawnego (64 małe znaki hex)"; return false;
    }

    struct stat dst{}, bak{};
    if (::lstat(destinationPath.c_str(), &dst) == 0 || errno != ENOENT ||
        ::lstat(backupPath.c_str(), &bak) == 0 || errno != ENOENT) {
        error = "Plik docelowy lub kopia zapasowa już istnieje; migrator nie nadpisuje plików"; return false;
    }

    std::string legacyEncoded;
    if (!readRegularFile(sourcePath, legacyEncoded, error)) return false;
    std::vector<unsigned char> encryptedBytes;
    if (!strictBase64Decode(legacyEncoded, encryptedBytes, error)) return false;

    std::string plaintext(encryptedBytes.size(), '\0');
    for (std::size_t i=0; i<encryptedBytes.size(); ++i)
        plaintext[i] = static_cast<char>(encryptedBytes[i] ^ static_cast<unsigned char>(legacyXorKey[i % legacyXorKey.size()]));
    OPENSSL_cleanse(encryptedBytes.data(), encryptedBytes.size());

    try {
        if (sha256Hex(plaintext) != expectedPlaintextSha256) {
            OPENSSL_cleanse(plaintext.data(), plaintext.size());
            error = "SHA-256 odszyfrowanych danych nie zgadza się z niezależnym wzorcem; błędny klucz XOR lub uszkodzone źródło";
            return false;
        }
    } catch (const std::exception& ex) {
        OPENSSL_cleanse(plaintext.data(), plaintext.size());
        error = ex.what(); return false;
    }
    std::string newCiphertext;
    try {
        newCiphertext = Encryption::encrypt(plaintext, aesKey);
        std::string verification = Encryption::decrypt(newCiphertext, aesKey);
        const bool plaintextMatches = (verification == plaintext);
        OPENSSL_cleanse(verification.data(), verification.size());
        if (!plaintextMatches) {
            OPENSSL_cleanse(plaintext.data(), plaintext.size());
            error = "Weryfikacja AES-GCM nie odtworzyła dokładnie danych wejściowych"; return false;
        }
    } catch (const std::exception& ex) {
        OPENSSL_cleanse(plaintext.data(), plaintext.size());
        error = std::string("Szyfrowanie lub weryfikacja AES-GCM nie powiodły się: ") + ex.what();
        return false;
    }
    const std::size_t plainSize = plaintext.size();
    OPENSSL_cleanse(plaintext.data(), plaintext.size());

    // Preserve the original legacy ciphertext before creating the new file.
    if (!writeExclusiveFile(backupPath, legacyEncoded, error)) return false;
    if (!syncParentDirectory(backupPath)) {
        error = "Nie udało się zsynchronizować katalogu kopii zapasowej"; return false;
    }
    std::string backupCheck;
    if (!readRegularFile(backupPath, backupCheck, error) || backupCheck != legacyEncoded) {
        error = "Kontrola kopii zapasowej nie powiodła się"; return false;
    }

    if (!writeExclusiveFile(destinationPath, newCiphertext, error)) return false;
    if (!syncParentDirectory(destinationPath)) {
        error = "Nie udało się zsynchronizować katalogu pliku wynikowego"; return false;
    }
    std::string outputCheck;
    if (!readRegularFile(destinationPath, outputCheck, error)) return false;
    try {
        std::string finalPlaintext = Encryption::decrypt(outputCheck, aesKey);
        const bool finalValid = finalPlaintext.size() == plainSize &&
                                sha256Hex(finalPlaintext) == expectedPlaintextSha256;
        OPENSSL_cleanse(finalPlaintext.data(), finalPlaintext.size());
        if (!finalValid) {
            error = "Końcowa kontrola zawartości pliku wynikowego nie powiodła się"; return false;
        }
    } catch (const std::exception& ex) {
        error = std::string("Końcowa kontrola szyfrogramu nie powiodła się: ") + ex.what();
        return false;
    }

    result.inputBytes = legacyEncoded.size();
    result.plaintextBytes = plainSize;
    result.backupPath = backupPath;
    result.destinationPath = destinationPath;
    return true;
}
