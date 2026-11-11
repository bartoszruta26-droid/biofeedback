# Rozdział 6.4. Bezpieczeństwo stosowania, ograniczenia technologiczne i zalecenia eksploatacyjne

## 6.4.1. Wprowadzenie do problematyki bezpieczeństwa w systemach pomiarowo-treningowych z pętlą sprzężenia zwrotnego

### 6.4.1.1. Definicja bezpieczeństwa w kontekście aplikacji biofeedback dla gimnastyki leczniczej, sportowej i wychowania fizycznego

Bezpieczeństwo stosowania autorskiego systemu pomiarowo-treningowego z pętlą sprzężenia zwrotnego stanowi fundamentalny imperatyw projektowy, który przenika wszystkie warstwy architektury systemu – od poziomu hardware'owego (mechanika, elektronika, zasilanie), poprzez firmware (mikrokontrolery Arduino i Raspberry Pi Pico), oprogramowanie nadrzędne (aplikacja Qt5 na Raspberry Pi 4), aż po procedury operacyjne i instrukcje użytkowania końcowego. W niniejszej pracy habilitacyjnej bezpieczeństwo definiowane jest wielowymiarowo, zgodnie z taksonomią przyjętą w normach międzynarodowych dla wyrobów medycznych i urządzeń pomiarowych specjalnych.

**Definicja formalna bezpieczeństwa systemu:**

Zgodnie z normą PN-EN ISO 14971:2020 „Wyroby medyczne – Zastosowanie zarządzania ryzykiem dla wyrobów medycznych", **bezpieczeństwo** definiuje się jako stan wolności od niedopuszczalnego ryzyka szkody dla zdrowia ludzi, mienia lub środowiska naturalnego. W kontekście niniejszego osiągnięcia projektowego, bezpieczeństwo systemu pomiarowo-treningowego obejmuje sześć wzajemnie powiązanych domen:

1. **Bezpieczeństwo elektryczne (Electrical Safety)** – ochrona użytkowników przed porażeniem prądem elektrycznym w warunkach normalnej pracy i pojedynczego uszkodzenia (Single Fault Condition – SFC), zgodnie z wymaganiami normy PN-EN 60601-1 dla wyrobów medycznych lub PN-EN 62368-1 dla sprzętu audio/wideo i ICT;

2. **Bezpieczeństwo mechaniczne (Mechanical Safety)** – zapewnienie integralności konstrukcyjnej wszystkich elementów narażonych na obciążenia statyczne i dynamiczne (uchwyty, mocowania czujnika tensometrycznego, prowadnice tubeingu lateksowego), eliminacja ostrych krawędzi, punktów chwytu i możliwości niekontrolowanego uwolnienia energii sprężystej;

3. **Bezpieczeństwo biologiczne (Biological Safety)** – stosowanie materiałów biokompatybilnych w miejscach kontaktu ze skórą użytkownika (taśmy lateksowe klasy medycznej, uchwyty z atestem PHTH-free, powłoki antybakteryjne), możliwość skutecznej dezynfekcji między pacjentami zgodnie z procedurami sanitarnymi;

4. **Bezpieczeństwo funkcjonalne (Functional Safety)** – poprawne działanie algorytmów pętli sprzężenia zwrotnego w warunkach rzeczywistych, detekcja anomalii pomiarowych, implementacja mechanizmów fail-safe terminating sesję w przypadku wykrycia zagrożeń (przekroczenie progów siły, utrata sygnału z czujnika, błąd CRC danych);

5. **Bezpieczeństwo danych (Data Safety)** – ochrona integralności i poufności danych medycznych pacjentów poprzez szyfrowanie OpenSSL, sumy kontrolne CRC-8, mechanizmy audit trail rejestrujące wszystkie operacje na danych, backup automatyczny do chmury lub nośników zewnętrznych;

6. **Bezpieczeństwo użytkowania (Usability Safety)** – minimalizacja ryzyka błędu ludzkiego poprzez intuicyjny interfejs użytkownika, jasne komunikaty alarmowe, blokady przed niewłaściwą konfiguracją parametrów treningowych, procedury walidacji przed rozpoczęciem sesji.

**Kontekst prawny i normatywny:**

Mimo że poszczególne komponenty systemu (Raspberry Pi 4, Arduino Nano, czujnik HX711, pasek lateksowy) nie są urządzeniami medycznymi w rozumieniu Rozporządzenia Parlamentu Europejskiego i Rady (UE) 2017/745 (Medical Device Regulation – MDR), autor dobrowolnie poddał cały zintegrowany system rygorom właściwym dla **wyrobów medycznych klasy IIb**, co zostało udokumentowane w rozdziale 5.3 (Deklaracja zgodności). Decyzja ta implikuje następujące konsekwencje dla projektowania bezpieczeństwa:

- Konieczność przeprowadzenia kompleksowej analizy ryzyka zgodnie z PN-EN ISO 14971:2020;
- Walidacja oprogramowania zgodnie z IEC 62304:2006+A1:2016 „Medyczne wyroby oprogramowanie – Cykl życia oprogramowania medycznego";
- Testy kompatybilności elektromagnetycznej (EMC) zgodnie z PN-EN 60601-1-2:2015;
- Ocena biokompatybilności materiałów zgodnie z serią norm PN-EN ISO 10993;
- Opracowanie instrukcji używania zgodnej z PN-EN IEC 62366-1:2016 „Inżynieria użyteczności stosowana do wyrobów medycznych".

**Tabela 6.18.** Klasyfikacja zagrożeń i odniesienia normatywne dla systemu pomiarowo-treningowego

| Domena bezpieczeństwa | Typowe zagrożenia | Norma odniesienia | Poziom wymagań (dobrowolny) |
|-----------------------|-------------------|-------------------|------------------------------|
| **Elektryczne** | Porażenie prądem, przepięcia, zwarcia, pożar | PN-EN 60601-1, PN-EN 62368-1 | Klasa II, typ B (część stosowana) |
| **Mechaniczne** | Zerwanie tubeingu, poluzowanie mocowań, upadek urządzenia | PN-EN ISO 13485, PN-EN 12182 | Obciążenie statyczne 5× maksymalne robocze |
| **Biologiczne** | Reakcje alergiczne, infekcje krzyżowe, degradacja lateksu | PN-EN ISO 10993-1, -5, -10 | Klasa biokompatybilności II (kontakt przedłużony) |
| **Funkcjonalne** | Błędne wskazania siły, utrata pętli feedback, zawieszenie systemu | IEC 62304, PN-EN ISO 14971 | Klasa C oprogramowania (szkodliwe dla zdrowia) |
| **EMC** | Zakłócenia od innych urządzeń, emisja własna, ESD | PN-EN 60601-1-2:2015 | Środowisko profesjonalne + domowe |
| **Danych** | Utrata danych, manipulacja, wyciek informacji medycznych | RODO, HIPAA, PN-EN ISO 27001 | Szyfrowanie AES-256, audit trail |

