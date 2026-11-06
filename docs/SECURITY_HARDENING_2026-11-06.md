# Security hardening — integration/all-work (2026-11-06)

## Scope

This change set introduces password hashing, authenticated encryption, atomic persistence and fail-closed patient-record handling. It is not a production security certification.

## Account credentials

- New credentials are stored as \`pbkdf2-sha256$v1$<iterations>$<salt-hex>$<digest-hex>\`.
- PBKDF2-HMAC-SHA-256 uses 600,000 iterations, a 16-byte CSPRNG salt and a 32-byte digest.
- Password verification uses constant-time digest comparison.
- A legacy plaintext account is migrated on its first successful login. The migrated account is marked \`mustChangePassword\`; the GUI blocks completion of login until a new password of at least 12 characters is chosen and persisted.
- New account creation is restricted to an authenticated administrator and requires a password of at least 12 characters.
- Legacy reversible XOR-encrypted password records are rejected. There is no automatic XOR fallback.
- \`config/users.json\` is not modified by this code change alone. The first successful legacy login writes the PBKDF2 migration atomically.

## Data encryption

- New encrypted values use the \`BFENC:v1:\` prefix and AES-256-GCM.
- The payload is Base64 of 12-byte nonce, 16-byte authentication tag and ciphertext.
- A fresh nonce is generated with OpenSSL \`RAND_bytes\` for every encryption.
- The supplied key material is SHA-256-derived into a 256-bit AES key. The application requires \`BIOFEEDBACK_ENCRYPTION_KEY\` to be supplied externally and to contain at least 32 bytes. Provision a cryptographically random secret; do not use a human password or commit the secret.
- Authentication-tag failure, wrong keys, malformed payloads and unsupported legacy formats fail closed.
- XOR ciphertexts are not migrated automatically. Preserve a protected backup and perform any required legacy migration as a separately reviewed, explicit offline operation.

## File persistence

- DataManager writes to a same-directory temporary file created with \`mkstemp\`, restricts it to owner read/write, writes all bytes, calls \`fsync\`, and atomically renames it over the destination.
- The parent directory is synchronized after rename where supported.
- Authentication account files use the same pattern.
- A failure before rename leaves the previous destination untouched. A directory-sync failure after rename can report a durability error even though the atomic replacement has already occurred.

## Patient records and JSON export

- Patient records, patient-linked exercise results and training plans always use AES-GCM; legacy boolean arguments cannot disable encryption.
- Patient reads without an encryption key or with plaintext/legacy data fail closed.
- JSON export/import now encrypts and authenticates the payload and requires the configured key.

## Verification status

Regression tests were added for PBKDF2 hashing/verification, legacy account migration and forced password change, account creation, AES-GCM round-trip, wrong-key rejection, tag tampering, legacy-format rejection, fail-closed patient writes and encrypted JSON export/import.

The tests have not yet been executed in this change session. CI is configured to build the core regression targets and run CTest. Hardware, penetration, crash-injection and filesystem-specific durability testing remain outstanding.
