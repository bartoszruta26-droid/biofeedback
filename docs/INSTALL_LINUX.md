# Instalacja Biofeedback w systemie Linux

## Obsługiwane środowisko

Pakiet DEB jest przygotowywany w CI dla Ubuntu 22.04. Powinien działać także w zgodnych dystrybucjach Debian/Ubuntu z bibliotekami Qt 5 i libserialport w wersjach ABI wymaganych przez pakiet.

## Instalacja gotowego pakietu

Po pobraniu pliku DEB:

    sudo apt install ./biofeedback_1.0.0_amd64.deb

Uruchomienie aplikacji graficznej:

    biofeedback

W menu środowiska graficznego pojawi się pozycja Biofeedback. Pakiet instaluje również ikonę SVG i program terminalowy biofeedback_tui, jeżeli został zbudowany z obsługą ncurses.

## Budowanie ze źródeł

Zależności kompilacyjne:

    sudo apt update
    sudo apt install build-essential cmake pkg-config git \
      qtbase5-dev qtbase5-dev-tools libqt5charts5-dev \
      libserialport-dev nlohmann-json3-dev libssl-dev libncurses-dev

Konfiguracja, kompilacja i testy:

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build --parallel 2
    ctest --test-dir build --output-on-failure

Utworzenie pakietu DEB:

    cpack --config build/CPackConfig.cmake -G DEB -B dist

Instalacja:

    sudo apt install ./dist/biofeedback_*.deb

## Dostęp do urządzenia

Dla portów szeregowych użytkownik zwykle musi należeć do grupy dialout:

    sudo usermod -aG dialout "$USER"

Po zmianie grupy należy wylogować się i zalogować ponownie. Port można sprawdzić poleceniem:

    ls -l /dev/ttyACM* /dev/ttyUSB* 2>/dev/null

## Ważne przed pomiarem

- Wybierz właściwy port urządzenia w aplikacji.
- Sprawdź, czy firmware raportuje skalibrowaną masę równoważną w gramach.
- Wykonaj zerowanie i kalibrację na znanym obciążeniu.
- Zweryfikuj wskazania względem wzorca; konwersja do niutonów zakłada poprawną kalibrację masy.
- Testy CI nie zastępują odbioru fizycznego czujnika, kontroli liniowości, powtarzalności i dryftu.

Dane syntetyczne służą do testowania algorytmów i nie mogą być przedstawiane jako rzeczywiste wyniki pacjenta.
