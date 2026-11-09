# 📚 Dokumentacja techniczna Biofeedback

> 📘 Indeks dokumentacji • Język podstawowy: polski • Aktualizuj wraz ze zmianami kodu, testów i konfiguracji.

[⬅️ Wróć do głównego README](../README.md)

## 🧭 Jak korzystać z dokumentacji

- **Instrukcje operacyjne** opisują czynności, które można wykonać w aktualnym projekcie. Zawsze sprawdź wymagania i ograniczenia.
- **Dokumenty architektury** opisują podział odpowiedzialności, kontrakty i kierunek rozwoju. Fragment oznaczony jako docelowy nie musi być już wdrożony.
- **Audyty historyczne** opisują stan kodu w określonym dniu, na konkretnej gałęzi lub commicie. Nie traktuj ich jako potwierdzenia obecnego stanu `main`.
- **Plany i mapy drogowe** zawierają zadania oraz kryteria odbioru. Samo wymienienie funkcji w planie nie oznacza jej implementacji.

## 🏷️ Legenda statusów

| Oznaczenie | Znaczenie |
|---|---|
| 🟢 **WDROŻONE** | Funkcja potwierdzona w aktualnym kodzie lub testach wskazanych w dokumencie |
| 🟡 **CZĘŚCIOWO** | Istnieje implementacja, ale brakuje części integracji lub walidacji |
| 🔵 **PLAN** | Rozwiązanie docelowe albo zadanie do wykonania |
| 🔴 **NIEZWERYFIKOWANE** | Brak wystarczającego potwierdzenia testami lub sprzętem |
| ⚪ **HISTORYCZNE** | Wnioski dotyczą określonej daty, gałęzi lub SHA |

Status w tabeli jest wskazówką redakcyjną, a nie wynikiem automatycznego audytu bieżącego commita.

## 🚀 Instalacja i uruchomienie

| Dokument | Zastosowanie |
|---|---|
| [🐧 Instalacja Linux](INSTALL_LINUX.md) | Zależności, instalacja pakietu i budowanie ze źródeł |
| [⚖️ Kalibracja HX711](HX711_CALIBRATION.md) | Zerowanie, kalibracja i ocena wiarygodności wskazań |
| [⌨️ Projekt interfejsu TUI](TUI_DESIGN.md) | Moduły, sterowanie i granice funkcjonalne klienta terminalowego |

## 📡 Pomiar, akwizycja i analiza

| Dokument | Zastosowanie |
|---|---|
| [📡 Wspólny silnik pomiarowy](MEASUREMENT_PIPELINE.md) | Model próbki, jednostki, jakość i integracja portu szeregowego |
| [📈 Audyt pomiaru czasu rzeczywistego](REALTIME_MEASUREMENT_AUDIT.md) | Przepływ próbek, archiwizacja i otwarte ryzyka |
| [💪 Analiza powtórzeń i serii](REPETITION_ANALYSIS.md) | Segmentacja, parametry powtórzeń i wyników serii |
| [🍓 Stan firmware Raspberry Pi Pico](PICO_FIRMWARE_MEASUREMENT_STATUS.md) | Zakres obecnej komunikacji diagnostycznej Pico |

## 🧱 Architektura i integracja

| Dokument | Zastosowanie |
|---|---|
| [🧱 Wspólny serwer danych](ARCHITECTURE_SHARED_DATA_SERVER.md) | Granice procesów, magazyny danych, API i plan migracji |
| [🧠 Centralna akwizycja i dystrybucja danych](SERVER_CENTRIC_ACQUISITION_ARCHITECTURE.md) | Właściciel strumienia, kolejki, backpressure i etapy integracji |
| [🧩 Wspólne usługi aplikacyjne](SHARED_APPLICATION_SERVICES.md) | Warstwa usług urządzeń i pomiarów |
| [🔐 Konfigurowalne szyfrowanie plików](CONFIGURABLE_FILE_ENCRYPTION.md) | Tryby szyfrowania i ustawienia |
| [🔐 Migracja danych XOR do AES-256-GCM](LEGACY_XOR_MIGRATION_2026-11-06.md) | Kontrolowana migracja danych historycznych |

## 🎮 Gry i interfejsy

| Dokument | Zastosowanie |
|---|---|
| [🎮 Plan rozwoju silnika gier](GAME_ENGINE_AND_ARCADE_ROADMAP.md) | Wymagania, zadania i kryteria walidacji |
| [🎮 Audyt silnika gier — 2026-11-07](GAME_ENGINE_AUDIT_2026-11-07.md) | Historyczny przegląd GameEngine i gier |
| [🧭 Docelowa architektura interfejsów](audit/GUI_ARCHITECTURE_TARGET.md) | Warstwy i zasady wspólnego rdzenia |
| [🧮 Macierz modułów interfejsów](audit/GUI_COMPONENT_MATRIX.md) | Porównanie Desktop, TUI, WebUI i Android |
| [🛠️ Plan refaktoryzacji GUI](audit/GUI_REFACTOR_ROADMAP.md) | Priorytety P0/P1 i bramki jakości |
| [🔐 Ustalenia bezpieczeństwa GUI](audit/GUI_SECURITY_FINDINGS.md) | Ryzyka i wymagane zabezpieczenia |
| [🧪 Strategia testowania GUI](audit/GUI_TEST_STRATEGY.md) | Testy jednostkowe, integracyjne, E2E i sprzętowe |
| [🔍 Audyt platform — 2026-11-07](audit/GUI_TUI_DESKTOP_WEB_ANDROID_AUDIT.md) | Historyczny audyt Desktop, TUI, WebUI i Android |

## 🔐 Bezpieczeństwo i audyty

| Dokument | Zastosowanie |
|---|---|
| [🔍 Audyt zgodności dokumentacji z kodem — 2026-11-09](audit/DOCUMENTATION_CODE_CONFORMANCE_2026-11-09.md) | Statusy funkcji, firmware, protokoły, API i macierz klientów |
|---|---|
| [🔐 Wzmocnienie bezpieczeństwa — 2026-11-06](SECURITY_HARDENING_2026-11-06.md) | Historyczny opis PBKDF2, AES-GCM i atomowego zapisu |
| [🔍 Audyt gałęzi integracyjnej — 2026-11-06](INTEGRATION_BRANCH_AUDIT_2026-11-06.md) | Historyczne ustalenia bezpieczeństwa i integracji |
| [🔗 Audyt integracyjny — 2026-11-07](INTEGRATION_AUDIT_2026-11-07.md) | Historyczny przegląd przepływu danych i integracji |

## ✍️ Standard redakcyjny

Zasady języka, nagłówków, linków, oznaczania statusu, wersjonowania i przeglądu dokumentów opisuje [📐 Standard dokumentacji](STANDARD_DOKUMENTACJI.md).

## 🔄 Zasada aktualizacji

Po zmianie kodu aktualizuj dokument opisujący zmieniony komponent w tym samym pull requeście. W opisie statusu podawaj zakres, gałąź/SHA i konkretny wynik testu. Nie usuwaj audytów historycznych tylko dlatego, że kod się zmienił — oznacz je jako historyczne i dodaj odnośnik do nowszego przeglądu.
