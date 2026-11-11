#include "core/PasswordHasher.hpp"
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <array>
#include <climits>
#include <stdexcept>
namespace biofeedback {
namespace {
constexpr int kSaltBytes=16, kDigestBytes=32, kMinIterations=100000, kMaxIterations=2000000;
const char* kPrefix="pbkdf2-sha256$v1$";
std::string toHex(const unsigned char* p, std::size_t n) {
 const char* d="0123456789abcdef"; std::string s; s.reserve(n*2);
 for(std::size_t i=0;i<n;++i){s.push_back(d[p[i]>>4]);s.push_back(d[p[i]&15]);} return s;
}
bool fromHex(const std::string& s,unsigned char* out,std::size_t n) {
 if(s.size()!=n*2)return false;
 auto v=[](char c){if(c>='0'&&c<='9')return c-'0';if(c>='a'&&c<='f')return c-'a'+10;if(c>='A'&&c<='F')return c-'A'+10;return -1;};
 for(std::size_t i=0;i<n;++i){int a=v(s[2*i]),b=v(s[2*i+1]);if(a<0||b<0)return false;out[i]=static_cast<unsigned char>((a<<4)|b);}return true;
}
}
std::string PasswordHasher::hash(const std::string& password,int iterations) {
 if(password.empty())throw std::invalid_argument("Hasło nie może być puste");
 if(password.size()>static_cast<std::size_t>(INT_MAX)||iterations<kMinIterations||iterations>kMaxIterations)throw std::invalid_argument("Nieprawidłowe parametry PBKDF2");
 std::array<unsigned char,kSaltBytes> salt{}; std::array<unsigned char,kDigestBytes> digest{};
 if(RAND_bytes(salt.data(),salt.size())!=1)throw std::runtime_error("CSPRNG nie wygenerował soli");
 if(PKCS5_PBKDF2_HMAC(password.data(),static_cast<int>(password.size()),salt.data(),salt.size(),iterations,EVP_sha256(),digest.size(),digest.data())!=1)throw std::runtime_error("PBKDF2 nie powiodło się");
 return std::string(kPrefix)+std::to_string(iterations)+"$"+toHex(salt.data(),salt.size())+"$"+toHex(digest.data(),digest.size());
}
bool PasswordHasher::isEncoded(const std::string& s) noexcept { return s.compare(0,std::char_traits<char>::length(kPrefix),kPrefix)==0; }
bool PasswordHasher::verify(const std::string& password,const std::string& encoded) {
 if(password.empty()||password.size()>static_cast<std::size_t>(INT_MAX)||!isEncoded(encoded))return false;
 const auto n=std::char_traits<char>::length(kPrefix); auto a=encoded.find('$',n); auto b=a==std::string::npos?a:encoded.find('$',a+1);
 if(a==std::string::npos||b==std::string::npos||encoded.find('$',b+1)!=std::string::npos)return false;
 int rounds=0; try {auto part=encoded.substr(n,a-n);std::size_t used=0;rounds=std::stoi(part,&used);if(used!=part.size()||rounds<kMinIterations||rounds>kMaxIterations)return false;}catch(...){return false;}
 std::array<unsigned char,kSaltBytes> salt{};std::array<unsigned char,kDigestBytes> expected{},actual{};
 if(!fromHex(encoded.substr(a+1,b-a-1),salt.data(),salt.size())||!fromHex(encoded.substr(b+1),expected.data(),expected.size()))return false;
 if(PKCS5_PBKDF2_HMAC(password.data(),static_cast<int>(password.size()),salt.data(),salt.size(),rounds,EVP_sha256(),actual.size(),actual.data())!=1)return false;
 return CRYPTO_memcmp(expected.data(),actual.data(),actual.size())==0;
}
}
