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

option_6_defaults() {
    clear
    echo -e "${CYAN}--- Przywracanie Ustawień Domyślnych ---${NC}"
    echo ""
    print_warning "Ta operacja przywróci domyślne wartości konfiguracyjne."
    echo ""
    read -p "Czy na pewno? (t/n): " confirm
    
    if [[ "$confirm" == "t" || "$confirm" == "T" ]]; then
        REPO_URL="https://github.com/bartoszruta26-droid/biofeedback"
        REPO_DIR="biofeedback"
        BUILD_DIR="build"
        LANG="pl"
        SERIAL_PORT=""
        BAUD_RATE="115200"
        
        # Usuń plik konfiguracyjny użytkownika
        if [ -f "$REPO_DIR/config/user.conf" ]; then
            rm "$REPO_DIR/config/user.conf"
            print_info "Usunięto plik user.conf"
        fi
        
        print_success "Przywrócono ustawienia domyślne!"
        echo "  REPO_URL: $REPO_URL"
        echo "  REPO_DIR: $REPO_DIR"
        echo "  BUILD_DIR: $BUILD_DIR"
        echo "  LANG: $LANG"
    else
        print_info "Operacja anulowana."
    fi
    
    wait_for_key
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