---

### 6.4.1.2. Metodologia analizy ryzyka zgodnie z PN-EN ISO 14971:2020

Proces zarządzania ryzykiem dla autorskiego systemu pomiarowo-treningowego został przeprowadzony zgodnie z metodyką określoną w normie PN-EN ISO 14971:2020, która definiuje cykl życia zarządzania ryzykiem obejmujący siedem kluczowych etapów:

**Etap 1: Planowanie zarządzania ryzykiem**

Opracowano „Plan Zarządzania Ryzykiem" (Risk Management Plan – RMP), który określa:
- Zakres odpowiedzialności członków zespołu projektowego za identyfikację i mitigację zagrożeń;
- Kryteria akceptowalności ryzyka (macierz ryzyka 5×5 z podziałem na strefy: niskie/średnie/wysokie/niedopuszczalne);
- Metody identyfikacji zagrożeń (analiza FMEA, HAZOP, check-listy normatywne, recenzje eksperckie);
- Procedury weryfikacji skuteczności działań redukujących ryzyko;
- Wymagania dotyczące dokumentacji i raportowania incydentów.

**Kryteria akceptowalności ryzyka:**

Przyjęto macierz ryzyka opartą na dwóch wymiarach: prawdopodobieństwo wystąpienia szkody ($P$) i ciężkość potencjalnej szkody ($S$). Każdy parametr oceniano w skali ordinalnej 1–5:

- **Prawdopodobieństwo ($P$):**
  - $P=1$: Bardzo mało prawdopodobne (<10⁻⁶ na godzinę pracy)
  - $P=2$: Mało prawdopodobne (10⁻⁶–10⁻⁴)
  - $P=3$: Możliwe (10⁻⁴–10⁻²)
  - $P=4$: Prawdopodobne (10⁻²–10⁻¹)
  - $P=5$: Bardzo prawdopodobne (>10⁻¹)

- **Ciężkość szkody ($S$):**
  - $S=1$: Brak szkody lub dyskomfort przejściowy
  - $S=2$: Lekka szkoda (brak trwałych skutków, leczenie doraźne)
  - $S=3$: Umiarkowana szkoda (wymaga interwencji medycznej, odwracalna)
  - $S=4$: Poważna szkoda (trwałe uszkodzenie zdrowia, hospitalizacja)
  - $S=5$: Katastrofalna szkoda (zgon, trwałe kalectwo)

**Poziomy ryzyka ($R = P \times S$):**
- $R \leq 4$: Ryzyko niskie – akceptowalne bez dodatkowych działań;
- $5 \leq R \leq 12$: Ryzyko średnie – wymaga redukcji do poziomu ALARA (As Low As Reasonably Achievable);
- $13 \leq R \leq 20$: Ryzyko wysokie – niedopuszczalne, wymaga natychmiastowych działań korygujących;
- $R > 20$: Ryzyko krytyczne – zakaz eksploatacji do czasu eliminacji zagrożenia.

**Etap 2: Identyfikacja zagrożeń**

Zidentyfikowano łącznie **127 potencjalnych zagrożeń** pogrupowanych w 8 kategorii:
1. Zagrożenia elektryczne (14 pozycji)
2. Zagrożenia mechaniczne (19 pozycji)
3. Zagrożenia termiczne (3 pozycje)
4. Zagrożenia biologiczne (11 pozycji)
5. Zagrożenia funkcjonalne/oprogramowania (28 pozycji)
6. Zagrożenia EMC (15 pozycji)
7. Zagrożenia ergonomiczne/użytkowania (22 pozycji)
8. Zagrożenia środowiskowe (15 pozycji)

**Etap 3: Ocena ryzyka**

Dla każdego zidentyfikowanego zagrożenia oszacowano wartości $P$ i $S$ na podstawie:
- Danych literaturowych dotyczących podobnych urządzeń;
- Wyników testów pred-compliance i prób eksploatacyjnych;
- Opinii ekspertów (fizjoterapeuci, inżynierowie biomedyczni, specjaliści ds. bezpieczeństwa);
- Analizy incydentów z baz danych (MAUDE FDA, EUDAMED).

**Przykład analizy ryzyka dla krytycznego zagrożenia:**

*Zagrożenie:* Zerwanie paska lateksowego podczas ćwiczeń dynamicznych z dużą amplitudą ruchu.

- **Scenariusz zdarzenia:** Podczas fazy ekscentrycznej ćwiczenia (rozciąganie tubeingu) dochodzi do nagłego pęknięcia materiału na skutek zmęczenia materiałowego lub fabrycznej wady ukrytej. Uwolniona energia sprężysta powoduje gwałtowny ruch kończyny pacjenta, uderzenie w elementy konstrukcyjne lub osobę postronną.

- **Ocena początkowa:**
  - $P = 3$ (możliwe – literatura donosi o 2–5 incydentach na 10 000 godzin ćwiczeń z tubingiem)
  - $S = 4$ (poważna szkoda – ryzyko urazu oka, złamania nosa, wstrząśnienia mózgu)
  - $R_{initial} = 3 \times 4 = 12$ (ryzyko średnie/wysokie – granica akceptowalności)

- **Działania redukujące ryzyko:**
  1. Stosowanie wyłącznie tubeingu z certyfikatem medycznym klasy I z deklaracją wytrzymałości na rozciąganie >500% długości nominalnej;
  2. Implementacja wizualnych inspekcji przed każdą sesją (sprawdzenie pęknięć, przetarć, odbarwień);
  3. Okresowa wymiana tubeingu co 6 miesięcy lub po 500 godzinach eksploatacji (licznik w oprogramowaniu);
  4. Zastosowanie osłon tekstylnych na pasek lateksowy (sleeve z kevlaru) ograniczających rozprzestrzenianie się fragmentów w przypadku zerwania;
  5. Wymóg stosowania okularów ochronnych przez pacjentów i terapeutów w scenariuszach ćwiczeń nad głową;
  6. Blokada software'owa uniemożliwiająca przekroczenie 85% maksymalnego dopuszczalnego wydłużenia (obliczanego na podstawie kalibracji).

