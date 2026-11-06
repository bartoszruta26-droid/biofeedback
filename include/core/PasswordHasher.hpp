#ifndef BIOFEEDBACK_PASSWORD_HASHER_HPP
#define BIOFEEDBACK_PASSWORD_HASHER_HPP

#include <string>

namespace biofeedback {

/**
 * Password hashing utility. Passwords are one-way hashed; they are never
 * encrypted or recoverable. Encoded values are versioned and self-describing.
 */
class PasswordHasher final {
public:
    PasswordHasher() = delete;

    // PBKDF2-HMAC-SHA-256 with a fresh 128-bit random salt and 256-bit output.
    // Returns: pbkdf2-sha256$v1$<iterations>$<salt-hex>$<digest-hex>
    static std::string hash(const std::string& password);

    // Rejects malformed/unsupported encodings and unreasonable work factors.
    static bool verify(const std::string& password, const std::string& encodedHash);

    static bool isEncodedHash(const std::string& encodedHash) noexcept;
};

} // namespace biofeedback

#endif // BIOFEEDBACK_PASSWORD_HASHER_HPP
