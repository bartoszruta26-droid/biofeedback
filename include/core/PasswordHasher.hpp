#ifndef CORE_PASSWORD_HASHER_HPP
#define CORE_PASSWORD_HASHER_HPP
#include <string>
namespace biofeedback {
class PasswordHasher {
public:
    static std::string hash(const std::string& password, int iterations = 600000);
    static bool verify(const std::string& password, const std::string& encoded);
    static bool isEncoded(const std::string& encoded) noexcept;
};
}
#endif
