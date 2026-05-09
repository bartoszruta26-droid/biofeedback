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
    while true; do
        clear
        echo -e "${CYAN}--- Wybór Języka / Language Selection ---${NC}"
        echo ""
        echo "1. Polski (PL)"
        echo "2. English (EN)"
        echo "0. Powrót"
        echo ""
        echo -e "Aktualny język / Current language: ${GREEN}$LANG${NC}"
        echo ""
        
        read -p "Wybierz opcję / Choose option: " sub_choice
        
        case $sub_choice in
            1)
                LANG="pl"
                print_success "Wybrano język polski."
                wait_for_key
                ;;
            2)
                LANG="en"
                print_success "English language selected."
                wait_for_key
                ;;
            0) return ;;
            *) print_error "Nieprawidłowa opcja / Invalid option"; sleep 1 ;;
        esac
    done
}

option_2_deps() {
    option_install_dependencies
}

# ------------------------------------------------------------------------------
# Implementacja Opcji 5: Ustawienia i Pliki Konfiguracyjne
# ------------------------------------------------------------------------------

CONFIG_FILE="config/config.json"
CONFIG_BACKUP_DIR="config/backup"

# Funkcja do tworzenia kopii zapasowej konfiguracji
backup_config() {
    if [ ! -f "$CONFIG_FILE" ]; then
        print_warning "Plik konfiguracyjny nie istnieje."
        return 1
    fi
    
    mkdir -p "$CONFIG_BACKUP_DIR"
    local timestamp=$(date +"%Y%m%d_%H%M%S")
    cp "$CONFIG_FILE" "$CONFIG_BACKUP_DIR/config_${timestamp}.json"
    print_success "Utworzono kopię zapasową: config_${timestamp}.json"
    return 0
}

# Funkcja do wyświetlania obecnej konfiguracji w czytelnej formie
show_current_config() {
    if [ ! -f "$CONFIG_FILE" ]; then
        print_warning "Plik konfiguracyjny nie istnieje."
        return 1
    fi
    
    echo -e "${CYAN}=== Obecna konfiguracja ===${NC}"
    echo ""
    
    # Sprawdź czy jq jest dostępne
    if command -v jq &> /dev/null; then
        jq '.' "$CONFIG_FILE"
    else
        # Fallback - wyświetl surowy JSON z kolorowaniem
        cat "$CONFIG_FILE" | while IFS= read -r line; do
            echo -e "${BLUE}$line${NC}"
        done
    fi
    echo ""
}

# Funkcja do zmiany wartości w config.json przy użyciu jq
update_config_value() {
    local key_path="$1"
    local new_value="$2"
    local value_type="$3" # string, number, boolean
    
    if [ ! -f "$CONFIG_FILE" ]; then
        print_error "Plik konfiguracyjny nie istnieje."
        return 1
    fi
    
    if ! command -v jq &> /dev/null; then
        print_error "jq nie jest zainstalowane. Instaluję..."
        if command -v apt &> /dev/null; then
            sudo apt install -y jq
        elif command -v dnf &> /dev/null; then
            sudo dnf install -y jq
        elif command -v pacman &> /dev/null; then
            sudo pacman -S --noconfirm jq
        else
            print_error "Nie można automatycznie zainstalować jq. Zainstaluj ręcznie."
            return 1
        fi
    fi
    
    # Tworzenie kopii zapasowej przed modyfikacją
    backup_config
    
    local typed_value
    case "$value_type" in
        string)
            typed_value="$new_value"
            ;;
        number)
            typed_value="$new_value"
            ;;
        boolean)
            if [[ "$new_value" == "true" || "$new_value" == "t" || "$new_value" == "1" ]]; then
                typed_value="true"
            else
                typed_value="false"
            fi
            ;;
        *)
            typed_value="$new_value"
            ;;
    esac
    
    # Aktualizacja pliku JSON
    if jq --argjson val "$typed_value" ".${key_path} = \$val" "$CONFIG_FILE" > "${CONFIG_FILE}.tmp" && mv "${CONFIG_FILE}.tmp" "$CONFIG_FILE"; then
        print_success "Zaktualizowano: $key_path = $typed_value"
        return 0
    else
        print_error "Błąd podczas aktualizacji konfiguracji."
        rm -f "${CONFIG_FILE}.tmp"
        return 1
    fi
}

# Funkcja do zmiany wartości w config.json bez jq (sed-based)
update_config_value_sed() {
    local key="$1"
    local new_value="$2"
    local value_type="$3"
    
    if [ ! -f "$CONFIG_FILE" ]; then
        print_error "Plik konfiguracyjny nie istnieje."
        return 1
    fi
    
    backup_config
    
    local typed_value
    case "$value_type" in
        string)
            typed_value="\"$new_value\""
            ;;
        number|boolean)
            typed_value="$new_value"
            ;;
    esac
    
    # Prosta zamiana sed (działa dla prostych przypadków)
    # Escape special characters in the replacement value for sed
    local escaped_value
    escaped_value=$(printf '%s\n' "$typed_value" | sed 's/[&/\]/\\&/g')
    # Use | as delimiter to avoid issues with / in values like /dev/ttyUSB0
    if sed -i "s|\"$key\": .*|\"$key\": $escaped_value|" "$CONFIG_FILE"; then
        print_success "Zaktualizowano: $key = $typed_value"
        return 0
    else
        print_error "Błąd podczas aktualizacji konfiguracji."
        return 1
    fi
}

# Podmenu: Konfiguracja Portu Szeregowego
config_serial_menu() {
    while true; do
        clear
        echo -e "${CYAN}--- Konfiguracja Portu Szeregowego ---${NC}"
        echo ""
        
        if [ -f "$CONFIG_FILE" ]; then
            if command -v jq &> /dev/null; then
                echo "Port:     $(jq -r '.serial.port' "$CONFIG_FILE")"
                echo "BaudRate: $(jq -r '.serial.baud_rate' "$CONFIG_FILE")"
                echo "Timeout:  $(jq -r '.serial.timeout_ms' "$CONFIG_FILE") ms"
                echo "Retries:  $(jq -r '.serial.retry_count' "$CONFIG_FILE")"
            else
                grep -E '"port"|"baud_rate"|"timeout_ms"|"retry_count"' "$CONFIG_FILE" | head -4
            fi
        else
            echo -e "${YELLOW}Brak pliku konfiguracyjnego${NC}"
        fi
        
        echo ""
        echo "1. Zmień port szeregowy"
        echo "2. Zmień baud rate"
        echo "3. Zmień timeout (ms)"
        echo "4. Zmień liczbę retry"
        echo "0. Powrót"
        echo ""
        
        read -p "Wybierz opcję: " sub_choice
        
        case $sub_choice in
            1)
                read -p "Podaj nowy port (np. /dev/ttyUSB0): " new_val
                if [ -n "$new_val" ]; then
                    update_config_value "serial.port" "\"$new_val\"" string || \
                    update_config_value_sed "port" "$new_val" string
                fi
                wait_for_key
                ;;
            2)
                read -p "Podaj baud rate (np. 115200): " new_val
                if [[ "$new_val" =~ ^[0-9]+$ ]]; then
                    update_config_value "serial.baud_rate" "$new_val" number || \
                    update_config_value_sed "baud_rate" "$new_val" number
                else
                    print_error "Nieprawidłowa wartość!"
                    sleep 1
                fi
                wait_for_key
                ;;
            3)
                read -p "Podaj timeout w ms (np. 1000): " new_val
                if [[ "$new_val" =~ ^[0-9]+$ ]]; then
                    update_config_value "serial.timeout_ms" "$new_val" number || \
                    update_config_value_sed "timeout_ms" "$new_val" number
                else
                    print_error "Nieprawidłowa wartość!"
                    sleep 1
                fi
                wait_for_key
                ;;
            4)
                read -p "Podaj liczbę retry (np. 3): " new_val
                if [[ "$new_val" =~ ^[0-9]+$ ]]; then
                    update_config_value "serial.retry_count" "$new_val" number || \
                    update_config_value_sed "retry_count" "$new_val" number
                else
                    print_error "Nieprawidłowa wartość!"
                    sleep 1
                fi
                wait_for_key
                ;;
            0) return ;;
            *) print_error "Nieprawidłowa opcja"; sleep 1 ;;
        esac
    done
}

