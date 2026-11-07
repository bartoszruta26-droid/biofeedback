# Security audit: data protection, authentication and cryptography

**Status:** initial static audit; remediation plan, not a security certification  
**Baseline:** `main` at `6b1e84d891ef5cb13ec20edb568f05d349978422`  
**Working branch:** `security/data-auth-crypto-hardening`

## Scope and limitations

This review covers repository-visible source, configuration and build declarations related to patient data, authentication, encryption and logging. No hardware tests, production deployment inspection, penetration test, runtime test or independent cryptographic validation has been performed. Findings below are based on source inspection and must be revalidated against the current branch before release.

## Executive assessment

The current implementation must not be represented as providing secure storage of patient data. The encryption implementation identifies itself as XOR plus Base64; Base64 is an encoding, not encryption, and repeating-key XOR does not provide confidentiality or integrity. The tracked user configuration contains a plaintext administrator credential. These are release-blocking findings for any deployment containing real patient information.

## Findings

### SEC-001 — Plaintext administrator credential committed to source control
- **Severity:** Critical
- **Location:** `config/users.json`
- **Evidence:** a tracked administrator record stores its password as plaintext and marks it as not encrypted.
- **Impact:** anyone with repository access, a clone, or an exposed source archive can recover the credential. Replacing the file in a later commit does not remove it from Git history.
- **Required action:** treat the credential as compromised; rotate it wherever it may have been reused; remove plaintext bootstrap credentials from tracked configuration; introduce first-run administrator provisioning with a one-time setup secret or interactive local setup; store only a password verifier using a password KDF (prefer Argon2id; PBKDF2-HMAC-SHA-256 only where policy/platform constraints require it). Do not ship a known default password.
- **Migration constraint:** preserve existing user identities and role semantics. Force password reset for legacy plaintext accounts; never silently convert a publicly known default password into an accepted production credential.

### SEC-002 — XOR plus Base64 is not secure encryption
- **Severity:** Critical
- **Location:** `src/data/Encryption.cpp`, `include/data/Encryption.hpp`
- **Evidence:** implementation comments and encrypt path explicitly use XOR followed by Base64. The source also contains legacy XOR/key-generation/statistics paths.
- **Impact:** no meaningful cryptographic confidentiality or authenticated integrity. Data can be decoded/modified by an attacker with repository knowledge.
- **Required action:** introduce a versioned authenticated-encryption envelope using a maintained crypto library already present in the build (OpenSSL EVP AES-256-GCM is a viable candidate); use a fresh cryptographically random nonce per encryption; authenticate the format/version and relevant metadata as associated data; reject malformed, truncated, unauthenticated and unsupported-version payloads. Never fall back to XOR after an authentication failure.
- **Migration constraint:** implement explicit read-only legacy detection and a controlled re-encryption migration. Keep legacy decryption isolated and clearly marked as insecure, and remove it only after all supported data has been migrated and verified. Do not claim legacy ciphertext is protected.

### SEC-003 — Cryptographic diagnostic output exposes operational metadata
- **Severity:** High
- **Location:** `src/data/Encryption.cpp`
- **Evidence:** debug defaults are enabled in source; log statements include input length and key length and verbose operation traces.
- **Impact:** logs can disclose security-relevant metadata, increase attack surface and create uncontrolled diagnostic output. Future changes could accidentally log secret material.
- **Required action:** default diagnostics off in release builds; never log keys, passwords, plaintext, ciphertext or patient identifiers; centralize redaction; ensure callbacks are invoked outside internal locks; use thread-safe time formatting.
- **Acceptance:** tests assert that representative secrets and patient identifiers do not appear in captured logs.

### SEC-004 — Authentication configuration permits insecure bootstrap state
- **Severity:** Critical
- **Location:** `config/users.json`, authentication/configuration paths
- **Evidence:** tracked plaintext administrator record exists; repository configuration has historically permitted authentication to be disabled.
- **Impact:** an installation may start with a known credential or without mandatory authentication.
- **Required action:** production-safe defaults must require authentication; fail closed if user storage is missing, malformed, insecurely provisioned or cannot be read; provide an explicit development-only bypass that cannot be enabled in release packaging accidentally.
- **Acceptance:** clean installation cannot reach patient data before an administrator is securely provisioned and authenticated.

