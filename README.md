# 🧠 Biofeedback — system pomiaru siły i treningu z biologicznym sprzężeniem zwrotnym

<p align="center">
  <strong>Aplikacja C++17 / Qt5 dla Raspberry Pi i systemów Linux</strong><br>
  Pomiar siły • analiza ruchu • trening • gry sterowane czujnikiem • archiwizacja danych
</p>

<p align="center">
  <a href="https://github.com/bartoszruta26-droid/biofeedback/actions"><img alt="GitHub Actions" src="https://img.shields.io/github/actions/workflow/status/bartoszruta26-droid/biofeedback/raspberry-pi-arm64-deb.yml?label=kompilacja%20Raspberry%20Pi&logo=githubactions"></a>
  <a href="https://github.com/bartoszruta26-droid/biofeedback"><img alt="Repozytorium" src="https://img.shields.io/badge/repozytorium-GitHub-181717?logo=github"></a>
  <a href="https://img.shields.io/badge/C%2B%2B-17-00599C"><img alt="C++17" src="https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus"></a>
  <a href="https://www.qt.io/"><img alt="Qt5" src="https://img.shields.io/badge/Qt-5-41CD52?logo=qt&logoColor=white"></a>
  <a href="https://www.raspberrypi.com/"><img alt="Raspberry Pi" src="https://img.shields.io/badge/Raspberry%20Pi-4%20%7C%20ARM64-C51A4A?logo=raspberrypi&logoColor=white"></a>
  <img alt="Licencja" src="https://img.shields.io/badge/licencja-nieokre%C5%9Blona-lightgrey">
</p>

> [!IMPORTANT]
> **Stan projektu należy oceniać na podstawie kodu, testów i aktualnych przebiegów CI.** Samo istnienie ekranu, klasy lub wpisu w fabryce nie oznacza zakończonej walidacji sprzętowej ani klinicznej. Projekt nie zastępuje certyfikowanego wyrobu medycznego.

---

## 🧭 Spis treści

