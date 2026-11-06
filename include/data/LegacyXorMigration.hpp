#pragma once

#include <cstddef>
#include <string>

struct LegacyXorMigrationResult {
    std::size_t inputBytes{0};
    std::size_t plaintextBytes{0};
    std::string backupPath;
    std::string destinationPath;
};

// Converts the historical Base64(XOR(plaintext, repeating_key)) format to
// BFENC:v1 (AES-256-GCM). Source is read-only; backup and destination must
// not exist. No legacy decryption is added to the runtime Encryption API.
bool migrateLegacyXorFile(const std::string& sourcePath,
                          const std::string& destinationPath,
                          const std::string& backupPath,
                          const std::string& legacyXorKey,
                          const std::string& aesKey,
                          LegacyXorMigrationResult& result,
                          std::string& error);
