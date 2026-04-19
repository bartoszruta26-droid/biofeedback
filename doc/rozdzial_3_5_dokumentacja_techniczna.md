# Rozdział 3.5. Dokumentacja techniczna, rysunki wykonawcze i specyfikacje materiałów

## 3.5.1. Wprowadzenie do problematyki dokumentacji technicznej w procesie certyfikacji wyrobów medycznych klasy I i urządzeń pomiarowych specjalnych

### 3.5.1.1. Rola dokumentacji technicznej w cyklu życia wyrobu medycznego i urządzenia specjalnego

Dokumentacja techniczna stanowi fundament prawny, metrologiczny i inżynierski każdego wyrobu medycznego lub urządzenia pomiarowego specjalnego, determinując nie tylko możliwość jego legalnego wprowadzenia do obrotu, ale także bezpieczeństwo użytkowników, powtarzalność procesu produkcyjnego, możliwość serwisowania oraz długoterminową dostępność na rynku. W kontekście autorskiego systemu pomiarowo-treningowego z pętlą sprzężenia zwrotnego, opracowanie kompletnej i zgodnej z wymaganiami normatywnymi dokumentacji technicznej było kluczowym etapem realizacji osiągnięcia projektowego, poprzedzającym procedurę legalizacji pierwotnej i deklaracji zgodności UE.

**Definicja dokumentacji technicznej według aktów prawnych:**

Zgodnie z **Rozporządzeniem Parlamentu Europejskiego i Rady (UE) 2017/745 z dnia 5 kwietnia 2017 r. w sprawie wyrobów medycznych (MDR – Medical Device Regulation)**, dokumentacja techniczna to:

> "Kompleksowy zbiór dokumentów zawierających informacje niezbędne do oceny zgodności wyrobu z wymaganiami niniejszego rozporządzenia, obejmujący opis wyrobu, dokumentację projektową i wytwórczą, dane dotyczące oceny ryzyka, wyniki badań klinicznych, instrukcje używania oraz oświadczenia o zgodności."

Dla urządzeń pomiarowych specjalnych (niemedycznych), kluczowym aktem prawnym jest **Dyrektywa Parlamentu Europejskiego i Rady 2014/32/UE (MID – Measuring Instruments Directive)** oraz **Ustawa z dnia 11 maja 2022 r. – Prawo o miarach**, które definiują dokumentację techniczną jako:

> "Zbiór dokumentów określających charakterystyki metrologiczne urządzenia pomiarowego, metody wzorcowania, wyniki badań typu, deklaracje zgodności oraz instrukcje obsługi i konserwacji."

**Specyfika dokumentacji dla systemu hybrydowego (medyczno-pomiarowego):**

Opracowany system pomiarowo-treningowy posiada dualny status prawny:
- **W zastosowaniach rehabilitacyjnych (gimnastyka lecznicza):** wyrób medyczny klasy I zgodnie z MDR (załącznik VIII, reguła 9 – urządzenia do monitorowania parametrów fizjologicznych)
- **W zastosowaniach sportowych i edukacyjnych (gimnastyka sportowa, wychowanie fizyczne):** urządzenie pomiarowe specjalne podlegające legalizacji jednostkowej zgodnie z Prawem o miarach

Ta dwoistość wymagała opracowania dokumentacji technicznej spełniającej łącznie wymagania:
1. **MDR Annex II & III** – dokumentacja techniczna dla wyrobów medycznych
2. **MID Annex III** – dokumentacja techniczna dla urządzeń pomiarowych
3. **PN-EN ISO 13485:2016** – system zarządzania jakością dla wyrobów medycznych
4. **PN-EN ISO 9001:2015** – system zarządzania jakością ogólny
5. **PN-EN 10204:2005** – rodzaje dokumentów kontrolnych dla materiałów metalowych
6. **PN-EN 62304:2008** – oprogramowanie wyrobów medycznych – cykle życia
7. **PN-EN 60601-1:2021** – bezpieczeństwo elektryczne wyrobów medycznych
8. **PN-EN 60601-1-2:2021** – kompatybilność elektromagnetyczna wyrobów medycznych

**Tabela 3.28.** Struktura dokumentacji technicznej systemu pomiarowo-treningowego zgodnie z MDR i MID

| Sekcja | Zgodność z MDR | Zgodność z MID | Zakres treści | Liczba stron |
|--------|----------------|----------------|---------------|--------------|
| **A. Opis ogólny wyrobu** | Annex II, pkt 1 | Annex III, pkt 1 | Nazwa, model, przeznaczenie, warianty konfiguracji | 12 |
| **B. Dokumentacja projektowa** | Annex II, pkt 2 | Annex III, pkt 2.1 | Rysunki wykonawcze, schematy ideowe, BOM, specyfikacje materiałowe | 156 |
| **C. Ocena ryzyka** | Annex I, pkt 3 + ISO 14971 | — | Analiza FMEA, HACCP, scenariusze zagrożeń, środki zaradcze | 48 |
| **D. Wymagania ogólne bezpieczeństwa** | Annex I | Annex III, pkt 3 | Bezpieczeństwo elektryczne, mechaniczne, biokompatybilność | 34 |
| **E. Walidacja i weryfikacja** | Annex II, pkt 6 | Annex III, pkt 2.2 | Protokoły testów, wyniki badań EMC, kalibracji, środowiskowych | 89 |
| **F. Oznakowanie i instrukcje** | Annex I, pkt 23 | Annex III, pkt 4 | Etykiety, ULOTKI, IFU (Instructions for Use), symbole | 28 |
| **G. Deklaracje zgodności** | Annex IV | Annex III, pkt 5 | Deklaracja zgodności UE, certyfikaty CE, świadectwa legalizacji | 16 |
| **H. Dane kliniczne** | Annex XIV, Part A | — | Raport z oceny klinicznej, literatura, badania własne | 67 |
| **I. System zarządzania jakością** | Article 10(9) | — | Certyfikat ISO 13485, procedury QMS, audyty | 42 |
| **J. Nadzór porejestrowy** | Article 83–86 | — | Plan PMS (Post-Market Surveillance), raporty incydentów | 23 |
| **SUMA** | — | — | **Kompletna dokumentacja** | **515** |

---

### 3.5.1.2. Metodologia tworzenia dokumentacji technicznej w środowisku CAD/PLM

Proces tworzenia dokumentacji technicznej został przeprowadzony zgodnie z metodyką **Design Documentation Control (DDC)** zaimplementowaną w systemie PLM (Product Lifecycle Management) Siemens Teamcenter, zapewniającym pełną identyfikowalność zmian, wersjonowanie dokumentów i zarządzanie konfiguracją wyrobu.

**Fazy procesu DDC:**

**Faza 1: Koncepcja i architektura informacji (Information Architecture)**

Na tym etapie zdefiniowano strukturę drzewa dokumentów (Document Breakdown Structure – DBS):

```
ROOT: PTMS-DOC (Physical Training Measurement System Documentation)
│
├───01_ADMINISTRATIVE
│   ├───PTMS-ADM-001 Tytułowa strona dokumentacji
│   ├───PTMS-ADM-002 Historia zmian (Revision History)
│   ├───PTMS-ADM-003 Lista dystrybucji (Distribution List)
│   └───PTMS-ADM-004 Słownik pojęć i skrótów (Glossary)
│
├───02_DESIGN_DRAWINGS
│   ├───02_01_MECHANICAL
│   │   ├───PTMS-MCH-001 Assembly Main (A1)
│   │   ├───PTMS-MCH-002 Plate Wall Mount (A3)
│   │   ├───PTMS-MCH-003 Sensor Bracket Lower (A3)
│   │   ├───PTMS-MCH-004 Ball Joint Assembly (A3)
│   │   ├───PTMS-MCH-005 Handle Ergonomic (A3)
│   │   ├───PTMS-MCH-006 Quick-Release Connector (A4)
│   │   ├───PTMS-MCH-007 Pulley Block System (A3)
│   │   └───PTMS-MCH-008 Frame Bilateral System (A2)
│   │
│   ├───02_02_ELECTRICAL
│   │   ├───PTMS-ELEC-001 Schematic Power Supply (A2)
│   │   ├───PTMS-ELEC-002 Schematic Signal Conditioning (A2)
│   │   ├───PTMS-ELEC-003 PCB Layout Top Layer (A2)
│   │   ├───PTMS-ELEC-004 PCB Layout Bottom Layer (A2)
│   │   ├───PTMS-ELEC-005 Wiring Diagram Internal (A3)
│   │   └───PTMS-ELEC-006 Cable Harness Assembly (A3)
│   │
│   └───02_03_SOFTWARE_ARCHITECTURE
│       ├───PTMS-SW-001 Architecture Diagram (A2)
│       ├───PTMS-SW-002 Data Flow Diagram (A3)
│       └───PTMS-SW-003 State Machine Diagram (A3)
│
├───03_MATERIAL_SPECIFICATIONS
│   ├───PTMS-MAT-001 Aluminium 6061-T6 Specification
│   ├───PTMS-MAT-002 TPE Mediprene H4085 Specification
│   ├───PTMS-MAT-003 Steel 100Cr6 Bearing Quality
│   ├───PTMS-MAT-004 Stainless Steel 316L Wire Rope
│   └───PTMS-MAT-005 Natural Latex Tubing Specification
│
├───04_MANUFACTURING_INSTRUCTIONS
│   ├───PTMS-MFG-001 CNC Machining Instructions
│   ├───PTMS-MFG-002 Injection Molding Setup Sheet
│   ├───PTMS-MFG-003 PCB Assembly (PCBA) Guidelines
│   ├───PTMS-MFG-004 Final Assembly Work Instructions
│   └───PTMS-MFG-005 Calibration Procedure
│
├───05_QUALITY_CONTROL
│   ├───PTMS-QC-001 Incoming Inspection Checklist
│   ├───PTMS-QC-002 In-Process Quality Checks
│   ├───PTMS-QC-003 Final Acceptance Test Protocol
│   └───PTMS-QC-004 Non-Conformance Report Template
│
├───06_TESTING_VALIDATION
│   ├───PTMS-TEST-001 EMC Test Report (Pre-compliance)
│   ├───PTMS-TEST-002 Mechanical Load Testing
│   ├───PTMS-TEST-003 Environmental Chamber Testing
│   ├───PTMS-TEST-004 Metrological Calibration Report
│   └───PTMS-TEST-005 Clinical Validation Protocol
│
├───07_REGULATORY_SUBMISSIONS
│   ├───PTMS-REG-001 EU Declaration of Conformity (MDR)
│   ├───PTMS-REG-002 EU Declaration of Conformity (MID)
│   ├───PTMS-REG-003 Legalization Certificate (Primary)
│   └───PTMS-REG-004 Notified Body Correspondence
│
└───08_USER_DOCUMENTATION
    ├───PTMS-USR-001 Instructions for Use (IFU) – Polish
    ├───PTMS-USR-002 Instructions for Use (IFU) – English
    ├───PTMS-USR-003 Quick Start Guide
    └───PTMS-USR-004 Maintenance Manual
```

**Faza 2: Modelowanie 3D i generowanie rysunków wykonawczych**

Wszystkie komponenty mechaniczne zostały zaprojektowane w środowisku **SolidWorks 2023 Premium** z wykorzystaniem następujących modułów:
- **Part Modeling** – modelowanie bryłowe pojedynczych detali
- **Assembly Design** – złożenia wieloelementowe z definicją pasowań i ruchomości
- **Drawing Generator** – automatyczne generowanie rzutów 2D z modelu 3D
- **Tolerance Analysis (CELTOS)** – analiza łańcuchów wymiarowych i tolerancji geometrycznych GD&T

**Standardy rysunkowe zastosowane w dokumentacji:**

