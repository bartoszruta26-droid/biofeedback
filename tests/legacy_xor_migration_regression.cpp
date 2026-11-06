#include "data/LegacyXorMigration.hpp"
#include "data/Encryption.hpp"

#include <openssl/evp.h>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <unistd.h>

namespace {
void expect(bool ok, const std::string& why) { if (!ok) throw std::runtime_error(why); }
std::string readAll(const std::string& p) {
    std::ifstream f(p, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}
std::string legacyEncode(const std::string& plain, const std::string& key) {
    std::vector<unsigned char> bytes(plain.size());
    for (std::size_t i=0;i<plain.size();++i) bytes[i]=static_cast<unsigned char>(plain[i]) ^ static_cast<unsigned char>(key[i%key.size()]);
    std::string out(4*((bytes.size()+2)/3)+1,'\0');
    const int n=EVP_EncodeBlock(reinterpret_cast<unsigned char*>(&out[0]),bytes.data(),static_cast<int>(bytes.size()));
    out.resize(static_cast<std::size_t>(n)); return out;
}
}
int main() {
    const std::string base="/tmp/biofeedback_legacy_migration_"+std::to_string(::getpid());
    const std::string src=base+".legacy", dst=base+".bfenc", bak=base+".backup";
    try {
        const std::string oldKey="old-test-xor-key";
        const std::string aesKey="regression-key-material-32-bytes";
        const std::string plain="{\"patientId\":\"synthetic-001\",\"note\":\"test-only\"}";
        const std::string legacy=legacyEncode(plain,oldKey);
        { std::ofstream f(src,std::ios::binary); f<<legacy; }
        LegacyXorMigrationResult result; std::string error;
        expect(migrateLegacyXorFile(src,dst,bak,oldKey,aesKey,result,error), "migration failed: "+error);
        expect(readAll(src)==legacy, "source was modified");
        expect(readAll(bak)==legacy, "backup differs from source");
        expect(Encryption::decrypt(readAll(dst),aesKey)==plain, "AES-GCM plaintext mismatch");

        LegacyXorMigrationResult second; error.clear();
        expect(!migrateLegacyXorFile(src,dst,bak,oldKey,aesKey,second,error), "existing output must not be overwritten");
        const std::string corrupt=base+".corrupt";
        { std::ofstream f(corrupt); f<<"%%%"; }
        expect(!migrateLegacyXorFile(corrupt,base+".badout",base+".badbak",oldKey,aesKey,second,error),
               "invalid Base64 must be rejected");
        std::remove(corrupt.c_str()); std::remove((base+".badout").c_str()); std::remove((base+".badbak").c_str());
        std::remove(src.c_str()); std::remove(dst.c_str()); std::remove(bak.c_str());
        std::cout<<"OK: legacy_xor_migration_regression\n"; return 0;
    } catch (const std::exception& e) {
        std::remove(src.c_str()); std::remove(dst.c_str()); std::remove(bak.c_str());
        std::cerr<<"FAIL: "<<e.what()<<"\n"; return 1;
    }
}