- [🎯 O projekcie](#-o-projekcie)
- [✨ Najważniejsze możliwości](#-najważniejsze-możliwości)
- [🎮 Gry biofeedbackowe — pełna lista](#-gry-biofeedbackowe--pełna-lista)
- [🖥️ Interfejsy użytkownika](#️-interfejsy-użytkownika)
- [🔌 Sprzęt i przepływ pomiaru](#-sprzęt-i-przepływ-pomiaru)
- [📈 Analiza pomiarów i treningu](#-analiza-pomiarów-i-treningu)
- [🔐 Dane, bezpieczeństwo i prywatność](#-dane-bezpieczeństwo-i-prywatność)
- [⚙️ Wymagania](#️-wymagania)
- [🚀 Szybkie uruchomienie](#-szybkie-uruchomienie)
- [🍓 Raspberry Pi — instalacja i pakiet DEB](#-raspberry-pi--instalacja-i-pakiet-deb)
- [🧰 Budowanie ze źródeł](#-budowanie-ze-źródeł)
- [🧪 Testowanie](#-testowanie)
- [🗂️ Struktura repozytorium](#️-struktura-repozytorium)
- [📚 Dokumentacja](#-dokumentacja)
- [⚠️ Ograniczenia i bezpieczeństwo użytkowania](#️-ograniczenia-i-bezpieczeństwo-użytkowania)
- [🤝 Współpraca](#-współpraca)

## 🎯 O projekcie

Biofeedback to rozwijana aplikacja do rejestracji sygnału z czujnika siły, wizualizacji pomiaru w czasie rzeczywistym oraz analizy powtórzeń i serii treningowych. Główna aplikacja komputerowa jest napisana w C++17 z wykorzystaniem Qt5. Typowa konfiguracja sprzętowa obejmuje Raspberry Pi, Arduino Nano i przetwornik tensometryczny HX711.

Dane z czujnika przechodzą przez warstwę komunikacji szeregowej, walidację ramek i silnik pomiarowy. Następnie mogą być prezentowane na wykresie, analizowane pod kątem parametrów biomechanicznych albo wykorzystywane jako sygnał sterujący grą.

**Główne założenia projektu:**
- rzetelny odczyt i walidacja danych zamiast generowania pozornych pomiarów;
- oddzielenie akwizycji próbek od odświeżania interfejsu;
- analiza pojedynczych powtórzeń, serii i całych sesji;
- możliwość wykorzystania gier jako uzupełnienia ćwiczeń;
- stopniowe ujednolicanie dostępu do danych w różnych interfejsach;
- jawne opisywanie funkcji gotowych, rozwijanych i wymagających dalszej walidacji.

## ✨ Najważniejsze możliwości

| Obszar | Możliwości |
|---|---|
| 👤 Kartoteka | Tworzenie, odczyt, edycja, usuwanie, import i eksport kart pacjentów — zakres zależy od klienta |
| 📡 Akwizycja | Odczyt danych z Arduino Nano/HX711 przez port szeregowy |
| 📉 Wizualizacja | Wykres siły, podgląd próbek i statystyki pomiaru |
| 💪 Analiza powtórzeń | Wykrywanie powtórzeń z histerezą, czas trwania, wartość szczytowa i średnia |
| 🧮 Analiza serii | Zestawienia serii, zmienność wyników i realizacja celu |
| 🕹️ Trening | Konfiguracja ćwiczenia, celu, serii, powtórzeń i odpoczynku |
| 🎮 Gry | Zestaw gier sterowanych sygnałem biofeedback |
| 📄 Raporty | Eksport wybranych danych i raportów do CSV |
| 🔐 Ochrona danych | Haszowanie haseł PBKDF2, kontrola dostępu, dziennik zdarzeń i opcjonalne szyfrowanie rekordów |
| 🌐 API | Serwer HTTP i wspólny model zasobów — integracja klientów jest stopniowo rozwijana |

## 🎮 Gry biofeedbackowe — pełna lista

W kodzie projektu znajduje się dziesięć pozycji gier rozpoznawanych przez GameFactory i uwzględnionych w konfiguracji budowania. Poniższa tabela przedstawia nazwy i opisane w projekcie koncepcje rozgrywki.

| Lp. | Gra | Identyfikator / klasa | Zasada rozgrywki |
|---:|---|---|---|
| 1 | 🌊 **Sin Force Trainer** — trening sinusoidalny | sin, sin_game, SinGame | Dopasowywanie sygnału siły do przebiegu sinusoidalnego i wyznaczonych stref celu. |
| 2 | 📐 **Tan Force Trainer** — trening tangensoidalny | tan, tan_game, TanGame | Ćwiczenie kontroli siły względem przebiegu tangensoidalnego; wartości w pobliżu asymptot są ograniczane. |
| 3 | 🏓 **Pong** | pong, PongGame | Sterowanie położeniem paletki sygnałem z czujnika, odbijanie piłki i zdobywanie punktów. |
| 4 | 🧱 **Arkanoid** | arkanoid, ArkanoidGame | Sterowanie paletką, odbijanie piłki i niszczenie bloków; projekt obejmuje życia i poziomy. |
| 5 | 🐦 **Flappy Bird** | flappy, flappy_bird, FlappyBirdGame | Sterowanie ruchem ptaka sygnałem siły i omijanie przeszkód. Narastające zbocze sygnału wyzwala impuls machnięcia. |
| 6 | 🍄 **Mario Run** | mario, mario_run, MarioGame | Bieg z przeszkodami i skokami sterowanymi wejściem biofeedback; ładowanie skoku jest zależne od czasu symulacji. |
| 7 | 👾 **Galaga** | galaga, galaga_force, GalagaGame | Strzelanka z falami przeciwników, pociskami, życiami i punktacją. |
| 8 | 🏹 **Archer** | archer, archer_force, ArcherGame | Strzelanie z łuku do ruchomych celów, z regulacją kąta i punktacją. |
| 9 | 🏎️ **Car Race** | car_race, carrace, CarRaceGame | Wyścig z przeszkodami i sterowaniem pozycją pojazdu na pasach. |
| 10 | 💥 **Scorched Earth** | scorched_earth, scorched-earth, ScorchedEarthGame | Turowa gra artyleryjska z kątem i siłą strzału, grawitacją, wiatrem oraz ukształtowaniem terenu. |

### ℹ️ Ważne informacje o grach

- Rejestracja gry w GameFactory oznacza, że aplikacja potrafi rozpoznać jej identyfikator i utworzyć odpowiedni obiekt.
- Nie oznacza to automatycznie, że każda gra przeszła pełne testy długiej rozgrywki, testy wszystkich poziomów trudności lub testy z fizycznym czujnikiem.
- Nie należy zakładać, że wszystkie gry są dostępne w TUI, WebUI i Androidzie. Głównym klientem gier pozostaje aplikacja Desktop.
- Balans rozgrywki, progi siły, ergonomia sterowania i walidacja na rzeczywistym stanowisku wymagają osobnej oceny.
- Wyniki gier i ich synchronizacja między wszystkimi klientami są odrębnym obszarem rozwoju.

## 🖥️ Interfejsy użytkownika

### 🪟 Desktop GUI — Qt5

Główna aplikacja okienkowa. Obejmuje moduły kartoteki, pomiarów, treningu, analityki, gier, urządzeń, diagnostyki i ustawień. Wykres i prezentacja danych korzystają z mechanizmów Qt. Pierwsze uruchomienie prowadzi przez utworzenie konta administratora — aplikacja nie powinna używać zaszytego domyślnego hasła.

### ⌨️ TUI — interfejs terminalowy

Klient C++/ncurses z ekranami pulpitu, pomiarów, pacjentów, treningu, urządzeń, diagnostyki i ustawień. Obsługuje nawigację klawiaturą, prezentację stanu połączenia i próbki oraz wybrane operacje treningowe. Szczegóły: [projekt interfejsu TUI](docs/TUI_DESIGN.md).

### 🌐 WebUI — interfejs przeglądarkowy

Responsywny interfejs HTML/CSS/JavaScript w katalogu platforms/webui. Można go uruchomić jako stronę statyczną:

~~~bash
python3 -m http.server 8080 --directory platforms/webui
~~~

Następnie otwórz adres http://localhost:8080. Sam serwer statyczny nie uruchamia API. Dostępność funkcji zależy od uruchomienia i konfiguracji serwera danych. Instrukcja: [platforms/webui/README.md](platforms/webui/README.md).

### 📱 Android

Natywny klient Kotlin/Jetpack Compose w katalogu platforms/android. Projekt zawiera widoki pulpitu, pomiarów, pacjentów, treningu i urządzeń. Nie należy traktować ekranów podglądowych jako gotowej obsługi akwizycji z czujnika. Instrukcja budowania: [platforms/android/README.md](platforms/android/README.md).

### 🔄 Rzeczywisty zakres integracji

Interfejsy rozwijają się niezależnie. Desktop jest głównym klientem sprzętowym. TUI korzysta ze wspólnego adaptera komunikacji szeregowej i wybranych operacji API. WebUI i Android mają częściową obsługę zasobów API, ale nie należy zakładać, że odbierają strumień pomiarowy w czasie rzeczywistym. Przed wdrożeniem konkretnego klienta sprawdź jego instrukcję i aktualne testy.

## 🔌 Sprzęt i przepływ pomiaru

### 🧩 Typowy zestaw sprzętowy

- 🍓 Raspberry Pi 4 lub zgodny komputer Linux;
- 🔧 Arduino Nano;
- ⚖️ czujnik tensometryczny i moduł wzmacniacza/przetwornika HX711;
- 🔌 przewód USB do komunikacji Arduino–Raspberry Pi;
- 🧪 znana masa odniesienia do kalibracji.

### 🔁 Schemat przepływu danych

~~~text
Czujnik tensometryczny
        │
        ▼
     Moduł HX711
        │
        ▼
  Arduino Nano
  (firmware pomiarowy)
        │ USB / port szeregowy
        ▼
  Adapter komunikacji
        │
        ▼
  Walidacja i silnik pomiarowy
        │
        ├──► wykres i podgląd próbek
        ├──► analiza powtórzeń i serii
        ├──► zapis sesji i raport CSV
        └──► sterowanie grą biofeedback
~~~

### ⚙️ Parametry komunikacji

| Parametr | Wartość |
|---|---|
| Prędkość transmisji | 115200 bit/s |
| Format | 8N1 |
| Typowe porty Linux | /dev/ttyUSB0, /dev/ttyACM0 |
| Tryb ciągły | nominalnie 80 próbek/s — zależnie od firmware i urządzenia |
| Ramka binarna | znacznik czasu 32-bit, wartość surowa 32-bit, CRC-8 |
| Znaczniki końca ramki | 0xAA 0x55 |

### 🗣️ Podstawowe komendy firmware

| Komenda | Znaczenie |
|---|---|
| ID / WHOAMI | Identyfikacja urządzenia |
| TARE / CALZERO | Zerowanie wskazania |
| DATA / READ | Żądanie danych |
| FORCE | Odczyt siły w formacie tekstowym |
| CONT | Włączenie ciągłej transmisji binarnych ramek 11-bajtowych |
| STOP | Zatrzymanie ciągłej transmisji binarnej |
| CAL:<grams> | Kalibracja z użyciem znanej masy, np. CAL:100.0 |
| RESET | Reset urządzenia |

Szczegółowe wymagania i procedurę kalibracji opisuje dokument [kalibracja HX711](docs/HX711_CALIBRATION.md). Aktualny firmware znajduje się w pliku [`hx711_advanced.ino`](hx711_advanced.ino). Kalibracja jest przechowywana tylko w pamięci RAM Arduino — po restarcie trzeba ją wykonać ponownie. `HX711_SAMPLES=1`, więc obecny odczyt nie uśrednia wielu próbek.

> [!WARNING]
> **Nie używaj surowych, nieskalibrowanych wartości ADC jako wiarygodnego pomiaru siły.** Przed treningiem wykonaj zerowanie i kalibrację znaną masą. Potwierdź stan kalibracji firmware (CAL:1) oraz poprawność wskazań w całym używanym zakresie obciążenia.

## 📈 Analiza pomiarów i treningu

Aplikacja rozwija analizę obejmującą między innymi:

- siłę szczytową, średnią i zakres zmian;
- czas trwania powtórzenia i jego faz;
- czas do osiągnięcia wartości szczytowej;
- realizację zadanego celu;
- liczbę wykrytych powtórzeń i serii;
- odpoczynek między powtórzeniami i seriami;
- zmienność wyników, w tym współczynnik zmienności (CV%);
- historię sesji oraz eksport wybranych wyników.

Sposób wyznaczania parametrów i stosowane równania opisuje [dokumentacja wzorów matematycznych](MATHEMATICAL_FORMULAS.md). Dodatkowe opracowania: [REPORT_FORMULAS.md](REPORT_FORMULAS.md) oraz [raport DOCX](MATHEMATICAL_REPORT.docx), jeśli plik jest dostępny w danym wydaniu repozytorium.

## 🔐 Dane, bezpieczeństwo i prywatność

### 👥 Konta i uwierzytelnianie

Pierwsze uruchomienie Desktop wymaga utworzenia konta administratora. Hasła są przechowywane jako skróty PBKDF2. Nie należy zakładać istnienia domyślnego loginu ani hasła.

### 🗄️ Magazyn danych

Bieżący wspólny magazyn plikowy wykorzystuje katalog danych aplikacji Qt (QStandardPaths::AppDataLocation/shared-data). Struktura logiczna obejmuje:

~~~text
shared-data/
├── patients/
├── measurements/
├── training-sessions/
├── training-results/
├── game-sessions/
└── game-results/
~~~

Rekordy używają identyfikatorów UUID, wersji i znaczników czasu. Nazwy plików nie powinny ujawniać numeru PESEL ani nazwiska. Zapis jest wykonywany atomowo.

### 🔒 Szyfrowanie

Opcjonalne szyfrowanie rekordów wykorzystuje AES-256-GCM. Klucz jest przekazywany przez zmienną środowiskową BIOFEEDBACK_ENCRYPTION_KEY; nie wolno umieszczać go w repozytorium, pliku konfiguracyjnym ani dziennikach. Gdy szyfrowanie jest włączone, a klucza brakuje, dostęp do zasobów powinien zostać zablokowany.

### 🧱 MariaDB

Konfiguracja przewiduje wybór backendu MariaDB, ale w bieżącym stanie repozytorium adapter repozytorium MariaDB nie jest dostępny. **Do czasu wdrożenia adaptera należy używać backendu files.** Samo wpisanie danych połączenia nie oznacza działającej integracji.

### 🌐 Serwer API

Serwer można zbudować jako cel biofeedback_api. Domyślnie nasłuchuje na 127.0.0.1. Dostęp do zasobów jest ograniczony do administratora. Nie wystawiaj prototypowego HTTP bezpośrednio do Internetu. Zdalny dostęp wymaga odpowiednio zabezpieczonej bramy HTTPS/WSS, kontroli dostępu, zarządzania kluczami i testów bezpieczeństwa.

## ⚙️ Wymagania

### 🖥️ System i kompilator

- Debian, Ubuntu lub Raspberry Pi OS;
- CMake;
- kompilator zgodny z C++17 (np. GCC);
- make lub inny generator obsługiwany przez CMake;
- pkg-config;
- Qt5 z modułami Qt Widgets i Qt Charts;
- biblioteki libserialport, nlohmann/json i OpenSSL;
- ncurses — dla budowania TUI.

### 📦 Instalacja zależności w Debianie / Ubuntu / Raspberry Pi OS

~~~bash
sudo apt update
sudo apt install -y \
  build-essential cmake git pkg-config \
  qtbase5-dev libqt5charts5-dev \
  libserialport-dev nlohmann-json3-dev \
  libssl-dev libncurses-dev
~~~

W zależności od wydania systemu nazwa pakietu Qt Charts może się różnić. Jeżeli libqt5charts5-dev nie jest dostępny, sprawdź nazwę pakietu dla używanej wersji dystrybucji.

## 🚀 Szybkie uruchomienie

### 1. Pobranie repozytorium

Jeżeli repozytorium nie jest jeszcze na komputerze:

```bash
git clone https://github.com/bartoszruta26-droid/biofeedback.git
cd biofeedback
```

Jeżeli masz już lokalną kopię i chcesz pobrać najnowsze zmiany z gałęzi `main`, przejdź do katalogu repozytorium i wykonaj:

```bash
cd biofeedback
git status
git switch main
git pull --ff-only origin main
```

Jeżeli `git status` pokazuje własne, niezapisane zmiany, najpierw je zatwierdź albo bezpiecznie odłóż (`git stash`). Nie używaj `git reset --hard`, jeżeli chcesz zachować lokalną pracę.

Sprawdź, jaki commit został pobrany:

```bash
git log -1 --oneline
```

### 2. Instalacja zależności (Debian / Ubuntu / Raspberry Pi OS)

```bash
sudo apt update
sudo apt install -y \
  build-essential cmake git pkg-config \
  qtbase5-dev qtbase5-dev-tools libqt5charts5-dev \
  libserialport-dev nlohmann-json3-dev \
  libssl-dev libncurses-dev
```

Pakiety Qt5 Widgets i Qt5 Charts są wymagane do zbudowania aplikacji Desktop. `libserialport-dev` zapewnia obsługę portu szeregowego, `nlohmann-json3-dev` obsługę JSON, a `libssl-dev` biblioteki OpenSSL. `libncurses-dev` jest potrzebny do opcjonalnego TUI.

### 3. Konfiguracja i kompilacja aplikacji Desktop

W katalogu głównym repozytorium wykonaj:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target biofeedback --parallel 2
```

Na komputerze z większą ilością pamięci możesz zwiększyć liczbę równoległych zadań, np. do `--parallel "$(nproc)"`. Na Raspberry Pi zalecane jest rozpoczęcie od `--parallel 2`, aby ograniczyć zużycie pamięci.

Jeżeli CMake był wcześniej skonfigurowany dla innego kompilatora, innej wersji Qt lub innej architektury, usuń wyłącznie katalog kompilacji i skonfiguruj projekt ponownie:

```bash
rm -rf build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target biofeedback --parallel 2
```

Nie usuwaj katalogu danych użytkownika aplikacji — nie jest on katalogiem kompilacji.

### 4. Uruchomienie aplikacji

```bash
./build/biofeedback
```

Jeśli plik wykonywalny nie powstał, sprawdź komunikaty konfiguracji CMake. Komunikat `Qt5 not found` albo brak wymaganych modułów Qt oznacza, że cel Desktop nie został skonfigurowany. Komunikat o braku `libserialport` oznacza konieczność doinstalowania `libserialport-dev` i ponownego uruchomienia konfiguracji CMake.

### 5. Uprawnienia portu szeregowego Arduino

Aby użytkownik mógł korzystać z portu Arduino bez uruchamiania aplikacji jako administrator:

```bash
sudo usermod -aG dialout "$USER"
```

Następnie wyloguj się i zaloguj ponownie. Sprawdź nazwę urządzenia:

```bash
ls -l /dev/ttyUSB* /dev/ttyACM* 2>/dev/null
```

### 6. Aktualizacja i ponowna kompilacja w przyszłości

Przy kolejnej aktualizacji istniejącej kopii repozytorium:

```bash
cd biofeedback
git switch main
git pull --ff-only origin main
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target biofeedback --parallel 2
```

Jeżeli zmiany obejmują wyłącznie kod źródłowy, zwykle nie trzeba usuwać katalogu `build`. CMake i system budowania przebudują zmienione pliki. Po zmianie zależności systemowych lub konfiguracji kompilatora ponownie wykonaj konfigurację; pełne czyszczenie `build` stosuj tylko wtedy, gdy cache CMake jest niezgodny.

## 🍓 Raspberry Pi — instalacja i pakiet DEB

Dla Raspberry Pi z 64-bitowym systemem Debian/Raspberry Pi OS wymagany jest pakiet o architekturze **arm64**. Pakiet amd64 przeznaczony dla typowego komputera PC nie jest zgodny z Raspberry Pi ARM64.

Sprawdź architekturę systemu:

~~~bash
dpkg --print-architecture
~~~

Oczekiwany wynik dla pakietu ARM64:

~~~text
arm64
~~~

Instrukcja instalacji i budowania pakietu: [docs/INSTALL_LINUX.md](docs/INSTALL_LINUX.md).

Przebieg budowania pakietu ARM64: [GitHub Actions — pakiet Raspberry Pi](https://github.com/bartoszruta26-droid/biofeedback/actions/workflows/raspberry-pi-arm64-deb.yml).

Po pobraniu artefaktu DEB i umieszczeniu go w bieżącym katalogu instalacja przebiega następująco:

~~~bash
sudo apt update
sudo apt install ./biofeedback_1.0.0_arm64.deb
~~~

Jeżeli APT zgłosi brakujące zależności:

~~~bash
sudo apt --fix-broken install
~~~

**Uwaga:** nazwa pakietu w poleceniu musi odpowiadać faktycznej nazwie pobranego pliku. Nie instaluj pakietu, dopóki przebieg CI nie zakończy się sukcesem, artefakt nie zostanie opublikowany, a jego architektura nie zostanie potwierdzona.

## 🧰 Budowanie ze źródeł

### 🧱 Budowanie aplikacji Desktop

Pełna instrukcja aktualizacji istniejącej kopii, instalacji zależności i uruchomienia aplikacji znajduje się w sekcji [Szybkie uruchomienie](#-szybkie-uruchomienie). Najkrótsza procedura kompilacji z katalogu głównego repozytorium:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target biofeedback --parallel 2
```

Cel `biofeedback` buduje główną aplikację okienkową Qt5. TUI jest celem opcjonalnym i jest konfigurowane, gdy CMake wykryje ncurses. Brak Qt5 Widgets/Charts lub libserialport uniemożliwia zbudowanie pełnej aplikacji Desktop.

### 🧪 Uruchomienie testów CTest

Po zbudowaniu celów testowych:

```bash
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

Aby zbudować wyłącznie aplikację Desktop, użyj `--target biofeedback`; aby uruchomić pełny zestaw testów, zbuduj wszystkie cele, a następnie wykonaj CTest.

### 📦 Utworzenie pakietu DEB (opcjonalnie)

Po pomyślnej konfiguracji i kompilacji:

```bash
cpack --config build/CPackConfig.cmake -G DEB -B dist
```

Pakiet można zainstalować poleceniem:

```bash
sudo apt install ./dist/biofeedback_*.deb
```

### 🖥️ Przykład Qt5

W repozytorium znajduje się przykład `examples/qt5_mandatory.cpp`. Cel przykładowy można zbudować poleceniem:

```bash
cmake --build build --target qt5_mandatory_example --parallel 2
```

### 🛠️ Geany

Projekt można otworzyć w środowisku Geany, jeżeli jest ono zainstalowane:

```bash
geany biofeedback.geany
```

Podstawowym i zalecanym źródłem konfiguracji pozostaje jednak `CMakeLists.txt`.

## 🧪 Testowanie

Projekt zawiera testy jednostkowe, regresyjne i kontrolne uruchamiane przez CTest oraz dodatkowe skrypty w katalogu tests/. W zależności od konfiguracji obejmują one m.in.:

- walidację ramek i protokołu HX711;
- konwersję wartości czujnika i kalibrację;
- analizę parametrów biomechanicznych;
- odświeżanie wykresu;
- obsługę sesji treningowych;
- silnik gier i fabrykę gier;
- kontrolę wejścia z czujnika i własności strumienia próbek;
- mechanizmy bezpieczeństwa i prywatności.

Uruchomienie testów lokalnie:

~~~bash
ctest --test-dir build --output-on-failure
~~~

Wynik testów należy przypisywać do konkretnego commita i konkretnej konfiguracji kompilacji. Zielony test programowy nie potwierdza jeszcze dokładności fizycznego toru pomiarowego, bezpieczeństwa klinicznego ani poprawności kalibracji całego stanowiska.

## 🗂️ Struktura repozytorium

~~~text
biofeedback/
├── .github/workflows/   # Automatyczna kompilacja i testy
├── config/              # Konfiguracja aplikacji
├── data/                # Dane przykładowe i konspekty
├── docs/                # Dokumentacja techniczna
├── examples/            # Przykłady
├── firmware/            # Oprogramowanie urządzeń, jeśli obecne w danym wydaniu
├── include/             # Publiczne nagłówki C++
├── packaging/           # Pliki integracji i pakowania systemowego
├── platforms/
│   ├── android/         # Klient Android
│   └── webui/           # Klient przeglądarkowy
├── resources/           # Ikony, grafiki i pozostałe zasoby
├── src/
│   ├── core/            # Logika aplikacji
│   ├── data/            # Magazyn i obsługa danych
│   ├── games/           # Silniki i implementacje gier
│   ├── gui/             # Główny interfejs Qt
│   ├── sensor/          # Komunikacja z czujnikiem
│   └── tab/             # Widoki i moduły aplikacji
├── tests/               # Testy jednostkowe i regresyjne
├── CMakeLists.txt       # Konfiguracja budowania
└── README.md            # Główny opis projektu
~~~

Rzeczywista zawartość katalogów może się zmieniać wraz z rozwojem projektu. W razie rozbieżności pierwszeństwo mają pliki źródłowe i aktualna konfiguracja CMake.

## 📚 Dokumentacja

Pełny katalog dokumentów, podział tematyczny i legenda statusów: [📚 Indeks dokumentacji technicznej](docs/README.md). Zasady redakcyjne: [📐 Standard dokumentacji](docs/STANDARD_DOKUMENTACJI.md).


| Dokument | Zakres |
|---|---|
| [Instalacja Linux](docs/INSTALL_LINUX.md) | Zależności, budowanie i instalacja w systemach Debian/Ubuntu |
| [Kalibracja HX711](docs/HX711_CALIBRATION.md) | Zerowanie, kalibracja i kontrola wskazań |
| [Projekt TUI](docs/TUI_DESIGN.md) | Obsługa interfejsu terminalowego |
| [Architektura wspólnego serwera danych](docs/ARCHITECTURE_SHARED_DATA_SERVER.md) | Model zasobów i integracja API |
| [Wzory matematyczne](MATHEMATICAL_FORMULAS.md) | Parametry biomechaniczne i obliczenia |
| [Raport wzorów](REPORT_FORMULAS.md) | Opracowanie parametrów w formacie Markdown |
| [Audyt silnika gier](docs/GAME_ENGINE_AUDIT_2026-11-07.md) | Historia audytu i zakres walidacji gier — dokument historyczny, może dotyczyć innej gałęzi |

## ⚠️ Ograniczenia i bezpieczeństwo użytkowania

1. **Kalibracja:** przed pomiarem siły wymagane są zerowanie i kalibracja z użyciem znanej masy odniesienia.
2. **Walidacja sprzętowa:** testy automatyczne nie zastępują kontroli czujnika, przewodów, mocowania, zakresu obciążenia i powtarzalności wskazań.
3. **Zastosowanie kliniczne:** projekt nie jest sam w sobie certyfikowanym wyrobem medycznym ani zatwierdzonym narzędziem diagnostycznym.
4. **Dane osobowe:** przed użyciem rzeczywistych danych należy skonfigurować kontrolę dostępu, szyfrowanie, kopie zapasowe i procedury ochrony danych.
5. **Sieć:** nie wystawiaj niezabezpieczonego serwera HTTP na publiczny interfejs sieciowy.
6. **Platformy:** nie wszystkie funkcje Desktop, TUI, WebUI i Android są równoważne.
7. **Gry:** dostępność gry w fabryce nie jest równoznaczna z zakończeniem testów funkcjonalnych, ergonomicznych i sprzętowych.

Oprogramowanie jest rozwijane do celów badawczo-rozwojowych, edukacyjnych i prototypowania. Odpowiedzialność za ocenę przydatności konkretnego zastosowania, bezpieczeństwo stanowiska i zgodność regulacyjną spoczywa na podmiocie wdrażającym.

## 🤝 Współpraca

1. Zapoznaj się z kodem i dokumentacją dotyczącą obszaru, który chcesz zmienić.
2. Zgłoś błąd lub opisz proponowane usprawnienie w GitHub Issues.
3. Przygotuj zmianę w osobnej gałęzi.
4. Dodaj lub zaktualizuj testy regresyjne.
5. Otwórz Pull Request z opisem zakresu, sposobu weryfikacji i ewentualnych ograniczeń.

Przy zmianach dotyczących akwizycji, kalibracji, danych pacjentów i sterowania grami szczególnie ważne są testy błędnych danych, utraty połączenia, przeciążenia strumienia i bezpiecznego zatrzymania.

---

<p align="center">
  🧠 <strong>Biofeedback</strong> — mierz świadomie, analizuj rzetelnie, trenuj bezpiecznie. 💪
</p>
