#!/bin/bash

# Biofeedback Installer - TUI/CLI Script
# Shell script for managing biofeedback application installation process

set -e

# Colors for TUI
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Global variables
CURRENT_LANG="en"
REPO_URL="https://github.com/bartoszruta26-droid/biofeedback"
WORK_DIR="$(pwd)"
CONFIG_FILE="config.ini"

# =============================================================================
# LANGUAGE STRINGS
# =============================================================================

declare -A LANG_EN=(
    [TITLE]="Biofeedback Installer"
    [MENU_TITLE]="Main Menu"
    [OPT1]="Select Language (pl/en)"
    [OPT2]="Install Dependencies"
    [OPT3]="Git Pull from Repository"
    [OPT4]="Compile Code"
    [OPT5]="Upload Code to Microcontroller"
    [OPT6]="Settings and Configuration Files"
    [OPT7]="Restore Default Settings"
    [OPT8]="Clean Terminal - Data Monitor"
    [OPT9]="Factory Reset"
    [OPT10]="System Information"
    [OPT11]="Check Updates"
    [OPT12]="Backup Configuration"
    [OPT0]="Exit"
    [PROMPT]="Select option (0-12): "
    [LANG_SELECTED]="Language selected: "
    [PRESS_KEY]="Press any key to continue..."
    [INVALID_OPT]="Invalid option. Please try again."
    [EXITING]="Exiting installer. Goodbye!"
    [DEP_INSTALL]="Installing dependencies..."
    [DEP_DONE]="Dependencies installed successfully."
    [GIT_PULL]="Pulling latest changes from repository..."
    [GIT_DONE]="Repository updated successfully."
    [COMPILING]="Compiling code..."
    [COMPILE_DONE]="Compilation completed successfully."
    [UPLOADING]="Uploading code to microcontroller..."
    [UPLOAD_DONE]="Upload completed successfully."
    [SETTINGS]="Opening settings menu..."
    [DEFAULTS]="Restoring default settings..."
    [DEFAULTS_DONE]="Default settings restored."
    [MONITOR]="Starting clean terminal data monitor..."
    [FACTORY]="Performing factory reset..."
    [FACTORY_DONE]="Factory reset completed."
    [SYS_INFO]="Displaying system information..."
    [UPDATES]="Checking for updates..."
    [BACKUP]="Creating configuration backup..."
    [BACKUP_DONE]="Backup created successfully."
)

declare -A LANG_PL=(
    [TITLE]="Instalator Biofeedback"
    [MENU_TITLE]="Menu Główne"
    [OPT1]="Wybierz Język (pl/en)"
    [OPT2]="Instaluj Zależności"
    [OPT3]="Git Pull z Repozytorium"
    [OPT4]="Kompiluj Kod"
    [OPT5]="Wgraj Kod do Mikrokontrolera"
    [OPT6]="Ustawienia i Pliki Konfiguracyjne"
    [OPT7]="Przywróć Ustawienia Domyślne"
    [OPT8]="Czysty Terminal - Monitor Danych"
    [OPT9]="Przywrócenie Ustawień Fabrycznych"
    [OPT10]="Informacje Systemowe"
    [OPT11]="Sprawdź Aktualizacje"
    [OPT12]="Kopia Zapasowa Konfiguracji"
    [OPT0]="Wyjście"
    [PROMPT]="Wybierz opcję (0-12): "
    [LANG_SELECTED]="Wybrano język: "
    [PRESS_KEY]="Naciśnij dowolny klawisz, aby kontynuować..."
    [INVALID_OPT]="Nieprawidłowa opcja. Spróbuj ponownie."
    [EXITING]="Wyjście z instalatora. Do widzenia!"
    [DEP_INSTALL]="Instalowanie zależności..."
    [DEP_DONE]="Zależności zainstalowane pomyślnie."
    [GIT_PULL]="Pobieranie najnowszych zmian z repozytorium..."
    [GIT_DONE]="Repozytorium zaktualizowane pomyślnie."
    [COMPILING]="Kompilowanie kodu..."
    [COMPILE_DONE]="Kompilacja zakończona pomyślnie."
    [UPLOADING]="Wgrywanie kodu do mikrokontrolera..."
    [UPLOAD_DONE]="Wgrywanie zakończone pomyślnie."
    [SETTINGS]="Otwieranie menu ustawień..."
    [DEFAULTS]="Przywracanie ustawień domyślnych..."
    [DEFAULTS_DONE]="Ustawienia domyślne przywrócone."
    [MONITOR]="Uruchamianie czystego terminala monitora danych..."
    [FACTORY]="Wykonywanie resetu fabrycznego..."
    [FACTORY_DONE]="Reset fabryczny zakończony."
    [SYS_INFO]="Wyświetlanie informacji systemowych..."
    [UPDATES]="Sprawdzanie aktualizacji..."
    [BACKUP]="Tworzenie kopii zapasowej konfiguracji..."
    [BACKUP_DONE]="Kopia zapasowa utworzona pomyślnie."
)

