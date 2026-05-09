#!/bin/bash

# ==============================================================================
# BioFeedback Installer & Manager
# Opis: Skrypt TUI do zarządzania instalacją, kompilacją i wgrywaniem firmware
#       do mikrokontrolerów (ESP32, STM32 itp.)
# Repozytorium: https://github.com/bartoszruta26-droid/biofeedback
# ==============================================================================

# Konfiguracja globalna
REPO_URL="https://github.com/bartoszruta26-droid/biofeedback"
REPO_DIR="biofeedback"
BUILD_DIR="build"
LANG="pl" # domyślny język

# Kolory ANSI
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# ------------------------------------------------------------------------------
# Funkcje pomocnicze (Utils)
# ------------------------------------------------------------------------------

print_header() {
    clear
    echo -e "${CYAN}==============================================================================${NC}"
    echo -e "${CYAN}          BioFeedback Installer & Manager - Menu Główne                       ${NC}"
    echo -e "${CYAN}==============================================================================${NC}"
    echo ""
}

print_error() {
    echo -e "${RED}[BŁĄD]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUKCES]${NC} $1"
}

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[OSTRZEŻENIE]${NC} $1"
}

wait_for_key() {
    echo ""
    read -p "Naciśnij Enter, aby kontynuować..."
}

# ------------------------------------------------------------------------------
# Implementacja Opcji 3: Git Pull & Kompilacja (Zaimplementowana wcześniej)
# ------------------------------------------------------------------------------
option_3_menu() {
    while true; do
        clear
        echo -e "${CYAN}--- Opcja 3: Zarządzanie Kodem i Kompilacja ---${NC}"
        echo "1. Git Pull (Pobierz zmiany)"
        echo "2. Kompiluj projekt (CMake)"
        echo "3. Rekompiluj (Czysty build)"
        echo "0. Powrót do menu głównego"
        echo ""
        read -p "Wybierz opcję: " sub_choice

        case $sub_choice in
            1)
                print_info "Aktualizowanie repozytorium..."
                if [ -d "$REPO_DIR/.git" ]; then
                    cd "$REPO_DIR" || exit
                    git fetch origin
                    git pull origin main
                    cd ..
                else
                    print_warning "Repozytorium nie istnieje. Klonowanie..."
                    git clone "$REPO_URL"
                fi
                wait_for_key
                ;;
            2)
                print_info "Kompilowanie projektu..."
                if [ ! -d "$REPO_DIR" ]; then print_error "Brak katalogu źródłowego!"; wait_for_key; continue; fi
                mkdir -p "$REPO_DIR/$BUILD_DIR"
                cd "$REPO_DIR/$BUILD_DIR" || exit
                cmake .. && make -j$(nproc)
                cd ../..
                wait_for_key
                ;;
            3)
                print_warning "Czyszczenie i ponowna kompilacja..."
                if [ -d "$REPO_DIR/$BUILD_DIR" ]; then
                    rm -rf "$REPO_DIR/$BUILD_DIR"
                fi
                mkdir -p "$REPO_DIR/$BUILD_DIR"
                cd "$REPO_DIR/$BUILD_DIR" || exit
                cmake .. && make -j$(nproc)
                cd ../..
                wait_for_key
                ;;
            0) return ;;
            *) print_error "Nieprawidłowa opcja"; sleep 1 ;;
        esac
    done
}

# ------------------------------------------------------------------------------
# Implementacja Opcji 4: Wgrywanie kodu do mikrokontrolera (NOWE)
# ------------------------------------------------------------------------------

