#include "core/Authentication.hpp"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
const char* kUsersFile="/tmp/biofeedback_auth_regression_users.json";
void expect(bool ok,const std::string& message){if(!ok)throw std::runtime_error(message);}
void writeUsersJson(){
 std::ofstream out(kUsersFile);
 out << "{\n  \"users\": [\n"
 " {\"username\":\"admin\",\"password\":\"legacy-admin\",\"role\":\"administrator\",\"encrypted\":false},\n"
 " {\"username\":\"doctor\\\"x\",\"password\":\"legacy\\\"pass\",\"role\":\"therapist\",\"encrypted\":false},\n"
 " {\"username\":\"brace-user\",\"password\":\"token-with-}-inside\",\"role\":\"tester\",\"encrypted\":false}\n"
 " ]\n}\n";
}
}
int main(){
 try{
  writeUsersJson();
  biofeedback::Authentication auth(kUsersFile);
  expect(auth.loadUsers(),"loadUsers");
  expect(auth.login("admin","legacy-admin"),"legacy admin login");
  expect(auth.requiresPasswordChange(),"legacy admin must change password");
  expect(!auth.isLoggedIn(),"session blocked pending password change");
  expect(auth.changePassword("new-admin-password-2026"),"admin password change");
  expect(auth.isLoggedIn(),"session after password change");
  expect(!auth.login("admin","wrong-password"),"wrong password rejected");
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
  std::remove(kUsersFile);
  std::cout<<"OK: auth_regression\n"; return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<"\n";std::remove(kUsersFile);return 1;}
}