# Current language dictionary (will be set based on selection)
declare -A LANG

# =============================================================================
# UTILITY FUNCTIONS
# =============================================================================

clear_screen() {
    clear
}

print_header() {
    echo -e "${CYAN}============================================${NC}"
    echo -e "${CYAN}          ${LANG[TITLE]}                  ${NC}"
    echo -e "${CYAN}============================================${NC}"
    echo ""
}

print_menu() {
    print_header
    echo -e "${YELLOW}${LANG[MENU_TITLE]}:${NC}"
    echo ""
    echo -e "  1. ${LANG[OPT1]}"
    echo -e "  2. ${LANG[OPT2]}"
    echo -e "  3. ${LANG[OPT3]}"
    echo -e "  4. ${LANG[OPT4]}"
    echo -e "  5. ${LANG[OPT5]}"
    echo -e "  6. ${LANG[OPT6]}"
    echo -e "  7. ${LANG[OPT7]}"
    echo -e "  8. ${LANG[OPT8]}"
    echo -e "  9. ${LANG[OPT9]}"
    echo -e " 10. ${LANG[OPT10]}"
    echo -e " 11. ${LANG[OPT11]}"
    echo -e " 12. ${LANG[OPT12]}"
    echo -e "  0. ${LANG[OPT0]}"
    echo ""
}

wait_for_key() {
    echo ""
    echo -e "${YELLOW}${LANG[PRESS_KEY]}${NC}"
    read -n 1 -s
}

# =============================================================================
# FEATURE FUNCTIONS (STUBS - TO BE IMPLEMENTED LATER)
# =============================================================================

option_language() {
    echo ""
    echo -e "${BLUE}--- ${LANG[OPT1]} ---${NC}"
    echo "  1. English"
    echo "  2. Polski"
    echo ""
    read -p "Choose language (1-2): " lang_choice
    
    case $lang_choice in
        1)
            CURRENT_LANG="en"
            for key in "${!LANG_EN[@]}"; do
                LANG[$key]="${LANG_EN[$key]}"
            done
            ;;
        2)
            CURRENT_LANG="pl"
            for key in "${!LANG_PL[@]}"; do
                LANG[$key]="${LANG_PL[$key]}"
            done
            ;;
        *)
            echo -e "${RED}Invalid choice${NC}"
            return
            ;;
    esac
    
    echo -e "${GREEN}${LANG[LANG_SELECTED]}${CURRENT_LANG}${NC}"
    wait_for_key
}

option_install_dependencies() {
    echo ""
    echo -e "${BLUE}--- ${LANG[OPT2]} ---${NC}"
    echo -e "${YELLOW}${LANG[DEP_INSTALL]}${NC}"
    # TODO: Implement dependency installation logic
    echo -e "${GREEN}${LANG[DEP_DONE]}${NC}"
    wait_for_key
}

option_git_pull() {
    echo ""
    echo -e "${BLUE}--- ${LANG[OPT3]} ---${NC}"
    echo -e "${YELLOW}${LANG[GIT_PULL]}${NC}"
    # TODO: Implement git pull logic
    echo -e "${GREEN}${LANG[GIT_DONE]}${NC}"
    wait_for_key
}

option_compile_code() {
    echo ""
    echo -e "${BLUE}--- ${LANG[OPT4]} ---${NC}"
    echo -e "${YELLOW}${LANG[COMPILING]}${NC}"
    # TODO: Implement compilation logic
    echo -e "${GREEN}${LANG[COMPILE_DONE]}${NC}"
    wait_for_key
}