# Podmenu: Konfiguracja UI
config_ui_menu() {
    while true; do
        clear
        echo -e "${CYAN}--- Konfiguracja Interfejsu Użytkownika (UI) ---${NC}"
        echo ""
        
        if [ -f "$CONFIG_FILE" ]; then
            if command -v jq &> /dev/null; then
                echo "Theme:       $(jq -r '.ui.theme' "$CONFIG_FILE")"
                echo "Language:    $(jq -r '.ui.language' "$CONFIG_FILE")"
                echo "Font Size:   $(jq -r '.ui.font_size' "$CONFIG_FILE")"
                echo "Update Int.: $(jq -r '.ui.graph_update_interval_ms' "$CONFIG_FILE") ms"
                echo "Samples:     $(jq -r '.ui.graph_samples_visible' "$CONFIG_FILE")"
            else
                grep -E '"theme"|"language"|"font_size"|"graph_update_interval_ms"|"graph_samples_visible"' "$CONFIG_FILE" | head -5
            fi
        else
            echo -e "${YELLOW}Brak pliku konfiguracyjnego${NC}"
        fi
        
        echo ""
        echo "1. Zmień motyw (theme)"
        echo "2. Zmień język (pl/en)"
        echo "3. Zmień rozmiar czcionki"
        echo "4. Zmień interwał odświeżania wykresu (ms)"
        echo "5. Zmień liczbę widocznych próbek na wykresie"
        echo "0. Powrót"
        echo ""
        
        read -p "Wybierz opcję: " sub_choice
        
        case $sub_choice in
            1)
                echo "Dostępne motywy: default, dark, light"
                read -p "Podaj nazwę motywu: " new_val
                if [ -n "$new_val" ]; then
                    update_config_value "ui.theme" "\"$new_val\"" string || \
                    update_config_value_sed "theme" "$new_val" string
                fi
                wait_for_key
                ;;
            2)
                echo "Dostępne języki: pl, en"
                read -p "Podaj kod języka (pl/en): " new_val
                if [[ "$new_val" == "pl" || "$new_val" == "en" ]]; then
                    update_config_value "ui.language" "\"$new_val\"" string || \
                    update_config_value_sed "language" "$new_val" string
                    LANG="$new_val"
                    print_success "Język zmieniony na: $new_val"
                else
                    print_error "Nieprawidłowy kod języka!"
                    sleep 1
                fi
                wait_for_key
                ;;
            3)
                read -p "Podaj rozmiar czcionki (np. 12): " new_val
                if [[ "$new_val" =~ ^[0-9]+$ ]]; then
                    update_config_value "ui.font_size" "$new_val" number || \
                    update_config_value_sed "font_size" "$new_val" number
                else
                    print_error "Nieprawidłowa wartość!"
                    sleep 1
                fi
                wait_for_key
                ;;
            4)
                read -p "Podaj interwał w ms (np. 100): " new_val
                if [[ "$new_val" =~ ^[0-9]+$ ]]; then
                    update_config_value "ui.graph_update_interval_ms" "$new_val" number || \
                    update_config_value_sed "graph_update_interval_ms" "$new_val" number
                else
                    print_error "Nieprawidłowa wartość!"
                    sleep 1
                fi
                wait_for_key
                ;;
            5)
                read -p "Podaj liczbę próbek (np. 300): " new_val
                if [[ "$new_val" =~ ^[0-9]+$ ]]; then
                    update_config_value "ui.graph_samples_visible" "$new_val" number || \
                    update_config_value_sed "graph_samples_visible" "$new_val" number
                else
                    print_error "Nieprawidłowa wartość!"
                    sleep 1
                fi
                wait_for_key
                ;;
            0) return ;;
            *) print_error "Nieprawidłowa opcja"; sleep 1 ;;
        esac
    done
}

# Podmenu: Konfiguracja Sensora
config_sensor_menu() {
    while true; do
        clear
        echo -e "${CYAN}--- Konfiguracja Sensora (HX711) ---${NC}"
        echo ""
        
        if [ -f "$CONFIG_FILE" ]; then
            if command -v jq &> /dev/null; then
                echo "Type:              $(jq -r '.sensor.type' "$CONFIG_FILE")"
                echo "Sample Rate (Hz):  $(jq -r '.sensor.sample_rate_hz' "$CONFIG_FILE")"
                echo "Gain:              $(jq -r '.sensor.gain' "$CONFIG_FILE")"
                echo "Calibration:       $(jq -r '.sensor.calibration_enabled' "$CONFIG_FILE")"
                echo "Zero Threshold:    $(jq -r '.sensor.zero_threshold' "$CONFIG_FILE")"
            else
                grep -E '"type"|"sample_rate_hz"|"gain"|"calibration_enabled"|"zero_threshold"' "$CONFIG_FILE" | head -5
            fi
        else
            echo -e "${YELLOW}Brak pliku konfiguracyjnego${NC}"
        fi
        
        echo ""
        echo "1. Zmień typ sensora"
        echo "2. Zmień sample rate (Hz)"
        echo "3. Zmień gain (wzmocnienie)"
        echo "4. Włącz/Wyłącz kalibrację"
        echo "5. Zmień zero threshold"
        echo "0. Powrót"
        echo ""
        
        read -p "Wybierz opcję: " sub_choice
        
        case $sub_choice in
            1)
                read -p "Podaj typ sensora (np. HX711): " new_val
                if [ -n "$new_val" ]; then
                    update_config_value "sensor.type" "\"$new_val\"" string || \
                    update_config_value_sed "type" "$new_val" string
                fi
                wait_for_key
                ;;
            2)
                echo "Dostępne wartości: 10, 80"
                read -p "Podaj sample rate (Hz): " new_val
                if [[ "$new_val" =~ ^[0-9]+$ ]]; then
                    update_config_value "sensor.sample_rate_hz" "$new_val" number || \
                    update_config_value_sed "sample_rate_hz" "$new_val" number
                else
                    print_error "Nieprawidłowa wartość!"
                    sleep 1
                fi
                wait_for_key
                ;;
            3)
                echo "Dostępne wartości: 32, 64, 128"
                read -p "Podaj gain: " new_val
                if [[ "$new_val" =~ ^[0-9]+$ ]]; then
                    update_config_value "sensor.gain" "$new_val" number || \
                    update_config_value_sed "gain" "$new_val" number
                else
                    print_error "Nieprawidłowa wartość!"
                    sleep 1
                fi
                wait_for_key
                ;;
            4)
                if command -v jq &> /dev/null; then
                    current=$(jq -r '.sensor.calibration_enabled' "$CONFIG_FILE")
                else
                    current=$(grep '"calibration_enabled"' "$CONFIG_FILE" | grep -o 'true\|false')
                fi
                
                if [[ "$current" == "true" ]]; then
                    new_val="false"
                else
                    new_val="true"
                fi
                
                update_config_value "sensor.calibration_enabled" "$new_val" boolean || \
                update_config_value_sed "calibration_enabled" "$new_val" boolean
                wait_for_key
                ;;
            5)
                read -p "Podaj zero threshold (np. 0.5): " new_val
                if [[ "$new_val" =~ ^[0-9]+\.?[0-9]*$ ]]; then
                    update_config_value "sensor.zero_threshold" "$new_val" number || \
                    update_config_value_sed "zero_threshold" "$new_val" number
                else
                    print_error "Nieprawidłowa wartość!"
                    sleep 1
                fi
                wait_for_key
                ;;
            0) return ;;
            *) print_error "Nieprawidłowa opcja"; sleep 1 ;;
        esac
    done
}