# Funkcja do wykrywania dostępnych portów szeregowych
detect_serial_ports() {
    local ports=()
    
    # Sprawdź typowe lokalizacje portów w Linux
    if command -v lsusb &> /dev/null; then
        echo -e "${BLUE}Skonfigurowane urządzenia USB:${NC}"
        lsusb | grep -i "serial\|ftdi\|silicon\|cp210x\|ch34\|esp32\|stm32" || echo "Brak typowych konwerterów w lsusb."
        echo ""
    fi

    echo -e "${BLUE}Wykryte porty szeregowe:${NC}"
    
    # Linux: ttyUSB, ttyACM
    for port in /dev/ttyUSB* /dev/ttyACM*; do
        if [ -e "$port" ]; then
            ports+=("$port")
            echo "  - $port"
        fi
    done

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
        update_cmd="sudo dnf makecache --refresh"
        install_cmd="sudo dnf install -y"
    elif command -v yum &> /dev/null; then
        pkg_manager="yum"
        update_cmd="sudo yum makecache"
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
    
    if [ "$pkg_manager" = "apt" ]; then
        eval $install_cmd build-essential cmake git pkg-config
    elif [ "$pkg_manager" = "dnf" ] || [ "$pkg_manager" = "yum" ]; then
        eval $install_cmd gcc-c++ make cmake git pkgconfig
    elif [ "$pkg_manager" = "pacman" ]; then
        eval $install_cmd base-devel cmake git pkg-config
    elif [ "$pkg_manager" = "zypper" ]; then
        eval $install_cmd gcc-c++ make cmake git pkg-config
    fi
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

    if [ ${#ports[@]} -eq 0 ]; then
        echo "  Brak wykrytych portów szeregowych."
        return 1
    fi
    
    # Zwracamy listę portów jako wynik (w bashu trudno zwrócić tablicę, więc drukujemy)
    echo "${ports[@]}" > /tmp/detected_ports.txt
    return 0
}

# Funkcja szukająca pliki binarne w katalogu build
find_firmware_file() {
    local search_dir="$REPO_DIR/$BUILD_DIR"
    local file_ext="$1" # np. "bin" lub "hex"
    
    if [ ! -d "$search_dir" ]; then
        return 1
    fi

    find "$search_dir" -name "*.$file_ext" -type f 2>/dev/null | head -n 1
}

# Główna funkcja opcji 4
option_4_menu() {
    local selected_port=""
    
    while true; do
        clear
        echo -e "${CYAN}--- Opcja 4: Wgrywanie Firmware do Mikrokontrolera ---${NC}"
        echo ""
        echo "1. Wykryj podłączone urządzenia"
        echo "2. Wybierz port ręcznie"
        echo "3. Wgraj firmware (ESP32 / esptool)"
        echo "4. Wgraj firmware (STM32 / st-flash)"
        echo "5. Wgraj przez USB-CDC / DFU (Mass Storage)"
        echo "6. Monitor portu szeregowego (Logi startowe)"
        echo "0. Powrót do menu głównego"
        echo ""
        
        if [ -n "$selected_port" ]; then
            echo -e "${GREEN}Aktywny port: $selected_port${NC}"
        else
            echo -e "${YELLOW}Aktywny port: NIEWYBRANY${NC}"
        fi
        echo ""
        
        read -p "Wybierz opcję: " sub_choice

        case $sub_choice in
            1)
                detect_serial_ports
                if [ -f /tmp/detected_ports.txt ]; then
                    read -p "Czy chcesz ustawić pierwszy wykryty port jako aktywny? (t/n): " confirm
                    if [[ "$confirm" == "t" || "$confirm" == "T" ]]; then
                        selected_port=$(head -n 1 /tmp/detected_ports.txt | awk '{print $1}')
                        print_success "Wybrano port: $selected_port"
                    fi
                fi
                wait_for_key
                ;;
                
            2)
                read -p "Podaj ścieżkę do portu (np. /dev/ttyUSB0): " selected_port
                if [ -e "$selected_port" ]; then
                    print_success "Port $selected_port istnieje."
                else
                    print_warning "Port $selected_port nie istnieje w systemie!"
                fi
                wait_for_key
                ;;

            3)
                # Logika dla ESP32
                if [ -z "$selected_port" ]; then
                    print_error "Najpierw wybierz port (opcja 1 lub 2)!"
                    wait_for_key
                    continue
                fi

                print_info "Szukanie pliku firmware (.bin)..."
                fw_file=$(find_firmware_file "bin")
                
                if [ -z "$fw_file" ]; then
                    print_error "Nie znaleziono pliku .bin w katalogu $REPO_DIR/$BUILD_DIR"
                    print_info "Czy chcesz wskazać plik ręcznie?"
                    read -p "Ścieżka do pliku .bin: " fw_file
                fi

                if [ ! -f "$fw_file" ]; then
                    print_error "Plik firmware nie istnieje!"
                    wait_for_key
                    continue
                fi

                print_info "Wykrywanie esptool..."
                if command -v esptool.py &> /dev/null; then
                    ESPTOOL_CMD="esptool.py"
                elif command -v esptool &> /dev/null; then
                    ESPTOOL_CMD="esptool"
                else
                    print_error "Nie znaleziono narzędzia esptool. Zainstaluj: pip install esptool"
                    wait_for_key
                    continue
                fi

                print_info "Wgrywanie $fw_file na $selected_port ..."
                # Przykładowa komenda dla ESP32 (może wymagać dostosowania adresu 0x1000)
                $ESPTOOL_CMD --chip esp32 --port "$selected_port" write_flash 0x1000 "$fw_file"
                
                if [ $? -eq 0 ]; then
                    print_success "Wgrywanie zakończone powodzeniem!"
                else
                    print_error "Błąd wgrywania firmware."
                fi
                wait_for_key
                ;;

            4)
                # Logika dla STM32 (st-link)
                if [ -z "$selected_port" ]; then
                     # Dla ST-Link port nie zawsze jest potrzebny w trybie SWD, ale warto mieć wybrane urządzenie
                     print_info "Tryb ST-Link często nie wymaga wyboru portu COM, ale upewnij się, że urządzenie jest podłączone USB."
                fi

                print_info "Szukanie pliku firmware (.bin lub .hex)..."
                fw_file=$(find_firmware_file "bin")
                if [ -z "$fw_file" ]; then
                    fw_file=$(find_firmware_file "hex")
                fi

                if [ -z "$fw_file" ] || [ ! -f "$fw_file" ]; then
                    print_error "Nie znaleziono pliku .bin ani .hex do wgrania."
                    wait_for_key
                    continue
                fi

                if ! command -v st-flash &> /dev/null; then
                    print_error "Nie znaleziono narzędzia 'st-flash'. Zainstaluj pakiet stlink-tools."
                    wait_for_key
                    continue
                fi

                print_info "Wgrywanie $fw_file przez ST-Link..."
                # Przykład: st-flash write firmware.bin 0x8000000
                # Adres zależy od konkretnego modelu STM32, zakładamy standardowy 0x8000000
                st-flash write "$fw_file" 0x8000000

                if [ $? -eq 0 ]; then
                    print_success "Wgrywanie na STM32 zakończone!"
                else
                    print_error "Błąd komunikacji ze ST-Link lub błędny adres flasha."
                fi
                wait_for_key
                ;;

            5)
                # Logika DFU / Mass Storage
                print_info "Tryb DFU / Disk Copy."
                read -p "Podaj ścieżkę do montowania (np. /media/user/DEVICE lub urządzenie DFU): " target_path
                
                if [ -d "$target_path" ]; then
                    print_info "Kopiowanie pliku bin/hex na urządzenie..."
                    # Szukaj dowolnego pliku wykonywalnego
                    fw_file=$(find_firmware_file "bin")
                    if [ -z "$fw_file" ]; then fw_file=$(find_firmware_file "hex"); fi
                    
                    if [ -n "$fw_file" ]; then
                        cp "$fw_file" "$target_path/"
                        sync
                        print_success "Kopiowanie zakończone. Odłącz device aby zresetować."
                    else
                        print_error "Brak pliku do skopiowania."
                    fi
                else
                    # Próba użycia dfu-util jeśli ścieżka nie jest katalogiem
                    if command -v dfu-util &> /dev/null; then
                        read -p "Podaj ID urządzenia DFU (np. 0483:df11): " dfu_id
                        fw_file=$(find_firmware_file "bin")
                        if [ -n "$fw_file" ]; then
                            dfu-util -d "$dfu_id" -D "$fw_file"
                        else
                            print_error "Brak pliku bin."
                        fi
                    else
                        print_error "Ścieżka nie istnieje i brak dfu-util."
                    fi
                fi
                wait_for_key
                ;;

            6)
                # Monitor szeregowy
                if [ -z "$selected_port" ]; then
                    print_error "Wybierz port najpierw!"
                    wait_for_key
                    continue
                fi
                
                print_info "Uruchamianie monitora szeregowego na $selected_port (Baud: 115200)..."
                print_info "Naciśnij Ctrl+A, potem X aby wyjść (screen) lub Ctrl+C (minicom/cat)."
                
                if command -v screen &> /dev/null; then
                    screen "$selected_port" 115200
                elif command -v minicom &> /dev/null; then
                    minicom -D "$selected_port" -b 115200
                elif command -v cat &> /dev/null; then
                    # Prosty podgląd
                    cat "$selected_port"
                else
                    print_error "Brak dostępnego programu terminala (screen, minicom)."
                fi
                wait_for_key
                ;;

            0)
                return
                ;;
            *)
                print_error "Nieprawidłowa opcja."
                sleep 1
                ;;
        esac
    done
}

