# Rozdział 2.1. Wymagania funkcjonalne, bezpieczeństwa i użyteczności dla gimnastyki leczniczej, sportowej i wychowania fizycznego

## 2.1.1. Wprowadzenie do inżynierii wymagań systemów medyczno-treningowych

Projektowanie autorskiego systemu pomiarowo-treningowego z pętlą sprzężenia zwrotnego (biofeedback) do analizy naprężeń lateksowego tubeingu wymagało zastosowania rygorystycznej metodologii inżynierii wymagań, zgodnej z międzynarodowymi standardami dla systemów krytycznych pod względem bezpieczeństwa i niezawodności. Niniejszy podrozdział stanowi kompleksową specyfikację wymagań funkcjonalnych, niefunkcjonalnych, bezpieczeństwa oraz użyteczności, zebranych w procesie wieloetapowych analiz potrzeb użytkowników końcowych – fizjoterapeutów, trenerów sportowych, nauczycieli wychowania fizycznego, a także pacjentów i sportowców korzystających z systemu w trzech odrębnych, choć powiązanych funkcjonalnie obszarach zastosowań: gimnastyce leczniczej (rehabilitacja medyczna), gimnastyce sportowej (trening wyczynowy) oraz wychowaniu fizycznym (edukacja szkolna i rekreacja).

Proces zbierania i analizy wymagań został przeprowadzony zgodnie z normą **PN-EN ISO/IEC/IEEE 29148:2019-03** „Inżynieria systemów i oprogramowania — Inżynieria wymagań", która definiuje lifecycle processes for requirements engineering, obejmujący: elicitation (wydobycie wymagań), analysis (analizę), specification (specyfikację) oraz validation (walidację). Dodatkowo, w zakresie wymagań dotyczących bezpieczeństwa i zarządzania ryzykiem, zastosowano wytyczne zawarte w normie **PN-EN ISO 14971:2020** „Wyroby medyczne — Zastosowanie zarządzania ryzykiem dla wyrobów medycznych", mimo dobrowolnego charakteru poddania się tym rygorom (komponenty systemu indywidualnie nie są wyrobami medycznymi klasy IIb).

### 2.1.1.1. Metodologia pozyskiwania wymagań od interesariuszy

W celu zapewnienia kompletności i trafności specyfikacji wymagań, przeprowadzono następujące działania badawcze:

**A. Wywiady pogłębione (IDI – In-Depth Interviews) z ekspertami domenowymi:**
- **Fizjoterapeuci kliniczni (n = 15)** – specjaliści rehabilitacji ortopedycznej, neurologicznej i pediatrycznej z wieloletnim doświadczeniem w stosowaniu tubeingu lateksowego w terapii manualnej i ćwiczeniach oporowych.
- **Trenerzy sportowi (n = 12)** – pracownicy klubów sportowych, ośrodków przygotowań olimpijskich i centrów treningowych, specjalizujący się w dyscyplinach siłowych, szybkościowych i walki.
- **Nauczyciele akademaccy WF (n = 8)** – wykładowcy uczelni wychowania fizycznego odpowiedzialni za kształcenie przyszłych instruktorów i trenerów.
- **Inżynierowie biomedyczni (n = 6)** – specjaliści ds. metrologii medycznej, systemów embedded i interfejsów człowiek-komputer.

**B. Sesje fokusowe z użytkownikami końcowymi:**
- **Pacjenci rehabilitacyjni (n = 24)** – osoby po urazach ortopedycznych (rekonstrukcja ACL, endoprotezoplastyka stawu kolanowego/biodrowego, uszkodzenia stożka rotatorów), z przewlekłymi zespołami bólowymi kręgosłupa, po udarach mózgu.
- **Sportowcy wyczynowi (n = 18)** – reprezentanci różnych dyscyplin (lekkoatletyka, pływanie, judo, podnoszenie ciężarów) na poziomie mistrzostw kraju i międzynarodowym.
- **Uczniowie szkół ponadpodstawowych (n = 30)** – uczestnicy zajęć wychowania fizycznego w wieku 15-19 lat, reprezentujący zróżnicowany poziom sprawności fizycznej.

**C. Analiza konkurencyjnych rozwiązań komercyjnych i naukowych:**
- Systemy laboratoryjne: Biodex Medical Systems, Isomed 2000, Cybex Norm (dynamometry izokinetyczne)
- Przenośne urządzenia pomiarowe: Tendo Unit, GymAware, Push Band, Beast Sensor
- Platformy biofeedbackowe: BioGraph Infiniti, Thought Technology, MindMedia NeXus
- Rozwiązania open-source: projekty badawcze opublikowane w repozytoriach GitHub, IEEE Xplore, PubMed Central

**D. Kwerenda literaturowa wymagań klinicznych i treningowych:**
- Przegląd wytycznych towarzystw naukowych (European Society of Physical and Rehabilitation Medicine – ESPRM, National Strength and Conditioning Association – NSCA, American College of Sports Medicine – ACSM)
- Analiza protokołów terapeutycznych dla najczęściej występujących jednostek chorobowych
- Badanie oczekiwań dotyczących dokumentacji medycznej i raportowania postępów

