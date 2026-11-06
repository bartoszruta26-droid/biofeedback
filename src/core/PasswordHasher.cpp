#include "core/PasswordHasher.hpp"

#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#include <array>
#include <climits>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace biofeedback {
namespace {

constexpr int kIterations = 600000;
constexpr int kMinIterations = 600000;
constexpr int kMaxIterations = 2000000;
constexpr size_t kSaltBytes = 16;
constexpr size_t kDigestBytes = 32;
constexpr char kHex[] = "0123456789abcdef";

std::string toHex(const unsigned char* bytes, size_t length)
{
    std::string result;
    result.reserve(length * 2);
    for (size_t i = 0; i < length; ++i) {
        result.push_back(kHex[bytes[i] >> 4]);
        result.push_back(kHex[bytes[i] & 0x0f]);
    }
    return result;
}

bool fromHex(const std::string& value, std::vector<unsigned char>& output)
{
    if (value.empty() || value.size() % 2 != 0) return false;
    output.clear();
    output.reserve(value.size() / 2);
    for (size_t i = 0; i < value.size(); i += 2) {
        auto nibble = [](char c) -> int {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            return -1;
        };
        const int high = nibble(value[i]);
        const int low = nibble(value[i + 1]);
        if (high < 0 || low < 0) return false;
        output.push_back(static_cast<unsigned char>((high << 4) | low));
    }
    return true;
}

bool derive(const std::string& password, const std::vector<unsigned char>& salt,
            int iterations, unsigned char* output)
{
    if (password.size() > static_cast<size_t>(INT_MAX) ||
        salt.size() > static_cast<size_t>(INT_MAX)) return false;
    return PKCS5_PBKDF2_HMAC(password.data(), static_cast<int>(password.size()),
                             salt.data(), static_cast<int>(salt.size()), iterations,
                             EVP_sha256(), static_cast<int>(kDigestBytes), output) == 1;
}

bool parse(const std::string& encoded, int& iterations,
           std::vector<unsigned char>& salt, std::vector<unsigned char>& digest)
{
    const std::string prefix = "pbkdf2-sha256$v1$";
    if (encoded.compare(0, prefix.size(), prefix) != 0) return false;

    const size_t first = encoded.find('$', prefix.size());
    if (first == std::string::npos) return false;
    const size_t second = encoded.find('$', first + 1);
    if (second == std::string::npos || encoded.find('$', second + 1) != std::string::npos) return false;

    const std::string count = encoded.substr(prefix.size(), first - prefix.size());
    if (count.empty() || count.size() > 10) return false;
    try {
        size_t consumed = 0;
        const long parsed = std::stol(count, &consumed);
        if (consumed != count.size() || parsed < kMinIterations || parsed > kMaxIterations) return false;
        iterations = static_cast<int>(parsed);
    } catch (...) {
        return false;
    }

    if (!fromHex(encoded.substr(first + 1, second - first - 1), salt) ||
        !fromHex(encoded.substr(second + 1), digest)) return false;
    return salt.size() == kSaltBytes && digest.size() == kDigestBytes;
}

} // namespace

std::string PasswordHasher::hash(const std::string& password)
{
    if (password.empty()) throw std::invalid_argument("Hasło nie może być puste");
    if (password.size() > static_cast<size_t>(INT_MAX))
        throw std::invalid_argument("Hasło przekracza maksymalny obsługiwany rozmiar");

    std::array<unsigned char, kSaltBytes> salt{};
    std::array<unsigned char, kDigestBytes> digest{};
    if (RAND_bytes(salt.data(), static_cast<int>(salt.size())) != 1)
        throw std::runtime_error("Nie udało się wygenerować losowej soli");
    const std::vector<unsigned char> saltVector(salt.begin(), salt.end());
    if (!derive(password, saltVector, kIterations, digest.data()))
        throw std::runtime_error("PBKDF2-HMAC-SHA-256 nie powiodło się");

    return "pbkdf2-sha256$v1$" + std::to_string(kIterations) + "$" +
           toHex(salt.data(), salt.size()) + "$" + toHex(digest.data(), digest.size());
}

bool PasswordHasher::verify(const std::string& password, const std::string& encodedHash)
{
    int iterations = 0;
    std::vector<unsigned char> salt;
    std::vector<unsigned char> expected;
    if (password.empty() || !parse(encodedHash, iterations, salt, expected)) return false;

    std::array<unsigned char, kDigestBytes> actual{};
    if (!derive(password, salt, iterations, actual.data())) return false;
    return CRYPTO_memcmp(actual.data(), expected.data(), actual.size()) == 0;
}

bool PasswordHasher::isEncodedHash(const std::string& encodedHash) noexcept
{
    return encodedHash.compare(0, 17, "pbkdf2-sha256$v1$") == 0;
}

} // namespace biofeedback