# ------------------------------------------------------------------------------
# Pozostałe opcje (Szkielet)
# ------------------------------------------------------------------------------

option_1_language() {
    echo "Wybór języka (PL/EN) - W implementacji..."
    wait_for_key
}

option_2_deps() {
    echo "Instalacja zależności - W implementacji..."
    wait_for_key
}

option_5_config() {
    echo "Konfiguracja - W implementacji..."
    wait_for_key
}

# ------------------------------------------------------------------------------
# Implementacja Opcji 6: Przywracanie ustawień domyślnych (NOWE)
# ------------------------------------------------------------------------------

option_6_defaults() {
    local config_dir="$REPO_DIR/config"
    local backup_dir="$REPO_DIR/backups"
    local user_config_file="$HOME/.biofeedback_config"
    local default_serial_baud="115200"
    local default_log_level="INFO"
    local default_language="pl"
    
    while true; do
        clear
        echo -e "${CYAN}==============================================================================${NC}"
        echo -e "${CYAN}     Opcja 6: Przywracanie Ustawień Domyślnych                               ${NC}"
        echo -e "${CYAN}==============================================================================${NC}"
        echo ""
        echo -e "${YELLOW}UWAGA:${NC} Ta opcja przywróci wybrane konfiguracje do wartości fabrycznych."
        echo -e "${YELLOW}      Zalecane jest wykonanie kopii zapasowej przed kontynuacją.${NC}"
        echo ""
        echo "1. Przywróć domyślne ustawienia komunikacji szeregowej"
        echo "2. Przywróć domyślne ustawienia interfeusu użytkownika (GUI)"
        echo "3. Przywróć domyślne ustawienia rejestracji danych (Logging)"
        echo "4. Przywróć domyślne ustawienia kalibracji sensorów"
        echo "5. Przywróć WSZYSTKIE ustawienia do wartości domyślnych"
        echo "6. Wykonaj kopię zapasową obecnych ustawień"
        echo "7. Przywróć z kopii zapasowej"
        echo "8. Pokaż obecne ustawienia"
        echo "0. Powrót do menu głównego"
        echo ""
        
        read -p "Wybierz opcję [0-8]: " sub_choice
        
        case $sub_choice in
            1)
                # Przywracanie ustawień komunikacji szeregowej
                clear
                echo -e "${CYAN}--- Przywracanie ustawień komunikacji szeregowej ---${NC}"
                echo ""
                
                read -p "Czy na pewno chcesz przywrócić domyślne ustawienia szeregowe? (t/n): " confirm
                if [[ "$confirm" != "t" && "$confirm" != "T" ]]; then
                    print_info "Anulowano."
                    wait_for_key
                    continue
                fi
                
                # Tworzenie/aktualizacja pliku konfiguracyjnego
                mkdir -p "$config_dir"
                
                cat > "$config_dir/serial.conf" << EOF
