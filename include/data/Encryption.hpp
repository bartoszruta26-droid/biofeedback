#ifndef ENCRYPTION_HPP
#define ENCRYPTION_HPP

#include <string>
#include <vector>
#include <cstdint>

class Encryption {
public:
    // Szyfruje dane za pomocą klucza (XOR) i koduje Base64
    static std::string encrypt(const std::string& data, const std::string& key);
    
    // Dekoduje Base64 i odszyfrowuje dane za pomocą klucza (XOR)
    static std::string decrypt(const std::string& encryptedData, const std::string& key);
    
    // Generuje losowy klucz o podanej długości
    static std::string generateKey(size_t length = 32);
    
    // Kodowanie Base64
    static std::string base64Encode(const std::vector<uint8_t>& input);
    
    // Dekodowanie Base64
    static std::vector<uint8_t> base64Decode(const std::string& input);

private:
    // Prosty algorytm XOR do szyfrowania/deszyfrowania
    static std::vector<uint8_t> xorCipher(const std::vector<uint8_t>& data, const std::vector<uint8_t>& key);
};

#endif // ENCRYPTION_HPP
