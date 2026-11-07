#include "core/Authentication.hpp"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {
const char* kUsersFile="/tmp/biofeedback_auth_regression_users.json";
void expect(bool ok,const std::string& message){if(!ok)throw std::runtime_error(message);}
std::string readAll(){
 std::ifstream in(kUsersFile,std::ios::binary);
 return std::string((std::istreambuf_iterator<char>(in)),std::istreambuf_iterator<char>());
}
void writeUsersJson(){
 std::ofstream out(kUsersFile);
 out << "{\n  \"users\": [\n"
 " {\"username\":\"admin\",\"password\":\"legacy-admin\",\"role\":\"administrator\",\"encrypted\":false},\n"
 " {\"username\":\"doctor\\\"x\",\"password\":\"legacy\\\"pass\",\"role\":\"therapist\",\"encrypted\":false},
 " {\"username\":\"brace-user\",\"password\":\"token-with-}-inside\",\"role\":\"tester\",\"encrypted\":false}\n"
 " ]\n}\n";
}
}
int main(){
 try{
  writeUsersJson();
  biofeedback::Authentication auth(kUsersFile);
  expect(auth.loadUsers(),"loadUsers");
  expect(!auth.createUser("unauthorized","strong-password-2026","therapist"),"unauthenticated account creation rejected");
  expect(auth.login("admin","legacy-admin"),"legacy admin login");
  expect(auth.requiresPasswordChange(),"legacy admin must change password");
  expect(!auth.isLoggedIn(),"session blocked pending password change");
  expect(!auth.changePassword("short"),"short password rejected");
  expect(!auth.changePassword("legacy-admin"),"compromised legacy password cannot be reused");
  expect(auth.changePassword("new-admin-password-2026"),"admin password change");
  expect(auth.isLoggedIn(),"session after password change");
  expect(!auth.login("admin","wrong-password"),"wrong password rejected");
  expect(!auth.login("admin","legacy-admin"),"legacy password rejected after migration");
  expect(auth.login("admin","new-admin-password-2026"),"PBKDF2 login");
  expect(auth.login("doctor\"x","legacy\"pass"),"escaped username legacy login");
  expect(auth.requiresPasswordChange(),"second account migration gate");
  expect(auth.changePassword("new-doctor-password-2026"),"second password change");
  expect(auth.login("brace-user","token-with-}-inside"),"brace legacy login");
  expect(auth.changePassword("new-brace-password-2026"),"third password change");
  expect(auth.login("admin","new-admin-password-2026"),"administrator re-login");
  expect(auth.createUser("new-user","a-strong-password-2026","therapist"),"secure account creation");
  expect(!auth.createUser("new-user","another-strong-password","therapist"),"duplicate user rejected");
  auth.logout();
  expect(auth.loadUsers(),"reload migrated accounts");
  expect(auth.login("new-user","a-strong-password-2026"),"new account PBKDF2 login");

  // The administrator and regular accounts share users.json. Verify that the
  // complete account store is encrypted at rest and remains readable.
  const std::string key="auth-regression-key-material-32-bytes";
  auth.setFileEncryption(true,key);
  expect(auth.saveUsers(),"encrypted account-store save");
  const std::string encryptedBytes=readAll();
  expect(encryptedBytes.rfind("BFENC:v1:",0)==0,"account store has BFENC envelope");
  expect(encryptedBytes.find("new-user")==std::string::npos,"account names are not visible in ciphertext");
  expect(encryptedBytes.find("new-admin-password-2026")==std::string::npos,"password text is not visible in ciphertext");

  biofeedback::Authentication encryptedAuth(kUsersFile);
  encryptedAuth.setFileEncryption(true,key);
  expect(encryptedAuth.loadUsers(),"encrypted account-store load");
  expect(encryptedAuth.login("new-user","a-strong-password-2026"),"login from encrypted account store");
  encryptedAuth.logout();

  biofeedback::Authentication wrongKeyAuth(kUsersFile);
  wrongKeyAuth.setFileEncryption(true,"wrong-auth-regression-key-material-32");
  expect(!wrongKeyAuth.loadUsers(),"wrong key must reject encrypted account store");
  expect(readAll()==encryptedBytes,"wrong-key read must not modify source file");

  // Switching encryption off affects subsequent writes only; encrypted data
  // must still be readable before it is intentionally rewritten as plaintext.
  encryptedAuth.setFileEncryption(false);
  expect(encryptedAuth.saveUsers(),"plaintext account-store save after disabling encryption");
  const std::string plaintextBytes=readAll();
  expect(plaintextBytes.rfind("BFENC:v1:",0)!=0,"disabled encryption writes plaintext JSON");
  expect(plaintextBytes.find("new-user")!=std::string::npos,"plaintext account store retains user record");

  biofeedback::Authentication plaintextAuth(kUsersFile);
  expect(plaintextAuth.loadUsers(),"plaintext account-store reload");
  expect(plaintextAuth.login("new-user","a-strong-password-2026"),"login after encrypted-to-plaintext switch");
  plaintextAuth.logout();

  // Re-enabling encryption must preserve the same logical records.
  plaintextAuth.setFileEncryption(true,key);
  expect(plaintextAuth.saveUsers(),"reencrypt account store");
  expect(readAll().rfind("BFENC:v1:",0)==0,"reenabled encryption writes BFENC envelope");
  biofeedback::Authentication reloadedEncryptedAuth(kUsersFile);
  reloadedEncryptedAuth.setFileEncryption(true,key);
  expect(reloadedEncryptedAuth.loadUsers(),"reload after plaintext-to-encrypted switch");
  expect(reloadedEncryptedAuth.login("admin","new-admin-password-2026"),"administrator survives config switching");
  expect(reloadedEncryptedAuth.login("new-user","a-strong-password-2026"),"regular user survives config switching");

  std::remove(kUsersFile);
  std::cout<<"OK: auth_regression\n"; return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<"\n";std::remove(kUsersFile);return 1;}
}
