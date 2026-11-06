#include "core/PasswordHasher.hpp"
#include <iostream>
int main(){auto h=biofeedback::PasswordHasher::hash("correct horse battery staple");if(!biofeedback::PasswordHasher::verify("correct horse battery staple",h))return 1;if(biofeedback::PasswordHasher::verify("wrong",h))return 2;if(biofeedback::PasswordHasher::verify("correct horse battery staple",h+"$tamper"))return 3;std::cout<<"OK: password_hasher_regression\n";return 0;}
