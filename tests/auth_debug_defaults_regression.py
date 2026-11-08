#!/usr/bin/env python3
"""Regression guard: authentication diagnostics must be opt-in by default."""

from pathlib import Path
import re
import sys

source = Path("src/core/Authentication.cpp").read_text(encoding="utf-8")
flags = (
    "DEBUG_AUTHENTICATION",
    "DEBUG_USER_OPERATIONS",
    "DEBUG_PASSWORD_ENCRYPTION",
    "DEBUG_JSON_PARSING",
    "DEBUG_FILE_OPERATIONS",
)

for flag in flags:
    pattern = rf"#ifndef\s+{flag}\s+#define\s+{flag}\s+0\b"
    if not re.search(pattern, source):
        print(f"FAIL: {flag} must default to 0 inside its #ifndef guard")
        sys.exit(1)

if "#define AUTH_DEBUG(level, msg)" not in source:
    print("FAIL: authentication debug macro is missing")
    sys.exit(1)
if "if (DEBUG_AUTHENTICATION && level <= DEBUG_AUTHENTICATION)" not in source:
    print("FAIL: AUTH_DEBUG must remain gated by the opt-in flag")
    sys.exit(1)
if "if (DEBUG_AUTHENTICATION)" not in source:
    print("FAIL: AUTH_WARN must remain gated by the opt-in flag")
    sys.exit(1)

print("Authentication diagnostic defaults are opt-in.")
