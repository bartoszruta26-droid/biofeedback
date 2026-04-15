#include "Encryption.hpp"
#include <cstdlib>
#include <stdexcept>

// Tabela kodowania Base64
static const std::string base64_chars = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

std::string Encryption::encrypt(const std::string& data, const std::string& key) {
    if (key.empty()) {
        throw std::invalid_argument("Klucz szyfrujący nie może być pusty");
    }
    
    // Konwersja danych i klucza do wektorów bajtów
    std::vector<uint8_t> dataBytes(data.begin(), data.end());
    std::vector<uint8_t> keyBytes(key.begin(), key.end());
    
    // Szyfrowanie XOR
    std::vector<uint8_t> encryptedBytes = xorCipher(dataBytes, keyBytes);
    
    // Kodowanie Base64
    return base64Encode(encryptedBytes);
}

std::string Encryption::decrypt(const std::string& encryptedData, const std::string& key) {
    if (key.empty()) {
        throw std::invalid_argument("Klucz szyfrujący nie może być pusty");
    }
    
    // Dekodowanie Base64
    std::vector<uint8_t> encryptedBytes = base64Decode(encryptedData);
    
    // Konwersja klucza do wektora bajtów
    std::vector<uint8_t> keyBytes(key.begin(), key.end());
    
    // Deszyfrowanie XOR (ta sama operacja co szyfrowanie)
    std::vector<uint8_t> decryptedBytes = xorCipher(encryptedBytes, keyBytes);
    
    // Konwersja do stringa
    return std::string(decryptedBytes.begin(), decryptedBytes.end());
}

std::string Encryption::generateKey(size_t length) {
    std::string key;
    key.reserve(length);
    
    for (size_t i = 0; i < length; ++i) {
        // Generowanie losowego znaku z zakresu ASCII drukowalnego
        char randomChar = static_cast<char>(33 + (std::rand() % 93));
        key += randomChar;
    }
    
    return key;
}

std::string Encryption::base64Encode(const std::vector<uint8_t>& input) {
    std::string encoded;
    int val = 0;
    int valb = -6;
    
    for (uint8_t c : input) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            encoded.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    
    if (valb > -6) {
        encoded.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    
    // Dodawanie paddingu
    while (encoded.size() % 4) {
        encoded.push_back('=');
    }
    
    return encoded;
}

std::vector<uint8_t> Encryption::base64Decode(const std::string& input) {
    std::vector<uint8_t> decoded;
    std::vector<int> T(256, -1);
    
    // Inicjalizacja tabeli dekodującej
    for (int i = 0; i < 64; i++) {
        T[base64_chars[i]] = i;
    }
    
    int val = 0;
    int valb = -8;
    
    for (char c : input) {
        if (T[c] == -1) break; // Ignoruj znaki inne niż Base64 (w tym '=')
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            decoded.push_back(static_cast<uint8_t>((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    
    return decoded;
}

std::vector<uint8_t> Encryption::xorCipher(const std::vector<uint8_t>& data, const std::vector<uint8_t>& key) {
    std::vector<uint8_t> result(data.size());
    
    for (size_t i = 0; i < data.size(); ++i) {
        result[i] = data[i] ^ key[i % key.size()];
    }
    
    return result;
}