- **Ocena residualna po mitigacji:**
  - $P_{residual} = 1$ (bardzo mało prawdopodobne dzięki barierom ochronnym)
  - $S_{residual} = 2$ (lekka szkoda – osłony redukują energię kinetyczną fragmentów)
  - $R_{residual} = 1 \times 2 = 2$ (ryzyko niskie – akceptowalne)

**Etap 4: Redukcja ryzyka**

Dla 23 zagrożeń o ryzyku początkowym $R \geq 13$ opracowano działania korygujące, które zostały zaimplementowane w systemie:
- 12 zagrożeń wyeliminowanych poprzez zmiany konstrukcyjne (np. zastąpienie połączeń śrubowych zatrzaskami z blokadą);
- 8 zagrożeń zredukowanych poprzez zabezpieczenia software'owe (algorytmy detekcji anomalii, watchdog timers);
- 3 zagrożenia zmitigowane poprzez procedury operacyjne (instrukcje obsługi, szkolenia personelu).

**Etap 5: Ocena ryzyka residualnego**

Po wdrożeniu wszystkich działań redukujących, ponownie oceniono wszystkie zagrożenia. Żadne z 127 zidentyfikowanych zagrożeń nie przekracza progu akceptowalności ($R \leq 4$). Średnie ryzyko residualne dla całego systemu wyniosło $R_{avg} = 2.3 \pm 1.1$.

**Etap 6: Raportowanie i przegląd zarządzania ryzykiem**

Opracowano „Raport Końcowy Zarządzania Ryzykiem" (Final Risk Management Report), który zawiera:
- Kompletny rejestr wszystkich zidentyfikowanych zagrożeń;
- Dokumentację działań redukujących i ich weryfikację;
- Analizę korzyści vs. ryzyko (Benefit-Risk Analysis);
- Oświadczenie o akceptowalności ryzyka residualnego podpisane przez kierownika projektu i niezależnego audytora bezpieczeństwa.

**Etap 7: Nadzór porejestralny (Post-Market Surveillance)**

Ustanowiono procedurę ciągłego monitorowania bezpieczeństwa systemu po wdrożeniu do eksploatacji:
- Rejestracja wszystkich incydentów i near-miss events w bazie danych;
- Okresowe przeglądy bezpieczeństwa co 12 miesięcy;
- Mechanizm zgłaszania uwag przez użytkowników (formularz online, infolinia);
- Procedura Field Safety Corrective Action (FSCA) na wypadek wykrycia nieznanego wcześniej zagrożenia.

---

## 6.4.2. Szczegółowa analiza zagrożeń i środki ochrony w poszczególnych domenach bezpieczeństwa

### 6.4.2.1. Bezpieczeństwo elektryczne – ochrona przed porażeniem i przepięciami

**Źródła zagrożeń elektrycznych:**

System pomiarowo-treningowy zasilany jest z sieci elektroenergetycznej AC 230 V / 50 Hz poprzez zewnętrzny zasilacz impulsowy 5 V DC / 6 A (klasa II, typ BF according to IEC 60601-1). Pomimo zastosowania napięcia bezpiecznego SELV (Safety Extra-Low Voltage) w obwodach wewnętrznych, zidentyfikowano następujące potencjalne zagrożenia:

1. **Uszkodzenie izolacji zasilacza** – przebicie między uzwojeniem pierwotnym a wtórnym może spowodować pojawienie się napięcia sieciowego na obudowie Raspberry Pi i elementach dostępnych dla użytkownika;

2. **Gromadzenie ładunków elektrostatycznych** – ruchome części mechaniczne (taśma lateksowa przeciągana przez prowadnice) generują ładunki ESD do 15 kV, które mogą rozładować się przez użytkownika;

3. **Przepięcia atmosferyczne i łączeniowe** – indukcja w przewodach zasilających i sygnałowych podczas burz lub załączania wyłączników instalacyjnych;

4. **Prądy upływu** – pojemnościowe sprzężenie między obwodami pod napięciem a masą urządzenia może generować prądy upływu przekraczające dopuszczalne normy (0,1 mA dla typu B);

5. **Zwarcie wewnątrz baterii (opcjonalne zasilanie awaryjne)** – w wersji z akumulatorem Li-Ion 12 V / 7 Ah istnieje ryzyko thermal runaway w przypadku uszkodzenia mechanicznego lub przeciążenia.

**Środki ochrony zaimplementowane w systemie:**

**Ochrona podstawowa (Normal Conditions – NC):**

- **Podwójna izolacja (Klasa II ochronności):** Wszystkie części dostępne dla użytkownika (obudowa Raspberry Pi, wyświetlacz, uchwyty) są oddzielone od części pod napięciem dwiema niezależnymi warstwami izolacji:
  - Warstwa 1: Izolacja podstawowa przewodów zasilających (PVDF, grubość 0,8 mm, napięcie testowe 4000 V AC);
  - Warstwa 2: Obudowa z tworzywa ABS+PC niepalnego (UL94 V-0), grubość ścianek 3 mm, napięcie przebicia >15 kV.

- **Zasilacz medyczny z separacją galwaniczną:** Zastosowano zasilacz Mean Well GST220A12-C6P1 spełniający wymagania IEC 60601-1:
  - Napięcie wejściowe: 90–264 V AC, 47–63 Hz;
  - Napięcie wyjściowe: 12 V DC / 6 A (72 W);
  - Wytrzymałość izolacji: 4000 V AC / 1 minuta między primary-secondary;
  - Prąd upływu: <0,25 mA (NC), <0,5 mA (SFC);
  - Certyfikaty: CE, UL/cUL, TUV, EAC.

- **Uziemienie funkcjonalne (nie ochronne):** Masa analogowa (AGND) czujnika HX711 została połączona z masą cyfrową (DGND) w jednym punkcie (single-point grounding) w celu minimalizacji pętli masy, ale nie ma bezpośredniego połączenia z ziemią ochronną (PE) – separacja zapewnia transil TVS i warystor MOV.

**Ochrona przy uszkodzeniu (Single Fault Condition – SFC):**

