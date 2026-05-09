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

    # macOS: cu.usb*, cu.serial*
    for port in /dev/cu.usb*; do
        if [ -e "$port" ]; then
            ports+=("$port")
            echo "  - $port"
        fi
    done

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

option_6_defaults() {
    echo "Przywracanie ustawień domyślnych - W implementacji..."
    wait_for_key
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
