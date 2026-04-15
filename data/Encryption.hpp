#ifndef ENCRYPTION_HPP
#define ENCRYPTION_HPP

#include <string>
#include <vector>

class Encryption {
public:
    // Szyfrowanie danych przy użyciu klucza
    static std::string encrypt(const std::string& data, const std::string& key);
    
    // Deszyfrowanie danych przy użyciu klucza
    static std::string decrypt(const std::string& encryptedData, const std::string& key);
    
    // Generowanie losowego klucza o określonej długości
    static std::string generateKey(size_t length = 32);
    
    // Kodowanie Base64
    static std::string base64Encode(const std::vector<unsigned char>& data);
    
    // Dekodowanie Base64
    static std::vector<unsigned char> base64Decode(const std::string& encoded);

private:
    // Proste szyfrowanie XOR
    static std::string xorCipher(const std::string& data, const std::string& key);
};

#endif // ENCRYPTION_HPP
