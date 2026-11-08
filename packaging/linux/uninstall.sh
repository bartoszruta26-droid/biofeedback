#!/usr/bin/env bash
set -euo pipefail

INSTALL_DIR="${BIOFEEDBACK_INSTALL_DIR:-$HOME/.local/opt/biofeedback}"
APP_DIR="$HOME/.local/share/applications"
DESKTOP_FILE="$APP_DIR/biofeedback.desktop"
DATA_DIR="${BIOFEEDBACK_DATA_DIR:-$HOME/.local/share/biofeedback/data}"

resolve_parent() {
  local target="$1"
  local parent
  parent="$(dirname -- "$target")"
  mkdir -p -- "$parent"
  (cd -- "$parent" && printf '%s/%s\n' "$PWD" "$(basename -- "$target")")
}

safe_remove_target() {
  local target="$1"
  local canonical
  canonical="$(resolve_parent "$target")"

  case "$canonical" in
    "$HOME/.local/opt/biofeedback"|"$HOME/.local/opt/biofeedback/"*)
      ;;
    *)
      printf 'Odmowa usunięcia ścieżki spoza katalogu instalacji: %s\n' "$canonical" >&2
      return 1
      ;;
  esac

  if [[ -L "$target" ]]; then
    printf 'Odmowa usuwania dowiązania symbolicznego: %s\n' "$target" >&2
    return 1
  fi
}

case "$INSTALL_DIR" in
  ""|"/"|"$HOME"|"$HOME/.local"|"$HOME/.local/opt")
    printf 'Odmowa usunięcia niebezpiecznej ścieżki instalacji: %s\n' "$INSTALL_DIR" >&2
    exit 1
    ;;
esac

safe_remove_target "$INSTALL_DIR"
if [[ -e "$INSTALL_DIR" ]]; then
  rm -rf -- "$INSTALL_DIR"
fi

case "$DESKTOP_FILE" in
  "$HOME/.local/share/applications/biofeedback.desktop") rm -f -- "$DESKTOP_FILE" ;;
  *) printf 'Odmowa usunięcia nieoczekiwanej ścieżki skrótu.\n' >&2; exit 1 ;;
esac

if [[ "${BIOFEEDBACK_REMOVE_DATA:-0}" == "1" ]]; then
  if [[ ! -d "$DATA_DIR" || -L "$DATA_DIR" ]]; then
    printf 'Dane nie zostały usunięte: katalog danych nie istnieje lub jest dowiązaniem.\n' >&2
    exit 1
  fi
  canonical_data="$(cd -- "$DATA_DIR" && pwd -P)"
  canonical_allowed="$(mkdir -p -- "$HOME/.local/share/biofeedback" && cd -- "$HOME/.local/share/biofeedback" && pwd -P)/data"
  if [[ "$canonical_data" != "$canonical_allowed" ]]; then
    printf 'Odmowa usunięcia danych spoza katalogu aplikacji: %s\n' "$canonical_data" >&2
    exit 1
  fi
  printf 'Usunąć trwale dane Biofeedback z %s? Wpisz dokładnie USUŃ DANE POMIAROWE: ' "$canonical_data"
  IFS= read -r confirmation
  if [[ "$confirmation" != "USUŃ DANE POMIAROWE" ]]; then
    printf 'Anulowano usuwanie danych.\n'
    exit 1
  fi
  rm -rf -- "$canonical_data"
fi

printf 'Usunięto pliki aplikacji. Dane pacjentów pozostawiono bez zmian.\n'