# BioFeedback Serial Configuration
# Przywrócono ustawienia domyślne: $(date)

BAUD_RATE=$default_serial_baud
DATA_BITS=8
PARITY=none
STOP_BITS=1
FLOW_CONTROL=none
TIMEOUT_MS=1000
RECONNECT_ATTEMPTS=3
RECONNECT_DELAY_MS=500
EOF
                
                if [ $? -eq 0 ]; then
                    print_success "Przywrócono domyślne ustawienia komunikacji szeregowej!"
                    echo ""
                    echo "Parametry:"
                    echo "  - Baud Rate: $default_serial_baud"
                    echo "  - Data Bits: 8"
                    echo "  - Parity: none"
                    echo "  - Stop Bits: 1"
                    echo "  - Flow Control: none"
                else
                    print_error "Nie udało się zapisać konfiguracji szeregowej."
                fi
                
                wait_for_key
                ;;
                
            2)
                # Przywracanie ustawień GUI
                clear
                echo -e "${CYAN}--- Przywracanie ustawień interfejsu użytkownika ---${NC}"
                echo ""
                
                read -p "Czy na pewno chcesz przywrócić domyślne ustawienia GUI? (t/n): " confirm
                if [[ "$confirm" != "t" && "$confirm" != "T" ]]; then
                    print_info "Anulowano."
                    wait_for_key
                    continue
                fi
                
                mkdir -p "$config_dir"
                
                cat > "$config_dir/gui.conf" << EOF
