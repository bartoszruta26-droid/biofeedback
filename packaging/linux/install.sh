#!/usr/bin/env bash
set -euo pipefail
BUNDLE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
INSTALL_DIR="${BIOFEEDBACK_INSTALL_DIR:-$HOME/.local/opt/biofeedback}"
APP_DIR="$HOME/.local/share/applications"
DESKTOP_FILE="$APP_DIR/biofeedback.desktop"

case "$INSTALL_DIR" in
  ""|"/"|"$HOME"|"$HOME/.local"|"$HOME/.local/opt")
    printf 'Niebezpieczna ścieżka instalacji: %s\\n' "$INSTALL_DIR" >&2
    exit 1
    ;;
esac
case "$INSTALL_DIR" in
  "$HOME/.local/opt/biofeedback"|"$HOME/.local/opt/biofeedback/"*) ;;
  *)
    printf 'Instalator może zapisywać wyłącznie w katalogu użytkownika Biofeedback.\\n' >&2
    exit 1
    ;;
esac
if [[ -L "$INSTALL_DIR" ]]; then
  printf 'Odmowa użycia dowiązania symbolicznego jako katalogu instalacji.\\n' >&2
  exit 1
fi
mkdir -p -- "$INSTALL_DIR" "$APP_DIR"
INSTALL_DIR="$(cd -- "$INSTALL_DIR" && pwd -P)"
cp -a -- "$BUNDLE_DIR/." "$INSTALL_DIR/"
chmod +x -- "$INSTALL_DIR/run.sh" "$INSTALL_DIR/uninstall.sh"

escaped_install_dir="$(printf '%s' "$INSTALL_DIR" | sed 's/[&|]/\\\\&/g')"
sed "s|@INSTALL_DIR@|$escaped_install_dir|g" "$BUNDLE_DIR/biofeedback.desktop.in" > "$DESKTOP_FILE"

if ! id -nG "${USER:-$(id -un)}" | tr ' ' '\\n' | grep -qx dialout; then
  printf '\\nUżytkownik nie należy do grupy dialout. Dostęp do USB wymaga dodania do grupy:\\n'
  printf '  sudo usermod -aG dialout %q\\n' "${USER:-$(id -un)}"
  printf 'Następnie wyloguj się i zaloguj ponownie.\\n'
fi

printf '\\nZainstalowano Biofeedback w: %s\\n' "$INSTALL_DIR"
printf 'Dane użytkownika i pacjentów nie są nadpisywane przez instalator.\\n'
