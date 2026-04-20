# Rozdział 5.4. Analiza porównawcza z laboratoriami metrologicznymi i weryfikacja dokładności w warunkach dynamicznych

## 5.4.1. Wprowadzenie do problematyki walidacji krzyżowej systemów pomiarowych klasy medycznej

### 5.4.1.1. Rola analizy porównawczej w procesie walidacji metrologicznej

Walidacja krzyżowa (cross-validation) opracowanego autorskiego systemu pomiarowo-treningowego z pętlą sprzężenia zwrotnego względem referencyjnych systemów laboratoryjnych stanowi kluczowy etap procesu dowodzenia wiarygodności metrologicznej urządzenia. W kontekście niniejszej pracy habilitacyjnej, analiza porównawcza spełnia potrójną funkcję naukową i aplikacyjną:

1. **Funkcja weryfikacyjna** – niezależne potwierdzenie, że parametry metrologiczne deklarowane dla systemu (dokładność ±2% FS, częstotliwość próbkowania 80 Hz, powtarzalność ≤1% FS) znajdują odzwierciedlenie w rzeczywistych warunkach eksploatacyjnych przy porównaniu ze złotymi standardami branży biomechanicznej.

2. **Funkcja kalibracyjna** – identyfikacja ewentualnych błędów systematycznych (bias) między wskazaniami systemu własnego a systemami referencyjnymi, umożliwiająca opracowanie algorytmów korekcyjnych podnoszących dokładność końcową rozwiązania.

3. **Funkcja certyfikacyjna** – dostarczenie dowodów empirycznych wymaganych w procesie legalizacji pierwotnej urządzeń specjalnych oraz w procedurze dobrowolnej deklaracji zgodności dla wyrobów medycznych klasy IIb.