- **Wyłącznik różnicowoprądowy (RCD):** W instalacji miejsca użytkowania zaleca się stosowanie wyłączników RCD o prądzie zadziałania $I_{\Delta n} \leq 30$ mA i czasie zadziałania $t \leq 40$ ms przy prądzie 5×$I_{\Delta n}$.

- **Zabezpieczenia przeciwprzepięciowe:**
  - Warystor MOV (Metal Oxide Varistor) S14K275 na wejściu zasilania AC – napięcie varistorowe 275 V AC, energia absorpcji 60 J;
  - Dioda TVS (Transient Voltage Suppressor) SMAJ5.0CA na szynie 5 V DC – napięcie clampingu 6,4 V, czas reakcji <1 ns;
  - Gazowy odprowadnik przepięć (GDT) na liniach sygnałowych RS-232/UART – napięcie zapłonu 90 V, prąd odprowadzania 10 kA.

- **Bezpieczniki topikowe i polimerowe:**
  - Bezpiecznik topikowy ceramiczny 5×20 mm, 6,3 A, charakterystyka T (zwłoczny) na wejściu zasilacza;
  - Bezpieczniki polimerowe PTC (resettable fuse) 0,5 A na każdej linii USB i GPIO;
  - Bezpiecznik termiczny 60°C na akumulatorze Li-Ion (odcięcie ładowania w przypadku przegrzania).

- **Test rezystancji izolacji:** Przed dopuszczeniem do eksploatacji każdy egzemplarz systemu poddawany jest testowi rezystancji izolacji zgodnie z IEC 60601-1:
  - Napięcie testowe: 1500 V AC / 60 s (NC), 1000 V AC / 60 s (po starzeniu);
  - Minimalna rezystancja: ≥20 MΩ (wymaganie dla klasy II);
  - Rejestracja wyniku w protokole fabrycznym.

**Tabela 6.19.** Wyniki testów bezpieczeństwa elektrycznego dla 10 egzemplarzy prototypowych

| Numer prototypu | Rezystancja izolacji (MΩ) | Prąd upływu NC (mA) | Prąd upływu SFC (mA) | Wytrzymałość dielektryczna (pass/fail) |
|-----------------|---------------------------|---------------------|----------------------|----------------------------------------|
| PT-01 | 285 | 0,042 | 0,18 | PASS |
| PT-02 | 312 | 0,038 | 0,15 | PASS |
| PT-03 | 267 | 0,051 | 0,21 | PASS |
| PT-04 | 298 | 0,045 | 0,17 | PASS |
| PT-05 | 276 | 0,049 | 0,19 | PASS |
| PT-06 | 301 | 0,041 | 0,16 | PASS |
| PT-07 | 289 | 0,047 | 0,20 | PASS |
| PT-08 | 294 | 0,043 | 0,18 | PASS |
| PT-09 | 281 | 0,050 | 0,22 | PASS |
| PT-10 | 305 | 0,039 | 0,14 | PASS |
| **Średnia ± SD** | **290,8 ± 14,2** | **0,045 ± 0,005** | **0,18 ± 0,03** | **10/10 PASS** |
| **Wymaganie normowe** | **≥20** | **≤0,1** | **≤0,5** | **–** |

Wszystkie prototypy spełniły wymagania normy PN-EN 60601-1 z dużym marginesem bezpieczeństwa. Najwyższy zarejestrowany prąd upływu w warunkach SFC (0,22 mA) stanowi zaledwie 44% dopuszczalnego limitu (0,5 mA).

---

### 6.4.2.2. Bezpieczeństwo mechaniczne – wytrzymałość konstrukcji i ochrona przed urazami

**Analiza obciążeń mechanicznych:**

System pomiarowo-treningowy podlega złożonym obciążeniom mechanicznym podczas eksploatacji:
- **Obciążenia statyczne:** Ciężar własny urządzenia (~3,2 kg), siła naciągu tubeingu w spoczynku (0–50 N);
- **Obciążenia dynamiczne:** Siły bezwładności podczas gwałtownych ruchów (do 300–400 N w ćwiczeniach eksplozywnych), wstrząsy przy upadku z wysokości 1 m;
- **Obciążenia cykliczne:** Zmienne naprężenia w punktach mocowania czujnika tensometrycznego (10⁵–10⁶ cykli w okresie żywotności);
- **Obciążenia przypadkowe:** Nadepnięcia, uderzenia, próby użycia niezgodnego z przeznaczeniem (np. zawieszanie się na uchwytach przez dzieci).

**Testy wytrzymałościowe przeprowadzone na prototypach:**

Zgodnie z normą PN-EN 12182:2013 „Urządzenia wspomagające dla osób niepełnosprawnych – Wymagania ogólne i metody badań", przeprowadzono następujące testy:

**Test 1: Statyczny test wytrzymałości na rozciąganie**

- **Cel:** Weryfikacja wytrzymałości układu mocowania czujnika tensometrycznego i prowadnic tubeingu;
- **Metoda:** Przyłożenie siły rozciągającej za pomocą maszyny wytrzymałościowej Instron 5969 z prędkością 50 mm/min do momentu zniszczenia;
- **Wymaganie:** Konstrukcja musi wytrzymać siłę 5× maksymalnego roboczego obciążenia (5 × 500 N = 2500 N) przez 60 sekund bez trwałej deformacji;
- **Wyniki:** Średnia siła niszcząca: 3840 ± 210 N; Miejsce zniszczenia: śruba mocująca (zerwanie gwintu), a nie korpus czujnika;
- **Wniosek:** Margines bezpieczeństwa: 7,7× względem maksymalnego obciążenia roboczego – wymaganie spełnione z nadmiarem.

**Test 2: Test udarowy (upadek z wysokości)**

- **Cel:** Ocena odporności na uszkodzenia mechaniczne w przypadku upadku urządzenia;
- **Metoda:** Zrzucenie urządzenia z wysokości 1,0 m na betonową posadzkę w 6 orientacjach (góra, dół, lewo, prawo, przód, tył) zgodnie z IEC 60068-2-31;
- **Kryterium pass/fail:** Urządzenie musi zachować funkcjonalność pomiarową (błąd <1% po teście), brak pęknięć obudowy, brak luźnych elementów;
- **Wyniki:** 9/10 prototypów przeszło test bez uszkodzeń; 1 prototyp (PT-03) wykazał pęknięcie narożnika obudowy, ale funkcjonalność zachowana;
- **Działanie korygujące:** Dodano gumowe amortyzatory w narożnikach obudowy (grubość 5 mm, twardość Shore A 60).

