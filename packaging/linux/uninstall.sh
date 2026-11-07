#!/usr/bin/env bash
set -euo pipefail
INSTALL_DIR="${BIOFEEDBACK_INSTALL_DIR:-$HOME/.local/opt/biofeedback}"
case "$INSTALL_DIR" in
  ""|"/"|"$HOME")
    printf 'Odmowa usunięcia niebezpiecznej ścieżki: %s\n' "$INSTALL_DIR" >&2
    exit 1
    ;;
esac
rm -rf -- "$INSTALL_DIR"
rm -f -- "$HOME/.local/share/applications/biofeedback.desktop"
printf 'Usunięto Biofeedback z katalogu użytkownika.\n'
