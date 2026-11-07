#!/usr/bin/env bash
set -euo pipefail
BUNDLE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
INSTALL_DIR="${BIOFEEDBACK_INSTALL_DIR:-$HOME/.local/opt/biofeedback}"
APP_DIR="$HOME/.local/share/applications"

mkdir -p "$INSTALL_DIR" "$APP_DIR"
cp -a "$BUNDLE_DIR/." "$INSTALL_DIR/"
chmod +x "$INSTALL_DIR/run.sh" "$INSTALL_DIR/uninstall.sh"

sed "s|@INSTALL_DIR@|$INSTALL_DIR|g" "$BUNDLE_DIR/biofeedback.desktop.in" > "$APP_DIR/biofeedback.desktop"

if ! id -nG "$USER" | tr ' ' '\n' | grep -qx dialout; then
    printf '\nUżytkownik nie należy do grupy dialout. Aby uzyskać dostęp do portu USB:\n'
    printf '  sudo usermod -aG dialout %q\n' "$USER"
    printf 'Następnie wyloguj się i zaloguj ponownie.\n'
fi

printf '\nZainstalowano Biofeedback w: %s\n' "$INSTALL_DIR"
printf 'Uruchom z menu aplikacji albo poleceniem: %s/run.sh\n' "$INSTALL_DIR"