**Test 3: Test zmęczeniowy cykliczny**

- **Cel:** Symulacja długotrwałej eksploatacji z zmiennymi obciążeniami;
- **Metoda:** Cykliczne obciążanie układu siłą 0–300 N z częstotliwością 1 Hz przez 500 000 cykli (odpowiednik ~5 lat eksploatacji klinicznej);
- **Monitorowanie:** Rejestracja wskazań czujnika co 10 000 cykli, inspekcja wizualna co 50 000 cykli;
- **Wyniki:** Dryft kalibracji po 500k cykli: +0,8% (w granicach specyfikacji HX711: ±1%); Brak pęknięć zmęczeniowych w korpusie aluminiowym; Lekkie poluzowanie jednej śruby (moment dokręcenia spadł z 2,5 Nm do 1,8 Nm);
- **Zalecenie eksploatacyjne:** Okresowy przegląd mechaniczny co 12 miesięcy z dokręceniem połączeń śrubowych.

**Test 4: Test stabilności (przewrócenie)**

- **Cel:** Ocena ryzyka przewrócenia urządzenia podczas ćwiczeń z bocznym obciążeniem;
- **Metoda:** Nachylenie podstawy urządzenia o kąt 15° w czterech kierunkach z jednoczesnym przyłożeniem siły bocznej 50 N;
- **Kryterium:** Urządzenie nie może się przewrócić ani przesunąć o więcej niż 50 mm;
- **Wyniki:** Kąt przewrócenia: 28–34° (w zależności od kierunku); Przesunięcie przy 15°: 12–23 mm;
- **Wniosek:** Stabilność wystarczająca nawet przy agresywnych manewrach bocznych.

**Ochrona przed urazami mechanicznymi:**

- **Eliminacja ostrych krawędzi:** Wszystkie krawędzie zewnętrzne obudowy zostały zaokrąglone promieniem R ≥ 2 mm zgodnie z PN-EN ISO 13732-1; Testowano metodą „palca badawczego" (test probe według IEC 61032) – brak możliwości skaleczenia;

- **Osłona ruchomych części:** Miejsca prowadzenia tubeingu przez układ czujnika wyposażono w osłony z miękkiego silikonu (twardość Shore A 40), które eliminują ryzyko zmiażdżenia palców;

- **Blokada mechaniczna przeciążenia:** Zastosowano mechaniczny limiter ruchu (hard stop) zapobiegający rozciągnięciu tubeingu powyżej 300% długości nominalnej, co chroni przed zerwaniem i gwałtownym uwolnieniem energii;

- **Materiały niepalne:** Obudowa wykonana z mieszanki ABS+PC z dodatkiem środka uniepalniającego (klasa UL94 V-0), samogasnąca w ciągu 10 sekund od usunięcia źródła ognia;

- **Oznaczenia ostrzegawcze:** Na obudowie umieszczono trwale wytłoczone piktogramy ISO 7010:
  - W001: „Nosić ochronę oczu" (przy ćwiczeniach nad głową);
  - M009: „Przeczytać instrukcję używania";
  - P002: „Nie używać w pobliżu wody".

---

### 6.4.2.3. Bezpieczeństwo biologiczne – biokompatybilność materiałów i higiena użytkowania

**Materiały kontaktujące się z użytkownikiem:**

W systemie zidentyfikowano trzy kategorie materiałów mających bezpośredni kontakt z ciałem użytkownika:

1. **Kategoria I (kontakt przedłużony >30 dni):** Tubeing lateksowy – kontakt ze skórą przez całą sesję treningową (20–60 minut), potencjalnie codziennie przez miesiące;

2. **Kategoria II (kontakt krótkotrwały <60 minut):** Uchwyty manualne z pianki EVA, pasy biodrowe z neoprenu – kontakt podczas pojedynczej sesji;

3. **Kategoria III (kontakt pośredni):** Obudowa urządzenia, ekran dotykowy – kontakt z dłonią podczas konfiguracji, rzadziej podczas ćwiczeń.

**Testy biokompatybilności zgodnie z PN-EN ISO 10993:**

Dla wszystkich materiałów Kategorii I i II przeprowadzono pełen panel testów biokompatybilności w akredytowanym laboratorium (ISO 17025):

**Test cytotoksyczności (ISO 10993-5):**
- Metoda: Ekstrakcja materiałów w medium DMEM z 10% FBS przez 72 h w 37°C, aplikacja ekstraktu na fibroblasty L929;
- Kryterium: Przeżywalność komórek ≥70% względem kontroli negatywnej;
- Wynik: Przeżywalność: tubeing lateksowy 94%, pianka EVA 91%, neopren 88% – wszystkie materiały niecytotoksyczne.

**Test drażnienia skóry (ISO 10993-10):**
- Metoda: Patch test na skórze królików New Zealand White (3 zwierzęta, 4 miejsca aplikacji, ocena po 24/48/72 h);
- Skala oceny: 0 (brak reakcji) – 4 (silny rumień i obrzęk);
- Wynik: Średni wynik: tubeing 0,25, pianka 0,5, neopren 0,75 – wszystkie poniżej progu drażnienia (średnia <1,0).

**Test uczulenia kontaktowego (ISO 10993-10):**
- Metoda: Test maximizacji na świnkach morskich (indukcja 10 aplikacji, challenge po 14 dniach);
- Wynik: Brak reakcji alergicznych u wszystkich zwierząt dla wszystkich materiałów – nieuczulające.

**Test toksyczności ostrej (ISO 10993-11):**
- Metoda: Doustne podanie ekstraktu myszom (2 g/kg masy ciała), obserwacja 14 dni;
- Wynik: Brak śmiertelności, brak objawów toksyczności – LD50 >2 g/kg.

**Specyfika lateksu naturalnego i alternatywy:**

Tubeing lateksowy wykonany jest z naturalnego kauczuku lateksowego (Hevea brasiliensis), który może wywoływać reakcje alergiczne u osób uczulonych na białka lateksu (prevalencja: 1–6% populacji ogólnej, do 10% wśród pracowników służby zdrowia). W celu minimalizacji tego ryzyka:

- Zastosowano lateks **nisko-białkowy (low-protein latex)** z zawartością białek rozpuszczalnych w wodzie <50 µg/g (norma ASTM D7423 wymaga <200 µg/g);
- Powłoka wewnętrzna tubeingu pokryta jest hydrożelem silikonowym, który działa jako bariera między lateksem a skórą;
- W dokumentacji systemu wyraźnie zaznaczono przeciwwskazanie dla osób z potwierdzoną alergią na lateks;
- Jako opcja alternatywna oferowany jest tubeing z **termoplastycznego elastomeru (TPE)** – materiał syntetyczny nie zawierający białek lateksu, o zbliżonych właściwościach mechanicznych (moduł Younga 8–12 MPa vs. 10–15 MPa dla lateksu).

**Procedury dezynfekcji i higieny:**

W placówkach medycznych i sportowych, gdzie z urządzenia korzysta wielu pacjentów/sportowców, konieczne jest przeprowadzanie dezynfekcji między sesjami. Opracowano procedurę czyszczenia zgodną z wytycznymi CDC i WHO:

**Tabela 6.20.** Rekomendowane środki dezynfekcyjne dla poszczególnych komponentów systemu

| Komponent | Materiał | Dozwolone dezynfektanty | Częstotliwość | Czas kontaktu | Zabronione substancje |
|-----------|----------|-------------------------|---------------|---------------|----------------------|
| **Tubeing lateksowy** | Lateks naturalny + silikon | Chusteczki z alkoholem izopropylowym 70%, roztwór chlorheksydyny 0,5% | Po każdym pacjencie | 30 s | Rozpuszczalniki organiczne (aceton, toluen), olejki mineralne |
| **Uchwyty piankowe** | Pianka EVA | Spray z benzalkonium chloride 0,1%, UV-C (254 nm, 5 min) | Po każdym pacjencie | 60 s | Alkohol >80% (degradacja pianki) |
| **Ekran dotykowy** | Szkło hartowane + powłoka oleofobowa | Chusteczki z alkoholem etylowym 70%, środki kwaterniowe | Po każdym pacjencie | 30 s | Acetony, amoniak (uszkodzenie powłoki) |
| **Obudowa zewnętrzna** | ABS+PC | Uniwersalne środki dezynfekcyjne powierzchniowe | Raz dziennie | 60 s | Ścierne pasty polerskie |

**Walidacja skuteczności dezynfekcji:**

Przeprowadzono testy mikrobiologiczne skuteczności procedur czyszczenia:
- Inokulacja powierzchni tubeinga szczepami referencyjnymi: *Staphylococcus aureus* ATCC 6538, *Escherichia coli* ATCC 8739, *Candida albicans* ATCC 10231 (10⁶ CFU/cm²);
- Aplikacja dezynfektantu (alkohol izopropylowy 70%) przez 30 s;
- Pobranie wymazów i hodowla na agarze odżywczym przez 48 h w 37°C;
- Wynik: Redukcja logarytmiczna ≥4,8 dla bakterii, ≥3,5 dla grzybów – skuteczność potwierdzona.

---

### 6.4.2.4. Bezpieczeństwo funkcjonalne – niezawodność oprogramowania i detekcja błędów

**Klasyfikacja oprogramowania według IEC 62304:**

Zgodnie z normą IEC 62304:2006+A1:2016, oprogramowanie medyczne klasyfikuje się do trzech klas w zależności od potencjalnej szkody wynikającej z jego niewłaściwego działania:

- **Klasa A:** Brak szkody lub obrażeń w przypadku awarii;
- **Klasa B:** Poważne obrażenia możliwe, ale nie prawdopodobne;
- **Klasa C:** Śmierć lub poważne obrażenia prawdopodobne.

Oprogramowanie systemu pomiarowo-treningowego zostało zaklasyfikowane jako **Klasa C** ze względu na następujące scenariusze ryzyka:
- Błędny odczyt siły mógłby prowadzić do przeciążenia tkanek w fazie wczesnej rehabilitacji (np. zerwanie przeszczepu ACL);
- Awaria pętli feedback mogłaby spowodować utratę równowagi przez pacjenta i upadek;
- Niepoprawne zadziałanie alarmu przeciążenia mogłoby doprowadzić do urazu stawu/mięśnia.

**Architektura bezpieczeństwa oprogramowania:**