option_upload_code() {
    echo ""
    echo -e "${BLUE}--- ${LANG[OPT5]} ---${NC}"
    echo -e "${YELLOW}${LANG[UPLOADING]}${NC}"
    # TODO: Implement upload logic
    echo -e "${GREEN}${LANG[UPLOAD_DONE]}${NC}"
    wait_for_key
}

option_settings() {
    echo ""
    echo -e "${BLUE}--- ${LANG[OPT6]} ---${NC}"
    echo -e "${YELLOW}${LANG[SETTINGS]}${NC}"
    # TODO: Implement settings menu
    wait_for_key
}

option_restore_defaults() {
    echo ""
    echo -e "${BLUE}--- ${LANG[OPT7]} ---${NC}"
    echo -e "${YELLOW}${LANG[DEFAULTS]}${NC}"
    # TODO: Implement restore defaults logic
    echo -e "${GREEN}${LANG[DEFAULTS_DONE]}${NC}"
    wait_for_key
}

option_clean_terminal() {
    echo ""
    echo -e "${BLUE}--- ${LANG[OPT8]} ---${NC}"
    echo -e "${YELLOW}${LANG[MONITOR]}${NC}"
    # TODO: Implement clean terminal monitor
    wait_for_key
}

option_factory_reset() {
    echo ""
    echo -e "${BLUE}--- ${LANG[OPT9]} ---${NC}"
    echo -e "${YELLOW}${LANG[FACTORY]}${NC}"
    # TODO: Implement factory reset logic
    echo -e "${GREEN}${LANG[FACTORY_DONE]}${NC}"
    wait_for_key
}

option_system_info() {
    echo ""
    echo -e "${BLUE}--- ${LANG[OPT10]} ---${NC}"
    echo -e "${YELLOW}${LANG[SYS_INFO]}${NC}"
    # TODO: Implement system information display
    wait_for_key
}

option_check_updates() {
    echo ""
    echo -e "${BLUE}--- ${LANG[OPT11]} ---${NC}"
    echo -e "${YELLOW}${LANG[UPDATES]}${NC}"
    # TODO: Implement update check logic
    wait_for_key
}

option_backup_config() {
    echo ""
    echo -e "${BLUE}--- ${LANG[OPT12]} ---${NC}"
    echo -e "${YELLOW}${LANG[BACKUP]}${NC}"
    # TODO: Implement backup logic
    echo -e "${GREEN}${LANG[BACKUP_DONE]}${NC}"
    wait_for_key
}

# =============================================================================
# MAIN MENU LOOP
# =============================================================================

main_menu() {
    local choice
    
    while true; do
        clear_screen
        print_menu
        read -p "${LANG[PROMPT]}" choice
        
        case $choice in
            1)
                option_language
                ;;
            2)
                option_install_dependencies
                ;;
            3)
                option_git_pull
                ;;
            4)
                option_compile_code
                ;;
            5)
                option_upload_code
                ;;
            6)
                option_settings
                ;;
            7)
                option_restore_defaults
                ;;
            8)
                option_clean_terminal
                ;;
            9)
                option_factory_reset
                ;;
            10)
                option_system_info
                ;;
            11)
                option_check_updates
                ;;
            12)
                option_backup_config
                ;;
            0)
                clear_screen
                echo -e "${GREEN}${LANG[EXITING]}${NC}"
                exit 0
                ;;
            *)
                echo -e "${RED}${LANG[INVALID_OPT]}${NC}"
                sleep 2
                ;;
        esac
    done
}

# =============================================================================
# INITIALIZATION
# =============================================================================

init() {
    # Set default language to English
    for key in "${!LANG_EN[@]}"; do
        LANG[$key]="${LANG_EN[$key]}"
    done
    
    # Check if running as root (optional, can be removed if not needed)
    # if [[ $EUID -ne 0 ]]; then
    #     echo -e "${YELLOW}Warning: Some operations may require root privileges${NC}"
    #     sleep 2
    # fi
}

# =============================================================================
# SCRIPT ENTRY POINT
# =============================================================================

init
main_menu
