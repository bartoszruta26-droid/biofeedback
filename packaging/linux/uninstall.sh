#!/usr/bin/env bash
set -euo pipefail
INSTALL_DIR="${BIOFEEDBACK_INSTALL_DIR:-$HOME/.local/opt/biofeedback}"
rm -rf -- "$INSTALL_DIR"
rm -f -- "$HOME/.local/share/applications/biofeedback.desktop"
printf 'Usunięto Biofeedback z katalogu użytkownika.\n'