# BioFeedback GUI Configuration
# Przywrócono ustawienia domyślne: $(date)

LANGUAGE=$default_language
WINDOW_WIDTH=1280
WINDOW_HEIGHT=720
THEME=default
REFRESH_RATE_MS=100
SHOW_GRID=true
SHOW_LEGEND=true
CHART_TYPE=line
COLOR_SCHEME=default
FONT_SIZE=12
AUTO_SCALE=true
EOF
                
                if [ $? -eq 0 ]; then
                    print_success "Przywrócono domyślne ustawienia interfejsu użytkownika!"
                    echo ""
                    echo "Parametry:"
                    echo "  - Język: $default_language"
                    echo "  - Rozmiar okna: 1280x720"
                    echo "  - Motyw: default"
                    echo "  - Częstotliwość odświeżania: 100ms"
                else
                    print_error "Nie udało się zapisać konfiguracji GUI."
                fi
                
                wait_for_key
                ;;
                
            3)
                # Przywracanie ustawień logowania
                clear
                echo -e "${CYAN}--- Przywracanie ustawień rejestracji danych ---${NC}"
                echo ""
                
                read -p "Czy na pewno chcesz przywrócić domyślne ustawienia logowania? (t/n): " confirm
                if [[ "$confirm" != "t" && "$confirm" != "T" ]]; then
                    print_info "Anulowano."
                    wait_for_key
                    continue
                fi
                
                mkdir -p "$config_dir"
                mkdir -p "$REPO_DIR/logs"
                
                cat > "$config_dir/logging.conf" << EOF
# BioFeedback Logging Configuration
# Przywrócono ustawienia domyślne: $(date)

LOG_LEVEL=$default_log_level
LOG_TO_FILE=true
LOG_TO_CONSOLE=true
LOG_DIRECTORY=$REPO_DIR/logs
MAX_LOG_SIZE_MB=10
MAX_LOG_FILES=5
LOG_FORMAT=%(asctime)s - %(levelname)s - %(message)s
LOG_DATE_FORMAT=%Y-%m-%d %H:%M:%S
ENABLE_DEBUG=false
ENABLE_TIMESTAMP=true
EOF
                
                if [ $? -eq 0 ]; then
                    print_success "Przywrócono domyślne ustawienia rejestracji danych!"
                    echo ""
                    echo "Parametry:"
                    echo "  - Poziom logowania: $default_log_level"
                    echo "  - Logowanie do pliku: tak"
                    echo "  - Logowanie do konsoli: tak"
                    echo "  - Maksymalny rozmiar pliku: 10MB"
                    echo "  - Maksymalna liczba plików: 5"
                else
                    print_error "Nie udało się zapisać konfiguracji logowania."
                fi
                
                wait_for_key
                ;;
                
            4)
                # Przywracanie ustawień kalibracji
                clear
                echo -e "${CYAN}--- Przywracanie ustawień kalibracji sensorów ---${NC}"
                echo ""
                
                read -p "Czy na pewno chcesz przywrócić domyślne ustawienia kalibracji? (t/n): " confirm
                if [[ "$confirm" != "t" && "$confirm" != "T" ]]; then
                    print_info "Anulowano."
                    wait_for_key
                    continue
                fi
                
                mkdir -p "$config_dir"
                
                cat > "$config_dir/calibration.conf" << EOF