# Podmenu: Konfiguracja Logowania
config_logging_menu() {
    while true; do
        clear
        echo -e "${CYAN}--- Konfiguracja Logowania ---${NC}"
        echo ""
        
        if [ -f "$CONFIG_FILE" ]; then
            if command -v jq &> /dev/null; then
                echo "Level:         $(jq -r '.logging.level' "$CONFIG_FILE")"
                echo "File:          $(jq -r '.logging.file' "$CONFIG_FILE")"
                echo "Max Size (MB): $(jq -r '.logging.max_size_mb' "$CONFIG_FILE")"
                echo "Rotate Count:  $(jq -r '.logging.rotate_count' "$CONFIG_FILE")"
                echo "Console Out:   $(jq -r '.logging.console_output' "$CONFIG_FILE")"
            else
                grep -E '"level"|"file"|"max_size_mb"|"rotate_count"|"console_output"' "$CONFIG_FILE" | head -5
            fi
        else
            echo -e "${YELLOW}Brak pliku konfiguracyjnego${NC}"
        fi
        
        echo ""
        echo "1. Zmień poziom logowania (DEBUG/INFO/WARNING/ERROR)"
        echo "2. Zmień ścieżkę do pliku logów"
        echo "3. Zmień maksymalny rozmiar pliku (MB)"
        echo "4. Zmień liczbę rotacji plików"
        echo "5. Włącz/Wyłącz output na konsolę"
        echo "0. Powrót"
        echo ""
        
        read -p "Wybierz opcję: " sub_choice
        
        case $sub_choice in
            1)
                echo "Dostępne poziomy: DEBUG, INFO, WARNING, ERROR"
                read -p "Podaj poziom logowania: " new_val
                new_val=$(echo "$new_val" | tr '[:lower:]' '[:upper:]')
                if [[ "$new_val" == "DEBUG" || "$new_val" == "INFO" || "$new_val" == "WARNING" || "$new_val" == "ERROR" ]]; then
                    update_config_value "logging.level" "\"$new_val\"" string || \
                    update_config_value_sed "level" "$new_val" string
                else
                    print_error "Nieprawidłowy poziom logowania!"
                    sleep 1
                fi
                wait_for_key
                ;;
            2)
                read -p "Podaj ścieżkę do pliku logów: " new_val
                if [ -n "$new_val" ]; then
                    update_config_value "logging.file" "\"$new_val\"" string || \
                    update_config_value_sed "file" "$new_val" string
                fi
                wait_for_key
                ;;
            3)
                read -p "Podaj maksymalny rozmiar w MB (np. 10): " new_val
                if [[ "$new_val" =~ ^[0-9]+$ ]]; then
                    update_config_value "logging.max_size_mb" "$new_val" number || \
                    update_config_value_sed "max_size_mb" "$new_val" number
                else
                    print_error "Nieprawidłowa wartość!"
                    sleep 1
                fi
                wait_for_key
                ;;
            4)
                read -p "Podaj liczbę rotacji (np. 5): " new_val
                if [[ "$new_val" =~ ^[0-9]+$ ]]; then
                    update_config_value "logging.rotate_count" "$new_val" number || \
                    update_config_value_sed "rotate_count" "$new_val" number
                else
                    print_error "Nieprawidłowa wartość!"
                    sleep 1
                fi
                wait_for_key
                ;;
            5)
                if command -v jq &> /dev/null; then
                    current=$(jq -r '.logging.console_output' "$CONFIG_FILE")
                else
                    current=$(grep '"console_output"' "$CONFIG_FILE" | grep -o 'true\|false')
                fi
                
                if [[ "$current" == "true" ]]; then
                    new_val="false"
                else
                    new_val="true"
                fi
                
                update_config_value "logging.console_output" "$new_val" boolean || \
                update_config_value_sed "console_output" "$new_val" boolean
                wait_for_key
                ;;
            0) return ;;
            *) print_error "Nieprawidłowa opcja"; sleep 1 ;;
        esac
    done
}