### SEC-005 — Patient-data serialization and logging require a complete data-flow review
- **Severity:** High
- **Location:** `src/data/DataManager.cpp`
- **Evidence:** patient records include direct identifiers and medical history; serialization/parsing has custom JSON handling and debug output.
- **Impact:** incomplete escaping/parsing, accidental log disclosure, partial writes or unencrypted storage may expose or corrupt sensitive records.
- **Required action:** trace every read/write/export/backup path; use a standards-compliant JSON parser consistently; define which fields are sensitive; encrypt at rest where required; apply restrictive file permissions and atomic writes; document retention, backup and deletion.
- **Acceptance:** round-trip, malformed-input, interrupted-write, permission and log-redaction tests pass.

### SEC-006 — Key management and encryption policy are not yet demonstrated
- **Severity:** High
- **Location:** `src/core/Application.cpp`, `src/data/Encryption.cpp`, configuration
- **Evidence:** repository has OpenSSL dependencies and key-related configuration/API paths, but secure key provisioning, rotation, storage and recovery are not established by the audited baseline.
- **Impact:** strong encryption can be defeated by a hard-coded, shared, weak, logged or unrecoverable key.
- **Required action:** define platform-specific secret storage; separate key-encryption keys from data-encryption keys; support rotation and recovery; do not place production secrets in source control, ordinary config files, command-line arguments or logs.
- **Acceptance:** threat model and operational key lifecycle documented and tested.

## Remediation sequence

1. **Containment:** rotate exposed credentials; prevent use of the tracked default account; establish a secure first-run provisioning path.
2. **Authentication:** fail-closed policy, password KDF with per-user salt and calibrated work factor, role/authorization checks, session invalidation and safe error handling.
3. **Cryptographic API:** versioned AES-256-GCM envelope through OpenSSL EVP, CSPRNG nonce, authentication tag, strict input limits and authenticated metadata.
4. **Data migration:** backup, detect legacy records, decrypt only through isolated compatibility code, re-encrypt, verify, report failures, retain recoverable backup; never silently downgrade.
5. **Data layer:** audit all patient-data persistence/export/backup paths, permissions, atomicity and logging.
6. **Verification:** unit tests, known-answer tests, tamper tests, migration fixtures, negative tests and sanitizer-enabled builds. Hardware testing is outside this security scope.

## Mandatory acceptance criteria

- No production credential or secret is tracked in Git.
- A clean installation requires secure administrator provisioning.
- Authentication and authorization fail closed.
- Passwords are stored as salted password-KDF verifiers, not reversible ciphertext.
- New sensitive-data encryption is authenticated; bit flips, wrong keys and truncated records fail closed.
- Nonces are generated using a cryptographically secure random source and are never reused with the same key.
- Legacy records are migrated with a documented, tested and recoverable procedure.
- Logs contain no passwords, keys, patient identifiers or patient content.
- Existing patient, session, exercise and export workflows retain their intended behavior, with explicit errors where legacy data cannot safely be migrated.
- CI covers cryptographic and authentication regressions.
- No claim of medical-device compliance or clinical validation is made by this work.

## Explicit non-goals for this change set

- No changes to measurement algorithms, sensor firmware or clinical behavior.
- No hardware acceptance claims.
- No deletion or destructive rewrite of existing patient records.
- No merge to `main` without explicit approval.
- No claim that this report alone makes the application safe for production use.

## Operational warning

The tracked default administrator credential must be considered compromised. Rotate it and any reused credential immediately. Removing it from the current tree is necessary but does not erase it from repository history or existing clones. If the repository has been shared, assess history cleanup and access exposure separately.

## Implemented increment — versioned password hashing primitive

A standalone `biofeedback::PasswordHasher` has been added on this branch:
- PBKDF2-HMAC-SHA-256, 600,000 iterations, 128-bit random per-password salt, 256-bit derived output;
- versioned self-describing representation: `pbkdf2-sha256$v1$<iterations>$<salt-hex>$<digest-hex>`;
- constant-time digest comparison via `CRYPTO_memcmp`;
- strict parsing and bounded work factor (600,000–2,000,000 iterations);
- malformed, unsupported, plaintext, and out-of-range encodings are rejected;
- regression assertions cover fresh salts, successful/failed verification, plaintext rejection, and malformed parameters.

This primitive is intentionally not wired into the existing `Authentication` persistence/login flow yet. That flow currently stores a reversible-encryption flag and calls the legacy XOR-based `Encryption` API; switching it without a complete migration and recovery design could lock out users or corrupt the user store. Existing users and patient data have not been rewritten.

No plaintext credential file has been added. Real credentials must be provisioned outside version control and stored in a password manager or a locally encrypted vault. Any credential previously committed in repository history must be considered exposed and rotated.