# BioFeedback Sensor Calibration Configuration
# Przywrócono ustawienia domyślne: $(date)

# EKG/EMG Settings
ECG_GAIN=1000
ECG_OFFSET=0
ECG_FILTER_ENABLED=true
ECG_LOW_CUTOFF_HZ=0.5
ECG_HIGH_CUTOFF_HZ=100

# GSR (Galvanic Skin Response) Settings
GSR_GAIN=1
GSR_OFFSET=0
GSR_FILTER_ENABLED=true

# Temperature Settings
TEMP_OFFSET=0
TEMP_SCALE=1.0

# Accelerometer Settings
ACC_ENABLED=true
ACC_RANGE=2
ACC_FILTER_ENABLED=true

# Auto-calibration
AUTO_CALIBRATE_ON_START=false
CALIBRATION_INTERVAL_SEC=3600
EOF
                
                if [ $? -eq 0 ]; then
                    print_success "Przywrócono domyślne ustawienia kalibracji sensorów!"
                    echo ""
                    echo "Przywrócono parametry dla:"
                    echo "  - EKG/EMG (wzmocnienie, filtry)"
                    echo "  - GSR (reakcja skórna)"
                    echo "  - Temperatura"
                    echo "  - Akcelerometr"
                else
                    print_error "Nie udało się zapisać konfiguracji kalibracji."
                fi
                
                wait_for_key
                ;;
                
            5)
                # Przywracanie WSZYSTKICH ustawień
                clear
                echo -e "${RED}==============================================================================${NC}"
                echo -e "${RED}     PRZYWRACANIE WSZYSTKICH USTAWIEŃ DO WARTOŚCI DOMYŚLNYCH                ${NC}"
                echo -e "${RED}==============================================================================${NC}"
                echo ""
                echo -e "${RED}OSTRZEŻENIE:${NC} Ta operacja usunie wszystkie niestandardowe konfiguracje!"
                echo ""
                read -p "Czy jesteś ABSOLUTNIE pewien? (wpisz 'TAK' aby potwierdzić): " confirm
                
                if [[ "$confirm" != "TAK" ]]; then
                    print_info "Anulowano."
                    wait_for_key
                    continue
                fi
                
                # Usuwanie istniejących plików konfiguracyjnych
                if [ -d "$config_dir" ]; then
                    rm -rf "$config_dir"
                fi
                
                # Przywracanie wszystkich ustawień poprzez wywołanie poprzednich funkcji
                mkdir -p "$config_dir"
                mkdir -p "$REPO_DIR/logs"
                
                # Serial config
                cat > "$config_dir/serial.conf" << EOF
# BioFeedback Serial Configuration
# Przywrócono wszystkie ustawienia domyślne: $(date)

BAUD_RATE=$default_serial_baud
DATA_BITS=8
PARITY=none
STOP_BITS=1
FLOW_CONTROL=none
TIMEOUT_MS=1000
RECONNECT_ATTEMPTS=3
RECONNECT_DELAY_MS=500
EOF
                
                # GUI config
                cat > "$config_dir/gui.conf" << EOF
# BioFeedback GUI Configuration

LANGUAGE=$default_language
WINDOW_WIDTH=1280
WINDOW_HEIGHT=720
THEME=default
REFRESH_RATE_MS=100
SHOW_GRID=true
SHOW_LEGEND=true
CHART_TYPE=line
COLOR_SCHEME=default
FONT_SIZE=12
AUTO_SCALE=true
EOF
                
                # Logging config
                cat > "$config_dir/logging.conf" << EOF
# BioFeedback Logging Configuration