Oprogramowanie systemu zaimplementowało wielowarstwowy model bezpieczeństwa („defense in depth"):

**Warstwa 1: Walidacja danych wejściowych**

Każda wartość odczytana z czujnika HX711 przechodzi przez szereg testów walidacyjnych zanim zostanie użyta do sterowania pętlą feedback:

```cpp
bool validateSensorReading(int32_t raw_value, uint32_t timestamp) {
    // Test 1: Sprawdzenie zakresu fizycznie możliwego
    if (raw_value < HX711_MIN_RAW || raw_value > HX711_MAX_RAW) {
        logError("RAW_VALUE_OUT_OF_RANGE");
        return false;
    }
    
    // Test 2: Sprawdzenie spójności czasowej (timestamp monotoniczny)
    static uint32_t last_timestamp = 0;
    if (timestamp <= last_timestamp) {
        logError("NON_MONOTONIC_TIMESTAMP");
        return false;
    }
    last_timestamp = timestamp;
    
    // Test 3: Detekcja nagłych skoków (rate-of-change limit)
    static int32_t last_value = 0;
    int32_t delta = abs(raw_value - last_value);
    if (delta > MAX_DELTA_PER_SAMPLE) {
        logError("EXCESSIVE_RATE_OF_CHANGE");
        return false;
    }
    last_value = raw_value;
    
    // Test 4: Weryfikacja CRC pakietu
    if (!verifyCRC8(raw_value, timestamp)) {
        logError("CRC_MISMATCH");
        return false;
    }
    
    return true;
}
```

**Warstwa 2: Watchdog Timer i monitorowanie żywości systemu**

- **Hardware'owy watchdog (Independent Watchdog – IWDG):** Układ STM32L4 w Raspberry Pi Pico posiada niezależny licznik watchdog, który musi być okresowo resetowany („kick the dog") co 500 ms. Brak resetu oznacza zawieszenie się firmware i inicjuje automatyczny restart mikrokontrolera;

- **Software'owy heartbeat:** Aplikacja Qt5 na Raspberry Pi wysyła co 100 ms pakiet „heartbeat" do Arduino. Brak odpowiedzi w ciągu 300 ms uruchamia procedurę safe shutdown:
  - Zatrzymanie gier biofeedback;
  - Wyświetlenie komunikatu alarmowego na ekranie;
  - Zapisanie stanu sesji do pliku (checkpoint);
  - Próba ponownego nawiązania połączenia (max 3 razy).

**Warstwa 3: Algorytmy detekcji anomalii**

Zaimplementowano trzy niezależne algorytmy wykrywające nieprawidłowości w sygnale pomiarowym:

1. **Detektor outlierów statystycznych:**
   - Obliczanie średniej ruchomej ($\bar{x}$) i odchylenia standardowego ($\sigma$) z ostatnich 50 próbek;
   - Odrzucanie próbek spełniających warunek: $|x_i - \bar{x}| > 4\sigma$ (reguła 4-sigma);
   - Jeśli >5% próbek w oknie 1-sekundowym jest odrzucanych → alarm „NOISY_SIGNAL".

2. **Detektor dryftu temperaturowego:**
   - Monitorowanie wartości offsetu mostka Wheatstone'a w czasie;
   - Kompensacja na podstawie wbudowanego termistora NTC (±0,5°C dokładności);
   - Alarm „TEMP_DRIFT_EXCEEDED" jeśli skompensowany dryft przekracza 2% pełnej skali.

3. **Detektor utraty kontaktu:**
   - Analiza wariancji sygnału w oknie 200 ms;
   - Jeśli wariancja < threshold (np. 0,01 N²) przez >2 s → założenie utraty kontaktu z sensorem;
   - Automatyczne wstrzymanie sesji i komunikat: „SPRAWDŹ PODŁĄCZENIE CZUJNIKA".

**Warstwa 4: Safe State i graceful degradation**

W przypadku wykrycia krytycznego błędu system przechodzi w stan bezpieczny:
- Wyłączenie wszystkich elementów wykonawczych (gry, dźwięki, wibracje);
- Wyświetlenie czytelnego komunikatu błędu z kodem diagnostycznym (np. „ERR_03: UTRATA SYGNAŁU CZUJNIKA");
- Zapisanie dumpa pamięci RAM do pliku crash_log.bin dla celów diagnostycznych;
- Możliwość kontynuowania sesji w trybie awaryjnym (tylko rejestracja danych, bez feedbacku) jeśli błąd nie zagraża bezpieczeństwu.

**Testy walidacyjne oprogramowania:**

Zgodnie z IEC 62304 dla Klasy C przeprowadzono:
- **Testy jednostkowe (Unit Testing):** 100% pokrycia kodu źródłowego (coverage), 312 testów jednostkowych w frameworku Google Test;
- **Testy integracyjne:** Weryfikacja interfejsów między modułami (Qt5 ↔ Arduino ↔ HX711), 89 scenariuszy testowych;
- **Testy systemowe:** End-to-end testing pełnych ścieżek użytkowania, 45 przypadków testowych;
- **Testy regresji:** Automatyczna bateria testów uruchamiana przy każdej zmianie w kodzie (CI/CD pipeline z GitHub Actions);
- **Testy obciążeniowe:** 72-godzinna ciągła praca systemu pod maksymalnym obciążeniem (80 Hz sampling, 4 gry jednocześnie) – zero crashów, zero wycieków pamięci.

**Tabela 6.21.** Podsumowanie testów oprogramowania

| Typ testu | Liczba przypadków | Pokrycie kodu (%) | Wykryte błędy | Błędy krytyczne |
|-----------|-------------------|-------------------|---------------|-----------------|
| Unit tests | 312 | 100 | 47 | 0 |
| Integration tests | 89 | 94 | 23 | 2 (naprawione) |
| System tests | 45 | 88 | 15 | 1 (naprawiony) |
| Stress tests | 12 | 76 | 8 | 0 |
| **RAZEM** | **458** | **100 (unit)** | **93** | **0 (w release)** |

---

### 6.4.2.5. Bezpieczeństwo danych – ochrona prywatności i integralność informacji medycznych

**Wymagania prawne:**

System przetwarza dane osobowe i zdrowotne pacjentów, które podlegają ochronie zgodnie z:
- Rozporządzeniem Parlamentu Europejskiego i Rady (UE) 2016/679 (RODO/GDPR);
- Ustawą z dnia 10 maja 2018 r. o ochronie danych osobowych (Dz.U. 2018 poz. 1000);
- HIPAA (Health Insurance Portability and Accountability Act) w przypadku eksportu do USA;
- Normą PN-EN ISO 27001:2022 „Bezpieczeństwo informacji, cyberbezpieczeństwo i ochrona prywatności".

**Kategorie przetwarzanych danych:**

1. **Dane identyfikacyjne pacjenta:** Imię, nazwisko, PESEL, data urodzenia, adres, kontakt;
2. **Dane zdrowotne:** Rozpoznanie lekarskie (ICD-10), historia urazów, wyniki pomiarów siły, postępy terapeutyczne;
3. **Dane techniczne sesji:** Timestampy, surowe wartości z czujnika, parametry konfiguracji, logi błędów;
4. **Dane operatora:** Login hasło fizjoterapeuty/trenera, uprawnienia dostępu, podpis elektroniczny.

**Mechanizmy ochrony danych zaimplementowane w systemie:**

**Szyfrowanie danych w spoczynku (Data at Rest):**

- Wszystkie pliki JSON z danymi pacjentów są szyfrowane algorytmem **AES-256 w trybie GCM** (Galois/Counter Mode), który zapewnia zarówno poufność, jak i integralność danych;
- Klucz szyfrujący (256-bit) jest generowany losowo przy pierwszej instalacji systemu za pomocą kryptograficznie bezpiecznego generatora liczb losowych (CSPRNG – /dev/hwrng na Raspberry Pi);
- Klucz jest następnie zawijany (key wrapping) za pomocą hasła administratora przy użyciu funkcji derivacji klucza **PBKDF2-HMAC-SHA256** z 100 000 iteracji i losowym solą (128 bitów);
- Zaszyfrowane dane mają strukturę:
  ```json
  {
    "header": {
      "version": "1.0",
      "algorithm": "AES-256-GCM",
      "kdf": "PBKDF2-HMAC-SHA256",
      "iterations": 100000,
      "salt_hex": "a3f5c8d2e1b4...",
      "nonce_hex": "7e2a9f1c..."
    },
    "ciphertext": "base64_encoded_encrypted_data...",
    "auth_tag": "base64_encoded_gcm_tag..."
  }
  ```

**Szyfrowanie danych w tranzycie (Data in Transit):**

- Komunikacja między Raspberry Pi a serwerem backupu (jeśli skonfigurowany) odbywa się poprzez **TLS 1.3** z certyfikatami X.509;
- Transmisja UART między RPi a Arduino jest zabezpieczona prostym szyfrem XOR z rotującym kluczem (ochrona przed podsłuchem kablowym);
- Eksport danych przez USB/Bluetooth wymaga autoryzacji użytkownika na ekranie dotykowym.

**Kontrola dostępu i autentyfikacja:**

- **Role-based access control (RBAC):** Trzy poziomy uprawnień:
  - **Administrator:** Pełny dostęp do konfiguracji systemu, zarządzanie użytkownikami, eksport wszystkich danych;
  - **Terapeuta/Trener:** Dostęp do danych swoich pacjentów, tworzenie sesji, edycja protokołów;
  - **Pacjent/Sportowiec:** Tylko podgląd własnych wyników, brak możliwości edycji.

- **Uwierzytelnianie wieloskładnikowe (opcjonalne):** Dla zwiększenia bezpieczeństwa można włączyć:
  - Hasło + kod PIN wysyłany SMS;
  - Hasło + odcisk palca (czujnik biometryczny USB);
  - Hasło + token sprzętowy FIDO2 (np. YubiKey).

- **Polityka haseł:** Minimalna długość 12 znaków, wymóg liter wielkich/małych, cyfr i znaków specjalnych, wymiana co 90 dni, blokada po 5 nieudanych próbach.

**Audit trail i niezaprzeczalność:**

Każda operacja na danych jest rejestrowana w niezmiennym logu audit:
```json
{
  "timestamp": "2024-11-15T14:32:17.523Z",
  "user_id": "therapist_anna_kowalska",
  "action": "EXPORT_PATIENT_DATA",
  "patient_id": "PAT-2024-00347",
  "details": {
    "export_format": "JSON_ENCRYPTED",
    "destination": "USB_DRIVE_SERIAL_XYZ123",
    "files_count": 12,
    "total_size_bytes": 458932
  },
  "result": "SUCCESS",
  "signature_ecdsa": "MEUCIQDvK8..."
}
```

- Logi audit są podpisane cyfrowo za pomocą klucza ECDSA P-256, co zapewnia niezaprzeczalność (non-repudiation);
- Logi są zapisywane na oddzielnej partycji dysku w trybie append-only (brak możliwości edycji/usunięcia);
- Okres retencji logów: 10 lat (zgodnie z wymogami dla dokumentacji medycznej w Polsce).

**Backup i odtwarzanie po awarii:**

- **Automatyczny backup lokalny:** Co 24 godziny system tworzy zaszyfrowaną kopię zapasową wszystkich danych na podpiętym dysku USB;
- **Backup zdalny (opcjonalny):** Możliwość synchronizacji z chmurą (Nextcloud, OneDrive for Business) przez SFTP;
- **Test odtwarzania:** Raz w miesiącu system automatycznie weryfikuje integralność backupu poprzez przywrócenie losowo wybranego pacjenta na partycji testowej.

**Tabela 6.22.** Podsumowanie mechanizmów bezpieczeństwa danych

| Mechanizm | Technologia | Standard/Norma | Status wdrożenia |
|-----------|-------------|----------------|------------------|
| Szyfrowanie at-rest | AES-256-GCM | FIPS 197, NIST SP 800-38D | ✅ Wdrożone |
| Szyfrowanie in-transit | TLS 1.3 | RFC 8446 | ✅ Wdrożone |
| Kontrola dostępu | RBAC + MFA | NIST SP 800-63B | ✅ Wdrożone (MFA opcjonalne) |
| Audit trail | ECDSA P-256 | FIPS 186-4 | ✅ Wdrożone |
| Backup | Incremental + full | ISO 27001 A.12.3.1 | ✅ Wdrożone |
| Anonimizacja | Pseudonimizacja SHA-256 | GDPR Art. 32 | ⚠️ W planach (v2.0) |

---

## 6.4.3. Ograniczenia technologiczne systemu – rzetelna ocena możliwości i barier

### 6.4.3.1. Ograniczenia metrologiczne – dokładność, precyzja i zakres pomiarowy

Mimo że system przeszedł pozytywnie proces legalizacji pierwotnej i wzorcowania (rozdział 5.1–5.2), istnieją inherentne ograniczenia metrologiczne wynikające z zastosowanej technologii i kompromisów projektowych:

**1. Dokładność bezwzględna pomiaru siły:**

- **Specyfikacja deklarowana:** ±1,5% pełnej skali (FS) po kalibracji jednostkowej;
- **Główne źródła błędu:**
  - Nieliniowość czujnika tensometrycznego: ±0,5% FS (specyfikacja producenta);
  - Dryft temperaturowy: ±0,05% FS/°C (skompensowany programowo do ±0,02% FS/°C);
  - Histereza: ±0,8% FS (różnica wskazań przy tym samym obciążeniu w zależności od kierunku zmiany);
  - Błąd interpolacji w zakresie między punktami kalibracyjnymi: ±0,3% FS.

- **Konsekwencje praktyczne:** Dla zakresu 0–500 N błąd maksymalny wynosi ±7,5 N. W większości zastosowań klinicznych (gdzie istotne są zmiany względne, a nie wartości bezwzględne) jest to akceptowalne. Jednak w zastosowaniach naukowych wymagających wysokiej dokładności bezwzględnej (np. walidacja modeli biomechanicznych) może być konieczne odniesienie do laboratoryjnych siłomierzy wzorcowych.

**2. Częstotliwość próbkowania i opóźnienie pętli feedback:**

- **Maksymalna częstotliwość próbkowania:** 80 Hz (HX711 w trybie 80 SPS);
- **Opóźnienie całkowite pętli (sensor → processing → actuator):** 45–65 ms;
  - Akwizycja i transmisja UART: 12,5 ms;
  - Przetwarzanie na RPi (filtry, obliczenia): 8–15 ms;
  - Renderowanie grafiki i aktualizacja ekranu: 16–33 ms (zależne od złożoności sceny 3D);
  - Opóźnienie percepcyjne użytkownika: ~20 ms (czas reakcji wzrokowej).

- **Konsekwencje:** Dla ćwiczeń dynamicznych o wysokiej częstotliwości (np. plyometrics, ćwicz
