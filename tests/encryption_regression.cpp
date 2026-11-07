#include "data/Encryption.hpp"
#include <iostream>
int main() {
    const std::string key = Encryption::generateKey();
    const std::string plain = "biofeedback-patient-record";
    const std::string cipher = Encryption::encrypt(plain, key);
    if (cipher.rfind("BFENC:v1:", 0) != 0) return 1;
    if (Encryption::decrypt(cipher, key) != plain) return 2;
    bool wrongKey = false;
    try { (void)Encryption::decrypt(cipher, std::string(32, 'x')); } catch (...) { wrongKey = true; }
    if (!wrongKey) return 3;
    std::string tampered = cipher;
    const auto pos = tampered.size() - 2;
    tampered[pos] = tampered[pos] == 'A' ? 'B' : 'A';
    bool badTag = false;
    try { (void)Encryption::decrypt(tampered, key); } catch (...) { badTag = true; }
    if (!badTag) return 4;
    bool legacyRejected = false;
    try { (void)Encryption::decrypt("YWJj", key); } catch (...) { legacyRejected = true; }
    if (!legacyRejected) return 5;
    std::cout << "OK: encryption_regression\n";
    return 0;
}
