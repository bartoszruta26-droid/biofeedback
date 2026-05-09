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
    echo ""
    
    # Detect package manager
    local pkg_manager=""
    local update_cmd=""
    local install_cmd=""
    
    if command -v apt &> /dev/null; then
        pkg_manager="apt"
        update_cmd="sudo apt update"
        install_cmd="sudo apt install -y"
    elif command -v dnf &> /dev/null; then
        pkg_manager="dnf"
        update_cmd="sudo dnf check-update"
        install_cmd="sudo dnf install -y"
    elif command -v yum &> /dev/null; then
        pkg_manager="yum"
        update_cmd="sudo yum check-update"
        install_cmd="sudo yum install -y"
    elif command -v pacman &> /dev/null; then
        pkg_manager="pacman"
        update_cmd="sudo pacman -Sy"
        install_cmd="sudo pacman -S --noconfirm"
    elif command -v zypper &> /dev/null; then
        pkg_manager="zypper"
        update_cmd="sudo zypper refresh"
        install_cmd="sudo zypper install -y"
    else
        echo -e "${RED}Error: No supported package manager found (apt, dnf, yum, pacman, zypper)${NC}"
        wait_for_key
        return 1
    fi
    
    echo -e "${CYAN}Detected package manager: ${pkg_manager}${NC}"
    echo ""
    
    # Update package lists
    echo -e "${YELLOW}[1/4] Updating package lists...${NC}"
    eval $update_cmd
    if [ $? -ne 0 ]; then
        echo -e "${RED}Failed to update package lists${NC}"
        wait_for_key
        return 1
    fi
    echo -e "${GREEN}Package lists updated successfully${NC}"
    echo ""
    
    # Install core build tools
    echo -e "${YELLOW}[2/4] Installing core build tools...${NC}"
    eval $install_cmd build-essential cmake git pkg-config
    if [ $? -ne 0 ]; then
        echo -e "${RED}Failed to install core build tools${NC}"
        wait_for_key
        return 1
    fi
    echo -e "${GREEN}Core build tools installed successfully${NC}"
    echo ""
    
    # Install Qt5 dependencies
    echo -e "${YELLOW}[3/4] Installing Qt5 development libraries...${NC}"
    
    if [ "$pkg_manager" = "apt" ]; then
        # Try primary package names first
        eval $install_cmd qtbase5-dev libqt5charts5-dev
        if [ $? -ne 0 ]; then
            # Fallback to alternative package names (some Raspberry Pi OS versions)
            echo -e "${YELLOW}Primary Qt5 packages failed, trying alternatives...${NC}"
            eval $install_cmd qtbase5-dev qtcharts5-dev
        fi
    elif [ "$pkg_manager" = "dnf" ] || [ "$pkg_manager" = "yum" ]; then
        eval $install_cmd qt5-qtbase-devel qt5-qtcharts-devel
    elif [ "$pkg_manager" = "pacman" ]; then
        eval $install_cmd qt5-base qt5-charts
    elif [ "$pkg_manager" = "zypper" ]; then
        eval $install_cmd libQt5Core-devel libQt5Charts-devel
    fi
    
    if [ $? -ne 0 ]; then
        echo -e "${RED}Warning: Some Qt5 packages may have failed to install${NC}"
        echo -e "${YELLOW}You may need to install Qt5 manually${NC}"
    else
        echo -e "${GREEN}Qt5 development libraries installed successfully${NC}"
    fi
    echo ""
    
    # Install additional dependencies
    echo -e "${YELLOW}[4/4] Installing additional dependencies...${NC}"
    
    if [ "$pkg_manager" = "apt" ]; then
        eval $install_cmd libserialport-dev nlohmann-json3-dev libssl-dev
    elif [ "$pkg_manager" = "dnf" ] || [ "$pkg_manager" = "yum" ]; then
        eval $install_cmd libserialport-devel nlohmann-json-devel openssl-devel
    elif [ "$pkg_manager" = "pacman" ]; then
        eval $install_cmd libserialport nlohmann-json openssl
    elif [ "$pkg_manager" = "zypper" ]; then
        eval $install_cmd libserialport-devel jsoncpp-devel libopenssl-devel
    fi
    
    if [ $? -ne 0 ]; then
        echo -e "${RED}Warning: Some additional packages may have failed to install${NC}"
    else
        echo -e "${GREEN}Additional dependencies installed successfully${NC}"
    fi
    echo ""
    
    # Configure serial port permissions (Debian/Ubuntu based only)
    if [ "$pkg_manager" = "apt" ]; then
        echo -e "${YELLOW}Configuring serial port permissions...${NC}"
        sudo usermod -a -G dialout $(whoami) 2>/dev/null || true
        
        # Create udev rule for Arduino
        echo -e "${CYAN}Creating udev rule for Arduino...${NC}"
        echo 'SUBSYSTEM=="tty", ATTRS{idVendor}=="2341", MODE="0666"' | sudo tee /etc/udev/rules.d/50-arduino.rules > /dev/null 2>&1
        
        # Reload udev rules
        sudo udevadm control --reload-rules 2>/dev/null || true
        sudo udevadm trigger 2>/dev/null || true
        
        echo -e "${GREEN}Serial port permissions configured${NC}"
        echo -e "${YELLOW}Note: You may need to log out and log back in for group changes to take effect${NC}"
        echo ""
    fi
    
    # Verify installations
    echo -e "${CYAN}Verifying installations...${NC}"
    echo ""
    
    local verify_ok=true
    
    # Check g++
    if command -v g++ &> /dev/null; then
        local gcc_ver=$(g++ --version | head -n1)
        echo -e "${GREEN}✓${NC} g++: ${gcc_ver}"
    else
        echo -e "${RED}✗${NC} g++: NOT INSTALLED"
        verify_ok=false
    fi
    
    # Check cmake
    if command -v cmake &> /dev/null; then
        local cmake_ver=$(cmake --version | head -n1)
        echo -e "${GREEN}✓${NC} cmake: ${cmake_ver}"
    else
        echo -e "${RED}✗${NC} cmake: NOT INSTALLED"
        verify_ok=false
    fi
    
    # Check git
    if command -v git &> /dev/null; then
        local git_ver=$(git --version)
        echo -e "${GREEN}✓${NC} git: ${git_ver}"
    else
        echo -e "${RED}✗${NC} git: NOT INSTALLED"
        verify_ok=false
    fi
    
    # Check pkg-config
    if command -v pkg-config &> /dev/null; then
        echo -e "${GREEN}✓${NC} pkg-config: installed"
    else
        echo -e "${RED}✗${NC} pkg-config: NOT INSTALLED"
        verify_ok=false
    fi
    
    # Check Qt5
    if pkg-config --exists Qt5Core 2>/dev/null; then
        local qt5_ver=$(pkg-config --modversion Qt5Core 2>/dev/null)
        echo -e "${GREEN}✓${NC} Qt5 Core: version ${qt5_ver}"
    else
        echo -e "${YELLOW}!${NC} Qt5 Core: NOT DETECTED by pkg-config"
    fi
    
    if pkg-config --exists Qt5Charts 2>/dev/null; then
        local qt5charts_ver=$(pkg-config --modversion Qt5Charts 2>/dev/null)
        echo -e "${GREEN}✓${NC} Qt5 Charts: version ${qt5charts_ver}"
    else
        echo -e "${YELLOW}!${NC} Qt5 Charts: NOT DETECTED by pkg-config"
    fi
    
    # Check OpenSSL
    if pkg-config --exists OpenSSL 2>/dev/null; then
        local openssl_ver=$(pkg-config --modversion OpenSSL 2>/dev/null)
        echo -e "${GREEN}✓${NC} OpenSSL: version ${openssl_ver}"
    else
        echo -e "${YELLOW}!${NC} OpenSSL: NOT DETECTED by pkg-config"
    fi
    
    # Check nlohmann-json
    if pkg-config --exists nlohmann_json 2>/dev/null; then
        local json_ver=$(pkg-config --modversion nlohmann_json 2>/dev/null)
        echo -e "${GREEN}✓${NC} nlohmann-json: version ${json_ver}"
    else
        # Check if header exists
        if [ -f "/usr/include/nlohmann/json.hpp" ] || [ -f "/usr/local/include/nlohmann/json.hpp" ]; then
            echo -e "${GREEN}✓${NC} nlohmann-json: installed (header found)"
        else
            echo -e "${YELLOW}!${NC} nlohmann-json: NOT DETECTED"
        fi
    fi
    
    # Check libserialport
    if pkg-config --exists libserialport 2>/dev/null; then
        local serialport_ver=$(pkg-config --modversion libserialport 2>/dev/null)
        echo -e "${GREEN}✓${NC} libserialport: version ${serialport_ver}"
    else
        echo -e "${YELLOW}!${NC} libserialport: NOT DETECTED by pkg-config"
    fi
    
    echo ""
    
    if [ "$verify_ok" = true ]; then
        echo -e "${GREEN}============================================${NC}"
        echo -e "${GREEN}  ${LANG[DEP_DONE]}${NC}"
        echo -e "${GREEN}============================================${NC}"
        echo ""
        echo -e "${CYAN}Next steps:${NC}"
        echo "  1. Log out and log back in (for group permissions)"
        echo "  2. Run 'git pull' to download the source code"
        echo "  3. Create build directory: mkdir build && cd build"
        echo "  4. Configure: cmake .."
        echo "  5. Build: make -j\$(nproc)"
        echo "  6. Run: ./biofeedback"
    else
        echo -e "${YELLOW}============================================${NC}"
        echo -e "${YELLOW}  Some critical dependencies are missing${NC}"
        echo -e "${YELLOW}  Please install them manually${NC}"
        echo -e "${YELLOW}============================================${NC}"
    fi
    
    echo ""
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
