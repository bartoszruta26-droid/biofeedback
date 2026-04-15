#include "Encryption.hpp"
#include <random>
#include <stdexcept>

// Tablica znaków do kodowania Base64
static const std::string base64Chars = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string Encryption::xorCipher(const std::string& data, const std::string& key) {
    if (key.empty()) {
        throw std::invalid_argument("Klucz nie może być pusty");
    }
    
    std::string result = data;
    for (size_t i = 0; i < data.length(); ++i) {
        result[i] = data[i] ^ key[i % key.length()];
    }
    return result;
}

std::string Encryption::encrypt(const std::string& data, const std::string& key) {
    // Najpierw szyfrujemy dane XOR
    std::string encrypted = xorCipher(data, key);
    // Następnie kodujemy do Base64 dla bezpiecznego przechowywania/transmisji
    std::vector<unsigned char> encryptedVec(encrypted.begin(), encrypted.end());
    return base64Encode(encryptedVec);
}

std::string Encryption::decrypt(const std::string& encryptedData, const std::string& key) {
    // Najpierw dekodujemy z Base64
    std::vector<unsigned char> decoded = base64Decode(encryptedData);
    std::string decodedStr(decoded.begin(), decoded.end());
    // Następnie deszyfrujemy XOR
    return xorCipher(decodedStr, key);
}

std::string Encryption::generateKey(size_t length) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    std::string key;
    key.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        key += static_cast<char>(dis(gen));
    }
    return key;
}

std::string Encryption::base64Encode(const std::vector<unsigned char>& data) {
    std::string encoded;
    int val = 0, valb = -6;
    
    for (unsigned char c : data) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            encoded.push_back(base64Chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    
    if (valb > -6) {
        encoded.push_back(base64Chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    
    while (encoded.size() % 4) {
        encoded.push_back('=');
    }
    
    return encoded;
}

std::vector<unsigned char> Encryption::base64Decode(const std::string& encoded) {
    std::vector<unsigned char> decoded;
    std::vector<int> T(256, -1);
    
    for (int i = 0; i < 64; i++) {
        T[base64Chars[i]] = i;
    }
    
    int val = 0, valb = -8;
    
    for (char c : encoded) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            decoded.push_back((val >> valb) & 0xFF);
            valb -= 8;
        }
    }
    
    return decoded;
}