LOG_LEVEL=$default_log_level
LOG_TO_FILE=true
LOG_TO_CONSOLE=true
LOG_DIRECTORY=$REPO_DIR/logs
MAX_LOG_SIZE_MB=10
MAX_LOG_FILES=5
LOG_FORMAT=%(asctime)s - %(levelname)s - %(message)s
LOG_DATE_FORMAT=%Y-%m-%d %H:%M:%S
ENABLE_DEBUG=false
ENABLE_TIMESTAMP=true
EOF
                
                # Calibration config
                cat > "$config_dir/calibration.conf" << EOF
# BioFeedback Sensor Calibration Configuration

ECG_GAIN=1000
ECG_OFFSET=0
ECG_FILTER_ENABLED=true
ECG_LOW_CUTOFF_HZ=0.5
ECG_HIGH_CUTOFF_HZ=100
GSR_GAIN=1
GSR_OFFSET=0
GSR_FILTER_ENABLED=true
TEMP_OFFSET=0
TEMP_SCALE=1.0
ACC_ENABLED=true
ACC_RANGE=2
ACC_FILTER_ENABLED=true
AUTO_CALIBRATE_ON_START=false
CALIBRATION_INTERVAL_SEC=3600
EOF
                
                # User config file
                cat > "$user_config_file" << EOF
# BioFeedback User Configuration
# Created: $(date)