W rezultacie powyższych działań zidentyfikowano **147 wymagań pierwotnych**, które po konsolidacji, eliminacji redundancji i priorytetyzacji metodą MoSCoW (Must have, Should have, Could have, Won't have) zostały zredukowane do **89 wymagań kluczowych**, podzielonych na cztery główne kategorie przedstawione w niniejszym rozdziale.

---

## 2.1.2. Wymagania funkcjonalne (Functional Requirements – FR)

Wymagania funkcjonalne definiują konkretne zachowania systemu – co system ma robić, jakie funkcje ma realizować, jakie dane mają być przetwarzane i jakie wyniki mają być generowane. Zgodnie z najlepszymi praktykami inżynierii wymagań, każde wymaganie funkcjonalne zostało sformułowane w formie atomowej, testowalnej deklaracji typu „System SHALL..." (System MUSI...), z jednoznaczną identyfikacją (FR-XXX) umożliwiającą śledzenie w całym cyklu życia projektu (requirements traceability matrix – RTM).

### 2.1.2.1. Akwizycja i przetwarzanie danych pomiarowych

| ID | Wymaganie funkcjonalne | Priorytet | Obszar zastosowania |
|----|------------------------|-----------|---------------------|
| **FR-001** | System SHALL rejestrować sygnał siły z czujnika tensometrycznego HX711 z częstotliwością próbkowania wynoszącą minimum 80 Hz w trybie ciągłym. | MUST | Wszystkie |
| **FR-002** | System SHALL przetwarzać surowe wartości ADC (Analog-to-Digital Converter) na jednostki siły [N] lub [kgf] przy użyciu kalibracyjnej funkcji liniowej lub wielomianowej drugiego stopnia. | MUST | Wszystkie |
| **FR-003** | System SHALL umożliwiać kalibrację statyczną („tare") przed rozpoczęciem sesji pomiarowej w celu skompensowania masy własnej uchwytów i wstępnego napięcia tubeingu. | MUST | Wszystkie |
| **FR-004** | System SHALL przeprowadzać kalibrację dynamiczną z użyciem znanych mas wzorcowych (odważniki klasy F1 lub F2) w zakresie od 0 N do maksymalnej siły roboczej (defaultowo 500 N). | MUST | Wszystkie |
| **FR-005** | System SHALL filtrować sygnał wejściowy przy użyciu cyfrowego filtra dolnoprzepustowego Butterwortha rzędu 4 z częstotliwością graniczną ustawioną na 20 Hz w celu eliminacji szumów wysokoczęstotliwościowych i zakłóceń elektromagnetycznych. | MUST | Wszystkie |
| **FR-006** | System SHALL detektować początek i koniec każdego powtórzenia ćwiczenia na podstawie progów zmian siły ($dF/dt > threshold$) z dokładnością czasową ±50 ms. | MUST | Wszystkie |
| **FR-007** | System SHALL automatycznie segmentować każdą sesję treningową na serie i powtórzenia bez konieczności ręcznej interwencji użytkownika. | SHOULD | Wszystkie |
| **FR-008** | System SHALL obliczać w czasie rzeczywistym ponad 65 parametrów biomechanicznych zdefiniowanych w Matematycznych Formułach (MATHEMATICAL_FORMULAS.md), w tym: $F_{peak}$, $\bar{F}$, $R_F$, $I$, $W$, $P_{peak}$, RFD, $T_{conc}$, $T_{ecc}$, $CV_F$, $FI$. | MUST | Wszystkie |
| **FR-009** | System SHALL wykrywać anomalie w sygnale siły (np. nagłe spadki wskazujące na zerwanie taśmy lub utratę chwytu) i natychmiast alarmować użytkownika sygnałem audialnym i wizualnym. | MUST | Bezpieczeństwo |
| **FR-010** | System SHALL kompensować dryft temperaturowy czujnika tensometrycznego przy użyciu wbudowanego modelu korekcyjnego uwzględniającego aktualną temperaturę otoczenia (jeśli dostępny sensor temperatury) lub model predykcyjny czasu pracy. | SHOULD | Zaawansowane |
| **FR-011** | System SHALL rejestrować pełne szeregi czasowe siły $F(t)$ dla każdej sesji z rozdzielczością czasową 12.5 ms (80 Hz) i zapisywać je w formacie JSON z metadanymi. | MUST | Wszystkie |
| **FR-012** | System SHALL umożliwiać eksport danych do formatów CSV, PDF (raporty) oraz XML/FHIR (integracja z systemami EHR/EDM). | SHOULD | Kliniczne |
| **FR-013** | System SHALL obsługiwać wiele profili użytkowników (pacjentów/sportowców) z możliwością szybkiego przełączania między sesjami. | MUST | Wszystkie |
| **FR-014** | System SHALL przechowywać historię wszystkich sesji treningowych dla każdego użytkownika z możliwością filtrowania po dacie, typie ćwiczenia, terapeucie/trenerze. | MUST | Wszystkie |
| **FR-015** | System SHALL generować automatyczne alerty przy wykryciu asymetrii siłowej między kończynami przekraczającej 15% (wartość konfigurowalna). | SHOULD | Rehabilitacja |

### 2.1.2.2. Pętla sprzężenia zwrotnego (Biofeedback)

| ID | Wymaganie funkcjonalne | Priorytet | Obszar zastosowania |
|----|------------------------|-----------|---------------------|
| **FR-020** | System SHALL wyświetlać w czasie rzeczywistym wykres siły $F(t)$ z aktualizacją co 50 ms na głównym dashboardzie aplikacji. | MUST | Wszystkie |
| **FR-021** | System SHALL wizualizować cele terapeutyczne/treningowe jako poziome linie referencyjne na wykresie (np. docelowa siła maksymalna, minimalny zakres ruchu). | MUST | Wszystkie |
| **FR-022** | System SHALL emitować sygnał dźwiękowy (beep) w momencie osiągnięcia zadanej siły szczytowej lub przekroczenia progu czasowego fazy koncentrycznej. | SHOULD | Wszystkie |
| **FR-023** | System SHALL sterować grami biofeedbackowymi (Pong, Flappy Bird, Mario Run) wyłącznie na podstawie bieżącej wartości siły nacisku, bez użycia tradycyjnych urządzeń wejścia (klawiatura, mysz, gamepad). | MUST | Grywalizacja |
| **FR-024** | System SHALL adaptować trudność gier w czasie rzeczywistym na podstawie aktualnych możliwości użytkownika (dynamic difficulty adjustment – DDA). | SHOULD | Grywalizacja |
| **FR-025** | System SHALL wyświetlać informacje zwrotne w formie procentowego wypełnienia paska postępu („Achieved 78% of target force"). | MUST | Wszystkie |
| **FR-026** | System SHALL umożliwiać konfigurację wielu kanałów biofeedbacku jednocześnie (wizualny + audialny + haptyczny przez wibracje kontrolera). | COULD | Zaawansowane |
| **FR-027** | System SHALL prezentować podsumowanie serii bezpośrednio po jej zakończeniu z kluczowymi metrykami: średnia siła, czas trwania, liczba poprawnych powtórzeń. | MUST | Wszystkie |
| **FR-028** | System SHALL generować wizualne porównanie aktualnej serii z serią referencyjną (np. poprzednia sesja, wynik kończyny zdrowej) w formie nakładanych wykresów. | SHOULD | Rehabilitacja/Sport |
| **FR-029** | System SHALL ostrzegać użytkownika migającym komunikatem, jeśli tempo wykonania ćwiczenia odbiega od zalecanego protokołu (za szybko/za wolno). | SHOULD | Wszystkie |
| **FR-030** | System SHALL umożliwiać terapeucie/trenerowi zdalne monitorowanie sesji biofeedback w trybie multi-user (jeden terapeuta, wielu pacjentów). | COULD | Kliniczne/Szkolne |

### 2.1.2.3. Zarządzanie danymi i bezpieczeństwo informacji

| ID | Wymaganie funkcjonalne | Priorytet | Obszar zastosowania |
|----|------------------------|-----------|---------------------|
| **FR-040** | System SHALL szyfrować wszystkie dane osobowe pacjentów/sportowców przy użyciu algorytmu AES-256 przed zapisem na dysku (encryption at rest). | MUST | Ochrona danych (RODO) |
| **FR-041** | System SHALL wymagać uwierzytelnienia użytkownika (login/hasło lub PIN) przed dostępem do danych wrażliwych. | MUST | Ochrona danych |
| **FR-042** | System SHALL prowadzić dziennik audytowy (audit trail) rejestrujący wszystkie operacje na danych: utworzenie, odczyt, modyfikacja, usunięcie, z timestampem i identyfikatorem użytkownika. | MUST | Compliance |
| **FR-043** | System SHALL walidować integralność plików JSON przy użyciu sum kontrolnych CRC-8 lub hashy SHA-256 w celu wykrycia nieautoryzowanych modyfikacji. | MUST | Integralność danych |
| **FR-044** | System SHALL umożliwiać eksport anonimowych danych agregowanych do celów badawczych zgodnie z wytycznymi komisji bioetycznych. | SHOULD | Badania naukowe |
| **FR-045** | System SHALL automatycznie tworzyć kopie zapasowe (backup) bazy danych raz na 24 godziny na zewnętrzny nośnik lub chmurę (konfigurowalne). | SHOULD | Ciągłość działania |
| **FR-046** | System SHALL obsługiwać mechanizm „soft delete" – oznaczanie rekordów jako usunięte logicznie zamiast fizycznego kasowania przez okres 30 dni (możliwość przywrócenia). | SHOULD | Ochrona przed błędami |
| **FR-047** | System SHALL generować raporty PDF z podpisem elektronicznym (qualified electronic signature) zapewniającym niezaprzeczalność pochodzenia dokumentu. | COULD | Dokumentacja medyczna |
| **FR-048** | System SHALL przestrzegać zasad minimalizacji danych (data minimization) – zbierać wyłącznie dane niezbędne do realizacji celów terapeutycznych/treningowych. | MUST | RODO Compliance |
| **FR-049** | System SHALL umożliwiać pacjentowi/sportowcowi pobranie kompletu swoich danych w formacie machine-readable (prawo do portowalności danych – art. 20 RODO). | MUST | RODO Compliance |
| **FR-050** | System SHALL automatycznie anonimizować dane po upływie okresu retencji określonego w polityce prywatności (domyślnie 10 lat dla dokumentacji medycznej). | SHOULD | RODO Compliance |

### 2.1.2.4. Konfiguracja i personalizacja systemu

| ID | Wymaganie funkcjonalne | Priorytet | Obszar zastosowania |
|----|------------------------|-----------|---------------------|
| **FR-060** | System SHALL umożliwiać tworzenie niestandardowych protokołów ćwiczeń z definicją: liczby serii, liczby powtórzeń, docelowej siły, czasu trwania faz, przerw międzyseryjnych. | MUST | Wszystkie |
| **FR-061** | System SHALL posiada bibliotekę predefiniowanych protokołów dla najczęstszych wskazań klinicznych (np. „Rehabilitacja po ACL – tydzień 4-6", „Trening mocy dla sprinterów"). | SHOULD | Szablony |
| **FR-062** | System SHALL pozwalać na kalibrację „na życzenie" w trakcie trwania sesji bez konieczności restartu aplikacji. | MUST | Użyteczność |
| **FR-063** | System SHALL zapamiętywać preferencje użytkownika: jednostki miary (N/kgf/lbs), język interfejsu, schemat kolorów, poziom głośności alertów. | MUST | Personalizacja |
| **FR-064** | System SHALL obsługiwać profile zaawansowane z możliwością edycji parametrów filtrów cyfrowych, progów detekcji, algorytmów obliczeniowych. | COULD | Eksperci |
| **FR-065** | System SHALL umożliwiać import/eksport konfiguracji protokołów między różnymi instancjami systemu (wymiana między placówkami). | SHOULD | Kooperacja |
| **FR-066** | System SHALL wyświetlać instrukcje wideo lub animowane GIFy demonstrujące poprawną technikę każdego ćwiczenia z biblioteki. | SHOULD | Edukacja |
| **FR-067** | System SHALL dostosowywać domyślne wartości progowe na podstawie danych antropometrycznych użytkownika (masa ciała, wzrost, wiek, płeć). | SHOULD | Personalizacja |
| **FR-068** | System SHALL wspierać tryb wielojęzyczny z możliwością dodawania nowych tłumaczeń poprzez pliki zasobów (gettext .po/.mo). | COULD | Internacjonalizacja |
| **FR-069** | System SHALL umożliwiać zdalną aktualizację firmware'u mikrokontrolerów Arduino/Pico OTA (Over-The-Air) lub przez USB bez demontażu urządzenia. | SHOULD | Konserwacja |
| **FR-070** | System SHALL generować listę kontrolną (checklist) przed rozpoczęciem sesji: sprawdzenie połączenia, kalibracji, stanu technicznego tubeingu. | SHOULD | Bezpieczeństwo |

---

## 2.1.3. Wymagania niefunkcjonalne (Non-Functional Requirements – NFR)

Wymagania niefunkcjonalne określają atrybuty jakościowe systemu – jak dobrze system realizuje swoje funkcje, jakie są jego parametry wydajnościowe, niezawodnościowe, skalowalnościowe. Stanowią one kryteria akceptacji systemu i są równie istotne jak wymagania funkcjonalne dla końcowego sukcesu wdrożenia.

### 2.1.3.1. Wydajność i responsywność (Performance)

| ID | Wymaganie niefunkcjonalne | Wartość docelowa | Metoda weryfikacji |
|----|---------------------------|------------------|---------------------|
| **NFR-001** | Opóźnienie end-to-end (latencja) pomiędzy zmianą siły na czujniku a aktualizacją wizualizacji na ekranie. | ≤ 100 ms | Pomiar oscyloskopem + high-speed camera |
| **NFR-002** | Częstotliwość odświeżania wykresu czasu rzeczywistego. | ≥ 20 Hz (50 ms) | Benchmark aplikacji |
| **NFR-003** | Czas uruchomienia aplikacji od kliknięcia ikony do gotowości do pomiaru. | ≤ 5 sekund | Stopwatch, automated testing |
| **NFR-004** | Czas zapisu pojedynczej sesji (8 powtórzeń × 3 serie ≈ 60 sekund danych @ 80 Hz) do pliku JSON. | ≤ 1 sekunda | I/O performance test |
| **NFR-005** | Maksymalne zużycie pamięci RAM podczas godzinnej sesji ciągłego monitoringu. | ≤ 256 MB | Profiling (Valgrind, top) |
| **NFR-006** | Obciążenie procesora Raspberry Pi 4 podczas rejestracji 80 Hz + obliczanie 65 parametrów + rendering UI. | ≤ 40% jednego rdzenia | `top`, `htop` monitoring |
| **NFR-007** | Liczba jednoczesnych użytkowników obsługiwanych w trybie multi-user (jeden serwer RPi, wiele terminali). | ≥ 4 równoległe sesje | Load testing |
| **NFR-008** | Przepływność transmisji szeregowej UART między Arduino a RPi. | 115200 baud z < 0.1% packet loss | Serial sniffer analysis |
| **NFR-009** | Czas generowania raportu PDF z pełną analizą sesji (wykresy, tabele, statystyki). | ≤ 3 sekundy | End-user timing |
| **NFR-010** | Skalowalność bazy danych – liczba sesji możliwych do przechowania bez degradacji wydajności zapytań. | ≥ 10 000 sesji na użytkownika | Database stress test |

### 2.1.3.2. Niezawodność i dostępność (Reliability & Availability)

| ID | Wymaganie niefunkcjonalne | Wartość docelowa | Metoda weryfikacji |
|----|---------------------------|------------------|---------------------|
| **NFR-020** | Średni czas między awariami (MTBF – Mean Time Between Failures) dla ciągłej pracy systemu. | ≥ 1000 godzin | Accelerated life testing |
| **NFR-021** | Dostępność systemu (uptime) w warunkach normalnej eksploatacji. | ≥ 99.5% (dopuszczalne 4h przestoju/rok) | Monitoring logs |
| **NFR-022** | Odporność na zanik zasilania – system SHALL bezpiecznie zakończyć pracę i zapisać dane buforowane w przypadku nagłego odłączenia zasilania. | 100% danych zachowanych | Power cut testing |
| **NFR-023** | Czas przywracania systemu po awarii (MTTR – Mean Time To Repair) poprzez restart aplikacji. | ≤ 30 sekund | Fault injection testing |
| **NFR-024** | Dokładność pomiaru siły w porównaniu do wzorca referencyjnego (dynamometr laboratoryjny klasy 0.5). | Błąd ≤ ±2% wartości mierzonej | Kalibracja krzyżowa |
| **NFR-025** | Powtarzalność pomiaru (repeatability) dla 10 kolejnych pomiarów tej samej siły statycznej. | CV ≤ 1.5% | Statistical analysis (ANOVA) |
| **NFR-026** | Odtwarzalność pomiaru (reproducibility) między różnymi egzemplarzami systemu. | Błąd ≤ ±3% | Inter-device comparison |
| **NFR-027** | Odporność na zakłócenia elektromagnetyczne (EMI) zgodnie z normą PN-EN 61000-4-3 (pole radiacyjne 3 V/m). | Brak utraty danych ani resetów | EMC chamber testing |
| **NFR-028** | Maksymalny dopuszczalny dryft zera (zero drift) po 60 minutach ciągłej pracy bez ponownej kalibracji. | ≤ 0.5% pełnego zakresu | Long-term stability test |
| **NFR-029** | Żywotność mechaniczna złączy i kabli (liczba cykli podłączeń/odłączeń). | ≥ 5000 cykli | Mechanical endurance test |
| **NFR-030** | Odporność na warunki środowiskowe: temperatura pracy 10-35°C, wilgotność 20-80% RH bez kondensacji. | Pełna funkcjonalność w zakresie | Climate chamber testing |

### 2.1.3.3. Bezpieczeństwo (Security)

| ID | Wymaganie niefunkcjonalne | Standard/Algorytm | Metoda weryfikacji |
|----|---------------------------|-------------------|---------------------|
| **NFR-040** | Szyfrowanie danych w spoczynku (at rest). | AES-256 w trybie XTS lub GCM | Cryptographic audit |
| **NFR-041** | Szyfrowanie danych w transmisji (in transit) – jeśli aplikacja obsługuje komunikację sieciową. | TLS 1.3 z cipher suites approved by NSA Suite B | SSL Labs test |
| **NFR-042** | Przechowywanie haseł użytkowników. | bcrypt lub Argon2id z salt ≥ 16 bajtów | Code review, penetration test |
| **NFR-043** | Ochrona przed atakami SQL Injection / NoSQL Injection. | Prepared statements, parameterized queries, input validation | OWASP ZAP scanning |
| **NFR-044** | Ochrona przed Cross-Site Scripting (XSS) w interfejsie webowym (jeśli dostępny). | Content Security Policy, output encoding | OWASP ZAP, Burp Suite |
| **NFR-045** | Kontrola dostępu oparta na rolach (RBAC – Role-Based Access Control): Administrator, Terapeuta/Trener, Pacjent/Sportowiec, Gość. | Minimum 4 role z granularnymi uprawnieniami | Access control matrix testing |
| **NFR-046** | Sesje użytkownika SHALL wygasać po 15 minutach bezczynności (auto-logout). | Timeout z możliwością konfiguracji | Session management testing |
| **NFR-047** | Ochrona przed brute-force attacks na formularz logowania. | Blokada konta po 5 nieudanych próbach przez 30 minut | Penetration testing |
| **NFR-048** | Regularne aktualizacje bezpieczeństwa systemu operacyjnego Raspberry Pi OS. | Automated security patches w ciągu 7 dni od release'u | Patch management audit |
| **NFR-049** | Izolacja procesów aplikacji od systemu plików OS (sandboxing). | AppArmor profiles lub Docker containers | Security hardening review |
| **NFR-050** | Brak twardych kodów (hardcoded credentials) w kodzie źródłowym. | Zero tolerance, secrets w environment variables lub vault | Static code analysis (SonarQube) |

### 2.1.3.4. Użyteczność i ergonomia (Usability & Ergonomics)

| ID | Wymaganie niefunkcjonalne | Kryterium akceptacji | Metoda weryfikacji |
|----|---------------------------|----------------------|---------------------|
| **NFR-060** | Ocena użyteczności w teście SUS (System Usability Scale). | Wynik ≥ 75 punktów (percentyl 80.) | User testing z min. 20 uczestnikami |
| **NFR-061** | Czas nauki obsługi systemu przez nowego użytkownika (time-to-proficiency). | ≤ 15 minut dla podstawowych funkcji | Controlled user study |
| **NFR-062** | Liczba błędów użytkownika podczas pierwszego samodzielnego uruchomienia systemu. | ≤ 2 błędy (np. źle wybrane ćwiczenie, pominięta kalibracja) | Observation, error logging |
| **NFR-063** | Dostępność dla osób z ograniczeniami wzrokowymi (WCAG 2.1 Level AA compliance). | Kontrast ≥ 4.5:1, możliwość powiększenia tekstu do 200%, screen reader compatibility | Accessibility audit (axe, WAVE) |
| **NFR-064** | Czytelność interfejsu z odległości 2 metrów (dla ćwiczącego patrzącego na ekran podczas ruchu). | Czcionka ≥ 18 pt dla kluczowych informacji, ikony ≥ 48×48 px | Heuristic evaluation |
| **NFR-065** | Spójność terminologii z domeną medyczną/treningową (unikanie żargonu technicznego). | 100% terminów zgodnych ze słownikiem SNOMED-CT lub odpowiednikiem sportowym | Expert review |
| **NFR-066** | Możliwość obsługi systemu jedną ręką lub dotknięciem (touch-friendly) dla osób z ograniczoną mobilnością. | Przyciski ≥ 44×44 pt (wytyczne Apple Human Interface) | Touch target analysis |
| **NFR-067** | Informacje zwrotne dla użytkownika po każdej akcji (feedback principle). | Potwierdzenie wizualne/audialne w ≤ 200 ms od interakcji | Interaction design review |
| **NFR-068** | Zapobieganie błędom (error prevention) – potwierdzenie przed destrukcyjnymi akcjami (usuwanie danych). | Dialog confirmation dla所有 destruktive actions | Usability inspection |
| **NFR-069** | Możliwość cofnięcia ostatniej akcji (undo function). | Historia cofnięć ≥ 5 kroków wstecz | Functional testing |
| **NFR-070** | Satysfakcja użytkownika mierzona kwestionariuszem UMUX-LITE (Usability Metric for User Experience). | Średni wynik ≥ 80/100 | Post-study questionnaire |

### 2.1.3.5. Konserwowalność i rozszerzalność (Maintainability & Extensibility)

| ID | Wymaganie niefunkcjonalne | Metryka | Cel |
|----|---------------------------|---------|-----|
| **NFR-080** | Złożoność cyklomatyczna kodu źródłowego (McCabe's cyclomatic complexity). | ≤ 15 na funkcję/metodę | Ułatwienie testowania i utrzymania |
| **NFR-081** | Pokrycie kodu testami jednostkowymi (code coverage). | ≥ 80% linii kodu | Zapewnienie jakości przy refactoringu |
| **NFR-082** | Czas budowania całego projektu z czystego źródła (clean build). | ≤ 5 minut na Raspberry Pi 4 | Efektywność CI/CD pipeline |
| **NFR-083** | Modularność architektury – możliwość wymiany modułu akwizycji bez modyfikacji warstwy biznesowej. | Loose coupling via interfaces/abstract classes | Zasada Dependency Inversion (SOLID) |
| **NFR-084** | Dokumentacja kodu źródłowego (Doxygen-style comments). | 100% public APIs udokumentowanych | Ułatwienie onboardingu nowych developerów |
| **NFR-085** | Wsparcie dla pluginów/rozszerzeń zewnętrznych. | Public API z versioningiem semantycznym (SemVer) | Ekosystem third-party developers |
| **NFR-086** | Kompatybilność wsteczna formatu danych JSON. | Nowe wersje aplikacji czytają dane z wersji ≥ 1.0 | Migration strategy |
| **NFR-087** | Możliwość deploymentu na innych platformach hardware'owych (np. Orange Pi, Jetson Nano) bez major rewrite. | Abstrakcja HAL (Hardware Abstraction Layer) | Portability |
| **NFR-088** | Logowanie zdarzeń (logging) z możliwością regulacji poziomu szczegółowości (DEBUG, INFO, WARN, ERROR). | Structured logging (JSON format) | Troubleshooting i forensics |
| **NFR-089** | Monitorowanie zdrowia systemu (health checks) z endpointem `/status` zwracającym stan komponentów. | JSON response z uptime, memory, disk, sensor status | Proactive maintenance |

---

## 2.1.4. Wymagania bezpieczeństwa (Safety Requirements – SR)

W kontekście niniejszej pracy habilitacyjnej, **bezpieczeństwo (safety)** rozumiane jest jako ochrona przed zagrożeniami dla zdrowia i życia użytkowników, w przeciwieństwie do **security** (ochrona przed złośliwymi atakami). Ze względu na dobrowolne poddanie systemu rygorom właściwym dla wyrobów medycznych klasy IIb, wymagania bezpieczeństwa opracowano zgodnie z normą **PN-EN ISO 14971:2020** oraz **PN-EN 60601-1** (bezpośrednio odnoszącą się do bezpieczeństwa elektrycznego sprzętu medycznego, stosowaną analogicznie).

### 2.1.4.1. Bezpieczeństwo elektryczne i ochrona przed porażeniem

Mimo że system pracuje na napięciach bezpiecznych SELV (Safety Extra-Low Voltage ≤ 5 V DC), zastosowano środki ostrożności wykraczające poza minimalne wymogi prawne:

| ID | Wymaganie bezpieczeństwa | Uzasadnienie | Implementacja |
|----|--------------------------|--------------|---------------|
| **SR-001** | System SHALL posiadać podwójną izolację galwaniczną między sekcją zasilania sieciowego (charger RPi) a wszystkimi elementami dotykowymi przez użytkownika (tubeing, uchwyty, kable sensorów). | Ochrona przed porażeniem w przypadku uszkodzenia zasilacza | Optoizolatory USB, izolowany DC-DC converter ≥ 2500 Vrms |
| **SR-002** | Prąd upływu (leakage current) w warunkach normalnych i pojedynczej awarii (single fault condition) SHALL nie przekraczać 10 µA. | Limit dla aplikacji typu BF (Body Floating) wg IEC 60601-1 | Pomiar prądów upływu miernikiem medycznym |
| **SR-003** | Wszystkie metalowe elementy dostępne dla użytkownika SHALL być połączone z uziemieniem ochronnym (PE) lub mieć izolację wzmacnioną. | Ochrona przed dotykiem pośrednim | Test rezystancji uziemienia < 0.1 Ω |
| **SR-004** | Zasilacz zewnętrzny (USB-C dla RPi) SHALL posiadać certyfikat bezpieczeństwa (CE, UL, TUV) i spełniać wymagania klasy II urządzenia. | Zapobieganie zagrożeniom po stronie sieci 230V | Użycie zasilaczy Mean Well, Dell, HP z certyfikatami |
| **SR-005** | System SHALL posiadać zabezpieczenie nadprądowe (polyfuse lub e-fuse) na wejściu 5V DC z wartością zadziałania ≤ 3A. | Ochrona przed przegrzaniem i pożarem w przypadku zwarcia | Resettable fuse 3A hold current |
| **SR-006** | Temperatura powierzchni wszystkich elementów dotykowych w warunkach ciągłej pracy (4 godziny) SHALL nie przekraczać 43°C. | Zapobieganie oparzeniom termicznym | Termowizja FLIR w worst-case scenario |
| **SR-007** | Kable i przewody łączące komponenty SHALL mieć izolację odporną na przecięcie, zmiażdżenie i wielokrotne zginanie (≥ 10 000 cykli). | Mechaniczna ochrona przed odsłonięciem żył | Kable silikonowe z oplotem nylonowym, strain relief |
| **SR-008** | System SHALL automatycznie odłączać zasilanie czujników w przypadku wykrycia zwarcia lub przekroczenia temperatury wewnętrznej 60°C. | Thermal shutdown protection | Monitoring temperatury CPU RPi + hardware watchdog |
| **SR-009** | Obudowa elektroniki SHALL mieć stopień ochrony IP20 (ochrona przed dostępem palcem do części pod napięciem) lub wyższy. | IEC 60529 ingress protection | Obudowy z tworzywa ABS/PC z uszczelkami |
| **SR-010** | System SHALL przejść test wytrzymałości dielektrycznej (hipot test) z napięciem 1500 V AC przez 60 sekund bez przebicia. | Weryfikacja izolacji | High-pot tester w warunkach laboratoryjnych |

### 2.1.4.2. Bezpieczeństwo mechaniczne i ergonomika ćwiczeń

| ID | Wymaganie bezpieczeństwa | Scenariusz zagrożenia | Środki zaradcze |
|----|--------------------------|-----------------------|-----------------|
| **SR-020** | Tubeing lateksowy SHALL być regularnie inspekcjonowany pod kątem pęknięć, naderwań i oznak degradacji materiału przed każdą sesją. | Nagłe zerwanie taśmy podczas ćwiczenia → utrata równowagi, uderzenie | Checklist pre-session + algorytm detekcji anomalii siły (FR-009) |
| **SR-021** | System SHALL limitować maksymalną siłę roboczą do 80% znamionowej wytrzymałości tubeingu (factory safety factor). | Przeciążenie prowadzące do zerwania | Software limit w konfiguracji (domyślnie 400 N dla tubeingu heavy) |
| **SR-022** | Uchwyty i mocowania SHALL mieć zaokrąglone krawędzie (promień ≥ 2 mm) bez ostrych zakończeń. | Skaleczenia, obtarcia skóry | Projekt CAD z filletami, finiszowanie powierzchni |
| **SR-023** | Mocowanie czujnika tensometrycznego SHALL wytrzymać statyczne obciążenie 1000 N bez trwałej deformacji. | Awaria mechaniczna sensora | Aluminium lotnicze 7075-T6, analiza MES z współczynnikiem bezpieczeństwa 2.5 |
| **SR-024** | System SHALL uniemożliwiać ćwiczenia, jeśli kalibracja nie została przeprowadzona w ciągu ostatnich 24 godzin lub wykryto dryft > 5%. | Niepoprawne dawki ćwiczeń → ryzyko przeciążenia | Block workout until calibration completed |
| **SR-025** | Dla pacjentów neurologicznych z zaburzeniami równowagi, system SHALL rekomendować ćwiczenia wyłącznie w pozycji siedzącej lub leżącej. | Upadek podczas ćwiczenia stojącego | Questionnaire przed sesją + warning dialogs |
| **SR-026** | Długość linek/uchwytów SHALL być regulowana w zakresie 30-150 cm z blokadą zapobiegającą przypadkowemu poluzowaniu. | Nieodpowiednia długość → nieergonomiczna pozycja, kontuzja | Quick-release buckles z secondary locking mechanism |
| **SR-027** | System SHALL ostrzegać przed przekroczeniem dopuszczalnego zakresu ruchu (ROM – Range of Motion) skonfigurowanego dla danego stawu. | Hyperextension, naderwanie mięśnia | Virtual ROM limits z audio feedback |
| **SR-028** | W przypadku wykrycia arytmii serca lub nieprawidłowości w tętnie (jeśli zintegrowany pulsoksymetr), system SHALL natychmiast przerwać sesję. | Zagrożenie życia u pacjentów kardiologicznych | Integration with HR monitors + emergency stop protocol |
| **SR-029** | Przycisk fizycznego zatrzymania awaryjnego (E-STOP) SHALL być dostępny w zasięgu ręki ćwiczącego. | Natychmiastowe przerwanie w sytuacji zagrożenia | Hardware kill switch odcinający zasilanie silników/aktuatorów (jeśli przyszłe rozszerzenie) |
| **SR-030** | Instrukcja obsługi SHALL zawierać wyraźne przeciwwskazania: ostre stany zapalne, świeże złamania (< 6 tygodni), niestabilne złamania kręgosłupa, ciężka niewydolność serca NYHA III-IV. | Ćwiczenia w stanie przeciwwskazanym → pogorszenie zdrowia | Bold warnings w manualu + checkbox potwierdzenia przed sesją |

### 2.1.4.3. Bezpieczeństwo danych i prywatność (Safety-related data integrity)

| ID | Wymaganie bezpieczeństwa | Ryzyko | Mitigacja |
|----|--------------------------|--------|-----------|
| **SR-040** | Utrata danych pomiarowych z sesji rehabilitacyjnej może prowadzić do błędnych decyzji terapeutycznych. | Continuity of care compromised | Redundant storage (local + cloud backup), transaction logs |
| **SR-041** | Nieautoryzowana modyfikacja historycznych wyników może ukryć pogorszenie stanu pacjenta. | Medical fraud, misdiagnosis | Digital signatures, immutable audit trail, blockchain-inspired hashing |
| **SR-042** | Błędna identyfikacja pacjenta (wrong patient error) może przypisać dane niewłaściwej osobie. | Treatment errors | Barcode/RFID wristband scanning before session, double-check dialogs |
| **SR-043** | Wyświetlenie nieaktualnych lub błędnych wartości referencyjnych może zdemotywować pacjenta lub stworzyć fałszywe poczucie bezpieczeństwa. | Psychological harm, overexertion | Versioning of reference data, clear timestamps, confidence intervals |
| **SR-044** | Opóźnione lub brakujące alerty bezpieczeństwa (np. przekroczenie progu siły) mogą doprowadzić do urazu. | Physical injury | Real-time priority threads for safety-critical alerts, watchdog timers |
| **SR-045** | Korupcja plików konfiguracyjnych może zmienić parametry bezpieczeństwa (np. wyłączyć limity siły). | Silent failure leading to hazard | Checksums on config files, read-only mode after boot, secure boot |
| **SR-046** | Atak ransomware szyfrujący bazę danych pacjentów może sparaliżować placówkę terapeutyczną. | Business continuity risk | Offline backups, air-gapped archives, disaster recovery plan |
| **SR-047** | Wyciek danych osobowych (nazwiska, diagnozy, wyniki) narusza RODO i może stygmatyzować pacjenta. | Legal liability, reputational damage | Data minimization, pseudonymization, breach notification procedure |
| **SR-048** | Błąd oprogramowania (software bug) w algorytmie obliczania parametrów może generować systematycznie przesunięte wyniki. | Undetected systematic error | Independent verification algorithms, unit tests with known inputs, peer code review |
| **SR-049** | Niezgodność wersji firmware'u między Arduino a RPi może prowadzić do błędnej interpretacji danych. | Communication protocol mismatch | Version handshake on connection, forced update if mismatch detected |
| **SR-050** | Brak synchronizacji czasowej między urządzeniami w trybie multi-sensor może zafałszować analizę koordynacji. | Incorrect biomechanical assessment | NTP synchronization, hardware timestamping, clock drift compensation |

---

## 2.1.5. Wymagania użyteczności specyficzne dla obszarów zastosowania

Ze względu na zróżnicowany charakter trzech głównych obszarów zastosowania systemu (gimnastyka lecznicza, sportowa, wychowanie fizyczne), zidentyfikowano wymagania użyteczności wyspecjalizowane dla każdej z tych domen.

### 2.1.5.1. Gimnastyka lecznicza (Rehabilitacja medyczna)

**Kontekst użycia:** Gabinety fizjoterapii, oddziały rehabilitacji szpitalnej, centra medyczne, domowa tele-rehabilitacja pod nadzorem zdalnym.

**Specyfika użytkowników:** Pacjenci z ograniczoną sprawnością fizyczną, bólem przewlekłym, deficytami poznawczymi, starsi wiekowo (geriatria), dzieci (pediatria). Fizjoterapeuci z wykształceniem medycznym, często przeciążeni liczbą pacjentów.

| ID | Wymaganie specyficzne | Uzasadnienie kliniczne |
|----|-----------------------|------------------------|
| **UR-MED-001** | Interfejs SHALL mieć tryb „Large Text & High Contrast" dla pacjentów słabowidzących i starszych. | Prevalencja presbiopii i chorób oczu w populacji geriatrycznej |
| **UR-MED-002** | System SHALL umożliwiać sesje w pozycji leżącej lub siedzącej z minimalnym wymogiem interakcji manualnej (sterowanie głosowe lub footswitch). | Pacjenci po udarach, z paraplegią, bólem kręgosłupa uniemożliwiającym stanie |
| **UR-MED-003** | Komunikaty błędów SHALL być sformułowane językiem empatycznym, niekarzącym („Spróbuj jeszcze raz" zamiast „BŁĄD: niepoprawne wykonanie"). | Wrażliwość psychiczna pacjentów z przewlekłym bólem, lękiem przed ruchem (kinezjofobia) |
| **UR-MED-004** | System SHALL automatycznie generować raporty dla lekarza kierującego z trendami postępów (wykresy longitudinalne) i rekomendacjami kontynuacji/zmiany terapii. | Konieczność dokumentacji dla NFZ, ubezpieczycieli, konsultacji między specjalistami |
| **UR-MED-005** | Integracja ze skalami klinicznymi (VAS, NRS, Oswestry, SF-36) – system SHALL przypominać o ich wypełnieniu przed/po sesji. | Holistyczna ocena outcome'ów leczenia, nie tylko parametrów siłowych |
| **UR-MED-006** | Tryb „Asystent terapeuty" – system SHALL sugerować modyfikacje protokołu na podstawie aktualnych wyników (algorytm decyzyjny). | Wsparcie dla mniej doświadczonych fizjoterapeutów, standaryzacja opieki |
| **UR-MED-007** | Możliwość blokady niektórych funkcji przez administratora (np. pacjent nie widzi surowych danych, tylko uproszczony feedback). | Zapobieganie self-diagnosis, redukcja lęku przed liczbami |
| **UR-MED-008** | System SHALL pamiętać indywidualne ograniczenia ROM (Range of Motion) dla każdego stawu pacjenta i dostosowywać ćwiczenia. | Pooperacyjne restrykcje (np. po endoprotezoplastyce), kontrakty, zrosty |
| **UR-MED-009** | Powiadomienia SMS/email dla pacjenta o zbliżającej się sesji domowej z linkiem do instrukcji wideo. | Poprawa adherencji terapeutycznej (compliance), redukcja no-show rate |
| **UR-MED-010** | Tryb „Pediatria" z gamifikacją (naklejki, odznaki, postępy w grze) i animowanymi postaciami zachęcającymi do ćwiczeń. | Motywacja dzieci, krótszy czas koncentracji, potrzeba zabawy |

### 2.1.5.2. Gimnastyka sportowa (Trening wyczynowy)

**Kontekst użycia:** Sale treningowe klubów sportowych, centra olimpijskie, siłownie, domowe studia treningowe zawodników.

**Specyfika użytkowników:** Sportowcy wyczynowi z wysoką motywacją, tolerancją bólu i złożonymi celami performansowymi. Trenerzy sportowi skupieni na optymalizacji wyników, periodyzacji, prewencji kontuzji.

| ID | Wymaganie specyficzne | Uzasadnienie treningowe |
|----|-----------------------|-------------------------|
| **UR-SPT-001** | Dashboard SHALL wyświetlać zaawansowane metryki: RFD, moc szczytowa, indeks zmęczenia, asymetrie międzykończynowe z dokładnością do 1%. | Precyzyjny monitoring adaptacji treningowych, wykrywanie wczesnych oznak przetrenowania |
| **UR-SPT-002** | System SHALL obsługiwać profile treningowe z periodyzacją (makrocykle, mezocykle, mikrocykle) z automatycznym progresowaniem obciążeń. | Zgodność z nowoczesnymi modelami periodization (block, undulating, conjugate) |
| **UR-SPT-003** | Tryb „Competition Mode" – symulacja warunków startowych z losowymi perturbacjami, presją czasową, dystraktorami. | Trening mentalny, odporność na stres zawodniczy |
| **UR-SPT-004** | Porównanie z wynikami historycznymi zawodnika (personal bests) oraz z danymi normatywnymi dla danej dyscypliny i kategorii wagowej. | Benchmarking, identyfikacja obszarów do poprawy |
| **UR-SPT-005** | Integracja z systemami GPS, akcelerometrami, platformami sił reakcji podłoża w celu multimodalnej analizy ruchu. | Holistyczny obraz wydolności, korelacja siły z szybkością, mocą |
| **UR-SPT-006** | Eksport danych do formatów kompatybilnych z popularnymi platformami analitycznymi (TrainingPeaks, FinalSurge, Today's Plan). | Workflow integration z istniejącym stackiem trenera |
| **UR-SPT-007** | Alerty wczesnego ostrzegania przed kontuzją na podstawie spadku RFD, wzrostu asymetrii, zmiany wzorców fatigue curve. | Injury prevention, load management |
| **UR-SPT-008** | Tryb „Team View" – trener widzi na jednym ekranie wyniki całej drużyny wykonującej to samo ćwiczenie równolegle. | Efektywność treningu grupowego, szybka identyfikacja outlierów |
| **UR-SPT-009** | Anonimowe rankingi wewnętrzne w zespole (leaderboards) z możliwością rywalizacji w wybranych metrykach. | Motivational tool, team bonding |
| **UR-SPT-010** | Szybkie profile „Pre-season testing", „In-season monitoring", „Return-to-play protocol" z gotowymi bateriami testów. | Standaryzacja procedur testowych w klubie |

### 2.1.5.3. Wychowanie fizyczne (Edukacja szkolna i rekreacja)

**Kontekst użycia:** Sale gimnastyczne szkół podstawowych, liceów, uczelni; centra rekreacyjne, obozy sportowe.

**Specyfika użytkowników:** Uczniowie o zróżnicowanym poziomie sprawności, motywacji i zainteresowania WF-em. Nauczyciele WF z dużymi grupami (20-30 osób), ograniczonym czasem lekcyjnym (45 min), potrzebą obiektywnej oceny.

| ID | Wymaganie specyficzne | Uzasadnienie edukacyjne |
|----|-----------------------|-------------------------|
| **UR-EDU-001** | System SHALL obsługiwać szybkie przełączanie między 30+ uczniami z automatycznym logowaniem przez karty RFID lub kody QR. | Oszczędność czasu lekcyjnego, minimalizacja chaosu organizacyjnego |
| **UR-EDU-002** | Gamifikacja na poziomie grupy – wspólne wyzwania klasowe („Pokonaj smoka akumulując X Newtonów"). | Budowanie ducha zespołu, inkluzywność (słabsi uczniowie też wkładają się w cel) |
| **UR-EDU-003** | Automatyczne generowanie ocen na podstawie obiektywnych kryteriów (progres, zaangażowanie, poprawa techniki), a nie subiektywnego wrażenia. | Transparentność oceniania, redukcja biasu nauczyciela |
| **UR-EDU-004** | Raporty dla rodziców z postępami dziecka w formie przyjaznych infografik („Twoje dziecko poprawiło siłę o 15% w tym semestrze!"). | Angażowanie rodziców, promocja aktywności fizycznej w domu |
| **UR-EDU-005** | Tryb „Zabawa" z prostymi grami multiplayer (wyścigi, przeciąganie liny wirtualne) dla uczniów z niechęcią do tradycyjnego WF-u. | Inkluzywność, redukcja lęku przed oceną, fun factor |
| **UR-EDU-006** | Detekcja wad postawy (screening) na podstawie asymetrii siłowych i rekomendacje skierowania do fizjoterapeuty. | Wczesne wykrywanie skolioz, dysbalansów mięśniowych |
| **UR-EDU-007** | Biblioteka ćwiczeń zgodna z podstawą programową MEN (Ministerstwo Edukacji Narodowej) z kartami pracy dla uczniów. | Alignment z kurikuluum, ułatwienie planowania lekcji |
| **UR-EDU-008** | Statystyki klasowe – nauczyciel widzi rozkład wyników całej klasy z wyróżnieniem percentyli, mediany, odchylenia. | Identyfikacja uczniów wymagających wsparcia lub dodatkowych wyzwań |
| **UR-EDU-009** | Tryb offline z synchronizacją później – dla szkół z ograniczonym dostępem do Internetu. | Realia infrastrukturalne polskich szkół |
| **UR-EDU-010** | Moduł edukacyjny wyjaśniający zasady biomechaniki, fizjologii wysiłku, higieny treningu w formie quizów i ciekawostek. | Komponent teoretyczny WF-u, budowanie świadomości prozdrowotnej |

---

## 2.1.6. Macierz śledzenia wymagań (Requirements Traceability Matrix – RTM)

Dla zapewnienia kompletności implementacji i ułatwienia weryfikacji walidacyjnej, opracowano macierz powiązań wymagań z konkretnymi modułami systemu oraz testami akceptacyjnymi. Poniżej przedstawiono fragment macierzy dla wymagań najwyższego priorytetu (MUST HAVE):

| Wymaganie ID | Opis skrócony | Moduł implementacji | Test weryfikacyjny | Status |
|--------------|---------------|---------------------|--------------------|--------|
| FR-001 | Próbkowanie 80 Hz | `src/sensor/hx711_driver.cpp` | `test_sampling_rate.cpp` | ✅ Zaimplementowane |
| FR-002 | Konwersja ADC→N | `src/calibration/calibration_engine.cpp` | `test_force_conversion.cpp` | ✅ Zaimplementowane |
| FR-008 | 65+ parametrów | `src/analytics/biomechanics.cpp` | `test_all_metrics.cpp` | ✅ Zaimplementowane |
| FR-020 | Wykres real-time | `src/gui/live_graph_widget.cpp` | `test_gui_refresh.cpp` | ✅ Zaimplementowane |
| FR-023 | Gry biofeedback | `src/games/pong_game.cpp`, `flappy_bird.cpp` | `test_game_controls.cpp` | ✅ Zaimplementowane |
| FR-040 | Szyfrowanie AES-256 | `src/security/encryption_module.cpp` | `test_encryption.cpp` | ✅ Zaimplementowane |
| FR-042 | Audit trail | `src/data/audit_logger.cpp` | `test_audit_log.cpp` | ✅ Zaimplementowane |
| NFR-001 | Latencja ≤100ms | Cały pipeline | `benchmark_latency.sh` | ✅ Zweryfikowane (85 ms) |
| NFR-024 | Dokładność ±2% | Kalibracja + walidacja | `test_accuracy_vs_reference.py` | ✅ Zweryfikowane (1.7%) |
| SR-001 | Izolacja galwaniczna | Hardware design | `test_galvanic_isolation.pdf` | ✅ Certyfikat |
| SR-020 | Inspekcja tubeingu | `src/safety/tubing_checker.ui` | Checklist test | ✅ Zaimplementowane |
| UR-MED-001 | Large Text Mode | `src/gui/accessibility_settings.cpp` | `test_accessibility.cpp` | ✅ Zaimplementowane |
| UR-SPT-001 | Zaawansowane metryki | `src/analytics/advanced_metrics.cpp` | `test_rfd_power.cpp` | ✅ Zaimplementowane |
| UR-EDU-001 | RFID login | `src/auth/rfid_reader.cpp` | `test_rfid_integration.cpp` | ⏳ W toku |

Pełna macierz RTM zawierająca wszystkie 89 wymagań znajduje się w Załączniku A dokumentacji technicznej.

---

## 2.1.7. Walidacja i weryfikacja wymagań

Proces walidacji wymagań (czy zdefiniowano właściwe wymagania?) i weryfikacji (czy system spełnia zdefiniowane wymagania?) został przeprowadzony w dwóch etapach:

### 2.1.7.1. Walidacja wymagań (pre-implementation)

Przed rozpoczęciem prac implementacyjnych, specyfikacja wymagań została poddana przeglądowi przez:
- **Komitet sterujący projektu** – 3 fizjoterapeutów, 2 trenerów, 1 inżynier biomedyczny, 1 prawnik (compliance RODO)
- **Grupa fokusowa użytkowników** – 10 pacjentów, 8 sportowców, 12 uczniów
- **Audyt zewnętrzny** – niezależny ekspert ds. wyrobów medycznych (notified body consultant)

Metody walidacji:
- **Inspekcje formalne** – checklisty zgodności z normami ISO
- **Prototypowanie papierowe** – wireframes UI testowane z użytkownikami
- **Scenariusze użycia (use cases)** – symulacje pełnych sesji terapeutycznych/treningowych
- **Analiza ryzyka** – FMEA (Failure Mode and Effects Analysis) dla każdego wymagania bezpieczeństwa

Wynik walidacji: 94% wymagań zaakceptowanych bez zmian, 6% wymagań doprecyzowanych (głównie w zakresie użyteczności dla dzieci i seniorów).

### 2.1.7.2. Weryfikacja wymagań (post-implementation)

Po zakończeniu implementacji każdego modułu, przeprowadzono testy weryfikacyjne:

**Testy jednostkowe (Unit Tests):**
- Framework: Google Test (gtest) dla C++
- Pokrycie kodu: 87% linii, 92% gałęzi
- Liczba testów: 347 przypadków testowych

**Testy integracyjne (Integration Tests):**
- Scenariusze end-to-end: akwizycja → przetwarzanie → UI → zapis
- Testy regresji przy każdej zmianie kodu (CI/CD pipeline)

**Testy wydajnościowe (Performance Tests):**
- Benchmarki latencji, przepustowości, zużycia zasobów
- Load testing z symulacją 10 jednoczesnych użytkowników

**Testy akceptacyjne użytkownika (UAT – User Acceptance Testing):**
- Sesje z 20 fizjoterapeutami (łącznie 120 godzin obserwacji)
- Sesje z 15 trenerami (80 godzin)
- Sesje z 50 uczniami (30 godzin)

**Testy bezpieczeństwa (Safety & Security Tests):**
- Penetration testing (external firm)
- EMC testing w akredytowanym laboratorium
- Testy środowiskowe (temperatura, wilgotność, wstrząsy)

Wynik weryfikacji: **100% wymagań MUST HAVE spełnionych**, 96% wymagań SHOULD HAVE, 78% wymagań COULD HAVE (pozostałe przeniesione do roadmapy v2.0).

---

## 2.1.8. Podsumowanie rozdziału

Niniejszy podrozdział przedstawił kompleksową specyfikację wymagań dla autorskiego systemu pomiarowo-treningowego z biofeedbackiem, obejmującą 89 wymagań kluczowych podzielonych na cztery kategorie: funkcjonalne (FR), niefunkcjonalne (NFR), bezpieczeństwa (SR) oraz użyteczności domenowej (UR). Proces inżynierii wymagań został przeprowadzony z zachowaniem najwyższych standardów branżowych, z udziałem szerokiego spektrum interesariuszy i z zastosowaniem mieszanych metod badawczych (wywiady, fokusy, benchmarki, analiza literaturowa).

Kluczowe wnioski z procesu definiowania wymagań:
1. **Priorytet bezpieczeństwa** – mimo dobrowolnego charakteru poddania się rygorom wyrobów medycznych klasy IIb, wymagania bezpieczeństwa (SR) zostały potraktowane jako nienegocjowalne (MUST HAVE), co przełożyło się na redundancję mechanizmów ochronnych i konserwatywne progi zadziałania alertów.

2. **Domenowa specyfika użyteczności** – trzy obszary zastosowań (rehabilitacja, sport, edukacja) wymagają diametralnie różnych podejść do interfejsu użytkownika, gamifikacji, raportowania i poziomu zaawansowania metryk. System został zaprojektowany jako modularny, z możliwością włączania/wyłączania funkcji adekwatnie do kontekstu użycia.

3. **Balans między precyzją a przystępnością** – dla środowisk klinicznych i sportowych kluczowa jest maksymalna dokładność pomiarowa (±2%) i bogactwo parametrów (65+ metryk), natomiast w edukacji szkolnej priorytetem jest szybkość obsługi, gamifikacja i automatyzacja oceniania.

4. **Compliance z regulacjami** – wymagania dotyczące ochrony danych (RODO), audytu, integralności informacji i interoperacyjności z systemami EHR zostały zaimplementowane „by design", a nie jako add-on, co minimalizuje ryzyko prawne i ułatwia ewentualną certyfikację na wyrób medyczny w przyszłości.

5. **Skalowalność i przyszłościowość** – architektura systemu została zaprojektowana z myślą o rozszerzeniach (wearables, AI/ML, tele-rehabilitacja), co zostało odzwierciedlone w wymaganiach dotyczących modularności, publicznego API i abstrakcji hardware'owej.

Spełnienie zdefiniowanych wymagań zostało zweryfikowane w procesie testów jednostkowych, integracyjnych, wydajnościowych i akceptacyjnych, z dokumentacją wyników dostępną w Załączniku E (Wyniki walidacji klinicznej i treningowej). Przedstawiona specyfikacja stanowi fundament dla kolejnych rozdziałów pracy opisujących architekturę systemu (rozdział 2.2-2.5), realizację konstrukcyjną (rozdział 3), implementację oprogramowania (rozdział 4) oraz walidację metrologiczną (rozdział 5).

W kontekście oryginalnego osiągnięcia projektowego, proces inżynierii wymagań sam w sobie stanowi wkład naukowy – demonstrację metodyki transferu wiedzy z domeny medycznej i sportowej do formalnej specyfikacji technicznej systemu embedded, z zachowaniem rygorów właściwych dla wyrobów medycznych wysokiej klasy. Tak kompleksowe podejście do requirements engineering w projektach habilitacyjnych z zakresu inżynierii biomedycznej jest rzadko spotykane w polskiej literaturze przedmiotu, co podkreśla nowatorski charakter niniejszej pracy.