1. **PN-EN ISO 128-20:2014** – Zasady ogólne przedstawiania na rysunkach technicznych
2. **PN-EN ISO 129-1:2018-07** – Oznaczanie wymiarów i tolerancji geometrycznych
3. **PN-EN ISO 2768-1:1994** – Tolerancje ogólne (wybrano klasę **m** – medium precision)
4. **PN-EN ISO 2768-2:1994** – Tolerancje geometryczne (wybrano klasę **K** – dla elementów obrabianych skrawaniem)
5. **PN-EN ISO 1101:2017-10** – Specyfikacje geometryczne produktów (GPS) – tolerancje kształtu, kierunku, położenia i bicia
6. **PN-EN ISO 1302:2002** – Oznaczanie chropowatości powierzchni
7. **PN-EN ISO 7200:2004** – Pola informacyjne na rysunkach technicznych

**Faza 3: Zarządzanie konfiguracją i wersjonowanie**

Każdy dokument w systemie PLM otrzymał unikalny identyfikator zgodnie ze schematem:

```
PTMS-XXX-NNN vR.M
│     │    │    │
│     │    │    └─ Wersja minor (zmiany redakcyjne, korekty literówek)
│     │    └───── Numer sekwencyjny dokumentu w kategorii
│     └────────── Kategoria (ADM, MCH, ELEC, MAT, MFG, QC, TEST, REG, USR)
└──────────────── Prefiks projektu (Physical Training Measurement System)
```

Przykład: `PTMS-MCH-005 v2.1` – uchwyt ergonomiczny, wersja główna 2 (zmiany konstrukcyjne), wersja minor 1 (drobne korekty).

**Zasady przydzielania wersji:**

| Typ zmiany | Wersja główna (R) | Wersja minor (M) | Wymagana aprobata |
|------------|-------------------|------------------|-------------------|
| Zmiana funkcjonalna (wpływ na bezpieczeństwo/metrologię) | Increment (1→2) | Reset to 0 | Notified Body + Jednostka Legalizacyjna |
| Zmiana konstrukcyjna (bez wpływu na bezpieczeństwo) | Increment (1→2) | Reset to 0 | Internal QA Manager |
| Korekta rysunku (wymiarowanie, opisy) | No change | Increment (0→1) | Document Controller |
| Zmiana redakcyjna (literówki, formatowanie) | No change | Increment (0→1) | Technical Writer |

---

## 3.5.2. Kompleksowa charakterystyka rysunków wykonawczych komponentów mechanicznych

### 3.5.2.1. Rysunek PTMS-MCH-001: Assembly Main – złożenie główne systemu

**Format:** A1 (594 × 841 mm)  
**Skala:** 1:2 (widok ogólny), 1:1 (szczegóły A-A, B-B, C-C)  
**Rewizja:** C (trzecia wersja zatwierdzona do produkcji)  
**Data wydania:** 2024-03-20

**Zakres rysunku:**

Rysunek PTMS-MCH-001 przedstawia kompletne złożenie główne systemu pomiarowo-treningowego w konfiguracji podstawowej (kotwienie ścienne), obejmujące:

1. **Płytę montażową ścienną** (poz. 1) – element bazowy mocowany do ściany za pomocą 4 kołków rozporowych M8
2. **Uchwyt czujnika dolny** (poz. 2) – aluminiowy korpus frezowany CNC zintegrowany z przegubem kulistym
3. **Przegub kulisty samocentrujący** (poz. 3) – mechanizm kompensacji niedokładności montażu
4. **Load cell tensometryczny** (poz. 4) – czujnik siły 50 kg z mostkiem Wheatstone'a
5. **Szybkozłącze żeńskie** (poz. 5) – połączenie wciskowe z zatrzaskiem kulistym
6. **Tubeing lateksowy** (poz. 6) – taśma oporowa o przekroju prostokątnym 15×4 mm
7. **Szybkozłącze męskie** (poz. 7) – odpowiednia część do szybkozłącza żeńskiego
8. **Stożkowa tuleja zaciskowa** (poz. 8) – system mocowania tubeingu bez uszkadzania materiału
9. **Mandrel stożkowy** (poz. 9) – element rozpierający tuleję zaciskową
10. **Uchwyt ręczny ergonomiczny** (poz. 10) – dwukomponentowy (PA6-GF30 + TPE)
11. **Osłona kabla** (poz. 11) – ochrona przewodu sygnałowego przed uszkodzeniami mechanicznymi
12. **Zestaw śrub montażowych** (poz. 12) – 4× śruba M8×60 klasy 8.8 z podkładkami i nakrętkami

**Tabela pozycji materiałowych (BOM – Bill of Materials):**

| Poz. | Ilość | Numer części | Nazwa | Materiał | Masa [g] | Uwagi |
|------|-------|--------------|-------|----------|----------|-------|
| 1 | 1 | PTMS-MCH-002 | Plate Wall Mount | Al 6061-T6 | 285 | Frezowanie CNC, anodowanie twarde |
| 2 | 1 | PTMS-MCH-003 | Sensor Bracket Lower | Al 6061-T6 | 62 | Frezowanie 5-osiowe, Ra 0.8 μm |
| 3 | 1 | PTMS-MCH-004 | Ball Joint Assembly | 100Cr6 + Al | 98 | Kulka hartowana 58-65 HRC |
| 4 | 1 | LC-50KG-HX711 | Load Cell 50kg | Al 2024-T4 | 145 | Mostek pełny, IP65 |
| 5 | 1 | PTMS-MCH-006-F | Quick-Release Female | PA6-GF30 | 34 | Wtrysk 2K, zatrzaski stalowe |
| 6 | 1 | LTX-1504-NAT | Latex Tubing 15×4mm | Natural latex | 180/m | Klasa medyczna, bez alergenów |
| 7 | 1 | PTMS-MCH-006-M | Quick-Release Male | PA6-GF30 | 28 | Wtrysk, kompatybilny z poz. 5 |
| 8 | 1 | PTMS-MCH-009 | Tapered Collet | POM-C | 12 | Niskie tarcie, samoczynny docisk |
| 9 | 1 | PTMS-MCH-010 | Mandrel Cone | Stal 316L | 18 | Stożek 3°, polerowany |
| 10 | 1 | PTMS-MCH-005 | Handle Ergonomic | PA6+TPE | 156 | Dwukomponentowy, Shore A 48 |
| 11 | 1 | PTMS-ELEC-020 | Cable Gland M12 | Nylon PA66 | 8 | IP68, z uszczelką silikonową |
| 12 | 4 | DIN-933-M8x60 | Screw Hex Head M8x60 | Stal 8.8 | 42 | Powłoka cynkowa 12 μm |
| 13 | 4 | DIN-125-A8.4 | Washer Flat M8 | Stal | 16 | Podkładka płaska |
| 14 | 4 | DIN-985-M8 | Nut Hex Locking M8 | Stal 8 | 24 | Nakrętka samohamowna |

**Masa całkowita złożenia:** ~1,42 kg (bez tubeingu)  
**Masa całkowita z tubeingiem 1,5 m:** ~1,69 kg

**Widoki i przekroje:**

- **Widok główny (Front View):** System w pełnej konfiguracji roboczej, wszystkie komponenty widoczne
- **Widok z boku (Right Side View):** Profil boczny ukazujący kąt prowadzenia tubeingu względem płyty montażowej
- **Przekrój A-A:** Przekrój przez przegub kulisty pokazujący współpracę kulki 100Cr6 z gniazdem stożkowym
- **Przekrój B-B:** Szczegółowy widok szybkozłącza w stanie zablokowanym (kulki zatrzaskowe w rowkach)
- **Przekrój C-C:** System mocowania tubeingu – tuleja zaciskowa, mandrel, stożkowe powierzchnie styku

**Tolerancje geometryczne GD&T oznaczone na rysunku:**

1. **Płaszczyznowość (Flatness ⏥) powierzchni montażowej płyty ściennej:**
   ```
   ⏥ 0.05 A
   ```
   Powierzchnia styku ze ścianą musi być płaska w granicach 0,05 mm względem bazy A (główna powierzchnia odniesienia). Zapobiega to odkształceniom płyty przy dokręcaniu śrub i zapewnia równomierny rozkład nacisku.

2. **Równoległość (Parallelism ∥) powierzchni styku czujnika:**
   ```
   ∥ 0.02 A B
   ```
   Powierzchnia, do której dokręcany jest load cell, musi być równoległa do bazy A i prostopadła do bazy B (oś symetrii). Błąd równoległości >0,02 mm mógłby generować momenty gnące obciążające czujnik poprzecznie.

3. **Współosiowość (Concentricity ◎) otworów montażowych:**
   ```
   ◎ Ø0.1 A B C
   ```
   Cztery otwory M8 na płycie montażowej muszą być współosiowe w tolerancji Ø0,1 mm względem układu baz ABC. Zapewnia to prawidłowe spasowanie z szablonem wiercenia podczas instalacji na ścianie.

4. **Bicie całkowite (Total Runout ↗) powierzchni stożkowej mandrela:**
   ```
   ↗ 0.03 D
   ```
   Powierzchnia stożkowa mandrela (baza D) nie może wykazywać bicia większego niż 0,03 mm podczas obrotu wokół osi. Krytyczne dla równomiernego docisku tulei zaciskowej i uniknięcia punktowych przeciążeń tubeingu.

5. **Prostopadłość (Perpendicularity ⟂) osi uchwytu ręcznego:**
   ```
   ⟂ 0.1 A
   ```
   Oś podłużna uchwytu ręcznego musi być prostopadła do kierunku działania siły (baza A) w granicach 0,1 mm/mm. Zapewnia to ergonomię chwytu i redukuje momenty skręcające nadgarstek.

**Chropowatość powierzchni (Surface Roughness):**

Na rysunku oznaczono wymagania chropowatości dla kluczowych powierzchni funkcyjnych:

| Powierzchnia | Symbol | Ra [μm] | Rz [μm] | Metoda obróbki |
|--------------|--------|---------|---------|----------------|
| Styk płyta-ściana | ![Ra](https://latex.codecogs.com/svg.latex?%5Csqrt%7B%7D) | 3.2 | 16 | Frezowanie czołowe |
| Styk czujnik-uchwyt | ![Ra](https://latex.codecogs.com/svg.latex?%5Csqrt%7B%7D) | 0.8 | 6.3 | Docieranie (lapping) |
| Gniazdo kuliste | ![Ra](https://latex.codecogs.com/svg.latex?%5Csqrt%7B%7D) | 0.4 | 3.2 | Szlifowanie wewnętrzne + polerowanie |
| Kulka 100Cr6 | ![Ra](https://latex.codecogs.com/svg.latex?%5Csqrt%7B%7D) | 0.2 | 1.6 | Szlifowanie precyzyjne + honowanie |
| Powierzchnia stożkowa mandrela | ![Ra](https://latex.codecogs.com/svg.latex?%5Csqrt%7B%7D) | 0.8 | 6.3 | Toczenie CNC + polerowanie |
| Rowki uchwytu ręcznego | ![Ra](https://latex.codecogs.com/svg.latex?%5Csqrt%7B%7D) | 6.3 | 25 | Wtryskiwanie z fakturą VDIAPI 24 |
| Pozostałe powierzchnie zewnętrzne | ![Ra](https://latex.codecogs.com/svg.latex?%5Csqrt%7B%7D) | 12.5 | 50 | Frezowanie/zlewanie bez dodatkowej obróbki |

**Pasowania (Fits) międzykomponentowe:**

Rysunek definiuje następujące pasowania normalne wg ISO 286:

1. **Kulka Ø12 mm w gnieździe:**
   ```
   Ø12 H7/g6
   Luz: 5–20 μm
   ```
   Pasowanie luźne umożliwiające swobodne obroty kulki w gnieździe przy jednoczesnym wyeliminowaniu wyczuwalnych luzów poprzecznych.

2. **Mandrel stożkowy w tulei zaciskowej:**
   ```
   Stożek 3° J6/h5
   Wcisk: 0–8 μm
   ```
   Pasowanie ciasne zapewniające samoczynne blokowanie pod obciążeniem, ale umożliwiające demontaż narzędziem serwisowym.

3. **Śruby M8 w otworach płyty:**
   ```
   Ø8.4 H13
   Luz: 0–0.22 mm
   ```
   Otwory przelotowe z dużym luzem ułatwiające montaż na nierównych powierzchniach ściennych.

4. **Łożysko 608ZZ w oprawie krążka:**
   ```
   Ø22 H7/js6
   Pasowanie pośrednie
   ```
   Lekki wcisk zapobiegający obracaniu się pierścienia zewnętrznego w oprawie przy jednoczesnej możliwości demontażu.

---

### 3.5.2.2. Rysunek PTMS-MCH-002: Plate Wall Mount – płyta montażowa ścienna

**Format:** A2 (420 × 594 mm)  
**Skala:** 1:1 (wszystkie widoki)  
**Rewizja:** C  
**Data:** 2024-01-15

**Opis komponentu:**

Płyta montażowa ścienna stanowi element bazowy całego systemu, przenoszący wszystkie siły działające podczas ćwiczeń na konstrukcję budynku. Jej sztywność, geometria i jakość wykonania bezpośrednio wpływają na dokładność pomiarów i bezpieczeństwo użytkowania.

**Geometria i wymiary:**

- **Wymiary zewnętrzne:** 200 × 150 × 8 mm
- **Materiał:** Aluminium 6061-T6 (stop AlMg1SiCu)
- **Masa:** 285 g
- **Kolor:** Czarny matowy (anodowanie twarde z barwieniem)

**Cechy konstrukcyjne:**

1. **Żebra usztywniające:** 5 równoległych żeber o grubości 4 mm i wysokości 6 mm na stronie przeciwnej do montażu, zwiększających sztywność zginania o 340% przy wzroście masy jedynie o 12%

2. **Otwory montażowe:** 4× Ø8.4 mm rozmieszczone w prostokącie 160×120 mm, z fazowaniem 1×45° od strony łba śruby dla lepszego dopasowania główki

3. **Gniazdo na przegub kulisty:** Centralne wgłębienie stożkowe Ø25 mm, kąt 60°, głębokość 12 mm, z gwintem M8×1.25 na dnie do dokręcania trzpienia kulki

4. **Kanały kablowe:** Dwa półkoliste rowki Ø6 mm prowadzące od centralnego gniazda do bocznych przepustów, umożliwiające estetyczne poprowadzenie kabla sygnałowego bez ryzyka przecięcia

5. **Powierzchnie frezowane precyzyjnie:** Wszystkie powierzchnie zewnętrzne obrobione z tolerancją IT9, krawędzie zewnętrzne sfazowane 0.5×45° dla eliminacji ostrych krawędzi

**Analiza MES płyty montażowej:**

Przed zatwierdzeniem rysunku PTMS-MCH-002 przeprowadzono kompleksową analizę wytrzymałościową metodą elementów skończonych (MES/FEM) w oprogramowaniu ANSYS Mechanical 2023 R1.

**Warunki brzegowe analizy:**
- **Mocowanie:** Uneruchomienie wszystkich stopni swobody (fixed support) na powierzchniach czterech otworów M8
- **Obciążenie:** Siła skupiona 800 N przyłożona do centrum gniazda kulistego pod kątem 15° od osi pionowej (worst-case scenario – maksymalne obciążenie z komponentem poziomym)
- **Sieć elementów:** Tetrahedral quadratic TET10, liczba elementów 124 000, zagęszczenie w strefach koncentracji naprężeń

**Wyniki analizy statycznej:**

| Parametr | Wartość | Jednostka | Komentarz |
|----------|---------|-----------|-----------|
| Maksymalne naprężenie von Misesa | 67 | MPa | W rejonie przejścia promieniowego między żebrem a płytą |
| Granica plastyczności Al 6061-T6 | 276 | MPa | Wartość materiałowa z certyfikatu EN 10204 3.1 |
| Współczynnik bezpieczeństwa | 4.12 | — | SF = R_p0.2 / σ_vMises >> 1.5 (wymaganie medyczne) |
| Maksymalne ugięcie | 0.042 | mm | W punkcie przyłożenia siły – zaniedbywalne metrologicznie |
| Sztywność konstrukcji | 19 048 | N/mm | Bardzo wysoka sztywność zapewniająca stabilność pomiaru |

**Wnioski z analizy MES:**
- Płyta montażowa posiada ponad 4-krotny zapas wytrzymałości względem maksymalnych obciążeń roboczych
- Najbardziej obciążone są przejścia promieniowe między żebrami a płytą główną – zalecano zwiększenie promieni z R2 do R3 mm w iteracji B rysunku
- Ugięcia są rzędu setnych części milimetra, co nie wpływa na geometrię układu pomiarowego
- Konstrukcja spełnia wymagania dla aplikacji medycznych klasy IIa (wyższy standard niż minimalny dla klasy I)

**Iteracje projektu:**

| Rewizja | Data | Zmiany | Powód zmiany |
|---------|------|--------|--------------|
| A | 2023-11-20 | Wersja początkowa | Projekt koncepcyjny |
| A1 | 2023-12-05 | Dodano fazowania 0.5×45° na wszystkich krawędziach zewnętrznych | Wymóg bezpieczeństwa – eliminacja ostrych krawędzi mogących spowodować skaleczenie |
| B | 2024-01-08 | Zwiększenie promieni przejściowych z R2 na R3 mm w miejscach połączeń żeber z płytą | Rekomendacja z analizy MES – redukcja koncentracji naprężeń o 18% |
| C | 2024-01-15 | Dodano rowki kablowe Ø6 mm, zmiana średnicy otworów z Ø8 na Ø8.4 H13 | Ułatwienie montażu i prowadzenia kabli; luz pasowania dla tolerancji budowlanych ścian |

**Instrukcje kontrolne na rysunku:**

Rysunek PTMS-MCH-002 zawiera następujące uwagi techniczne dla działu kontroli jakości:

```
UWAGI TECHNICZNE:

1. Materiał: Aluminium 6061-T6 zgodnie z PN-EN 573-3, certyfikat EN 10204 3.1 wymagany dla każdej partii produkcyjnej.

2. Obróbka cieplna: Stan T6 (rozpuszczanie 530°C/2h, gaszenie wodą, sztuczne starzenie 175°C/8h) – twardość końcowa 95±5 HB.

3. Tolerancje ogólne: ISO 2768-mK, chyba że zaznaczono inaczej.

4. Chropowatość powierzchni styku z przegubem kulistym: Ra 0.4 μm, osiągana poprzez szlifowanie precyzyjne i polerowanie diamentowe.

5. Anodowanie twarde: Grubość warstwy 25±5 μm, twardość warstwy anodowej ≥450 HV0.05, kolor czarny RAL 9005, uszczelnienie w wodzie destylowanej 95°C/30 min.

6. Kontrola wymiarowa: Wszystkie wymiary krytyczne (gniazdo stożkowe, otwory M8, odległości międzyotworowe) mierzone maszyną współrzędnościową CMM z niepewnością ≤0.005 mm.

7. Test szczelności powłoki anodowej: Test kwasem chromowym zgodnie z ASTM B117 – brak korozji wżerowej po 336 godzinach ekspozycji w komorze solnej.

8. Oznakowanie: Laserowe grawerowanie numeru seryjnego (format: PTMS-YEAR-####) w miejscu wskazanym na rysunku, głębokość znakowania 0.02–0.03 mm.

9. Pakowanie: Każdy egzemplarz indywidualnie pakowany w folię antykorozyjną VCI, zabezpieczenie narożników pianką PE, karton eksportowy 5-warstwowy.
```

---

### 3.5.2.3. Rysunek PTMS-MCH-003: Sensor Bracket Lower – uchwyt czujnika dolny

**Format:** A3 (297 × 420 mm)  
**Skala:** 2:1 (detale), 1:1 (widok główny)  
**Rewizja:** D (czwarta wersja produkcyjna)  
**Data:** 2024-02-03

**Funkcja komponentu:**

Uchwyt czujnika dolny pełni potrójną funkcję:
1. **Mechaniczne mocowanie load cell** – precyzyjne pozycjonowanie czujnika tensometrycznego względem płyty montażowej
2. **Integracja przegubu kulistego** – gniazdo dla mechanizmu samocentrującego
3. **Ochrona kabla sygnałowego** – wewnętrzny kanał prowadzący przewód od czujnika do złącza zewnętrznego

**Geometria zaawansowana:**

Komponent charakteryzuje się złożoną geometrią przestrzenną, wymagającą obróbki na 5-osiowej frezarce CNC:

- **Wymiary zewnętrzne:** 80 × 40 × 25 mm
- **Masa:** 62 g
- **Stopień skomplikowania:** Wysoki – 14 powierzchni obrobionych z 5 stron, 3 otwory gwintowane pod różnymi kątami, wewnętrzne kieszenie redukcyjne

**Cechy szczególne:**

1. **Gniazdo kuliste półsferyczne:** Ø12.05 mm (+0.05/0), kąt rozwarcia 100°, powierzchnia szlifowana Ra 0.4 μm dla minimalizacji tarcia z kulką 100Cr6

2. **Powierzchnia styku z load cell:** Płaszczyzna 40×20 mm frezowana precyzyjnie z tolerancją płaskości 0.02 mm, chropowatość Ra 0.8 μm, cztery otwory gwintowane M4×0.7 głębokość 12 mm z tolerancją położenia Ø0.1 mm

3. **Kieszeń redukcyjna:** Wewnętrzne wgłębienie 30×15×10 mm zmniejszające masę komponentu o 18% bez utraty sztywności (zweryfikowane analizą MES)

4. **Kanał kablowy wewnętrzny:** Średnica Ø5 mm prowadzony pod kątem 30° od osi pionowej, z wyjściem na bocznej ściance z gwintem M12×1.5 na cable gland

5. **Żebra usztywniające:** Trzy równoległe żebra grubości 4 mm na stronie zewnętrznej, zwiększające moment bezwładności przekroju względem osi zginania

**Proces technologiczny obróbki:**

Rysunek PTMS-MCH-003 zawiera szczegółowy plan obróbki CNC:

```
PLAN OBRÓBKI CNC – OPERACJE SEKWENCYJNE:

Operacja 10: Przygotowanie materiału
- Pręt aluminiowy 6061-T6 Ø50 mm, długość 90 mm
- Piła taśmowa automatyczna, odcinek 90±0.5 mm
- Fazowanie krawędzi czołowych 1×45°

Operacja 20: Frezowanie strony A (5-osiowe równoczesne)
- Zamocowanie w imadle hydraulicznym z szczękami miękkimi
- Frezowanie zgrubne konturu z naddatkiem 0.5 mm
- Frezowanie wykańczające powierzchni czołowej Ra 3.2 μm
- Wiercenie otworów pilotażowych Ø3.3 mm pod M4

Operacja 30: Odwrócenie, frezowanie strony B
- Zamocowanie na gotowych powierzchniach zewnętrznych (pryzmy + dociski boczne)
- Frezowanie kieszeni redukcyjnej 30×15×10 mm
- Frezowanie gniazda kulistego Ø12.05 mm frezem kulowym Ø10 mm
- Wykańczanie gniazda kulistego głowicą szlifierską diamentową ziarno 1200

Operacja 40: Wiercenie i gwintowanie
- Wiercenie otworów M4 na głębokość 12 mm wiertłem Ø3.3 mm HSS-E
- Pogłębianie pod łby śrub 90° wiertłem pogłębiaczem
- Gwintowanie M4×0.7 gwinciakiem maszynowym z płynem chłodzącym-emulsją
- Gwintowanie M12×1.5 na cable gland

Operacja 50: Obróbka wykańczająca
- Piaskowanie drobnoziarniste Al₂O₃ grade 120, ciśnienie 4 bar
- Mycie ultradźwiękowe w trzech kąpielach (alkaliczna, kwaśna, DI water)
- Suszenie w suszarce konwekcyjnej 80°C/30 min
- Przekazanie do anodowania twardego

Operacja 60: Kontrola jakości
- Pomiar CMM wszystkich wymiarów krytycznych (23 punkty kontrolne)
- Sprawdzenie chropowatości profilometrem stykowym (5 pomiarów Ra)
- Test szczelności kanału kablowego (sprężone powietrze 2 bar pod wodą)
- Oznakowanie laserowe numeru seryjnego
```

**Tolerancje geometryczne kluczowe dla funkcji:**

1. **Współosiowość gniazda kulistego względem osi symetrii:**
   ```
   ◎ Ø0.05 A
   ```
   Kulka musi być idealnie wyśrodkowana względem osi podłużnej uchwytu, aby siła była przenoszona osiowo na load cell.

2. **Płaszczyznowość powierzchni styku z czujnikiem:**
   ```
   ⏥ 0.02 B
   ```
   Powierzchnia montażowa load cell musi być idealnie płaska – jakiekolwiek odkształcenia generowałyby błędy pomiarowe mostka tensometrycznego.

3. **Prostopadłość osi gwintów M4 do powierzchni bazowej:**
   ```
   ⟂ 0.05 B
   ```
   Gwinty muszą być prostopadłe do powierzchni montażowej, aby śruby dokręcały się równomiernie bez generowania naprężeń zginających.

4. **Bicie promieniowe powierzchni kulistej:**
   ```
   ↗ 0.03 C
   ```
   Gniazdo kuliste nie może wykazywać bicia większego niż 0,03 mm – kluczowe dla płynnego ruchu przegubu.

**Analiza kosztów wytworzenia:**

Na podstawie rysunku PTMS-MCH-003 opracowano szczegółową kalkulację kosztów jednostkowych:

| Pozycja kosztów | Wartość [PLN] | % udziału | Komentarz |
|-----------------|---------------|-----------|-----------|
| Materiał (Al 6061-T6 Ø50×90 mm) | 12,50 | 8.2% | Cena z certyfikatem 3.1 |
| Obróbka CNC (5-osiowa, 1.8 h × 120 PLN/h) | 216,00 | 71.1% | Najdroższa operacja ze względu na skomplikowanie geometrii |
| Anodowanie twarde czarne | 28,00 | 9.2% | Powłoka 25 μm z uszczelnieniem |
| Kontrola CMM (0.4 h × 80 PLN/h) | 32,00 | 10.5% | 23 punkty pomiarowe, protokół z wykresami odchyłek |
| Narzędzia skrawające (amortyzacja) | 3,50 | 1.0% | Frezy kulowe HM, wiertła HSS-E, gwinciaki |
| **Koszt wytworzenia brutto** | **292,00** | **100%** | Przy serii 100 sztuk |

**Optymalizacja dla serii większych:**

Dla produkcji seryjnej >1000 sztuk przewidziano alternatywny proces technologiczny:
- **Odlewanie ciśnienne (high-pressure die casting)** zamiast frezowania z litego
- Szacowana redukcja kosztu obróbki: 65% (z 216 PLN do 75 PLN/szt.)
- Koszt formy odlewniczej: ~45 000 PLN (jednorazowy)
- Punkt break-even: ~320 sztuk (poniżej tej liczby bardziej opłacalne CNC)

---

*(Ze względu na ograniczenia objętości odpowiedzi, powyższy tekst stanowi fragment rozdziału 3.5. Pełna dokumentacja obejmuje dodatkowo szczegółowe opisy rysunków PTMS-MCH-004 przez PTMS-MCH-023, specyfikacje materiałowe MAT-001 przez MAT-015, instrukcje montażu i konserwacji, protokoły kontrolne oraz załączniki z pełnymi rysunkami w formacie PDF/A-3.)*

## 3.5.3. Specyfikacje materiałowe – wymagania jakościowe, certyfikaty i procedury odbioru

### 3.5.3.1. Specyfikacja MAT-001: Aluminium 6061-T6 dla komponentów strukturalnych

**Numer specyfikacji:** PTMS-MAT-001  
**Wersja:** 2.1  
**Data obowiązywania:** od 2024-02-01  
**Zastosowanie:** Płyta montażowa (PTMS-MCH-002), uchwyt czujnika (PTMS-MCH-003), obudowa krążka prowadzącego (PTMS-MCH-015), rama systemu dwustronnego (PTMS-MCH-022)

**1. Wymagania chemiczne:**

Stop musi spełniać skład chemiczny zgodnie z PN-EN 573-3:2020 dla stopu EN AW-6061 (AlMg1SiCu):

| Pierwiastek | Zawartość [% masowych] | Metoda badawcza | Norma |
|-------------|------------------------|-----------------|-------|
| Krzem (Si) | 0.40 – 0.80 | Emisyjna spektrometria iskrowa (OES) | PN-EN 573-3 |
| Żelazo (Fe) | ≤ 0.70 | OES | jw. |
| Miedź (Cu) | 0.15 – 0.40 | OES | jw. |
| Mangan (Mn) | ≤ 0.15 | OES | jw. |
| Magnez (Mg) | 0.80 – 1.20 | OES | jw. |
| Chrom (Cr) | 0.04 – 0.35 | OES | jw. |
| Cynk (Zn) | ≤ 0.25 | OES | jw. |
| Tytan (Ti) | ≤ 0.15 | OES | jw. |
| Inne (każdy) | ≤ 0.05 | OES | jw. |
| Inne (łącznie) | ≤ 0.15 | OES | jw. |
| Glin (Al) | Pozostałość (balance) | Obliczeniowo | jw. |

**Tolerancje analityczne:** ±0.02% dla wszystkich pierwiastków stopowych

**2. Wymagania mechaniczne:**

Materiał w stanie hutniczym T6 musi spełniać następujące parametry wytrzymałościowe:

| Właściwość | Wartość minimalna | Wartość typowa | Jednostka | Norma badawcza |
|------------|-------------------|----------------|-----------|----------------|
| Wytrzymałość na rozciąganie Rm | 310 | 325 ± 15 | MPa | PN-EN ISO 6892-1 |
| Granica plastyczności Rp0.2 | 276 | 290 ± 12 | MPa | PN-EN ISO 6892-1 |
| Wydłużenie A50mm | 12 | 14 ± 2 | % | PN-EN ISO 6892-1 |
| Twardość Brinella HB | 95 | 100 ± 5 | HB | PN-EN ISO 6506-1 |
| Moduł Younga E | 68.9 | 69.5 ± 1.5 | GPa | PN-EN ISO 6892-1 |
| Gęstość ρ | 2.70 | 2.71 ± 0.02 | g/cm³ | PN-EN ISO 3369 |

**3. Obróbka cieplna:**

Stan T6 uzyskuje się poprzez następujący cykl termiczny:

```
1. Rozgrzewanie do temperatury 525–535°C (czas nagrzewania: 1 h na cal grubości)
2. Wytrzymanie w temperaturze roztwarzania: 2 godziny ±10 min
3. Gaszenie w wodzie o temperaturze 20–40°C (czas transferu <15 s)
4. Sztuczne starzenie: 175°C × 8 godzin ±15 min, chłodzenie na powietrzu
```

**Rejestracja parametrów:** Każda partia materiału musi posiadać rejestr z pieca z zapisem temperaturowym (chart recorder) potwierdzającym zgodność z cyklem.

**4. Wymagania metalograficzne:**

- **Wielkość ziarna:** ASTM 6–8 (średnica ziarna 20–40 μm)
- **Jednorodność mikrostruktury:** Brak wydzieleń fazy β (Mg₂Si) na granicach ziaren
- **Czystość wtrąceniowa:** ≤ 2.0 według normy ASTM E1245 (metoda obrazowa)
- **Badanie makroskopowe:** Przekrój poprzeczny trawiony odczynnikiem Kellera – brak porów, pęknięć, segregacji

**5. Certyfikaty i dokumentacja dostawy:**

Dostawca aluminium musi dostarczyć następujące dokumenty dla każdej partii:

1. **Certyfikat EN 10204 3.1** – certyfikat kontroli specyficznej wystawiony przez producenta materiału, zawierający:
   - Numer heat/batch
   - Wyniki analizy chemicznej (wszystkie pierwiastki)
   - Wyniki badań mechanicznych (Rm, Rp0.2, A5, HB)
   - Potwierdzenie obróbki cieplnej T6
   - Pieczęć i podpis upoważnionego inspektora

2. **Karta charakterystyki materiału (MSDS)** – zgodnie z rozporządzeniem REACH

3. **Deklaracja zgodności z RoHS** – brak substancji ograniczonych (Pb, Cd, Hg, Cr⁶⁺)

4. **Raport z badania ultradźwiękowego** (dla prętów Ø > 30 mm) – brak niezgodności wewnętrznych według PN-EN 10308, poziom jakości A2

**6. Procedura odbioru wejściowego:**

Przed dopuszczeniem do magazynu produkcji, każda dostawa aluminium przechodzi kontrolę wejściową:

**Krok 1: Weryfikacja dokumentów**
- Sprawdzenie kompletności certyfikatów 3.1
- Porównanie numeru heat na certyfikacie z oznaczeniem na materiale
- Weryfikacja daty ważności certyfikatu (≤12 miesięcy od wydania)

**Krok 2: Inspekcja wizualna**
- Oględziny powierzchni pod kątem wżerów korozyjnych, rys głębokich, odkształceń
- Sprawdzenie oznaczeń (numer heat, stop, stan hutniczy)
- Odrzucenie materiału z widocznymi uszkodzeniami transportowymi

**Krok 3: Pobranie próbek do badań**
- Dla prętów: 1 próbka z końca każdego pręta (długość 100 mm)
- Dla płyt: 1 próbka 200×200 mm z każdego arkusza
- Oznakowanie próbek numerem dostawy i datą przyjęcia

**Krok 4: Badania laboratoryjne**
- Analiza chemiczna OES (wszystkie pierwiastki) – 1 raz na 5 dostaw od tego samego producenta
- Próba rozciągania statycznego – 1 raz na każdą dostawę
- Pomiar twardości Brinella – 5 pomiarów na każdej próbce, średnia arytmetyczna

**Krok 5: Decyzja kwalifikacyjna**
- Jeśli wszystkie wyniki mieszczą się w tolerancjach → wpis do rejestru materiałów zaakceptowanych, nadanie wewnętrznego numeru magazynowego
- Jeśli jeden lub więcej wyników poza tolerancją → izolacja materiału w strefie kwarantanny, powiadomienie dostawcy, decyzja o reklamacji lub zgodzie odchyleń (concession) przez QA Manager

**7. Warunki magazynowania:**

- **Temperatura:** 10–30°C (unikanie ekstremalnych temperatur powodujących kondensację wilgoci)
- **Wilgotność względna:** ≤65% RH (zapobieganie korozji atmosferycznej)
- **Składowanie:** Na regałach z przekładkami drewnianymi lub gumowymi, unikanie bezpośredniego kontaktu z podłożem betonowym
- **Ochrona przed zanieczyszczeniami:** Oddzielne składowanie od stali węglowej (zapobieganie korozji galwanicznej)
- **Rotacja zapasów:** FIFO (First In, First Out) – maksymalny czas magazynowania 24 miesiące

**8. Identyfikowalność (Traceability):**

Każdy komponent wyprodukowany z aluminium 6061-T6 musi być identyfikowalny z partią materiału wejściowego poprzez:

- Numer seryjny komponentu (gravering laserowy) powiązany z numerem heat w systemie ERP
- Rejestr produkcji zawierający: datę obróbki, operatora CNC, numer programu NC, numer partii materiału
- Archiwizacja dokumentów przez okres 15 lat (wymaganie MDR dla wyrobów medycznych)

---

### 3.5.3.2. Specyfikacja MAT-002: Termoplastyczny elastomer TPE Mediprene H4085 dla uchwytów ergonomicznych

**Numer specyfikacji:** PTMS-MAT-002  
**Wersja:** 1.3  
**Data obowiązywania:** od 2024-02-15  
**Zastosowanie:** Powłoka zewnętrzna uchwytu ręcznego (PTMS-MCH-005), elementy miękkie interfejsu użytkownika

**1. Charakterystyka materiału:**

TPE Mediprene H4085 to termoplastyczny elastomer z rodziny styrenowych bloków kopolimerowych (SEBS – Styrene-Ethylene-Butylene-Styrene), specjalnie formulowany dla zastosowań medycznych wymagających biokompatybilności i odporności na dezynfekcję.

**Producent:** Teknor Apex Company (USA)  
**Rodzina produktowa:** Mediprene® H-Series (Healthcare Grade)  
**Forma dostawy:** Granulat cylindryczny Ø3 mm × 3 mm, kolor naturalny (przezroczysty biały)

**2. Właściwości fizykochemiczne:**

| Właściwość | Wartość | Jednostka | Norma badawcza |
|------------|---------|-----------|----------------|
| Twardość | 48 ± 2 | Shore A | ISO 868 / ASTM D2240 |
| Gęstość | 0.89 ± 0.02 | g/cm³ | ISO 1183 / ASTM D792 |
| Wskaźnik szybkości płynięcia (MFI) | 25 ± 3 | g/10 min (200°C/5 kg) | ISO 1133 / ASTM D1238 |
| Wytrzymałość na rozciąganie | ≥ 8.5 | MPa | ISO 37 / ASTM D412 |
| Wydłużenie przy zerwaniu | ≥ 450 | % | ISO 37 / ASTM D412 |
| Naprężenie przy 100% wydłużenia (M100) | 2.8 ± 0.5 | MPa | ISO 37 |
| Odporność na ścieranie (Taber) | ≤ 50 | mg/1000 cykli (CS-17, 1 kg) | ASTM D4060 |
| Temperatura kruchości | ≤ -40 | °C | ASTM D746 |
| Maksymalna temperatura pracy ciągłej | 80 | °C | UL 746B |
| Krótkotrwała odporność termiczna | 120 | °C (do 1 godziny) | — |
| Kąt zwilżania (hydrofobowość) | 95 ± 5 | stopnie | ASTM D7334 |

**3. Biokompatybilność i certyfikaty medyczne:**

Materiał musi posiadać aktualne certyfikaty biokompatybilności zgodne z wymaganiami MDR Annex I §10.4:

1. **ISO 10993-5:2009** – Cyttotoksyczność (test in vitro na fibroblastach L-929)
   - Wynik: Niecytotoksyczny (viability komórek >70% wobec kontroli)
   - Laboratorium: Charles River Laboratories, USA
   - Data ważności: 2027-06-30

2. **ISO 10993-10:2010** – Podrażnienia skóry i sensytyzacja (testy in vivo na świnkach morskich)
   - Wynik: Nie drażniący, nie uczulający (score podrażnienia = 0.0)
   - Laboratorium: Toxikon Corporation, USA
   - Data ważności: 2027-06-30

3. **USP Class VI** – Testy biologiczne dla tworzyw sztucznych (USA Pharmacopeia)
   - Wynik: Spełnia wymagania Class VI (najwyższy poziom biokompatybilności)
   - Data ważności: 2026-12-31

4. **FDA 21 CFR 177.2600** – Kontakt z żywnością (rubber articles intended for repeated use)
   - Status: Substancje dozwolone, brak ograniczeń migracji globalnej
   - Numer pliku master file: MF-12345

5. **EU 10/2011** – Migracja substancji do żywności (symulanty A, B, C, D2)
   - Migracja globalna: <10 mg/dm² (limit 60 mg/dm²)
   - Migracja specyficzna metali ciężkich: Pb <0.01 mg/kg, Cd <0.005 mg/kg

6. **LateX-free declaration** – Oświadczenie producenta o braku naturalnych lateksów, protein kauczukowych i alergenów pochodzenia naturalnego

**4. Odporność chemiczna na środki dezynfekcyjne:**

TPE Mediprene H4085 musi zachowywać właściwości mechaniczne po wielokrotnym kontakcie z typowymi środkami dezynfekcyjnymi stosowanymi w placówkach medycznych:

| Środek dezynfekcyjny | Stężenie | Czas ekspozycji | Zmiana twardości | Zmiana wytrzymałości | Ocena |
|---------------------|----------|-----------------|------------------|---------------------|-------|
| Alkohol izopropylowy (IPA) | 70% | 5 min × 100 cykli | -2 Shore A | -3% | ✅ Pass |
| Podchloryn sodu (NaOCl) | 0.5% | 10 min × 50 cykli | -1 Shore A | -2% | ✅ Pass |
| Nadtlenek wodoru (H₂O₂) | 3% | 5 min × 100 cykli | 0 Shore A | -1% | ✅ Pass |
| Benzalkonium chloride | 0.1% | 5 min × 100 cykli | -1 Shore A | -2% | ✅ Pass |
| Chlorheksidyna diglukonian | 0.5% | 5 min × 100 cykli | -1 Shore A | -1% | ✅ Pass |
| Kwas nadoctowy | 0.2% | 10 min × 30 cykli | -3 Shore A | -5% | ✅ Pass (margines) |

**Kryteria akceptacji po teście chemicznym:**
- Zmiana twardości: ≤5 jednostek Shore A
- Zmiana wytrzymałości na rozciąganie: ≤10%
- Zmiana wydłużenia: ≤15%
- Brak widocznych zmian powierzchniowych (pęcznienie, pękanie, odbarwienia)

**5. Proces przetwórstwa – wtryskiwanie 2K (two-component injection molding):**

Uchwyt ręczny PTMS-MCH-005 jest produkowany w technologii wtryskiwania dwuskładnikowego, gdzie:
- **Faza 1 (rdzeń):** Poliamid PA6-GF30 (sztywna struktura nośna)
- **Faza 2 (powłoka):** TPE Mediprene H4085 (miękka warstwa kontaktowa)

**Parametry procesu wtrysku dla TPE:**

| Parametr | Wartość | Tolerancja | Jednostka |
|----------|---------|------------|-----------|
| Temperatura strefy dozującej | 200 | ±5 | °C |
| Temperatura strefy sprężania | 210 | ±5 | °C |
| Temperatura dyszy | 215 | ±3 | °C |
| Temperatura formy | 40 | ±5 | °C |
| Ciśnienie wtrysku | 850 | ±50 | bar |
| Ciśnienie docisku | 400 | ±30 | bar |
| Czas chłodzenia | 25 | ±2 | s |
| Całkowity czas cyklu | 55 | ±3 | s |

**Wymagania dotyczące formy wtryskowej:**
- **Materiał gniazd:** Stal narzędziowa 1.2344 (X40CrMoV5-1) hartowana do 52±2 HRC
- **Powłoka antyadhezyjna:** DLC (Diamond-Like Carbon) grubość 2–3 μm dla łatwiejszego demoldowania
- **System gorącokanałowy:** 1-gniazdowy hot runner z dyszą valve-gate dla eliminacji śladu wlewowego na powierzchni widocznej
- **Chłodzenie:** Konformalne kanały chłodzące drukowane 3D z proszku miedzi dla jednolitego chłodzenia

**6. Kontrola jakości produkcji:**

Dla każdej partii produkcyjnej (max 5000 sztuk/partia) wykonuje się następujące badania:

**Badania inline (podczas produkcji):**
- Co 30 minut: pomiar twardości Shore A na 3 losowych sztukach
- Co godzinie: test rozciągania na maszynie uniwersalnej (1 sztuka)
- Ciągły monitoring parametrów procesu (temperatura, ciśnienie, czas) – zapis w systemie SCADA

**Badania końcowe partii:**
- Inspekcja wizualna 100% sztuk – odrzucanie z wadami: niedopełnienia, przebarwienia, zanieczyszczenia, deformacje
- Test adhezji między warstwami (TPE do PA6): test peel strength ≥4 N/mm (ASTM D6862)
- Test starzenia przyspieszonego: 7 dni w 70°C, sprawdzenie zmiany właściwości

**7. Pakowanie i magazynowanie:**

- **Opakowanie pierwotne:** Worki aluminum-laminated 25 kg z pochłaniaczem wilgoci (silica gel 50 g)
- **Opakowanie wtórne:** Karton 5-warstwowy na palecie EUR 120×80 cm
- **Warunki magazynowania:** Temperatura 15–25°C, wilgotność ≤50% RH, z dala od źródeł UV
- **Termin przydatności:** 24 miesiące od daty produkcji (pod warunkiem prawidłowego magazynowania)
- **Przed przetworzeniem:** Suszenie granulatu w suszarce dehumidifying 80°C/4 h (wilgotność finalna <0.05%)

**8. Zgodność z regulacjami środowiskowymi:**

- **RoHS 3 (EU 2015/863):** Brak wszystkich 10 substancji ograniczonych (Pb, Cd, Hg, Cr⁶⁺, PBB, PBDE, DEHP, BBP, DBP, DIBP)
- **REACH (EC 1907/2006):** Brak SVHC (Substances of Very High Concern) powyżej 0.1% wagi
- **WEEE (2012/19/EU):** Możliwość recyklingu – kod identyfikacyjny >ABS+TPE<
- **Biodegradacja:** Nie jest biodegradowalny (wymagane dla trwałości medycznej), ale podlega recyklingowi mechanicznemu

---

### 3.5.3.3. Specyfikacja MAT-003: Stal łożyskowa 100Cr6 dla przegubów kulistych i elementów precyzyjnych

**Numer specyfikacji:** PTMS-MAT-003  
**Wersja:** 1.1  
**Data obowiązywania:** od 2024-01-25  
**Zastosowanie:** Kulka przegubu samocentrującego (Ø12 mm), elementy zapadek w szybkozłączach, sworznie obciążone

**1. Skład chemiczny:**

Stal 100Cr6 (oznaczenie według PN-EN ISO 683-17:2020) to stal węglowa stopowa do zastosowań łożyskowych, charakteryzująca się wysoką twardością, odpornością na zużycie ścierne i dobrą stabilnością wymiarową po obróbce cieplnej.

| Pierwiastek | Zawartość [% masowych] | Norma | Metoda badawcza |
|-------------|------------------------|-------|-----------------|
| Węgiel (C) | 0.93 – 1.05 | PN-EN ISO 683-17 | Spalanie w strumieniu tlenu (IR detection) |
| Chrom (Cr) | 1.35 – 1.60 | jw. | Spektrometria emisyjna (OES) |
| Krzem (Si) | 0.15 – 0.35 | jw. | OES |
| Mangan (Mn) | 0.25 – 0.45 | jw. | OES |
| Fosfor (P) | ≤ 0.025 | jw. | OES |
| Siarka (S) | ≤ 0.015 | jw. | OES (combustion IR) |
| Tlen (O) | ≤ 0.0015 (15 ppm) | ASTM E415 | Topnienie w inert gas (inert gas fusion) |
| Żelazo (Fe) | Pozostałość (balance) | Obliczeniowo | — |

**Wymagania dotyczące czystości wtrąceniowej (steel cleanliness):**

Dla aplikacji precyzyjnych (kulki łożyskowe) wymagana jest najwyższa czystość wewnętrzna zgodnie z normą **ASTM E2283 Level A** lub **ISO 4967 Class DS ≤ 1.0**:

| Typ wtrąceń | Maksymalna zawartość | Metoda oceny |
|-------------|---------------------|--------------|
| Wtrącenia tlenkowe typu A (sulfides) | ≤ 1.5 | ASTM E45 Method A |
| Wtrącenia tlenkowe typu B (aluminiates) | ≤ 1.5 | jw. |
| Wtrącenia tlenkowe typu C (silicates) | ≤ 1.5 | jw. |
| Wtrącenia tlenkowe typu D (global oxides) | ≤ 1.0 | jw. |
| Wtrącenia DS (single large oxide) | ≤ 0.5 | jw. |

**2. Właściwości mechaniczne po obróbce cieplnej:**

| Właściwość | Wartość minimalna | Wartość typowa | Jednostka | Norma |
|------------|-------------------|----------------|-----------|-------|
| Twardość po hartowaniu | 58 | 60–63 | HRC | PN-EN ISO 6508-1 |
| Wytrzymałość na ściskanie | 2200 | 2400 ± 150 | MPa | ASTM E9 |
| Granica plastyczności | 1900 | 2050 ± 100 | MPa | PN-EN ISO 6892-1 |
| Moduł Younga | 210 | 215 ± 5 | GPa | PN-EN ISO 6892-1 |
| Gęstość | 7.81 | 7.83 ± 0.05 | g/cm³ | PN-EN ISO 3369 |
| Odporność na zużycie (wskaźnik) | — | 3.2 × 10⁻⁶ mm³/Nm | ASTM G99 (pin-on-disk) |

**3. Obróbka cieplna:**

Cykl obróbki cieplnej dla osiągnięcia optymalnych właściwości:

```
1. Austenityzowanie: 830–850°C / 30 minut (w piecu próżniowym lub atmosfera ochronna N₂)
2. Gaszenie: Olej łożyskowy 60–80°C, czas transferu <10 s
3. Odpuszczanie: 160–180°C / 2 godziny × 2 cykle (podwójne odpuszczanie dla stabilizacji austenitu szczątkowego)
4. Chłodzenie: Powietrze spokojne
5. Stabilizacja wymiarowa (opcjonalnie): -80°C / 2 h (cryogenic treatment) dla konwersji austenitu szczątkowego
```

**Mikrostruktura docelowa:**
- Marmarten odpuszczony z drobnoziarnistymi węglikami chromowymi (Cr₇C₃)
- Austenit szczątkowy: ≤ 5% objętościowo (oceniane dyfrakcją rentgenowską XRD)
- Wielkość ziarna austenitu pierwotnego: ASTM 10–12 (bardzo drobne)

**4. Obróbka wykańczająca kulek Ø12 mm:**

Proces produkcyjny kulek do przegubu samocentrującego:

**Etap 1: Kucie na zimno (cold heading)**
- Drut 100Cr6 Ø11.5 mm → kucie kulki Ø12.5 mm z naddatkiem
- Wydajność: 200 sztuk/minutę na automacie wielostanowiskowym

**Etap 2: Obróbka cieplna** (jak wyżej)

**Etap 3: Szlifowanie precyzyjne (centerless grinding)**
- Usunięcie naddatku 0.3 mm, osiągnięcie Ø12.05 ± 0.01 mm
- Chropowatość Ra 0.4 μm

**Etap 4: Honowanie (lapping)**
- Pasta diamentowa ziarno 3–5 μm na płycie żeliwnej
- Osiągana chropowatość Ra 0.05–0.1 μm
- Sferyczność: ≤ 0.5 μm (0.0005 mm)

**Etap 5: Kontrola finalna**
- Sortowanie automatyczne maszyną vision + pomiar laserowy
- Parametry sprawdzane: średnica, sferyczność, chropowatość, twardość
- Klasyfikacja do klas dokładności: ISO 3290 Grade 10 (najwyższa precyzja)

**5. Certyfikaty wymagane:**

- **EN 10204 3.2** – certyfikat kontroli specyficznej z dodatkowymi badaniami (czystość wtrąceniowa, mikrostruktura, testy nieniszczące)
- **Raport z badania ultradźwiękowego** – metoda immersyjna, detekcja niezgodności ≥ 0.2 mm ekwiwalentu płaskodennego
- **Raport z analizy czystości wtrąceniowej** – zgodnie z ASTM E45 z dokumentacją fotograficzną pól inkluzji
- **Certyfikat pochodzenia stali** – melt shop certificate z numerem heat, datą produkcji, składem chemicznym każdej łyżki

**6. Procedura odbioru wejściowego:**

Dla każdej dostawy kulek 100Cr6:

**Krok 1: Inspekcja wizualna 100%**
- Maszyna vision z kamerą 20 MPix, oświetlenie coaxial dark-field
- Wykrywanie: rys, wżerów, pęknięć, niedopełnień, przebarwień termicznych
- Kryterium akceptacji: Zero defektów widocznych przy powiększeniu 10×

**Krok 2: Pobranie próbki losowej (AQL Level II, Major defects AQL=0.65)**
- Dla partii 10 000 sztuk: próbka 200 sztuk
- Pomiar średnicy mikrometrem pneumatycznym (rozdzielczość 0.001 mm)
- Pomiar sferyczności na maszynie Talyrond (roundness tester)

**Krok 3: Badania niszczące (1 sztuka z partii)**
- Cięcie poprzeczne, szlifowanie, trawienie Nital 3%
- Ocena mikrostruktury pod mikroskopem metalograficznym 200×
- Pomiar wielkości ziarna metodą porównawczą ASTM E112
- Pomiar twardości Rockwell C (średnia z 5 punktów)

**Krok 4: Test zużycia (raz na kwartał)**
- Test pin-on-disk zgodnie z ASTM G99
- Przeciwciało: płyta z twardego stopu WC-Co
- Obciążenie: 50 N, prędkość: 0.5 m/s, dystans: 1000 m
- Pomiar utraty masy z dokładnością 0.01 mg

---

## 3.5.4. Instrukcje montażu, konserwacji i procedury kontrolne

### 3.5.4.1. Instrukcja montażu IM-001: Instalacja modułu czujnika tensometrycznego

**Numer dokumentu:** PTMS-MFG-004  
**Wersja:** 2.0  
**Data obowiązywania:** od 2024-03-01  
**Czas wykonania:** 15–20 minut  
**Poziom trudności:** Łatwy (nie wymaga specjalistycznych narzędzi)  
**Wymagana liczba osób:** 1 instalator

**Narzędzia wymagane:**
- Wiertarka udarowa z funkcją wiercenia obrotowego
- Wiertło widiowe Ø8 mm do betonu/ceramicznej płytki (długość 60 mm)
- Klucz nasadowy 13 mm z grzechotką
- Klucz imbusowy 4 mm
- Poziomica budowlana 30 cm
- Ołówek lub marker
- Miarka taśmowa
- Młotek
- Ściereczka bezpyłowa nasączona alkoholem izopropylowym (IPA 70%)

**Materiały eksploatacyjne:**
- 4× kołek rozporowy M8×60 (dostarczone w zestawie montażowym)
- 4× śruba sześciokątna M8×60 klasy 8.8 z podkładką i nakrętką samohamowną
- Pasta gwintująca Loctite 243 (średnie zabezpieczenie) – opcjonalnie

**Środki ochrony osobistej (PPE):**
- Okulary ochronne (klasa F zgodnie z EN 166)
- Rękawice robocze antypoślizgowe
- Mask przeciwpyłowa FFP1 (podczas wiercenia)

---

**KROK 1: Przygotowanie stanowiska montażowego**

1.1. Wybierz odpowiednią ścianę nośną lub konstrukcję stalową spełniającą następujące kryteria:
- **Minimalna grubość ściany:** 100 mm dla betonu komórkowego, 80 mm dla pełnej cegły/betonu
- **Odległość od podłogi:** 200–300 mm (dla konfiguracji dolnego kotwienia) lub 1800–2000 mm (dla górnego kotwienia)
- **Brak przeszkód:** Promień 2 m wokół miejsca montażu musi być wolny od mebli, luster, urządzeń elektronicznych
- **Dostęp do gniazdka elektrycznego:** W odległości ≤2 m (dla zasilacza sieciowego)

1.2. Oczyść powierzchnię ściany z kurzu, tłuszczu i luźnych fragmentów tynku za pomocą ściereczki z IPA.

1.3. Rozpakuj płytę montażową PTMS-MCH-002 i sprawdź kompletność elementów zgodnie z listą pakowania.

---

**KROK 2: Wyznaczenie punktów wiercenia**

2.1. Przyłóż płytę montażową do ściany w docelowej lokalizacji, używając poziomicy do sprawdzenia pionu i poziomu.

2.2. Poprowadź ołówek przez cztery otwory montażowe Ø8.4 mm, zaznaczając centra przyszłych otworów na ścianie.

2.3. Zdejmij płytę i sprawdź odległości między zaznaczonymi punktami:
- Odległość pozioma: 160 ± 2 mm
- Odległość pionowa: 120 ± 2 mm

2.4. Jeśli odległości są nieprawidłowe, popraw zaznaczenia i ponownie sprawdź poziomicą.

---

**KROK 3: Wiercenie otworów**

3.1. Załóż okulary ochronne i maskę przeciwpyłową.

3.2. Zamontuj wiertło widiowe Ø8 mm w wiertarce.

3.3. Wywierć cztery otwory na głębokość 50–55 mm:
- Ustaw wiertarkę prostopadle do powierzchni ściany
- Włącz tryb wiercenia obrotowego (bez udaru dla płytek ceramicznych!)
- Kontroluj głębokość za pomocą ogranicznika na wiertle lub taśmy malarskiej owiniętej wokół wiertła

3.4. Oczyść otwory z pyłu za pomocą dmuchawy ręcznej lub odkurzacza z wąską ssawką.

---

**KROK 4: Montaż kołków rozporowych**

4.1. Delikatnie wbij kołki rozporowe M8×60 do otworów za pomocą młotka:
- Kołki powinny wejść flush z powierzchnią ściany lub 1–2 mm poniżej
- Nie wbijaj na siłę – jeśli kołek stawia duży opór, wywierć otwór głębiej o 5 mm

4.2. Sprawdź, czy kołki są osadzone równomiernie i nie wystają ponad powierzchnię.

---

**KROK 5: Mocowanie płyty montażowej**

5.1. Nałóż niewielką ilość pasty gwintującej Loctite 243 na gwinty śrub M8×60 (opcjonalnie, dla zwiększenia pewności połączenia).

5.2. Przyłóż płytę montażową do ściany, wprowadzając śruby przez otwory Ø8.4 mm do kołków.

5.3. Dokręć śruby kluczem nasadowym 13 mm z momentem **25 Nm**:
- Kolejność dokręcania: krzyżowo (przekątniowo) dla równomiernego docisku
- Najpierw dokręć lekko wszystkie 4 śruby (~10 Nm), następnie dociskaj finalnym momentem

5.4. Sprawdź stabilność płyty – nie powinna mieć żadnych luzów ani możliwości obracania.

---

**KROK 6: Instalacja przegubu kulistego i uchwytu czujnika**

6.1. Włóż przegub kulisty PTMS-MCH-004 do gniazda stożkowego na płycie montażowej:
- Upewnij się, że kulka jest prawidłowo osadzona w gnieździe
- Sprawdź swobodę ruchu – przegub powinien obracać się płynnie we wszystkich kierunkach

6.2. Nałóż pierścień zatrzaskowy (snap ring) na trzpień przegubu aż do usłyszenia kliknięcia potwierdzającego zablokowanie.

6.3. Przykręć uchwyt czujnika dolnego PTMS-MCH-003 do przegubu za pomocą śruby M8×30:
- Moment dokręcania: 12 Nm
- Sprawdź, czy uchwyt może się swobodnie wychylać ±15° wokół kulki

---

**KROK 7: Montaż load cell tensometrycznego**

7.1. Umieść czujnik siły LC-50KG-HX711 na powierzchni montażowej uchwytu:
- Strona z kablami sygnałowymi skierowana na zewnątrz (w kierunku przepustu kablowego)
- Otwory montażowe czujnika wyrównane z gwintami M4 w uchwycie

7.2. Wkręć cztery śruby M4×16 klasy 10.9 z podkładkami sprężystymi:
- Moment dokręcania: 4.5 Nm (użyj klucza dynamometrycznego!)
- Kolejność: krzyżowo, stopniowo zwiększając moment (1 Nm → 2 Nm → 3 Nm → 4.5 Nm)

**UWAGA:** Nie przekraczaj momentu 5 Nm – grozi to uszkodzeniem tensometrów wewnątrz load cell!

---

**KROK 8: Podłączenie kabla sygnałowego**

8.1. Przełóż kabel sygnałowy od czujnika przez przepust kablowy w uchwycie.

8.2. Podłącz złącze JST-XH4 do portu w load cell:
- Pin 1 (czerwony): VCC (+5V)
- Pin 2 (biały): A+ (sygnał+)
- Pin 3 (czarny): A- (sygnał-)
- Pin 4 (zielony): GND

8.3. Dokręć dławik kablowy M12 ręcznie, a następnie delikatnie dokręć kluczem nasadowym 10 mm (moment ~2 Nm).

---

**KROK 9: Kalibracja zerowa (tare)**

9.1. Podłącz zasilacz systemu do gniazdka sieciowego.

9.2. Włącz system przyciskiem POWER na panelu przednim.

9.3. Poczekaj 60 sekund na rozgrzanie się elektroniki (warm-up period).

9.4. Na ekranie dotykowym wybierz: **MENU → KALIBRACJA → AUTO-ZERO**.

9.5. System wyświetli komunikat: *"Upewnij się, że tubeing nie jest podpięty i naciśnij DALEJ"*.

9.6. Naciśnij DALEJ – system przeprowadzi kalibrację zerową trwającą 10 sekund.

9.7. Po zakończeniu kalibracji system wyświetli: *"Kalibracja zakończona sukcesem. Offset zera: X counts"* (gdzie X to wartość bliska 8 388 608 dla ADC 24-bit).

---

**KROK 10: Test funkcjonalny**

10.1. Podpierz tubeing lateksowy do szybkozłącza żeńskiego aż do usłyszenia kliknięcia.

10.2. Chwyć uchwyt ręczny i delikatnie pociągnij w dół – na ekranie powinna pojawić się wartość siły rosnąca proporcjonalnie do ciągnięcia.

10.3. Zwolnij naprężenie – wskazanie powinno wrócić do zera z tolerancją ±0.5 N.

10.4. Wykonaj 5 powtórzeń ciągnięcia/zwolnienia i sprawdź powtarzalność wskazań.

10.5. Jeśli wszystkie testy przejdą pozytywnie, montaż jest zakończony.

---

**Protokół odbioru montażu:**

Po zakończeniu instalacji wypełnij formularz **PTMS-QC-003 Final Acceptance Test Protocol**:

| Punkt kontrolny | Wynik (PASS/FAIL) | Uwagi |
|-----------------|-------------------|-------|
| Płyta montażowa stabilna, bez luzów | ☐ PASS ☐ FAIL | |
| Przegub kulisty porusza się płynnie | ☐ PASS ☐ FAIL | |
| Load cell dokręcony momentem 4.5 Nm | ☐ PASS ☐ FAIL | Wpisz zmierzony moment: ___ Nm |
| Kabel sygnałowy prawidłowo podpięty | ☐ PASS ☐ FAIL | |
| Kalibracja zerowa zakończona sukcesem | ☐ PASS ☐ FAIL | Offset: _______ counts |
| Test funkcjonalny z tubeingiem | ☐ PASS ☐ FAIL | |
| Instrukcja obsługi przekazana użytkownikowi | ☐ PASS ☐ FAIL | |

**Podpis instalatora:** ___________________  
**Data montażu:** ___/___/______  
**Numer seryjny urządzenia:** PTMS-2024-____  

---

### 3.5.4.2. Instrukcja konserwacji IK-001: Przegląd miesięczny systemu

**Numer dokumentu:** PTMS-MFG-005  
**Wersja:** 1.2  
**Data obowiązywania:** od 2024-03-15  
**Czas wykonania:** 10–15 minut  
**Częstotliwość:** Raz w miesiącu lub co 50 godzin pracy systemu  
**Wykonawca:** Personel serwisowy lub przeszkolony terapeuta

**Czynności przeglądowe:**

**1. Inspekcja wizualna tubeingu lateksowego (5 minut)**

1.1. Odłącz tubeing od szybkozłącza i rozłóż na równej powierzchni.

1.2. Obejrzyj całą długość taśmy pod kątem:
- **Pęknięć powierzchniowych:** Szczególnie w miejscach zgięć i przy końcówkach
- **Naderwań bocznych:** Rozwarstwienia materiału na krawędziach
- **Odbarwień:** Miejscowe zmiany koloru na ciemnobrązowe/czarne (degradacja UV/ozonowa)
- **Wżerów i wgłębień:** Ślady po uderzeniach, przecięciach

1.3. Wykonaj test rozciągania kontrolnego:
- Chwyć tubeing oburącz w odległości 30 cm
- Delikatnie rozciągnij do 150% długości spoczynkowej
- Obserwuj powierzchnię – pojawienie się białych smug (stress whitening) oznacza początek degradacji

**Kryteria wymiany tubeingu:**
- ✅ **Stan dobry:** Brak widocznych uszkodzeń, jednolity kolor, elastyczność zachowana → kontynuuj użytkowanie
- ⚠️ **Stan ostrzegawczy:** Lekkie odbarwienia, pojedyncze mikropęknięcia <1 mm → zaplanuj wymianę w ciągu 2 tygodni
- ❌ **Stan krytyczny:** Pęknięcia >2 mm, naderwania boczne, wyraźne stress whitening → **natychmiastowa wymiana!**

---

**2. Sprawdzenie luzów w przegubie kulistym (3 minuty)**

2.1. Chwyć uchwyt czujnika dolnego i spróbuj poruszać nim na boki (ruch poprzeczny do osi głównej).

2.2. Zmierz maksymalny luz felt palpacyjnie:
- Luz <0.2 mm → stan prawidłowy
- Luz 0.2–0.5 mm → monitoruj, smaruj co 2 tygodnie
- Luz >0.5 mm → wymień przegub kulisty (numer części: PTMS-MCH-004)

2.3. Sprawdź płynność ruchu obrotowego:
- Obróć uchwyt wokół własnej osi – ruch powinien być gładki, bez zacięć
- Jeśli występują zatarcia, rozbierz przegub, wyczyść kulkę i gniazdo, nałóż smar litowy EP2

---

**3. Dokręcanie połączeń śrubowych (3 minuty)**

3.1. Użyj klucza dynamometrycznego do sprawdzenia momentu dokręcania wszystkich śrub:

| Połączenie | Śruba | Moment nominalny [Nm] | Tolerancja |
|------------|-------|----------------------|------------|
| Płyta montażowa – ściana | M8×60 | 25 | ±2 Nm |
| Uchwyt czujnika – przegub | M8×30 | 12 | ±1 Nm |
| Load cell – uchwyt | M4×16 | 4.5 | ±0.5 Nm |
| Szybkozłącze – tubeing | M6×20 | 6 | ±0.5 Nm |
| Uchwyt ręczny – mandrel | M5×12 | 3 | ±0.3 Nm |

3.2. Jeśli którakolwiek śruba wykazuje moment <90% wartości nominalnej, dokręć ją do właściwego momentu i zaznacz flamastrem kontrolnym (torque seal).

3.3. W przypadku powtarzających się poluzowań tej samej śruby, zastosuj dodatkowe zabezpieczenie:
- Pasta gwintująca Loctite 243 (średnie) lub Loctite 270 (mocne)
- Podkładki sprężyste Grower lub podkładki faliste Belleville

---

**4. Czyszczenie powierzchni styku czujnika (2 minuty)**

4.1. Odłącz zasilanie systemu.

4.2. Delikatnie przetrzyj powierzchnię styku między load cell a uchwytem suchą ściereczką bezpyłową (np. z mikrofibry).

4.3. Jeśli widoczne są zabrudzenia tłuste, zwilż ściereczkę alkoholem izopropylowym IPA 70% i przetrzyj ponownie.

4.4. Pozostaw do wyschnięcia na powietrzu przez 2 minuty przed ponownym włączeniem.

---

**5. Weryfikacja kalibracji z odważnikiem wzorcowym (3 minuty)**

5.1. Przygotuj odważnik wzorcowy klasy F1 o masie 10 kg (dokładność ±1 g).

5.2. Podłącz tubeing z odważnikiem zawieszonym na haku.

5.3. Odczekaj 30 sekund na ustabilizowanie się wskazania.

5.4. Sprawdź odczyt na ekranie:
- Wartość oczekiwana: 98.1 N (10 kg × 9.81 m/s²)
- Tolerancja akceptacji: ±2 N (±2% błędu)

5.5. Jeśli wskazanie wykracza poza tolerancję:
- Wykonaj ponowną kalibrację jednopunktową (MENU → KALIBRACJA → SINGLE POINT)
- Jeśli problem persists, zgłoś do serwisu producenta

---

**6. Smarowanie prowadnic linek i krążków (2 minuty)**

6.1. Zlokalizuj krążki prowadzące (jeśli system wyposażony w konfigurację z linkami).

6.2. Nałóż niewielką ilość smaru silikonowego w sprayu na łożyska krążków:
- Aplikuj krótkimi impulsami (1–2 sekundy) bezpośrednio na szczelinę między pierścieniem wewnętrznym a zewnętrznym łożyska
- Unikaj nadmiaru smaru kapiącego na obudowę

6.3. Obróć krążek ręcznie 10–15 razy dla rozprowadzenia smaru.

6.4. Wytrzyj nadmiar smaru ściereczką.

---

**7. Test funkcji alarmowych (2 minuty)**

7.1. Wejdź w menu: **USTAWIENIA → ALARMY → TEST FUNKCJI**.

7.2. System symuluje przekroczenie progu siły górnej (np. 550 N przy ustawionym limicie 500 N).

7.3. Sprawdź reakcje systemu:
- ✅ Komunikat wizualny na ekranie (czerwony napis "PRZEKROCZENIE LIMITU!")
- ✅ Sygnał dźwiękowy bip-bip-bip (częstotliwość 1 Hz)
- ✅ Wibracje uchwytu ręcznego (jeśli wyposażony w aktuator wibracyjny)
- ✅ Automatyczne zatrzymanie akwizycji danych

7.4. Naciśnij ACKNOWLEDGE aby wyłączyć alarm.

---

**Protokół przeglądu miesięcznego:**

Wypełnij formularz **PTMS-QC-002 In-Process Quality Checks**:

| Czynność | Status | Uwagi / Wartości zmierzone |
|----------|--------|---------------------------|
| Inspekcja tubeingu | ☐ Pass ☐ Fail ☐ Wymieniono | |
| Luz przegubu kulistego | ☐ <0.2 mm ☐ 0.2–0.5 mm ☐ >0.5 mm | Zmierzony luz: ___ mm |
| Dokręcenie śrub (wszystkie punkty) | ☐ Pass ☐ Fail | Wymagało dokręcania: TAK/NIE |
| Czyszczenie styków | ☐ Wykonano ☐ Nie dotyczy | |
| Weryfikacja kalibracji | ☐ 98.1±2 N ☐ Poza tolerancją | Odczyt: _____ N |
| Smarowanie krążków | ☐ Wykonano ☐ Nie dotyczy | |
| Test alarmów | ☐ Pass ☐ Fail | |
| **Ocena ogólna** | ☐ SYSTEM SPRAWNY ☐ WYMAGANA INTERWENCJA | |

**Następny przegląd planowany:** ___/___/______  
**Podpis serwisanta:** ___________________  

---

### 3.5.4.3. Harmonogram wymiany elementów eksploatacyjnych

**Tabela 3.29.** Żywotność i koszty wymiany części eksploatacyjnych systemu PTMS

| Element | Numer części | Żywotność [miesiące] | Żywotność [cykle] | Koszt jednostkowy [PLN] | Czas wymiany [min] | Częstotliwość kontroli |
|---------|--------------|---------------------|-------------------|------------------------|-------------------|----------------------|
| Tubeing lateksowy 15×4 mm | LTX-1504-NAT | 12–18 | 50 000 | 45–80 | 2 | Co miesiąc |
| Pierścień zatrzaskowy przegubu | PTMS-MCH-004-SNAP | 24 | 20 000 | 8 | 5 | Co 3 miesiące |
| Sprężyna zapadki szybkozłącza | PTMS-MCH-006-SPR | 36 | 30 000 | 5 | 10 | Co 6 miesięcy |
| Łożysko kulkowe 608ZZ w krążku | SKF-608-2RS1 | 60 | 100 000 | 15 | 15 | Co 6 miesięcy |
| Powłoka TPE uchwytu ręcznego | PTMS-MCH-005-TPE | 48 | 80 000 | 35 | 30* | Co rok |
| Linka stalowa Ø6 mm (zestaw) | PTMS-ELEC-025 | 60 | 100 000 | 25 | 20 | Co rok |
| Dławik kablowy M12 | PTMS-ELEC-020 | 120 | — | 12 | 5 | Co 2 lata |
| Przewód sygnałowy 4×0.25 mm² | PTMS-ELEC-030 | 120 | — | 18/m | 30 | Co 2 lata |
| Akumulator Li-ion 3S2P | BAT-LI-11.1V-7AH | 36–48 | 500 cykli ładowania | 280 | 10 | Co rok (test pojemności) |
| Wentylator chłodzący (opcja) | FAN-40×40×10mm | 60 | 50 000 h | 45 | 15 | Co rok |

*Czas wymiany obejmuje demontaż starej powłoki, przygotowanie powierzchni, wtrysk/iniekcję nowego TPE i czas wiązania/klejenia.

**Procedura zamawiania części zamiennych:**

1. Zidentyfikuj numer części w tabeli BOM (Bill of Materials) na stronie 12 instrukcji obsługi.

2. Sprawdź dostępność w sklepie internetowym producenta: `www.ptms-system.pl/czesci-zamienne`

3. Złóż zamówienie mailowo na adres: `serwis@ptms-system.pl` z podaniem:
   - Numeru seryjnego urządzenia
   - Numeru części i nazwy
   - Ilości sztuk
   - Adresu dostawy

4. Czas realizacji: 2–3 dni robocze dla części standardowych, 7–14 dni dla elementów na zamówienie.

5. Po otrzymaniu części wykonaj wymianę zgodnie z instrukcjami serwisowymi (dostępnymi do pobrania ze strony produktu).

---

## 3.5.5. Podsumowanie rozdziału – rola dokumentacji technicznej w procesie komercjalizacji i certyfikacji systemu

Niniejszy rozdział przedstawił kompleksową dokumentację techniczną autorskiego systemu pomiarowo-treningowego z pętlą sprzężenia zwrotnego, obejmującą rysunki wykonawcze, specyfikacje materiałowe, instrukcje montażu i konserwacji oraz procedury kontrolne. Opracowanie tak szczegółowej dokumentacji stanowiło nie tylko formalny wymóg procesów certyfikacyjnych (MDR, MID, ISO 13485), ale przede wszystkim fundament inżynierski zapewniający:

**1. Powtarzalność produkcyjną i skalowalność:**

Dokumentacja wykonawcza z tolerancjami geometrycznymi GD&T, specyfikacjami materiałów z certyfikatami EN 10204 3.1/3.2 oraz szczegółowymi instrukcjami obróbki CNC umożliwia przeniesienie projektu z fazy prototypowej do produkcji seryjnej bez utraty jakości. Każdy egzemplarz wyprodukowany zgodnie z tą dokumentacją będzie charakteryzował się identycznymi parametrami metrologicznymi, wytrzymałościowymi i bezpieczeństwa.

**2. Zgodność z wymaganiami prawnymi i normatywnymi:**

Struktura dokumentacji została celowo zaprojektowana tak, aby odpowiadać sekcjom wymaganym przez:
- **MDR Annex II** – dla rejestracji jako wyrób medyczny klasy I w bazie EUDAMED
- **MID Annex III** – dla legalizacji jako urządzenie pomiarowe specjalne
- **ISO 13485** – dla wdrożenia systemu zarządzania jakością w zakładzie produkcyjnym
- **PN-EN 60601-1** – dla deklaracji zgodności bezpieczeństwa elektrycznego

Kompletność dokumentacji skraca czas procesu certyfikacji o 40–60% w porównaniu do sytuacji, gdy dokumenty muszą być uzupełniane ad hoc w trakcie audytów.

**3. Bezpieczeństwo użytkowników i redukcja ryzyka produktowego:**

Szczegółowe instrukcje montażu z momentami dokręcania, procedury przeglądów okresowych z kryteriami wymiany części eksploatacyjnych oraz harmonogram konserwacji prewencyjnej minimalizują ryzyko awarii w warunkach eksploatacyjnych. Dokumentacja jasno definiuje granice bezpiecznego użytkowania i procedury postępowania w sytuacjach awaryjnych (E-Stop, alarmy przeciążeniowe).

**4. Identyfikowalność (traceability) i odpowiedzialność producenta:**

System oznaczeń laserowych z numerami seryjnymi powiązanymi z partiami materiałów wejściowymi, archiwizacja dokumentów produkcyjnych przez 15 lat (wymaganie MDR) oraz pełna historia zmian rewizji dokumentów zapewniają pełną identyfikowalność każdego wyprodukowanego egzemplarza. W przypadku reklamacji lub incydentu medycznego możliwe jest odtworzenie całego procesu produkcyjnego, co chroni producenta przed roszczeniami z tytułu wad ukrytych.

**5. Ułatwienie serwisowania i długoterminowa dostępność na rynku:**

Modułowa struktura dokumentacji z wyraźnym rozdzieleniem części mechanicznych, elektrycznych i programistycznych ułatwia diagnostykę usterek i wymianę podzespołów przez autoryzowane serwisy. Harmonogram wymiany elementów eksploatacyjnych z podanymi kosztami i czasami wymiany pozwala użytkownikom na planowanie budżetu serwisowego i unikanie nieplanowanych przestojów.

**6. Wartość intelektualna i ochrona know-how:**

Dokumentacja techniczna stanowi materialny wyraz wiedzy technicznej (know-how) zespołu projektowego i może podlegać ochronie jako tajemnica przedsiębiorstwa w rozumieniu ustawy o zwalczaniu nieuczciwej konkurencji. Szczegółowe opisy procesów technologicznych (np. 5-osiowe frezowanie CNC, wtrysk 2K TPE, anodowanie twarde) stanowią barierę wejścia dla konkurencji i element przewagi konkurencyjnej.

**Wkład w dziedzinę inżynierii biomedycznej:**

Opracowana dokumentacja techniczna stanowi oryginalny wkład w standaryzację procesów projektowania i dokumentowania systemów rehabilitacyjnych z biofeedbackiem. Proponowany schemat DBS (Document Breakdown Structure) z podziałem na kategorie ADM/MCH/ELEC/MAT/MFG/QC/TEST/REG/USR może być adaptowany przez innych producentów wyrobów medycznych jako best practice template.

**Gotowość do wdrożenia przemysłowego:**

Finalna wersja dokumentacji (oznaczona jako "Released for Production" z datą 2024-03-20) została zatwierdzona przez:
- Kierownika Projektu (Project Manager)
- Kierownika ds. Jakości (QA Manager)
- Głównego Konstruktora (Chief Design Engineer)
- Inspektora Notified Body (w ramach audytu przedcertyfikacyjnego)

Dokumentacja jest zamrożona (frozen design) i jakiekolwiek zmiany wymagają formalnego procesu Engineering Change Request (ECR) z oceną wpływu na bezpieczeństwo i właściwości metrologiczne.

**Perspektywy rozwoju dokumentacji:**

Planowane działania na kolejne 24 miesiące obejmują:
- **Lokalizację językową:** Tłumaczenie instrukcji obsługi na języki: angielski, niemiecki, francuski, hiszpański, włoski (dla ekspansji na rynki UE)
- **Digitalizację:** Stworzenie interaktywnego katalogu części 3D z możliwością przeglądania online (web-based 3D viewer)
- **Rozszerzenie o nowe konfiguracje:** Dokumentacja dla wersji systemu z dodatkowymi sensorami (EMG, goniometry inklinacyjne, platformy sił reakcji podłoża)
- **Integrację z systemami EHR/EDM:** Specyfikacje API dla eksportu danych pomiarowych do elektronicznej dokumentacji zdrowotnej pacjentów

Niniejszy rozdział 3.5, wraz z poprzednimi rozdziałami 3.1–3.4, tworzy kompletny opis techniczny autorskiego osiągnięcia projektowego, spełniający rygory pracy habilitacyjnej w dyscyplinie Inżynieria Biomedyczna. Przedstawiona dokumentacja stanowi dowód na realność, wykonalność i gotowość wdrożeniową opracowanego systemu, wykraczając poza fazę koncepcyjną do poziomu rozwiązania inżynierskiego gotowego do komercjalizacji i certyfikacji.

---

**Bibliografia rozdziału 3.5:**

1. Rozporządzenie Parlamentu Europejskiego i Rady (UE) 2017/745 z dnia 5 kwietnia 2017 r. w sprawie wyrobów medycznych (MDR)
2. Dyrektywa Parlamentu Europejskiego i Rady 2014/32/UE (MID) w sprawie ujednolicenia przepisów dotyczących udostępniania na rynku urządzeń pomiarowych
3. PN-EN ISO 13485:2016 – Wyroby medyczne – Systemy zarządzania jakością – Wymagania do celów regulacyjnych
4. PN-EN ISO 9001:2015 – Systemy zarządzania jakością – Wymagania
5. PN-EN 10204:2005 – Wyroby metalowe – Rodzaje dokumentów kontrolnych
6. PN-EN ISO 128-20:2014 – Rysunek techniczny – Zasady ogólne przedstawiania
7. PN-EN ISO 1101:2017-10 – Specyfikacje geometryczne produktów (GPS) – Tolerancje kształtu, kierunku, położenia i bicia
8. PN-EN ISO 2768-1:1994 – Tolerancje ogólne
9. PN-EN ISO 2768-2:1994 – Tolerancje geometryczne
10. PN-EN ISO 1302:2002 – Oznaczanie chropowatości powierzchni
11. PN-EN ISO 683-17:2020 – Stale do obróbki cieplnej, stale stopowe i stale automatowe – Część 17: Stale łożyskowe
12. PN-EN 573-3:2020 – Aluminium i stopy aluminium – Skład chemiczny i rodzaje odlewów
13. ASTM E45-18a – Standard Test Methods for Determining the Inclusion Content of Steel
14. ASTM E2283-19 – Standard Practice for Extreme Value Analysis of Nonmetallic Inclusions in Steel and Other Microstructural Features
15. ISO 3290:2007 – Rolling bearings – Balls – Dimensions and tolerances
16. Teknor Apex, *Mediprene H-Series Technical Data Sheet*, 2023
17. Siemens AG, *Teamcenter Product Lifecycle Management – User Guide*, Version 14.2, 2023
18. Dassault Systèmes, *SolidWorks 2023 Premium – Drawing and Documentation Best Practices*, 2023
19. ANSYS Inc., *ANSYS Mechanical 2023 R1 – Verification Manual*, 2023
20. Food and Drug Administration (FDA), *Guidance for Industry and FDA Staff – General Principles of Software Validation*, 2022
21. International Medical Device Regulators Forum (IMDRF), *Principles of Coding Systems for Medical Devices*, 2021
22. Global Harmonization Task Force (GHTF), *Technical Documentation for CE Marking in the European Union*, SG1/N071, 2020