# Podmenu: Zarządzanie Kopiami Zapasowymi
config_backup_menu() {
    while true; do
        clear
        echo -e "${CYAN}--- Zarządzanie Kopiami Zapasowymi Konfiguracji ---${NC}"
        echo ""
        
        if [ -d "$CONFIG_BACKUP_DIR" ]; then
            echo -e "${BLUE}Dostępne kopie zapasowe:${NC}"
            ls -la "$CONFIG_BACKUP_DIR"/*.json 2>/dev/null || echo "Brak kopii zapasowych"
        else
            echo "Brak katalogu z kopiami zapasowymi."
        fi
        
        echo ""
        echo "1. Utwórz kopię zapasową TERAZ"
        echo "2. Przywróć z ostatniej kopii"
        echo "3. Przywróć z wybranej kopii"
        echo "4. Usuń wszystkie kopie zapasowe"
        echo "0. Powrót"
        echo ""
        
        read -p "Wybierz opcję: " sub_choice
        
        case $sub_choice in
            1)
                backup_config
                wait_for_key
                ;;
            2)
                if [ -d "$CONFIG_BACKUP_DIR" ]; then
                    latest=$(ls -t "$CONFIG_BACKUP_DIR"/*.json 2>/dev/null | head -n 1)
                    if [ -n "$latest" ]; then
                        print_info "Przywracanie z: $latest"
                        cp "$latest" "$CONFIG_FILE"
                        print_success "Przywrócono konfigurację!"
                    else
                        print_warning "Brak kopii zapasowych do przywrócenia."
                    fi
                else
                    print_warning "Brak katalogu z kopiami zapasowymi."
                fi
                wait_for_key
                ;;
            3)
                if [ -d "$CONFIG_BACKUP_DIR" ]; then
                    echo "Dostępne kopie:"
                    select backup_file in "$CONFIG_BACKUP_DIR"/*.json; do
                        if [ -n "$backup_file" ] && [ -f "$backup_file" ]; then
                            print_info "Przywracanie z: $backup_file"
                            cp "$backup_file" "$CONFIG_FILE"
                            print_success "Przywrócono konfigurację!"
                            break
                        else
                            print_error "Nieprawidłowy wybór."
                        fi
                    done
                else
                    print_warning "Brak katalogu z kopiami zapasowymi."
                fi
                wait_for_key
                ;;
            4)
                read -p "Czy na pewno usunąć wszystkie kopie zapasowe? (t/n): " confirm
                if [[ "$confirm" == "t" || "$confirm" == "T" ]]; then
                    rm -rf "$CONFIG_BACKUP_DIR"
                    print_success "Usunięto wszystkie kopie zapasowe."
                fi
                wait_for_key
                ;;
            0) return ;;
            *) print_error "Nieprawidłowa opcja"; sleep 1 ;;
        esac
    done
}

# Główna funkcja Opcji 5
option_5_config() {
    while true; do
        clear
        echo -e "${CYAN}--- Opcja 5: Ustawienia i Pliki Konfiguracyjne ---${NC}"
        echo ""
        echo "1. Wyświetl aktualną konfigurację"
        echo "2. Edytuj plik konfiguracyjny (nano)"
        echo "3. Edytuj plik konfiguracyjny (vim)"
        echo "4. Zmień parametry połączenia szeregowego"
        echo "5. Zmień ścieżkę repozytorium"
        echo "0. Powrót do menu głównego"
        echo ""
        
        read -p "Wybierz opcję: " sub_choice
        
        case $sub_choice in
            1)
                print_info "Aktualna konfiguracja:"
                echo "  REPO_URL: $REPO_URL"
                echo "  REPO_DIR: $REPO_DIR"
                echo "  BUILD_DIR: $BUILD_DIR"
                echo "  LANG: $LANG"
                
                if [ -f "$REPO_DIR/config/default.conf" ]; then
                    echo ""
                    echo -e "${BLUE}Zawartość pliku konfiguracyjnego:${NC}"
                    cat "$REPO_DIR/config/default.conf"
                fi
                wait_for_key
                ;;
            2)
                if command -v nano &> /dev/null; then
                    if [ -f "$REPO_DIR/config/default.conf" ]; then
                        nano "$REPO_DIR/config/default.conf"
                    else
                        print_warning "Plik konfiguracyjny nie istnieje. Tworzenie..."
                        mkdir -p "$REPO_DIR/config"
                        touch "$REPO_DIR/config/default.conf"
                        nano "$REPO_DIR/config/default.conf"
                    fi
                else
                    print_error "Brak nano. Zainstaluj: sudo apt install nano"
                fi
                wait_for_key
                ;;
            3)
                if command -v vim &> /dev/null; then
                    if [ -f "$REPO_DIR/config/default.conf" ]; then
                        vim "$REPO_DIR/config/default.conf"
                    else
                        print_warning "Plik konfiguracyjny nie istnieje. Tworzenie..."
                        mkdir -p "$REPO_DIR/config"
                        touch "$REPO_DIR/config/default.conf"
                        vim "$REPO_DIR/config/default.conf"
                    fi
                else
                    print_error "Brak vim. Zainstaluj: sudo apt install vim"
                fi
                wait_for_key
                ;;
            4)
                read -p "Podaj nowy port szeregowy (np. /dev/ttyUSB0): " new_port
                read -p "Podaj nową szybkość baud (np. 115200): " new_baud
                SERIAL_PORT="$new_port"
                BAUD_RATE="$new_baud"
                print_success "Zmieniono ustawienia połączenia."
                echo "  Port: $SERIAL_PORT"
                echo "  Baud: $BAUD_RATE"
                wait_for_key
                ;;
            5)
                read -p "Podaj nową ścieżkę repozytorium: " new_repo
                REPO_DIR="$new_repo"
                print_success "Zmieniono ścieżkę repozytorium na: $REPO_DIR"
                wait_for_key
                ;;
            0) return ;;
            *) print_error "Nieprawidłowa opcja"; sleep 1 ;;
        esac
    done
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

# ------------------------------------------------------------------------------
# Opcja 7: Czysty Terminal Monitor Danych (SZCZEGÓŁOWA IMPLEMENTACJA)
# ------------------------------------------------------------------------------

# Funkcja do wykrywania dostępnych portów dla monitora
detect_ports_for_monitor() {
    local ports=()
    
    # Sprawdź porty ttyUSB i ttyACM
    for port in /dev/ttyUSB* /dev/ttyACM*; do
        if [ -e "$port" ]; then
            ports+=("$port")
        fi
    done
    
    # Sprawdź też porty ttyS (szeregowe sprzętowe)
    for port in /dev/ttyS*; do
        if [ -e "$port" ]; then
            # Pomiń ttyS0 jeśli to konsola systemowa
            if [ "$port" != "/dev/ttyS0" ] || ! mount | grep -q "on / "; then
                ports+=("$port")
            fi
        fi
    done
    
    echo "${ports[@]}"
}

# Funkcja czytająca dane z portu szeregowego i wyświetlająca je w czasie rzeczywistym
serial_monitor_clean() {
    local port="$1"
    local baud="${2:-115200}"
    
    if [ ! -e "$port" ]; then
        print_error "Port $port nie istnieje!"
        return 1
    fi
    
    # Sprawdź uprawnienia
    if [ ! -r "$port" ]; then
        print_error "Brak uprawnień do odczytu portu $port"
        print_info "Spróbuj: sudo usermod -a -G dialout \$USER"
        print_info "Lub uruchom skrypt z sudo"
        return 1
    fi
    
    clear
    echo -e "${CYAN}==============================================================================${NC}"
    echo -e "${CYAN}          CZYSTY MONITOR DANYCH SZEREGOWYCH                                   ${NC}"
    echo -e "${CYAN}==============================================================================${NC}"
    echo ""
    echo -e "${BLUE}Port:${NC} $port"
    echo -e "${BLUE}Szybkość:${NC} $baud baud"
    echo -e "${BLUE}Czas startu:${NC} $(date '+%Y-%m-%d %H:%M:%S')"
    echo ""
    echo -e "${YELLOW}Naciśnij Ctrl+C aby wyjść${NC}"
    echo -e "${YELLOW}Naciśnij Ctrl+\\ aby wysłać sygnał quit${NC}"
    echo ""
    echo -e "${GREEN}--- STRUMIEŃ DANYCH ---${NC}"
    echo ""
    
    # Używamy cat do czytania danych z portu
    # stty konfiguruje port przed otwarciem
    stty -F "$port" "$baud" raw -echo -icanon isig 2>/dev/null
    
    # Czytaj dane z portu
    cat "$port" 2>/dev/null
}

# Funkcja monitora z filtrowaniem i statystykami
serial_monitor_with_stats() {
    local port="$1"
    local baud="${2:-115200}"
    
    if [ ! -e "$port" ]; then
        print_error "Port $port nie istnieje!"
        return 1
    fi
    
    local line_count=0
    local byte_count=0
    local start_time=$(date +%s)
    
    clear
    echo -e "${CYAN}==============================================================================${NC}"
    echo -e "${CYAN}          MONITOR DANYCH ZE STATYSTYKAMI                                      ${NC}"
    echo -e "${CYAN}==============================================================================${NC}"
    echo ""
    echo -e "${BLUE}Port:${NC} $port"
    echo -e "${BLUE}Szybkość:${NC} $baud baud"
    echo -e "${BLUE}Start:${NC} $(date '+%Y-%m-%d %H:%M:%S')"
    echo ""
    echo -e "${YELLOW}Ctrl+C aby wyjść${NC}"
    echo ""
    
    # Konfiguracja portu
    stty -F "$port" "$baud" raw -echo -icanon isig 2>/dev/null
    
    # Czytaj i przetwarzaj dane
    while IFS= read -r line; do
        ((line_count++))
        byte_count=$((byte_count + ${#line}))
        
        local current_time=$(date +%s)
        local elapsed=$((current_time - start_time))
        local rate=0
        if [ $elapsed -gt 0 ]; then
            rate=$((line_count / elapsed))
        fi
        
        # Wyświetl linię z numerem
        echo -e "${GREEN}[$line_count]${NC} $line"
        
        # Co 100 linii pokaż statystyki
        if [ $((line_count % 100)) -eq 0 ]; then
            echo ""
            echo -e "${CYAN}--- Statystyki ---${NC}"
            echo "  Linie: $line_count"
            echo "  Bajty: ~$byte_count"
            echo "  Czas: ${elapsed}s"
            echo "  Śr. linie/s: $rate"
            echo ""
        fi
    done < "$port" 2>/dev/null
}

# Funkcja monitora z wykresem ASCII (dla danych numerycznych)
serial_monitor_ascii_graph() {
    local port="$1"
    local baud="${2:-115200}"
    local graph_width=60
    local max_value=100
    local min_value=0
    
    if [ ! -e "$port" ]; then
        print_error "Port $port nie istnieje!"
        return 1
    fi
    
    clear
    echo -e "${CYAN}==============================================================================${NC}"
    echo -e "${CYAN}          MONITOR Z WYKRESEM ASCII                                            ${NC}"
    echo -e "${CYAN}==============================================================================${NC}"
    echo ""
    echo -e "${BLUE}Port:${NC} $port"
    echo -e "${BLUE}Szybkość:${NC} $baud baud"
    echo -e "${BLUE}Zakres:${NC} $min_value - $max_value"
    echo ""
    echo -e "${YELLOW}Ctrl+C aby wyjść${NC}"
    echo ""
    
    # Konfiguracja portu
    stty -F "$port" "$baud" 2>/dev/null
    
    # Czytaj dane i rysuj wykres
    while IFS= read -r line; do
        # Spróbuj wyodrębnić liczbę z linii
        local number=$(echo "$line" | grep -oE '[-]?[0-9]+\.?[0-9]*' | head -n 1)
        
        if [ -n "$number" ]; then
            # Oblicz długość paska
            local range=$((max_value - min_value))
            if [ $range -eq 0 ]; then range=1; fi
            
            # Skaluj wartość do zakresu 0-graph_width
            local scaled=$(( (number - min_value) * graph_width / range ))
            
            # Ogranicz do zakresu
            if [ $scaled -lt 0 ]; then scaled=0; fi
            if [ $scaled -gt $graph_width ]; then scaled=$graph_width; fi
            
            # Narysuj pasek
            local bar=""
            for ((i=0; i<scaled; i++)); do
                bar="${bar}#"
            done
            
            # Wyświetl wartość i pasek
            printf "\r${GREEN}%8.2f${NC} |%-${graph_width}s| " "$number" "$bar"
            echo ""
        else
            # Jeśli nie liczba, wyświetl surowe dane
            echo -e "${YELLOW}RAW:${NC} $line"
        fi
    done < "$port" 2>/dev/null
}

# Główna funkcja opcji 7
option_7_monitor() {
    local selected_port=""
    local selected_baud="115200"
    
    while true; do
        clear
        echo -e "${CYAN}==============================================================================${NC}"
        echo -e "${CYAN}          OPCJA 7: CZYSTY TERMINAL MONITOR DANYCH                             ${NC}"
        echo -e "${CYAN}==============================================================================${NC}"
        echo ""
        echo "1. Wykryj dostępne porty szeregowe"
        echo "2. Wybierz port ręcznie"
        echo "3. Ustaw szybkość transmisji (baud rate)"
        echo "4. Uruchom czysty monitor (surowe dane)"
        echo "5. Uruchom monitor ze statystykami"
        echo "6. Uruchom monitor z wykresem ASCII"
        echo "7. Test połączenia (krótki odczyt)"
        echo "8. Zapisz dane do pliku"
        echo "0. Powrót do menu głównego"
        echo ""
        
        if [ -n "$selected_port" ]; then
            echo -e "${GREEN}Aktywny port: $selected_port${NC}"
            echo -e "${GREEN}Baud rate: $selected_baud${NC}"
        else
            echo -e "${YELLOW}Aktywny port: NIEWYBRANY${NC}"
            echo -e "${YELLOW}Baud rate: $selected_baud${NC}"
        fi
        echo ""
        
        read -p "Wybierz opcję: " sub_choice
        
        case $sub_choice in
            1)
                clear
                echo -e "${CYAN}--- Wykrywanie Portów Szeregowych ---${NC}"
                echo ""
                
                # Sprawdź lsusb
                if command -v lsusb &> /dev/null; then
                    echo -e "${BLUE}Urządzenia USB:${NC}"
                    lsusb 2>/dev/null | grep -iE "serial|ftdi|silicon|cp210|ch34|esp32|stm32|arduino" || echo "  Brak typowych konwerterów USB-Serial"
                    echo ""
                fi
                
                # Sprawdź porty
                echo -e "${BLUE}Dostępne porty szeregowe:${NC}"
                local found=0
                
                for port in /dev/ttyUSB* /dev/ttyACM* /dev/ttyS*; do
                    if [ -e "$port" ]; then
                        # Pomiń ttyS0 jeśli to konsola
                        if [ "$port" = "/dev/ttyS0" ]; then
                            if mount | grep -q "on / "; then
                                continue
                            fi
                        fi
                        
                        echo "  - $port"
                        found=1
                        
                        # Spróbuj uzyskać informacje
                        if command -v setserial &> /dev/null && [[ "$port" == /dev/ttyS* ]]; then
                            setserial "$port" 2>/dev/null | head -n 1
                        fi
                    fi
                done
                
                if [ $found -eq 0 ]; then
                    echo "  Brak wykrytych portów szeregowych."
                    echo ""
                    print_info "Upewnij się, że:"
                    echo "  - Mikrokontroler jest podłączony przez USB"
                    echo "  - Sterowniki są zainstalowane (CP210x, CH340, FTDI itp.)"
                    echo "  - Masz uprawnienia do portów (grupa dialout)"
                fi
                
                echo ""
                
                # Propozycja automatycznego wyboru
                if [ $found -gt 0 ]; then
                    read -p "Czy ustawić pierwszy wykryty port jako aktywny? (t/n): " confirm
                    if [[ "$confirm" == "t" || "$confirm" == "T" ]]; then
                        for port in /dev/ttyUSB* /dev/ttyACM*; do
                            if [ -e "$port" ]; then
                                selected_port="$port"
                                print_success "Wybrano port: $selected_port"
                                break
                            fi
                        done
                    fi
                fi
                
                wait_for_key
                ;;
                
            2)
                read -p "Podaj ścieżkę do portu (np. /dev/ttyUSB0): " selected_port
                
                if [ -e "$selected_port" ]; then
                    print_success "Port $selected_port istnieje."
                    
                    # Sprawdź uprawnienia
                    if [ -r "$selected_port" ]; then
                        echo -e "${GREEN}Masz uprawnienia do odczytu.${NC}"
                    else
                        echo -e "${RED}Brak uprawnień do odczytu!${NC}"
                        print_info "Spróbuj: sudo usermod -a -G dialout \$USER"
                        print_info "Potem wyloguj się i zaloguj ponownie."
                    fi
                else
                    print_warning "Port $selected_port nie istnieje w systemie!"
                fi
                wait_for_key
                ;;
                
            3)
                echo "Dostępne szybkości transmisji:"
                echo "  1. 9600"
                echo "  2. 19200"
                echo "  3. 38400"
                echo "  4. 57600"
                echo "  5. 115200 (domyślna)"
                echo "  6. 230400"
                echo "  7. 460800"
                echo "  8. 921600"
                echo "  9. Własna wartość"
                echo ""
                
                read -p "Wybierz szybkość: " baud_choice
                
                case $baud_choice in
                    1) selected_baud="9600" ;;
                    2) selected_baud="19200" ;;
                    3) selected_baud="38400" ;;
                    4) selected_baud="57600" ;;
                    5) selected_baud="115200" ;;
                    6) selected_baud="230400" ;;
                    7) selected_baud="460800" ;;
                    8) selected_baud="921600" ;;
                    9) 
                        read -p "Podaj własną wartość baud: " selected_baud
                        ;;
                    *) print_error "Nieprawidłowy wybór"; sleep 1; continue ;;
                esac
                
                print_success "Ustawiono baud rate: $selected_baud"
                wait_for_key
                ;;
                
            4)
                if [ -z "$selected_port" ]; then
                    print_error "Najpierw wybierz port (opcja 1 lub 2)!"
                    wait_for_key
                    continue
                fi
                
                print_info "Uruchamianie czystego monitora..."
                print_info "Naciśnij Ctrl+C aby wyjść."
                sleep 2
                
                # Uruchom monitor w tle z obsługą sygnałów
                serial_monitor_clean "$selected_port" "$selected_baud"
                ;;
                
            5)
                if [ -z "$selected_port" ]; then
                    print_error "Najpierw wybierz port (opcja 1 lub 2)!"
                    wait_for_key
                    continue
                fi
                
                print_info "Uruchamianie monitora ze statystykami..."
                print_info "Naciśnij Ctrl+C aby wyjść."
                sleep 2
                
                serial_monitor_with_stats "$selected_port" "$selected_baud"
                ;;
                
            6)
                if [ -z "$selected_port" ]; then
                    print_error "Najpierw wybierz port (opcja 1 lub 2)!"
                    wait_for_key
                    continue
                fi
                
                # Konfiguracja zakresu wykresu
                echo ""
                echo "Konfiguracja wykresu ASCII:"
                read -p "Wartość minimalna (domyślnie 0): " min_v
                read -p "Wartość maksymalna (domyślnie 100): " max_v
                
                if [ -n "$min_v" ]; then
                    # Dla uproszczenia używamy globalnych zmiennych w funkcji
                    :
                fi
                
                print_info "Uruchamianie monitora z wykresem ASCII..."
                print_info "Naciśnij Ctrl+C aby wyjść."
                sleep 2
                
                serial_monitor_ascii_graph "$selected_port" "$selected_baud"
                ;;
                
            7)
                if [ -z "$selected_port" ]; then
                    print_error "Najpierw wybierz port (opcja 1 lub 2)!"
                    wait_for_key
                    continue
                fi
                
                print_info "Testowanie połączenia z $selected_port (${selected_baud} baud)..."
                
                # Konfiguracja portu
                stty -F "$selected_port" "$selected_baud" 2>/dev/null
                
                # Przeczytaj krótki fragment danych (timeout 3 sekundy)
                echo -e "${BLUE}Odczyt danych (3 sekundy timeout):${NC}"
                echo ""
                
                timeout 3 cat "$selected_port" 2>/dev/null
                
                local exit_code=$?
                echo ""
                
                if [ $exit_code -eq 0 ]; then
                    print_success "Połączenie działa poprawnie!"
                elif [ $exit_code -eq 124 ]; then
                    print_success "Połączenie działa (timeout po 3s - normalne)."
                else
                    print_warning "Możliwe problemy z połączeniem."
                fi
                
                wait_for_key
                ;;
                
            8)
                if [ -z "$selected_port" ]; then
                    print_error "Najpierw wybierz port (opcja 1 lub 2)!"
                    wait_for_key
                    continue
                fi
                
                local timestamp=$(date +%Y%m%d_%H%M%S)
                local filename="serial_data_${timestamp}.log"
                
                print_info "Zapisywanie danych do pliku: $filename"
                print_info "Naciśnij Ctrl+C aby zakończyć zapis."
                echo ""
                
                # Konfiguracja portu
                stty -F "$selected_port" "$selected_baud" 2>/dev/null
                
                # Zapisz dane do pliku
                timeout 60 cat "$selected_port" >> "$filename" 2>/dev/null &
                local pid=$!
                
                echo -e "${GREEN}Zapis w toku... (PID: $pid)${NC}"
                echo "Dane będą zapisywane przez 60 sekund lub do naciśnięcia Ctrl+C"
                echo ""
                
                wait $pid 2>/dev/null
                
                if [ -f "$filename" ]; then
                    local lines=$(wc -l < "$filename")
                    local size=$(du -h "$filename" | cut -f1)
                    print_success "Zapisano dane do $filename"
                    echo "  Linie: $lines"
                    echo "  Rozmiar: $size"
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
# Opcja 8: Przywrócenie Ustawień Fabrycznych
# ------------------------------------------------------------------------------

option_8_factory() {
    clear
    echo -e "${CYAN}==============================================================================${NC}"
    echo -e "${CYAN}          OPCJA 8: PRZYWRÓCENIE USTAWIEŃ FABRYCZNYCH                          ${NC}"
    echo -e "${CYAN}==============================================================================${NC}"
    echo ""
    
    print_warning "UWAGA! Ta operacja:"
    echo "  - Usunie wszystkie pobrane pliki źródłowe"
    echo "  - Usunie wszystkie skompilowane pliki"
    echo "  - Usunie pliki konfiguracyjne użytkownika"
    echo "  - Usunie logi i dane tymczasowe"
    echo "  - Przywróci domyślne ustawienia skryptu"
    echo ""
    
    read -p "Czy jesteś ABSOLUTNIE pewien? (wpisz 'TAK' aby potwierdzić): " confirm
    
    if [[ "$confirm" == "TAK" ]]; then
        print_info "Rozpoczynanie przywracania ustawień fabrycznych..."
        echo ""
        
        # Usuń katalog repozytorium
        if [ -d "$REPO_DIR" ]; then
            echo -e "${YELLOW}[1/5] Usuwanie katalogu źródłowego ($REPO_DIR)...${NC}"
            rm -rf "$REPO_DIR"
            echo -e "${GREEN}Usunięto.${NC}"
        else
            echo -e "${BLUE}[1/5] Katalog źródłowy nie istnieje - pominięto.${NC}"
        fi
        
        # Usuń katalog build
        if [ -d "$BUILD_DIR" ]; then
            echo -e "${YELLOW}[2/5] Usuwanie katalogu build...${NC}"
            rm -rf "$BUILD_DIR"
            echo -e "${GREEN}Usunięto.${NC}"
        else
            echo -e "${BLUE}[2/5] Katalog build nie istnieje - pominięto.${NC}"
        fi
        
        # Usuń pliki konfiguracyjne
        echo -e "${YELLOW}[3/5] Usuwanie plików konfiguracyjnych...${NC}"
        rm -f config/user.conf 2>/dev/null
        rm -f config/custom.conf 2>/dev/null
        rm -f *.conf 2>/dev/null
        echo -e "${GREEN}Usunięto.${NC}"
        
        # Usuń logi
        echo -e "${YELLOW}[4/5] Czyszczenie logów...${NC}"
        rm -f logs/*.log 2>/dev/null
        rm -f serial_data_*.log 2>/dev/null
        rm -f /tmp/detected_ports.txt 2>/dev/null
        echo -e "${GREEN}Usunięto.${NC}"
        
        # Przywróć domyślne zmienne
        echo -e "${YELLOW}[5/5] Przywracanie domyślnych ustawień...${NC}"
        REPO_URL="https://github.com/bartoszruta26-droid/biofeedback"
        REPO_DIR="biofeedback"
        BUILD_DIR="build"
        LANG="pl"
        SERIAL_PORT=""
        BAUD_RATE="115200"
        echo -e "${GREEN}Przywrócono.${NC}"
        
        echo ""
        echo -e "${GREEN}==============================================================================${NC}"
        echo -e "${GREEN}  PRZYWRÓCONO USTAWIENIA FABRYCZNE!${NC}"
        echo -e "${GREEN}==============================================================================${NC}"
        echo ""
        print_info "Skrypt jest teraz w stanie początkowym."
        print_info "Aby ponownie używać aplikację, należy:"
        echo "  1. Wybrać opcję 2 (Instalacja zależności)"
        echo "  2. Wybrać opcję 3 (Git Pull & Kompilacja)"
    else
        print_info "Operacja anulowana."
    fi
    
    wait_for_key
}

# ------------------------------------------------------------------------------
# Opcja 9: Inne Opcje (Dodatkowe Funkcje)
# ------------------------------------------------------------------------------

option_9_extra() {
    while true; do
        clear
        echo -e "${CYAN}==============================================================================${NC}"
        echo -e "${CYAN}          OPCJA 9: INNE OPCJE I NARZĘDZIA                                     ${NC}"
        echo -e "${CYAN}==============================================================================${NC}"
        echo ""
        echo "1. Informacje o systemie"
        echo "2. Sprawdź dostępne narzędzia"
        echo "3. Backup konfiguracji"
        echo "4. Przywróć backup konfiguracji"
        echo "5. Podgląd logów systemowych"
        echo "6. Test dostępności repozytorium"
        echo "7. Wyczyść terminal"
        echo "0. Powrót do menu głównego"
        echo ""
        
        read -p "Wybierz opcję: " sub_choice
        
        case $sub_choice in
            1)
                clear
                echo -e "${CYAN}--- Informacje o Systemie ---${NC}"
                echo ""
                
                echo -e "${BLUE}System operacyjny:${NC}"
                if [ -f /etc/os-release ]; then
                    cat /etc/os-release | grep -E "^PRETTY_NAME|^NAME|^VERSION"
                fi
                echo ""
                
                echo -e "${BLUE}Informacje o kernelu:${NC}"
                uname -a
                echo ""
                
                echo -e "${BLUE}Architektura:${NC}"
                dpkg --print-architecture 2>/dev/null || echo "nieznana"
                echo ""
                
                echo -e "${BLUE}Uptime:${NC}"
                uptime
                echo ""
                
                echo -e "${BLUE}Pamięć RAM:${NC}"
                free -h 2>/dev/null || cat /proc/meminfo | head -n 3
                echo ""
                
                echo -e "${BLUE}Przestrzeń dyskowa:${NC}"
                df -h . | tail -n 1
                echo ""
                
                echo -e "${BLUE}Użytkownik:${NC}"
                whoami
                echo "Grupy: $(groups)"
                echo ""
                
                wait_for_key
                ;;
                
            2)
                clear
                echo -e "${CYAN}--- Sprawdzenie Dostępnych Narzędzi ---${NC}"
                echo ""
                
                local tools=(
                    "g++"
                    "cmake"
                    "git"
                    "make"
                    "pkg-config"
                    "nano"
                    "vim"
                    "screen"
                    "minicom"
                    "esptool.py"
                    "esptool"
                    "st-flash"
                    "dfu-util"
                    "lsusb"
                    "setserial"
                )
                
                for tool in "${tools[@]}"; do
                    if command -v "$tool" &> /dev/null; then
                        local ver=""
                        case $tool in
                            g++) ver=$($tool --version | head -n1) ;;
                            cmake) ver=$($tool --version | head -n1) ;;
                            git) ver=$($tool --version) ;;
                            *) ver="zainstalowany" ;;
                        esac
                        echo -e "${GREEN}✓${NC} $tool: $ver"
                    else
                        echo -e "${RED}✗${NC} $tool: NIE ZNALEZIONO"
                    fi
                done
                
                echo ""
                echo -e "${BLUE}Sprawdzenie pakietów Qt5:${NC}"
                if pkg-config --exists Qt5Core 2>/dev/null; then
                    echo -e "${GREEN}✓${NC} Qt5Core: $(pkg-config --modversion Qt5Core)"
                else
                    echo -e "${YELLOW}!${NC} Qt5Core: nie wykryto"
                fi
                
                if pkg-config --exists Qt5Charts 2>/dev/null; then
                    echo -e "${GREEN}✓${NC} Qt5Charts: $(pkg-config --modversion Qt5Charts)"
                else
                    echo -e "${YELLOW}!${NC} Qt5Charts: nie wykryto"
                fi
                
                echo ""
                wait_for_key
                ;;
                
            3)
                clear
                echo -e "${CYAN}--- Backup Konfiguracji ---${NC}"
                echo ""
                
                local timestamp=$(date +%Y%m%d_%H%M%S)
                local backup_dir="backup_config_${timestamp}"
                
                mkdir -p "$backup_dir"
                
                print_info "Tworzenie backupu w katalogu: $backup_dir"
                echo ""
                
                # Backup plików konfiguracyjnych
                if [ -d "config" ]; then
                    echo "Kopiowanie katalogu config..."
                    cp -r config "$backup_dir/"
                fi
                
                # Backup plików .conf z głównego katalogu
                for conf in *.conf; do
                    if [ -f "$conf" ]; then
                        echo "Kopiowanie $conf..."
                        cp "$conf" "$backup_dir/"
                    fi
                done
                
                # Backup ukrytych plików konfiguracyjnych
                if [ -f ~/.biofeedback.conf ]; then
                    echo "Kopiowanie ~/.biofeedback.conf..."
                    cp ~/.biofeedback.conf "$backup_dir/"
                fi
                
                # Spakuj backup
                if [ -d "$backup_dir" ]; then
                    tar -czf "${backup_dir}.tar.gz" "$backup_dir" 2>/dev/null
                    rm -rf "$backup_dir"
                    
                    local size=$(du -h "${backup_dir}.tar.gz" | cut -f1)
                    echo ""
                    print_success "Utworzono backup: ${backup_dir}.tar.gz (rozmiar: $size)"
                else
                    print_warning "Brak plików do backupu."
                    rmdir "$backup_dir" 2>/dev/null
                fi
                
                echo ""
                wait_for_key
                ;;
                
            4)
                clear
                echo -e "${CYAN}--- Przywracanie Backupu Konfiguracji ---${NC}"
                echo ""
                
                echo "Dostępne backupy:"
                ls -la backup_config_*.tar.gz 2>/dev/null || echo "Brak znalezionych backupów."
                echo ""
                
                read -p "Podaj nazwę pliku backupu: " backup_file
                
                if [ -f "$backup_file" ]; then
                    print_info "Przywracanie z $backup_file..."
                    
                    # Rozpakuj
                    tar -xzf "$backup_file"
                    
                    # Znajdź rozpakowany katalog
                    local extract_dir=$(tar -tzf "$backup_file" | head -n1 | cut -f1 -d/)
                    
                    if [ -d "$extract_dir" ]; then
                        # Przywróć config
                        if [ -d "$extract_dir/config" ]; then
                            cp -r "$extract_dir/config/"* config/ 2>/dev/null
                            echo -e "${GREEN}Przywrócono pliki konfiguracyjne.${NC}"
                        fi
                        
                        # Przywróć pliki .conf
                        for conf in "$extract_dir"/*.conf; do
                            if [ -f "$conf" ]; then
                                cp "$conf" .
                                echo -e "${GREEN}Przywrócono $(basename $conf).${NC}"
                            fi
                        done
                        
                        # Posprzątaj
                        rm -rf "$extract_dir"
                        
                        print_success "Przywracanie zakończone!"
                    else
                        print_error "Błąd podczas przywracania."
                    fi
                else
                    print_error "Plik $backup_file nie istnieje!"
                fi
                
                echo ""
                wait_for_key
                ;;
                
            5)
                clear
                echo -e "${CYAN}--- Podgląd Logów Systemowych ---${NC}"
                echo ""
                
                echo "Wybierz typ logów:"
                echo "1. Ostatnie logi systemowe (dmesg)"
                echo "2. Logi USB (podłączenia urządzeń)"
                echo "3. Logi szeregowe"
                echo "4. Logi aplikacji biofeedback"
                echo ""
                
                read -p "Wybierz: " log_choice
                
                case $log_choice in
                    1)
                        echo -e "${BLUE}--- Ostatnie komunikaty kernela (dmesg) ---${NC}"
                        dmesg | tail -n 50
                        ;;
                    2)
                        echo -e "${BLUE}--- Logi USB ---${NC}"
                        dmesg | grep -i usb | tail -n 30
                        ;;
                    3)
                        echo -e "${BLUE}--- Logi Szeregowe ---${NC}"
                        dmesg | grep -iE "tty|serial" | tail -n 30
                        ;;
                    4)
                        echo -e "${BLUE}--- Logi Aplikacji ---${NC}"
                        if [ -d "logs" ]; then
                            ls -la logs/
                            echo ""
                            read -p "Podaj nazwę pliku logu (Enter dla ostatniego): " logfile
                            
                            if [ -z "$logfile" ]; then
                                logfile=$(ls -t logs/*.log 2>/dev/null | head -n1)
                            fi
                            
                            if [ -f "$logfile" ]; then
                                tail -n 100 "$logfile"
                            else
                                echo "Brak logów."
                            fi
                        else
                            echo "Katalog logs nie istnieje."
                        fi
                        ;;
                    *)
                        print_error "Nieprawidłowy wybór."
                        ;;
                esac
                
                echo ""
                wait_for_key
                ;;
                
            6)
                clear
                echo -e "${CYAN}--- Test Dostępności Repozytorium ---${NC}"
                echo ""
                
                print_info "Testowanie połączenia z GitHub..."
                echo ""
                
                if command -v curl &> /dev/null; then
                    echo "Sprawdzanie dostępności github.com..."
                    if curl -s --head --connect-timeout 5 https://github.com > /dev/null; then
                        print_success "GitHub jest dostępny."
                    else
                        print_error "Brak połączenia z GitHub."
                    fi
                elif command -v ping &> /dev/null; then
                    echo "Pingowanie github.com..."
                    ping -c 3 github.com
                else
                    print_warning "Brak narzędzi curl lub ping."
                fi
                
                echo ""
                print_info "Sprawdzanie lokalnego repozytorium..."
                
                if [ -d "$REPO_DIR/.git" ]; then
                    cd "$REPO_DIR" || exit
                    
                    echo "Status repozytorium:"
                    git status --short
                    
                    echo ""
                    echo "Ostatni commit:"
                    git log -1 --oneline
                    
                    echo ""
                    echo "Zdalne repozytorium:"
                    git remote -v
                    
                    cd ..
                else
                    print_warning "Lokalne repozytorium nie istnieje."
                    print_info "Użyj opcji 3 aby pobrać kod."
                fi
                
                echo ""
                wait_for_key
                ;;
                
            7)
                clear
                print_success "Terminal wyczyszczony."
                sleep 1
                return
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