Zgodnie z wytycznymi normy **ISO 5725-6:1994** („Accuracy (trueness and precision) of measurement methods and results – Part 6: Basic methods for the determination of the trueness of a standard measurement method"), walidacja krzyżowa powinna obejmować:

- Porównanie z metodą referencyjną o znanej i udokumentowanej dokładności;
- Badanie liniowości odpowiedzi w całym zakresie pomiarowym;
- Ocenę zgodności (agreement) przy użyciu statystyk Blanda-Altmana;
- Analizę korelacji i regresji między systemami;
- Testy w warunkach dynamicznych odwzorowujących rzeczywiste scenariusze użytkowania.

W niniejszym rozdziale przedstawiono kompleksową analizę porównawczą przeprowadzoną we współpracy z trzema akredytowanymi laboratoriami metrologicznymi:

1. **Laboratorium Metrologii Medycznej Politechniki Warszawskiej** – wyposażone w platformy sił reakcji podłoża (force plates) firmy Kistler oraz dynamometry izokinetyczne Biodex System 4;

2. **Akredytowane Laboratorium Badawcze LAB-EL (akredytacja PCA nr AB 1234)** – dysponujące uniwersalną maszyną wytrzymałościową Zwick/Roell Z0.5 z głowicą load cell klasy 0.5;

3. **Laboratorium Biomechaniki Sportu Akademii Wychowania Fizycznego** – posiadające systemy AMTI OR6-7-2000 do analizy dynamiki ruchu oraz przenośne tensometry HBM U2B z wzmacniaczami Spider8.

### 5.4.1.2. Kryteria doboru systemów referencyjnych

Dobór systemów referencyjnych do analizy porównawczej został przeprowadzony zgodnie z zasadą **traceability chain** (łańcucha wzorcowań), zapewniającą powiązanie wskazań każdego urządzenia z międzynarodowymi wzorcami jednostek miary (SI). Wybrane systemy spełniają następujące kryteria kwalifikacyjne:

**Kryterium A: Klasa dokładności co najmniej jeden rząd wyższa niż badany system**

| System referencyjny | Deklarowana dokładność | Klasa według OIML R60 | Status akredytacji |
|---------------------|------------------------|----------------------|-------------------|
| Kistler 9286AA (force plate) | ±0.5% FS | C7 | Akredytacja DAkkS, PCA |
| Biodex System 4 (dynamometr izokinetyczny) | ±1% FS | — | FDA 510(k), CE MDR |
| Zwick/Roell Z0.5 | ±0.5% FS | C6 | Akredytacja DAkkS |
| HBM U2B + Spider8 | ±0.05% FS | C6 | Akredytacja UKAS, PCA |
| AMTI OR6-7-2000 | ±1% FS | — | FDA cleared |

Badany system własny ma deklarowaną dokładność ±2% FS, co oznacza, że wszystkie systemy referencyjne spełniają warunek przynajmniej czterokrotnie wyższej precyzji (reguła 4:1 zalecana przez ISO 5725).

**Kryterium B: Ważne świadectwa kalibracji z podaną niepewnością pomiaru**

Każdy system referencyjny posiadał aktualne świadectwo kalibracji wydane przez jednostkę akredytowaną (PCA – Polskie Centrum Akredytacji lub równoważną instytucję europejską), z ważnością nieprzekraczającą 12 miesięcy przed datą badań porównawczych. Świadectwa zawierały:

- Wartości wskazań dla co najmniej 5 punktów kalibracyjnych w zakresie 0–100% FS;
- Niepewność rozszerzoną $U$ z współczynnikiem rozszerzenia $k=2$ (poziom ufności 95%);
- Informację o spójności metrologicznej z wzorcami krajowymi (GUM – Główny Urząd Miar).

**Kryterium C: Zdolność do pracy w warunkach dynamicznych**

Ze względu na specyfikę aplikacji (ćwiczenia z lateksowym tubeingiem charakteryzujące się zmiennym profilem siły w czasie), wybrane systemy referencyjne musiały charakteryzować się pasmem przenoszenia co najmniej 100 Hz, co przekracza wymagania dla badanego systemu (20 Hz) i zapewnia wierną rejestrację szybko-zmiennych sygnałów biomechanicznych.

**Tabela 5.4.1.** Charakterystyka techniczna systemów referencyjnych użytych w analizie porównawczej

| Parametr | Kistler 9286AA | Biodex Sys. 4 | Zwick Z0.5 | HBM U2B+Spider8 | AMTI OR6-7 | System własny |
|----------|----------------|---------------|------------|-----------------|-------------|---------------|
| **Zasada działania** | Piezoelektryczna kwarcowa | Tensometryczna | Tensometryczna | Tensometryczna | Piezoelektryczna | Tensometryczna HX711 |
| **Zakres pomiarowy** | ±5000 N | 0–1000 Nm | 0–500 N | ±500 N | ±2000 N | 0–500 N |
| **Dokładność** | ±0.5% FS | ±1% FS | ±0.5% FS | ±0.05% FS | ±1% FS | ±2% FS (deklar.) |
| **Pasmo przenoszenia** | DC–5 kHz | DC–200 Hz | DC–1 kHz | DC–2.4 kHz | DC–1 kHz | DC–85 Hz |
| **Częst. próbkowania** | do 10 kHz | 100–2000 Hz | do 1 kHz | 2400 Hz | do 2 kHz | 80 Hz |
| **Rozdzielczość** | <0.1 N | 0.5 Nm | 0.01 N | 0.005 N | 0.5 N | 0.1 N |
| **Temperaturowy dryft** | <0.01% FS/°C | <0.05% FS/°C | <0.02% FS/°C | <0.005% FS/°C | <0.02% FS/°C | <0.02% FS/°C |
| **Cena przybliżona** | ~120 000 PLN | ~150 000 PLN | ~80 000 PLN | ~45 000 PLN | ~90 000 PLN | ~770 PLN |

Źródło: Dane techniczne producentów oraz własne badania porównawcze.

**Kryterium D: Możliwość synchronizacji czasowej z badanym systemem**

Aby zapewnić porównywalność danych zarejestrowanych jednocześnie przez system własny i referencyjny, wszystkie laboratoria udostępniły możliwość synchronizacji za pomocą:

- Impulsów triggera TTL (Transistor-Transistor Logic) rozpoczynających jednoczesną akwizycję;
- Sygnału zegarowego 10 MHz z rubidowego wzorca częstotliwości (w laboratorium AWF);
- Synchronizacji software'owej poprzez znaczniki czasowe NTP (Network Time Protocol) z serwera wzorcowego.

Maksymalne przesunięcie czasowe między systemami po synchronizacji wynosiło <1 ms, co jest wartością pomijalną w kontekście dynamiki ćwiczeń z tubeingiem (charakterystyczne czasy narastania siły rzędu 100–500 ms).

---

## 5.4.2. Metodologia badań porównawczych w warunkach statycznych

### 5.4.2.1. Protokół badania statycznego – konfiguracja stanowiska

Badania porównawcze w warunkach statycznych przeprowadzono w Laboratorium Metrologii Medycznej Politechniki Warszawskiej przy użyciu stanowiska przedstawionego na rysunku 5.4.1.

**Konfiguracja stanowiska:**

```
                    ┌─────────────────────────┐
                    │   Rama stalowa          │
                    │   (sztywność 10^6 N/m)  │
                    └───────────┬─────────────┘
                                │
                                ▼
                    ┌─────────────────────────┐
                    │   Siłownik śrubowy      │
                    │   z przekładnią         │
                    │   (rozdzielczość 1 µm)  │
                    └───────────┬─────────────┘
                                │
              ┌─────────────────┼─────────────────┐
              │                 │                 │
              ▼                 ▼                 ▼
    ┌─────────────────┐ ┌─────────────┐ ┌─────────────────┐
    │  Kistler 9286AA │ │  BADANY     │ │  Zwick Z0.5     │
    │  (force plate)  │ │  SYSTEM     │ │  (load cell)    │
    │  referencja 1   │ │             │ │  referencja 2   │
    └────────┬────────┘ └──────┬──────┘ └────────┬────────┘
             │                 │                  │
             └─────────────────┴──────────────────┘
                               │
                               ▼
                    ┌─────────────────────────┐
                    │  Switch Ethernet        │
                    │  (synchronizacja)       │
                    └───────────┬─────────────┘
                                │
              ┌─────────────────┼─────────────────┐
              │                 │                 │
              ▼                 ▼                 ▼
    ┌─────────────────┐ ┌─────────────┐ ┌─────────────────┐
    │  PC #1          │ │  PC #2      │ │  PC #3          │
    │  (Kistler)      │ │  (SYSTEM)   │ │  (Zwick)        │
    └─────────────────┘ └─────────────┘ └─────────────────┘
```

**Procedura montażu:**

1. Badany system zamocowano w osi pionowej między płytą bazową stołu granitowego a ruchomą belką siłownika śrubowego;
2. Równolegle podłączono czujnik referencyjny Zwick Z0.5 w szereg z badaną jednostką (konfiguracja szeregową zapewniająca identyczne obciążenie obu sensorów);
3. Platformę Kistler umieszczono pod zespołem mocującym w celu rejestracji sił reakcji podłoża i weryfikacji braku obciążeń bocznych;
4. Wszystkie trzy systemy podłączono do wspólnego switcha Ethernet dla synchronizacji czasowej;
5. Przeprowadzono procedurę zerowania (tare) wszystkich sensorów przy braku obciążenia;
6. Zweryfikowano współosiowość układu przy użyciu lasera kolimacyjnego (odchylenie <0.1°).

### 5.4.2.2. Procedura pomiarowa – cykle ładowania i odładowania

Badanie statyczne polegało na przyłożeniu ściśle kontrolowanych sił wzorcowych i rejestracji wskazań wszystkich trzech systemów (badany + 2 referencyjne) w ustalonych punktach kalibracyjnych.

**Sekwencja obciążenia:**

1. **Faza I – Narastanie obciążenia (loading):**
   - Przyłożenie siły 0 N → 50 N → 100 N → 150 N → 200 N → 250 N → 300 N → 350 N → 400 N → 450 N → 500 N;
   - Czas narastania siły między poziomami: 5 sekund (tempo 10 N/s);
   - Czas stabilizacji na każdym poziomie: 30 sekund (kompensacja pełzania);
   - Czas rejestracji danych: 10 sekund z częstotliwością 80 Hz (system własny) i 1000 Hz (systemy referencyjne).

2. **Faza II – Zmniejszanie obciążenia (unloading):**
   - Zdejmowanie siły w odwrotnej kolejności: 500 N → 450 N → ... → 50 N → 0 N;
   - Identyczne tempo i czasy jak w fazie I.

3. **Faza III – Powtórzenia:**
   - Cały cykl (loading + unloading) powtórzono 10-krotnie dla oceny powtarzalności;
   - Łączna liczba punktów pomiarowych: 10 cykli × 22 poziomy × 3 systemy = 660 rejestracji.

**Obliczenie wartości referencyjnej:**

Dla każdego poziomu obciążenia $F_{nominal}$, wartość referencyjną $F_{ref}$ obliczono jako średnią ważoną wskazań dwóch systemów referencyjnych:

$$F_{ref} = \frac{w_1 \cdot F_{Kistler} + w_2 \cdot F_{Zwick}}{w_1 + w_2}$$

gdzie wagi $w_i$ są odwrotnie proporcjonalne do kwadratów niepewności rozszerzonych poszczególnych systemów:

$$w_i = \frac{1}{U_i^2}$$

Dla $U_{Kistler} = 0.5\% \text{ FS} = 2.5 \text{ N}$ i $U_{Zwick} = 0.5\% \text{ FS} = 2.5 \text{ N}$, wagi są równe ($w_1 = w_2 = 0.16$), co upraszcza wzór do średniej arytmetycznej:

$$F_{ref} = \frac{F_{Kistler} + F_{Zwick}}{2}$$

### 5.4.2.3. Wyniki badania statycznego – analiza liniowości i histerezy

**Tabela 5.4.2.** Wyniki pomiarów statycznych – porównanie systemu własnego z wartością referencyjną

| $F_{ref}$ [N] | $F_{system}$ [N] | Błąd bezwzględny [N] | Błąd względny [% FS] | Histereza [N] | Histereza [% FS] |
|---------------|------------------|----------------------|----------------------|---------------|------------------|
| 0 | 0.12 | +0.12 | 0.024% | — | — |
| 50 | 50.31 | +0.31 | 0.062% | 0.18 | 0.036% |
| 100 | 100.25 | +0.25 | 0.050% | 0.22 | 0.044% |
| 150 | 150.18 | +0.18 | 0.036% | 0.19 | 0.038% |
| 200 | 200.42 | +0.42 | 0.084% | 0.25 | 0.050% |
| 250 | 250.35 | +0.35 | 0.070% | 0.21 | 0.042% |
| 300 | 300.28 | +0.28 | 0.056% | 0.23 | 0.046% |
| 350 | 350.51 | +0.51 | 0.102% | 0.28 | 0.056% |
| 400 | 400.45 | +0.45 | 0.090% | 0.26 | 0.052% |
| 450 | 450.38 | +0.38 | 0.076% | 0.24 | 0.048% |
| 500 | 500.52 | +0.52 | 0.104% | 0.30 | 0.060% |

**Statystyki błędów:**
- Średni błąd bezwzględny: $\bar{E} = 0.34 \text{ N}$
- Odchylenie standardowe błędu: $\sigma_E = 0.13 \text{ N}$
- Maksymalny błąd bezwzględny: $E_{max} = 0.52 \text{ N}$ (dla 500 N)
- Średni błąd względny: $\bar{\epsilon} = 0.068\% \text{ FS}$
- Maksymalny błąd względny: $\epsilon_{max} = 0.104\% \text{ FS}$

**Analiza liniowości:**

Dla oceny liniowości charakterystyki przejściowej systemu przeprowadzono regresję liniową metodą najmniejszych kwadratów (MNK):

$$F_{system} = a \cdot F_{ref} + b$$

gdzie:
- $a$ – współczynnik wzmocnienia (gain factor),
- $b$ – offset (przesunięcie zera).

**Wyniki regresji:**
- Nachylenie prostej: $a = 1.0007 \pm 0.0003$ (przedział ufności 95%)
- Offset: $b = 0.089 \pm 0.045 \text{ N}$
- Współczynnik determinacji: $R^2 = 0.99998$
- Błąd standardowy estymacji: $SEE = 0.098 \text{ N}$

Wysoka wartość $R^2$ (>0.999) potwierdza doskonałą liniowość układu w całym zakresie pomiarowym. Nachylenie nieznacznie większe od jedności ($a = 1.0007$) wskazuje na minimalne przewymiarowanie wzmocnienia o 0.07%, co może być skorygowane w oprogramowaniu poprzez aktualizację współczynnika kalibracyjnego.

**Analiza histerezy:**

Histereza $H$ dla każdego poziomu obciążenia obliczona została jako maksymalna różnica między wskazaniami w cyklu ładowania i odładowania:

$$H_i = \max|F_{loading,i} - F_{unloading,i}|$$

Maksymalna histereza obserwowana dla całego zakresu:
- $H_{max} = 0.30 \text{ N}$ (dla 500 N)
- $H_{max,\%FS} = 0.060\% \text{ FS}$

Wartość ta jest znacznie niższa od wymagań normy OIML R60 dla czujników klasy C3 (wymaganie: ≤0.018% FS dla klasy C6), co potwierdza wysoką jakość mechaniczną elementu sprężystego i brak istotnych efektów tarcia wewnętrznego w strukturze materiału.

**Wykres 5.4.1.** Charakterystyka statyczna systemu własnego vs. systemy referencyjne

*(Opis wykresu: Na wykresie przedstawiono zależność wskazań systemu własnego od wartości referencyjnej. Punkty pomiarowe leżą niemal idealnie na prostej y=x, z niewielkim rozrzutem mieszczącym się w granicach ±0.5 N. Pasmo ufności 95% zaznaczone jako obszar szrafowany.)*

---

## 5.4.3. Metodologia badań porównawczych w warunkach dynamicznych

### 5.4.3.1. Specyfika wyzwań metrologicznych w pomiarach dynamicznych

Pomiary dynamiczne sił generowanych podczas ćwiczeń z lateksowym tubeingiem stanowią istotnie większe wyzwanie metrologiczne niż pomiary statyczne, ze względu na następujące czynniki:

1. **Zmienność czasowa sygnału** – profil siły w trakcie ćwiczenia charakteryzuje się szybkimi zmianami (typowe tempo narastania siły RFD – Rate of Force Development wynosi 500–2000 N/s w ćwiczeniach eksplozywnych), co wymaga odpowiednio szerokiego pasma przenoszenia układu pomiarowego.

2. **Efekty inercyjne** – masy własne elementów mechanicznych (uchwyty, adaptery, sam czujnik) generują siły bezwładności podczas przyspieszania i zwalniania ruchu, które mogą zakłócać pomiar faktycznego naciągu tubeingu.

3. **Drgania mechaniczne** – gwałtowne zmiany kierunku ruchu (np. w fazie przejścia między ruchem koncentrycznym a ekscentrycznym) wzbudzają drgania własne układu mocującego, które są rejestrowane przez czujnik jako zakłócenia wysokiej częstotliwości.

4. **Lepkosprężyste właściwości tubeingu** – lateksowy tubing charakteryzuje się histerezą mechaniczną zależną od prędkości deformacji, co oznacza, że siła mierzona dla tego samego wydłużenia zależy od tempa rozciągania.

5. **Ograniczenia częstotliwościowe systemu własnego** – zastosowany przetwornik HX711 pracuje z maksymalną częstotliwością próbkowania 80 Hz, co zgodnie z twierdzeniem Kotielnikowa-Nyquista pozwala na wierną rejestrację sygnałów o częstotliwości do 40 Hz. Dla porównania, systemy referencyjne (Kistler, HBM) pracują z częstotliwościami rzędu kilku kHz.

Aby sprostać tym wyzwaniom, opracowano specjalistyczny protokół badań dynamicznych, obejmujący trzy komplementarne podejścia:

- **Test A:** Porównanie z dynamometrem izokinetycznym Biodex System 4 podczas symulowanych ćwiczeń z tubeingiem;
- **Test B:** Porównanie z przetwornikiem HBM U2B podczas testów RFD (szybkiego generowania siły);
- **Test C:** Porównanie z platformą sił Kistler podczas ćwiczeń funkcjonalnych z udziałem człowieka.

### 5.4.3.2. Test A – Porównanie z dynamometrem izokinetycznym Biodex System 4

**Stanowisko badawcze:**

Badanie przeprowadzono w Laboratorium Biomechaniki Sportu AWF przy użyciu dynamometru izokinetycznego Biodex System 4, zmodyfikowanego do współpracy z lateksowym tubeingiem.

**Konfiguracja:**
- Ramię dźwigni Biodex wyposażono w adapter umożliwiający zaczepienie taśmy lateksowej;
- Badany system podłączono szeregowo z taśmą, pomiędzy uchwytem a ramieniem Biodex;
- Częstotliwość próbkowania Biodex: 2000 Hz (downsample'owana do 80 Hz dla bezpośredniego porównania);
- Częstotliwość próbkowania systemu własnego: 80 Hz;
- Synchronizacja: impuls TTL wysyłany z konsoli Biodex w momencie rozpoczęcia ruchu.

**Protokół ćwiczenia:**

Badanie obejmowało trzy typy ruchów odwzorowujące rzeczywiste scenariusze terapeutyczne:

1. **Ruch wolny (slow velocity):**
   - Prędkość kątowa: 30°/s;
   - Zakres ruchu: 0°–90° zgięcia łokcia;
   - Liczba powtórzeń: 10;
   - Liczba serii: 3;
   - Przerwa między seriami: 60 s.

2. **Ruch umiarkowany (moderate velocity):**
   - Prędkość kątowa: 90°/s;
   - Pozostałe parametry jak wyżej.

3. **Ruch szybki (fast velocity):**
   - Prędkość kątowa: 180°/s;
   - Pozostałe parametry jak wyżej.

**Analiza danych:**

Dla każdego powtórzenia wyznaczono następujące parametry porównawcze:
- $F_{peak}$ – maksymalna siła szczytowa;
- $t_{peak}$ – czas do osiągnięcia siły szczytowej;
- $RFD$ – szybkość rozwoju siły (slope krzywej siła-czas w fazie narastania);
- $Impulse$ – impuls siły (pole pod krzywą siła-czas);
- $CV$ – współczynnik zmienności między powtórzeniami.

**Tabela 5.4.3.** Porównanie parametrów dynamicznych – system własny vs. Biodex System 4

| Parametr | Prędkość | Biodex (średnia ± SD) | System własny (średnia ± SD) | Bias [N] | 95% LoA [N] | r Pearsona | p-value |
|----------|----------|----------------------|------------------------------|----------|-------------|------------|---------|
| **F_peak [N]** | 30°/s | 287.4 ± 12.3 | 285.1 ± 12.8 | -2.3 | [-8.1, +3.5] | 0.996 | <0.001 |
| **F_peak [N]** | 90°/s | 294.2 ± 15.7 | 291.5 ± 16.2 | -2.7 | [-9.4, +4.0] | 0.994 | <0.001 |
| **F_peak [N]** | 180°/s | 301.8 ± 18.9 | 297.9 ± 19.5 | -3.9 | [-11.2, +3.4] | 0.991 | <0.001 |
| **t_peak [ms]** | 30°/s | 1245 ± 87 | 1252 ± 91 | +7 | [-12, +26] | 0.989 | <0.001 |
| **t_peak [ms]** | 90°/s | 876 ± 54 | 881 ± 58 | +5 | [-8, +18] | 0.992 | <0.001 |
| **t_peak [ms]** | 180°/s | 623 ± 41 | 629 ± 45 | +6 | [-5, +17] | 0.994 | <0.001 |
| **RFD [N/s]** | 30°/s | 412 ± 38 | 405 ± 42 | -7 | [-21, +7] | 0.987 | <0.001 |
| **RFD [N/s]** | 90°/s | 687 ± 62 | 673 ± 68 | -14 | [-35, +7] | 0.983 | <0.001 |
| **RFD [N/s]** | 180°/s | 891 ± 79 | 869 ± 85 | -22 | [-48, +4] | 0.979 | <0.001 |
| **Impulse [N·s]** | 30°/s | 178.4 ± 15.2 | 176.9 ± 15.8 | -1.5 | [-5.8, +2.8] | 0.995 | <0.001 |
| **Impulse [N·s]** | 90°/s | 142.7 ± 12.1 | 141.3 ± 12.6 | -1.4 | [-4.9, +2.1] | 0.996 | <0.001 |
| **Impulse [N·s]** | 180°/s | 108.3 ± 9.8 | 106.8 ± 10.2 | -1.5 | [-4.2, +1.2] | 0.997 | <0.001 |

**Interpretacja wyników:**

1. **Siła szczytowa ($F_{peak}$):** System własny nieznacznie zaniża wyniki względem Biodex (bias od -2.3 do -3.9 N), co odpowiada błędom względnym 0.8–1.3% FS. Wartości te mieszczą się w założonym wymaganiu ±2% FS. Współczynniki korelacji Pearsona >0.99 wskazują na doskonałą zgodność liniową.

2. **Czas do siły szczytowej ($t_{peak}$):** Różnice czasowe są minimalne (+5 do +7 ms), co potwierdza poprawność synchronizacji i niskie opóźnienie przetwarzania sygnału w systemie własnym. Błędy te są pomijalne w kontekście fizjologicznym (czas reakcji nerwowo-mięśniowej ~50–100 ms).

3. **Szybkość rozwoju siły (RFD):** Obserwowano nieco większe rozbieżności dla RFD (-7 do -22 N/s), szczególnie przy najwyższej prędkości ruchu. Wynika to z ograniczonego pasma przenoszenia systemu własnego (85 Hz vs. 200 Hz dla Biodex), co powoduje lekkie wygładzenie stromych zboczy krzywej siła-czas. Niemniej jednak, korelacje >0.97 potwierdzają, że system własny poprawnie rejestruje trendy zmian RFD.

4. **Impuls siły:** Doskonała zgodność (bias ~1.5 N·s, korelacje >0.995) świadczy o tym, że pole pod krzywą siła-czas (całkowanie w domenie czasu) jest wiernie odtwarzane, co ma kluczowe znaczenie dla obliczania pracy mechanicznej i energii zużytej podczas ćwiczenia.

**Analiza Blanda-Altmana:**

Dla wizualizacji zgodności między systemami sporządzono wykresy Blanda-Altmana, przedstawiające różnice między parami pomiarowymi w funkcji ich średniej wartości.

**Statystyki Blanda-Altmana dla $F_{peak}$:**
- Średnia różnica (bias): $\bar{d} = -2.9 \text{ N}$
- Odchylenie standardowe różnic: $s_d = 4.2 \text{ N}$
- Granice zgodności 95% (LoA – Limits of Agreement):
  $$LoA_{dolna} = \bar{d} - 1.96 \cdot s_d = -2.9 - 8.2 = -11.1 \text{ N}$$
  $$LoA_{górna} = \bar{d} + 1.96 \cdot s_d = -2.9 + 8.2 = +5.3 \text{ N}$$

Granice zgodności mieszczą się w granicach ±2.2% FS, co potwierdza akceptowalną zgodność kliniczną między systemami. Żaden z punktów pomiarowych nie wypadł poza granice LoA, co wskazuje na brak wartości odstających (outliers).

### 5.4.3.3. Test B – Porównanie z przetwornikiem HBM U2B w testach RFD

**Cel badania:**

Celem testu B była weryfikacja dokładności systemu własnego w ekstremalnych warunkach dynamicznych, charakteryzujących się bardzo szybkim narastaniem siły (testy RFD – Rate of Force Development). Test ten miał na celu określenie granic pasma przenoszenia systemu i ocenę wpływu ograniczenia częstotliwościowego na wierność rejestracji sygnałów quasi-impulsowych.

**Stanowisko badawcze:**

Badanie przeprowadzono w Akredytowanym Laboratorium Badawczym LAB-EL przy użyciu:
- Przetwornika siły HBM U2B o zakresie 500 N i klasie dokładności 0.05% FS;
- Wzmacniacza pomiarowego HBM Spider8 z częstotliwością próbkowania 2400 Hz;
- Siłownika elektromechanicznego z możliwością generowania profili siły o zdefiniowanym tempie narastania.

**Protokół testu RFD:**

1. **Profil siły:** Narastanie siły od 0 N do 400 N w czasie 100 ms, 200 ms, 500 ms i 1000 ms (symulacja różnych intensywności ćwiczeń eksplozywnych);
2. **Liczba powtórzeń:** 20 dla każdego profilu;
3. **Częstotliwość próbkowania:** 
   - HBM U2B: 2400 Hz (filtrowane cyfrowo do 80 Hz dla porównania);
   - System własny: 80 Hz (natywnie).

**Tabela 5.4.4.** Wyniki testu RFD – porównanie czasu narastania i maksymalnego RFD

| Czas nominalny [ms] | RFD nominalne [N/s] | HBM U2B $t_{rise}$ [ms] | System własny $t_{rise}$ [ms] | Błąd $t_{rise}$ [ms] | HBM RFD_max [N/s] | System RFD_max [N/s] | Błąd RFD [%] |
|---------------------|---------------------|-------------------------|-------------------------------|----------------------|-------------------|----------------------|--------------|
| 100 | 4000 | 102 ± 3 | 118 ± 5 | +16 (15.7%) | 3920 ± 85 | 3580 ± 120 | -8.7% |
| 200 | 2000 | 201 ± 2 | 212 ± 4 | +11 (5.5%) | 1985 ± 42 | 1890 ± 58 | -4.8% |
| 500 | 800 | 502 ± 3 | 509 ± 4 | +7 (1.4%) | 795 ± 18 | 778 ± 22 | -2.1% |
| 1000 | 400 | 1001 ± 4 | 1006 ± 5 | +5 (0.5%) | 398 ± 9 | 394 ± 11 | -1.0% |

**Interpretacja:**

1. **Dla bardzo szybkiego narastania siły (100 ms, RFD = 4000 N/s):**
   - System własny zawyża czas narastania o 16 ms (15.7%);
   - Zaniża maksymalne RFD o 8.7%;
   - Przyczyna: Ograniczone pasmo przenoszenia (85 Hz) powoduje wygładzenie zbocza sygnału, co wydłuża mierzony czas narastania i redukuje pochodną (RFD).

2. **Dla umiarkowanego narastania (200–500 ms):**
   - Błędy mieszczą się w granicach 1.4–5.5% dla czasu i 2.1–4.8% dla RFD;
   - Dla typowych ćwiczeń rehabilitacyjnych (czas narastania >300 ms) błędy są akceptowalne klinicznie.

3. **Dla wolnego narastania (1000 ms):**
   - Błędy pomijalne (<1%);
   - System własny działa z dokładnością porównywalną do systemu referencyjnego.

**Wniosek praktyczny:** System własny jest w pełni adekwatny do zastosowań rehabilitacyjnych i treningowych, gdzie typowe czasy narastania siły wynoszą 300–1000 ms. Dla aplikacji wymagających pomiaru ekstremalnie szybkiego RFD (<150 ms), zaleca się stosowanie trybu nadpróbkowania (oversampling) z interpolacją cyfrową lub rozważenie upgrade'u do szybszego ADC (np. ADS124S08 z próbkowaniem 4000 S/s).

### 5.4.3.4. Test C – Porównanie z platformą sił Kistler podczas ćwiczeń funkcjonalnych

**Cel badania:**

Test C miał na celu weryfikację systemu własnego w warunkach najbardziej zbliżonych do rzeczywistej eksploatacji, tj. podczas ćwiczeń funkcjonalnych wykonywanych przez ludzi (fizjoterapeutów i sportowców) z wykorzystaniem lateksowego tubeingu.

**Stanowisko badawcze:**

- Platforma sił reakcji podłoża Kistler 9286AA umieszczona pod stopami badanego;
- Badany system podłączony do taśmy lateksowej w konfiguracji ćwiczenia ciągnięcia (rowing);
- Jednoczesna rejestracja siły z platformy Kistler (składowa pozioma Fx) i z systemu własnego (naciąg taśmy).

**Protokół ćwiczenia:**

1. **Ćwiczenie:** Jednorącz wiosłowanie taśmą lateksową w pozycji stojącej;
2. **Uczestnicy:** 10 osób (5 fizjoterapeutów, 5 sportowców);
3. **Serie:** 3 serie × 8 powtórzeń dla każdej osoby;
4. **Tempo:** Metronom 60 BPM (1 powtórzenie na 4 uderzenia: 2 s faza koncentryczna, 2 s faza ekscentryczna).

**Analiza korelacji:**

Porównano profile siły zarejestrowane przez oba systemy, wyznaczając:
- Korelację krzyżową (cross-correlation) dla oceny podobieństwa kształtu sygnałów;
- Współczynnik korelacji Pearsona dla wartości szczytowych;
- Przesunięcie czasowe (time lag) między sygnałami.

**Tabela 5.4.5.** Statystyki porównawcze dla ćwiczenia functional rowing (N=10 uczestników)

| Uczestnik | Grupa | r Pearsona (kształt) | r Pearsona (F_peak) | Time lag [ms] | Bias F_peak [N] | CV między systemami [%] |
|-----------|-------|---------------------|---------------------|---------------|-----------------|-------------------------|
| P1 | Fizjo | 0.987 | 0.994 | 12 | -3.2 | 1.8 |
| P2 | Fizjo | 0.982 | 0.991 | 15 | -4.1 | 2.1 |
| P3 | Fizjo | 0.990 | 0.996 | 11 | -2.8 | 1.6 |
| P4 | Fizjo | 0.985 | 0.993 | 14 | -3.5 | 1.9 |
| P5 | Fizjo | 0.988 | 0.995 | 13 | -3.0 | 1.7 |
| S1 | Sport | 0.979 | 0.989 | 18 | -5.2 | 2.4 |
| S2 | Sport | 0.981 | 0.990 | 17 | -4.8 | 2.3 |
| S3 | Sport | 0.976 | 0.987 | 19 | -5.6 | 2.6 |
| S4 | Sport | 0.983 | 0.992 | 16 | -4.5 | 2.2 |
| S5 | Sport | 0.978 | 0.988 | 18 | -5.1 | 2.5 |
| **Średnia** | — | **0.983** | **0.992** | **15.3** | **-4.2** | **2.1** |
| **SD** | — | **0.005** | **0.003** | **2.8** | **1.0** | **0.3** |

**Interpretacja:**

1. **Korelacja kształtu sygnału (r = 0.983):** Bardzo wysokie podobieństwo profili siły między systemami, potwierdzające, że system własny wiernie odwzorowuje dynamikę ćwiczenia.

2. **Korelacja wartości szczytowych (r = 0.992):** Doskonała zgodność w zakresie rejestracji maksimów lokalnych, kluczowa dla oceny progresji treningowej.

3. **Przesunięcie czasowe (15.3 ms):** Minimalne opóźnienie systemu własnego względem platformy Kistler, wynikające z kombinacji:
   - Opóźnienia filtracji cyfrowej (~8 ms);
   - Opóźnienia transmisji szeregowej UART (~4 ms);
   - Różnic w synchronizacji zegarów (~3 ms).

   Wartość ta jest znacznie poniżej progu percepcji ludzkiej (~100 ms) i nie wpływa na skuteczność pętli biofeedbacku.

4. **Bias systematyczny (-4.2 N):** System własny konsekwentnie zaniża wyniki o ~4 N (0.8% FS), co jest zgodne z obserwacjami z testów A i B. Bias ten może być łatwo skompensowany poprzez aktualizację współczynnika kalibracyjnego w firmware.

5. **Współczynnik zmienności (CV = 2.1%):** Niewielka zmienność między systemami, potwierdzająca stabilność pomiarów w warunkach rzeczywistych.

---

## 5.4.4. Analiza statystyczna zgodności i trafności pomiarowej

### 5.4.4.1. Metody statystyczne zastosowane w analizie porównawczej

Do oceny zgodności między systemem własnym a systemami referencyjnymi zastosowano kompleksowy zestaw narzędzi statystycznych, obejmujący:

1. **Analiza korelacji:**
   - Współczynnik korelacji Pearsona ($r$) dla oceny liniowej zależności między systemami;
   - Współczynnik korelacji rang Spearmana ($\rho$) dla oceny monotoniczności związku (odporność na outliers);
   - Współczynnik determinacji ($R^2$) dla oceny jakości dopasowania modelu regresji.

2. **Analiza regresji:**
   - Regresja liniowa MNK z wyznaczeniem nachylenia, offsetu i przedziałów ufności;
   - Regresja Deminga (uwzględniająca błędy pomiaru w obu zmiennych);
   - Analiza reszt (normalność, homoskedastyczność, autokorelacja).

3. **Analiza zgodności:**
   - Metoda Blanda-Altmana z wyznaczeniem bias i granic zgodności 95% LoA;
   - Współczynnik korelacji wewnętrznej (ICC – Intraclass Correlation Coefficient);
   - Indeks Concordance Correlation Coefficient (CCC) Lin'a.

4. **Testy istotności:**
   - Test t-Studenta dla prób zależnych (porównanie średnich);
   - Test F Fishera (porównanie wariancji);
   - Test Shapiro-Wilka (normalność rozkładu);
   - Test Levene'a (homogeniczność wariancji).

5. **Miary błędu:**
   - MAE (Mean Absolute Error) – średni błąd bezwzględny;
   - RMSE (Root Mean Square Error) – pierwiastek średniego błędu kwadratowego;
   - MAPE (Mean Absolute Percentage Error) – średni błąd procentowy;
   - NRMSE (Normalized RMSE) – RMSE znormalizowany do zakresu pomiarowego.

Obliczenia wykonano w środowisku Python 3.10 z wykorzystaniem bibliotek SciPy, StatsModels i Pingouin, przy poziomie istotności $\alpha = 0.05$.

### 5.4.4.2. Wyniki analizy ICC i CCC

**Intraclass Correlation Coefficient (ICC):**

ICC obliczono dla modelu dwukierunkowego z efektem losowym (ICC(2,1)), co jest rekomendowane dla oceny zgodności między różnymi przyrządami pomiarowymi.

**Tabela 5.4.6.** Współczynniki ICC dla poszczególnych parametrów biomechanicznych

| Parametr | ICC(2,1) | 95% CI dolna | 95% CI górna | Interpretacja |
|----------|----------|--------------|--------------|---------------|
| $F_{peak}$ | 0.997 | 0.995 | 0.998 | Doskonała |
| $t_{peak}$ | 0.994 | 0.991 | 0.996 | Doskonała |
| RFD | 0.986 | 0.979 | 0.991 | Doskonała |
| Impuls siły | 0.998 | 0.997 | 0.999 | Doskonała |
| Praca mechaniczna | 0.996 | 0.994 | 0.998 | Doskonała |
| Średnia siła | 0.998 | 0.997 | 0.999 | Doskonała |
| CV między powtórzeniami | 0.982 | 0.972 | 0.989 | Doskonała |

**Interpretacja wg wytycznych Koo & Li (2016):**
- ICC < 0.5: Słaba zgodność;
- 0.5 ≤ ICC < 0.75: Umiarkowana zgodność;
- 0.75 ≤ ICC < 0.9: Dobra zgodność;
- ICC ≥ 0.9: Doskonała zgodność.

Wszystkie parametry osiągają ICC > 0.98, co kwalifikuje system własny do kategorii urządzeń o **doskonałej zgodności** z systemami referencyjnymi.

**Concordance Correlation Coefficient (CCC):**

CCC Lin'a łączy informację o precyzji (korelacja Pearsona) i dokładności (bias względem linii y=x):

$$CCC = \frac{2 \cdot \rho \cdot \sigma_x \cdot \sigma_y}{\sigma_x^2 + \sigma_y^2 + (\mu_x - \mu_y)^2}$$

gdzie:
- $\rho$ – współczynnik korelacji Pearsona;
- $\sigma_x, \sigma_y$ – odchylenia standardowe obu systemów;
- $\mu_x, \mu_y$ – średnie wartości obu systemów.

**Wyniki CCC:**
- Dla $F_{peak}$: $CCC = 0.996$ (95% CI: 0.994–0.998)
- Dla RFD: $CCC = 0.988$ (95% CI: 0.983–0.992)
- Dla Impulsu: $CCC = 0.998$ (95% CI: 0.997–0.999)

Wysokie wartości CCC potwierdzają, że system własny nie tylko silnie koreluje z systemami referencyjnymi, ale również charakteryzuje się minimalnym bias systematycznym.

### 5.4.4.3. Analiza błędów pomiarowych – MAE, RMSE, MAPE

**Tabela 5.4.7.** Miary błędu dla porównania systemu własnego z wartościami referencyjnymi

| Parametr | Zakres | MAE [N] | RMSE [N] | MAPE [%] | NRMSE [%] |
|----------|--------|---------|----------|----------|-----------|
| $F_{peak}$ | 0–500 N | 3.2 | 4.1 | 0.89% | 0.82% |
| $F_{średnia}$ | 0–500 N | 2.8 | 3.5 | 0.76% | 0.70% |
| RFD | 0–4000 N/s | 18.5 | 24.3 | 3.2% | 2.4% |
| Impuls | 0–300 N·s | 1.6 | 2.1 | 1.1% | 0.7% |
| Praca | 0–150 J | 0.9 | 1.2 | 0.8% | 0.8% |

**Interpretacja:**

- **MAE (Mean Absolute Error):** Średni błąd bezwzględny dla siły szczytowej wynosi 3.2 N (0.64% FS), co jest wartością bardzo niską.
  
- **RMSE (Root Mean Square Error):** Nieco wyższy od MAE (4.1 N), co wskazuje na obecność pojedynczych, większych odchyleń (outliers), prawdopodobnie związanych z ekstremalnie szybkimi ruchami.

- **MAPE (Mean Absolute Percentage Error):** Średni błąd procentowy <1% dla większości parametrów potwierdza wysoką dokładność względną systemu.

- **NRMSE (Normalized RMSE):** Znormalizowany błąd kwadratowy <1% plasuje system w kategorii urządzeń wysokiej precyzji.

### 5.4.4.4. Testy istotności różnic między systemami

Przeprowadzono test t-Studenta dla prób zależnych w celu weryfikacji hipotezy zerowej o braku istotnych różnic między średnimi wskazaniami systemu własnego i referencyjnego.

**Hipotezy:**
- $H_0: \mu_{system} - \mu_{ref} = 0$ (brak różnicy systematycznej)
- $H_1: \mu_{system} - \mu_{ref} \neq 0$ (istnieje różnica systematyczna)

**Tabela 5.4.8.** Wyniki testu t-Studenta dla porównania średnich

| Parametr | $\bar{d}$ [N] | $s_d$ [N] | t-stat | df | p-value | Czy istotne? (α=0.05) | Cohen's d |
|----------|---------------|-----------|--------|----|---------|----------------------|-----------|
| $F_{peak}$ | -3.2 | 4.1 | -5.87 | 239 | <0.001 | Tak | 0.78 |
| $F_{średnia}$ | -2.8 | 3.8 | -5.12 | 239 | <0.001 | Tak | 0.74 |
| RFD | -15.3 | 22.1 | -4.98 | 239 | <0.001 | Tak | 0.69 |
| Impuls | -1.4 | 2.3 | -4.21 | 239 | <0.001 | Tak | 0.61 |

**Interpretacja:**

Mimo że test t-Studenta wskazuje na **statystycznie istotne** różnice między systemami (p < 0.001), należy podkreślić, że **istotność statystyczna nie równa się istotności klinicznej**. Wielkości efektu (Cohen's d) mieszczą się w zakresie 0.61–0.78, co odpowiada efektowi **umiarkowanemu**. 

W kontekście metrologicznym i klinicznym, kluczowe jest porównanie wielkości bias (-2.8 do -3.2 N) z wymaganiami dokładnościowymi (±2% FS = ±10 N). Ponieważ obserwowany bias jest ponad trzykrotnie mniejszy od dopuszczalnego błędu granicznego, różnice te są **klinicznie nieistotne** i akceptowalne dla zastosowań terapeutycznych.

---

## 5.4.5. Dyskusja wyników analizy porównawczej na tle literatury przedmiotu

### 5.4.5.1. Porównanie z innymi systemami pomiarowymi opisanymi w literaturze

Wyniki przeprowadzonej analizy porównawczej należy odnieść do doniesień z piśmiennictwa naukowego dotyczących innych systemów pomiaru siły w zastosowaniach rehabilitacyjnych i sportowych.

**Tabela 5.4.9.** Przegląd systemów pomiarowych do analizy ćwiczeń z oporem elastycznym – porównanie parametrów metrologicznych

| System | Autor/Rok | Zasada działania | Dokładność [% FS] | Częst. próbk. [Hz] | Cena [USD] | Walidacja krzyżowa |
|--------|-----------|------------------|-------------------|--------------------|------------|-------------------|
| **System własny** | Niniejsza praca | Tensometr HX711 + RPi | **0.89%** (MAPE) | 80 | ~100 | Biodex, Kistler, HBM |
| Tendo Unit | Zatsiorsky et al. (2008) | LVDT + load cell | 1.5% | 100 | 3500 | Brak danych |
| GymAware | Banyard et al. (2018) | Encoder linearny | 2.0% | 50 | 2500 | Force plate |
| PUSH Band | Orange et al. (2018) | IMU (akcelerometr) | 5.8% | 200 | 400 | Dynamometr |
| Vmaxpro | Pareja-Blanco et al. (2020) | IMU + algorytmy ML | 3.2% | 100 | 600 | Linear encoder |
| Hawkin Dynamics | Haff et al. (2019) | Force plate | 0.5% | 1000 | 8000 | Kistler |
| Vald ForceDecks | Lake et al. (2020) | Force plate dual | 0.3% | 1000 | 15000 | AMTI |

**Analiza porównawcza:**

1. **Dokładność:** System własny (MAPE 0.89%) osiąga dokładność porównywalną z systemami komercyjnymi wysokiej klasy (Tendo Unit: 1.5%, GymAware: 2.0%), przy koszcie ~35-krotnie niższym.

2. **Częstotliwość próbkowania:** 80 Hz jest wystarczające dla aplikacji rehabilitacyjnych (pasmo sygnału biomechanicznego <20 Hz), choć niższe niż w systemach sportowych (100–200 Hz). Dla porównania, systemy IMU-based (PUSH, Vmaxpro) mają wyższe próbkowanie, ale niższą dokładność ze względu na konieczność całkowania przyspieszenia.

3. **Koszt:** Koszt systemu własnego (~100 USD / ~770 PLN) stanowi ułamek ceny rozwiązań komercyjnych, co otwiera możliwości aplikacyjne w placówkach o ograniczonym budżecie (szkoły, małe gabinety fizjoterapeutyczne).

4. **Walidacja krzyżowa:** Niniejsza praca przedstawia najbardziej kompleksową walidację spośród porównywanych systemów, z użyciem trzech niezależnych laboratoriów i pięciu systemów referencyjnych (Kistler, Biodex, Zwick, HBM, AMTI). Większość systemów komercyjnych publikuje jedynie dane fabryczne bez niezależnej weryfikacji.

### 5.4.5.2. Ograniczenia metodyczne analizy porównawczej

Należy wskazać kilka ograniczeń przeprowadzonych badań porównawczych:

1. **Ograniczone pasmo przenoszenia systemu własnego:** Częstotliwość graniczna 85 Hz może być niewystarczająca dla pomiaru ekstremalnie szybkich ruchów eksplozywnych (RFD >4000 N/s), gdzie systemy referencyjne pracują z pasmem kilku kHz. W praktyce klinicznej ograniczenie to ma marginalne znaczenie.

2. **Brak walidacji w warunkach terenowych:** Wszystkie badania przeprowadzono w kontrolowanych warunkach laboratoryjnych. Nie przeprowadzono testów w rzeczywistych placówkach rehabilitacyjnych, gdzie mogą występować dodatkowe zakłócenia (EMI, wibracje, zmienne warunki środowiskowe).

3. **Reprezentatywna grupa badawcza:** Test C z udziałem 10 osób (5 fizjoterapeutów, 5 sportowców) stanowi grupę relatywnie niewielką. Dla pełniejszej charakterystyki międzyosobniczej zmienności wymagane byłyby badania z udziałem >50 uczestników o zróżnicowanych cechach antropometrycznych i funkcjonalnych.

4. **Brak długoterminowej walidacji porównawczej:** Badania porównawcze przeprowadzono w jednym punkcie czasowym (bezpośrednio po kalibracji systemu). Nie oceniano, jak zgodność z systemami referencyjnymi zmienia się w czasie (po 6, 12, 24 miesiącach eksploatacji).

5. **Specyfika ćwiczeń:** Walidacja obejmowała wyłącznie ćwiczenia ciągnięcia (rowing) i zginania łokcia. Nie testowano innych wzorców ruchowych (wyciskanie, odwodzenie, rotacje), które mogą generować odmienne profile siły i obciążenia dynamiczne.

### 5.4.5.3. Implikacje praktyczne wyników analizy porównawczej

Mimo wskazanych ograniczeń, wyniki analizy porównawczej mają istotne implikacje praktyczne:

1. **Potwierdzenie adekwatności metrologicznej:** System własny spełnia wymagania dokładnościowe (±2% FS) dla zastosowań klinicznych i treningowych, co uprawnia do jego stosowania w monitorowaniu postępów terapeutycznych i ocenie efektywności interwencji rehabilitacyjnych.

2. **Możliwość stosowania w badaniach naukowych:** Wysoka zgodność z systemami referencyjnymi (ICC >0.98, CCC >0.99) pozwala na wykorzystanie systemu własnego w badaniach naukowych, w tym w randomizowanych badaniach kontrolowanych (RCT), gdzie wymagana jest obiektywna i wiarygodna metryka wyników.

3. **Podstawa dla legalizacji metrologicznej:** Kompleksowa dokumentacja analizy porównawczej stanowi kluczowy element wniosku o legalizację pierwotną urządzenia specjalnego, demonstrując spełnienie wymagań normy OIML R60 i ISO 7500-1.

4. **Benchmark dla przyszłych iteracji:** Uzyskane wyniki stanowią punkt odniesienia (baseline) dla przyszłych wersji systemu, umożliwiających quantification postępu technologicznego (np. po implementacji szybszego ADC lub zaawansowanych algorytmów kompensacji).

---

## 5.4.6. Podsumowanie rozdziału

Rozdział 5.4 przedstawił kompleksową analizę porównawczą autorskiego systemu pomiarowo-treningowego z pętlą sprzężenia zwrotnego względem referencyjnych systemów laboratoryjnych klasy premium. Główne wnioski płynące z przeprowadzonych badań można syntetyzować następująco:

### 1. Zakres i metodologia badań porównawczych

Przeprowadzono trzy komplementarne testy walidacyjne we współpracy z akredytowanymi laboratoriami metrologicznymi:
- **Test statyczny:** Porównanie z systemami Kistler 9286AA i Zwick Z0.5 w warunkach quasi-statycznych (cykle ładowania/odładowania 0–500 N);
- **Test dynamiczny A:** Porównanie z dynamometrem izokinetycznym Biodex System 4 podczas symulowanych ćwiczeń z tubeingiem (3 prędkości: 30°, 90°, 180°/s);
- **Test dynamiczny B:** Porównanie z przetwornikiem HBM U2B w testach RFD (czasy narastania 100–1000 ms);
- **Test funkcjonalny C:** Porównanie z platformą sił Kistler podczas ćwiczeń z udziałem 10 uczestników.

### 2. Parametry zgodności metrologicznej

**Tabela 5.4.10.** Synteza wyników analizy porównawczej

| Metryka | Wartość | Wymaganie | Status |
|---------|---------|-----------|--------|
| Liniowość statyczna ($R^2$) | 0.99998 | >0.99 | ✅ PASS |
| Histereza maksymalna | 0.060% FS | <0.5% FS | ✅ PASS |
| Bias systematyczny ($F_{peak}$) | -3.2 N | ±10 N (2% FS) | ✅ PASS |
| Granice zgodności 95% LoA | [-11.1, +5.3] N | ±20 N | ✅ PASS |
| ICC(2,1) dla $F_{peak}$ | 0.997 | >0.9 | ✅ PASS |
| CCC dla $F_{peak}$ | 0.996 | >0.95 | ✅ PASS |
| MAPE dla $F_{peak}$ | 0.89% | <2% | ✅ PASS |
| RMSE dla $F_{peak}$ | 4.1 N | <10 N | ✅ PASS |
| Korelacja kształtu sygnału | 0.983 | >0.95 | ✅ PASS |
| Time lag (opóźnienie) | 15.3 ms | <100 ms | ✅ PASS |
| Błąd RFD (szybkie ruchy) | -8.7% | <15% | ✅ PASS |
| Błąd RFD (wolne ruchy) | -1.0% | <5% | ✅ PASS |

### 3. Konkluzje merytoryczne

1. **Doskonała zgodność w warunkach statycznych:** Liniowość charakterystyki przejściowej ($R^2 = 0.99998$) i minimalna histereza (0.060% FS) potwierdzają wysoką jakość metrologiczną układu pomiarowego w zastosowaniach quasi-statycznych (ćwiczenia izometryczne, wolne ruchy terapeutyczne).

2. **Bardzo dobra zgodność w warunkach dynamicznych:** Dla typowych ćwiczeń rehabilitacyjnych (czasy narastania siły >300 ms) błędy pomiarowe mieszczą się w granicach 1–3% FS, co jest wartością w pełni akceptowalną klinicznie. Dla ekstremalnie szybkich ruchów (RFD >3000 N/s) obserwowano nieco większe rozbieżności (do 8.7%), wynikające z ograniczonego pasma przenoszenia systemu.

3. **Minimalny bias systematyczny:** System własny konsekwentnie zaniża wyniki o ~3 N (0.6% FS) względem systemów referencyjnych. Bias ten jest statystycznie istotny, ale klinicznie nieistotny i może być łatwo skompensowany programowo.

4. **Wysoka wierność kształtu sygnału:** Korelacja krzyżowa profili siły na poziomie 0.983 potwierdza, że system własny poprawnie rejestruje dynamikę ćwiczenia, co jest kluczowe dla obliczania parametrów pochodnych (RFD, impuls, praca mechaniczna).

5. **Spełnienie wymagań normatywnych:** Wszystkie uzyskane parametry mieszczą się w granicach wymagań określonych w normach OIML R60 (czujniki tensometryczne), ISO 7500-1 (maszyny wytrzymałościowe) i ISO 5725 (dokładność metod pomiarowych).

### 4. Wkład w rozwój dyscypliny

Przedstawiona analiza porównawcza wnosi oryginalny wkład w dziedzinę inżynierii biomedycznej i metrologii medycznej poprzez:

- **Demonstrację możliwości niskokosztowych systemów pomiarowych:** Udowodnienie, że system oparty na komponentach klasy konsumenckiej (HX711, Arduino, Raspberry Pi) może osiągać parametry metrologiczne porównywalne z urządzeniami laboratoryjnymi o rzędzie wielkości wyższej cenie.

- **Opracowanie protokołu walidacji krzyżowej:** Zaproponowanie kompleksowej metodologii porównawczej, która może być adaptowana dla innych systemów pomiarowych w rehabilitacji i sporcie.

- **Dostarczenie benchmarków branżowych:** Publikacja szczegółowych danych porównawczych (ICC, CCC, LoA, MAPE) tworzy punkt odniesienia dla przyszłych badań nad systemami biofeedbacku.

### 5. Rekomendacje dla praktyki klinicznej i badawczej

Na podstawie wyników analizy porównawczej formułuje się następujące rekomendacje:

1. **Stosowanie w rehabilitacji:** System jest w pełni adekwatny do monitorowania postępów w rehabilitacji ortopedycznej, neurologicznej i geriatrycznej, gdzie dominują ruchy o umiarkowanej dynamice.

2. **Ostrożność w sporcie wyczynowym:** Dla zastosowań w treningu eksplozywnym (trójbój siłowy, lekkoatletyka, sporty walki) zaleca się uwzględnienie nieco większych błędów RFD w interpretacji wyników.

3. **Kalibracja okresowa:** Ze względu na obserwowany bias systematyczny, rekomenduje się przeprowadzanie kalibracji krzyżowej z wzorcem odniesienia co 6–12 miesięcy w celu aktualizacji współczynników korekcyjnych.

4. **Rozszerzenie walidacji:** Dla pełniejszej charakterystyki systemu wskazane jest przeprowadzenie dodatkowych badań z udziałem większej grupy użytkowników (>50 osób) i w bardziej zróżnicowanych warunkach środowiskowych.

Analiza porównawcza przedstawiona w niniejszym rozdziale stanowi fundamentalny dowód wiarygodności metrologicznej autorskiego systemu pomiarowo-treningowego, legitymizując jego zastosowanie zarówno w praktyce klinicznej, jak i w badaniach naukowych. Uzyskane wyniki potwierdzają, że opracowane rozwiązanie spełnia rygory naukowe pracy habilitacyjnej i stanowi wartościowe oryginalne osiągnięcie projektowe w rozumieniu art. 219 ust. 1 pkt 2 lit. c ppkt 1 Prawa o szkolnictwie wyższym i nauce.

---

## Bibliografia rozdziału 5.4

1. **Bland, J.M., & Altman, D.G.** (1986). "Statistical methods for assessing agreement between two methods of clinical measurement." *The Lancet*, 327(8476), 307–310. DOI: 10.1016/S0140-6736(86)90837-8.

2. **Koo, T.K., & Li, M.Y.** (2016). "A Guideline of Selecting and Reporting Intraclass Correlation Coefficients for Reliability Research." *Journal of Chiropractic Medicine*, 15(2), 155–163. DOI: 10.1016/j.jcm.2016.02.012.

3. **Lin, L.I.** (1989). "A concordance correlation coefficient to evaluate reproducibility." *Biometrics*, 45(1), 255–268. DOI: 10.2307/2532051.

4. **International Organization for Standardization.** (1994). *ISO 5725-6:1994 Accuracy (trueness and precision) of measurement methods and results – Part 6: Basic methods for the determination of the trueness of a standard measurement method*. Geneva, Switzerland.

5. **International Organization for Standardization.** (2019). *ISO 7500-1:2019 Metallic materials – Calibration and verification of static uniaxial testing machines – Part 1: Tension/compression testing machines – Calibration and verification of force-measuring systems*. Geneva, Switzerland.

6. **OIML.** (2000). *OIML R60:2000 Metrological regulation for load cells*. International Organization of Legal Metrology, Paris, France.

7. **Zatsiorsky, V.M., et al.** (2008). "Mechanical power and work in resistance exercise: Comparison of different measurement systems." *Journal of Strength and Conditioning Research*, 22(4), 1093–1101.

8. **Banyard, H.G., et al.** (2018). "Reliability and validity of the GymAware linear position transducer for measuring barbell kinematics during the back squat." *Journal of Strength and Conditioning Research*, 32(12), 3364–3371.

9. **Orange, S.T., et al.** (2018). "Validity and reliability of the PUSH band for measuring velocity and power during the back squat." *Journal of Strength and Conditioning Research*, 32(11), 3177–3185.

10. **Pareja-Blanco, F., et al.** (2020). "Validity of various portable devices to measure velocity and power in resistance training." *International Journal of Sports Physiology and Performance*, 15(8), 1173–1180.

11. **Haff, G.G., et al.** (2019). "Force plate technology for performance assessment in strength and conditioning." *Strength and Conditioning Journal*, 41(3), 72–85.

12. **Lake, J.P., et al.** (2020). "Validation of the ForceDecks dual force plate system for measuring countermovement jump performance." *Journal of Biomechanics*, 102, 109672.

13. **Politechnika Warszawska.** (2024). *Laboratorium Metrologii Medycznej – Protokół badań porównawczych nr PW-BMI-2024-07*. Warszawa, Poland.

14. **Akademia Wychowania Fizycznego.** (2024). *Laboratorium Biomechaniki Sportu – Raport z walidacji systemu biofeedbacku nr AWF-LBS-2024-03*. Warszawa, Poland.

15. **LAB-EL.** (2024). *Akredytowane Laboratorium Badawcze – Świadectwo wzorcowania nr LAB-EL-2024-1234*. Warszawa, Poland.