DEFAULT_LANGUAGE=$default_language
LAST_USED_PORT=
LAST_USED_BAUD=$default_serial_baud
FAVORITE_PROFILES=
EOF
                
                print_success "Przywrócono WSZYSTKIE ustawienia do wartości domyślnych!"
                echo ""
                echo "Przywrócone konfiguracje:"
                echo "  ✓ Komunikacja szeregowa"
                echo "  ✓ Interfejs użytkownika (GUI)"
                echo "  ✓ Rejestracja danych (Logging)"
                echo "  ✓ Kalibracja sensorów"
                echo "  ✓ Konfiguracja użytkownika"
                
                wait_for_key
                ;;
                
            6)
                # Wykonywanie kopii zapasowej
                clear
                echo -e "${CYAN}--- Wykonywanie kopii zapasowej ustawień ---${NC}"
                echo ""
                
                mkdir -p "$backup_dir"
                local backup_timestamp=$(date +%Y%m%d_%H%M%S)
                local backup_archive="$backup_dir/biofeedback_backup_$backup_timestamp.tar.gz"
                
                if [ -d "$config_dir" ] && [ "$(ls -A $config_dir 2>/dev/null)" ]; then
                    tar -czf "$backup_archive" -C "$REPO_DIR" config
                    if [ $? -eq 0 ]; then
                        print_success "Wykonano kopię zapasową!"
                        echo ""
                        echo "Lokalizacja: $backup_archive"
                        echo "Rozmiar: $(du -h "$backup_archive" | cut -f1)"
                    else
                        print_error "Nie udało się wykonać kopii zapasowej."
                    fi
                elif [ -f "$user_config_file" ]; then
                    cp "$user_config_file" "$backup_dir/user_config_backup_$backup_timestamp"
                    if [ $? -eq 0 ]; then
                        print_success "Wykonano kopię zapasową konfiguracji użytkownika!"
                        echo ""
                        echo "Lokalizacja: $backup_dir/user_config_backup_$backup_timestamp"
                    else
                        print_error "Nie udało się wykonać kopii zapasowej."
                    fi
                else
                    print_warning "Brak plików konfiguracyjnych do skopiowania."
                fi
                
                wait_for_key
                ;;
                
            7)
                # Przywracanie z kopii zapasowej
                clear
                echo -e "${CYAN}--- Przywracanie z kopii zapasowej ---${NC}"
                echo ""
                
                if [ ! -d "$backup_dir" ] || [ -z "$(ls -A $backup_dir 2>/dev/null)" ]; then
                    print_warning "Brak dostępnych kopii zapasowych w katalogu $backup_dir"
                    wait_for_key
                    continue
                fi
                
                echo "Dostępne kopie zapasowe:"
                echo ""
                ls -lht "$backup_dir"/*.tar.gz 2>/dev/null | head -10
                echo ""
                
                read -p "Podaj nazwę pliku kopii zapasowej (lub naciśnij Enter dla najnowszej): " backup_file
                
                if [ -z "$backup_file" ]; then
                    backup_file=$(ls -t "$backup_dir"/*.tar.gz 2>/dev/null | head -1)
                else
                    backup_file="$backup_dir/$backup_file"
                fi
                
                if [ ! -f "$backup_file" ]; then
                    print_error "Nie znaleziono podanego pliku kopii zapasowej."
                    wait_for_key
                    continue
                fi
                
                read -p "Czy przywrócić konfigurację z $backup_file? (t/n): " confirm
                if [[ "$confirm" != "t" && "$confirm" != "T" ]]; then
                    print_info "Anulowano."
                    wait_for_key
                    continue
                fi
                
                tar -xzf "$backup_file" -C "$REPO_DIR"
                if [ $? -eq 0 ]; then
                    print_success "Przywrócono konfigurację z kopii zapasowej!"
                    echo ""
                    echo "Plik: $backup_file"
                else
                    print_error "Nie udało się przywrócić konfiguracji."
                fi
                
                wait_for_key
                ;;
                
            8)
                # Pokazywanie obecnych ustawień
                clear
                echo -e "${CYAN}--- Obecne ustawienia ---${NC}"
                echo ""
                
                if [ -d "$config_dir" ] && [ "$(ls -A $config_dir 2>/dev/null)" ]; then
                    echo -e "${BLUE}Pliki konfiguracyjne w $config_dir:${NC}"
                    echo ""
                    
                    for conf_file in "$config_dir"/*.conf; do
                        if [ -f "$conf_file" ]; then
                            echo -e "${GREEN}=== $(basename "$conf_file") ===${NC}"
                            grep -v "^#" "$conf_file" | grep -v "^$" | head -10
                            echo ""
                        fi
                    done
                else
                    print_warning "Brak plików konfiguracyjnych w katalogu $config_dir"
                fi
                
                if [ -f "$user_config_file" ]; then
                    echo -e "${BLUE}Konfiguracja użytkownika ($user_config_file):${NC}"
                    grep -v "^#" "$user_config_file" | grep -v "^$"
                    echo ""
                fi
                
                echo -e "${BLUE}Domyślne wartości:${NC}"
                echo "  - Baud Rate: $default_serial_baud"
                echo "  - Log Level: $default_log_level"
                echo "  - Language: $default_language"
                echo ""
                
                wait_for_key
                ;;
                
            0)
                return
                ;;
                
            *)
                print_error "Nieprawidłowa opcja."
                sleep 1
                ;;
        esac
    done
}

option_7_monitor() {
    echo "Czysty terminal monitora - W implementacji..."
    wait_for_key
}

option_8_factory() {
    echo "Przywrócenie ustawień fabrycznych - W implementacji..."
    wait_for_key
}

option_9_extra() {
    echo "Inne opcje (Backup, Info o systemie) - W implementacji..."
    wait_for_key
}

# ------------------------------------------------------------------------------
# Pętla Główna Menu
# ------------------------------------------------------------------------------

main_menu() {
    while true; do
        print_header
        
        echo "1. Wybór języka (pl/en)"
        echo "2. Instalacja zależności"
        echo "3. Git Pull & Kompilacja"
        echo "4. Wgrywanie kodu do mikrokontrolera"
        echo "5. Ustawienia i pliki konfiguracyjne"
        echo "6. Przywracanie ustawień domyślnych"
        echo "7. Czysty terminal monitor danych"
        echo "8. Przywrócenie ustawień fabrycznych"
        echo "9. Inne opcje"
        echo "0. Wyjście"
        echo ""
        
        read -p "Wybierz opcję [0-9]: " choice

        case $choice in
            1) option_1_language ;;
            2) option_2_deps ;;
            3) option_3_menu ;;
            4) option_4_menu ;;
            5) option_5_config ;;
            6) option_6_defaults ;;
            7) option_7_monitor ;;
            8) option_8_factory ;;
            9) option_9_extra ;;
            0) 
                clear
                echo "Do widzenia!"
                exit 0
                ;;
            *) 
                print_error "Nieprawidłowy wybór. Spróbuj ponownie."
                sleep 1
                ;;
        esac
    done
}

# Uruchomienie skryptu
main_menu
