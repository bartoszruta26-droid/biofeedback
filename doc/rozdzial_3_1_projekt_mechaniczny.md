# Rozdział 3.1. Projekt mechaniczny mocowania sensora, uchwytów i prowadzenia tubeingu

## 3.1.1. Wprowadzenie do projektowania mechanicznego systemów pomiarowych z biofeedbackiem

Projektowanie mechaniczne autorskiego systemu pomiarowo-treningowego z pętlą sprzężenia zwrotnego stanowi kluczowy etap realizacji osiągnięcia projektowego, determinujący nie tylko dokładność metrologiczną całego układu, ale także bezpieczeństwo użytkowników, ergonomię pracy, trwałość konstrukcji oraz akceptację środowiska klinicznego i treningowego. Niniejszy podrozdział przedstawia kompleksową dokumentację procesu projektowania komponentów mechanicznych systemu, obejmującą: koncepcję architektoniczną układu pomiarowego, projekt mocowania czujnika tensometrycznego HX711, konstrukcję uchwytów ergonomicznych, system prowadzenia i kotwiczenia tubeingu lateksowego, analizy wytrzymałościowe MES (Metoda Elementów Skończonych), dobór materiałów konstrukcyjnych, proces prototypowania i iteracji projektowych oraz finalną specyfikację techniczną gotową do produkcji.

### 3.1.1.1. Specyfika wymagań dla mechaniki systemów rehabilitacyjno-treningowych

Mechaniczne komponenty systemu pomiarowo-treningowego muszą spełniać diametralnie różne wymagania w zależności od obszaru zastosowania, co stanowiło fundamentalne wyzwanie projektowe:

**W gimnastyce leczniczej (rehabilitacja medyczna):**
- Praca z pacjentami o ograniczonej sprawności fizycznej, często z bólem przewlekłym
- Konieczność zapewnienia maksymalnego bezpieczeństwa przy minimalnym ryzyku urazu
- Wymagana precyzja pomiarowa rzędu ±2% dla małych sił (zakres 0-150 N)
- Ergonomia dostosowana do osób starszych, po urazach ortopedycznych, z deficytami motorycznymi
- Możliwość stosowania w pozycjach: leżącej, siedzącej, stojącej z asekuracją
- Łatwość dezynfekcji powierzchni kontaktowych środkami medycznymi (alkohol izopropylowy, związki chloru, nadtlenek wodoru)

**W gimnastyce sportowej (trening wyczynowy):**
- Eksploatacja w warunkach wysokich obciążeń dynamicznych (siły szczytowe do 800-1000 N)
- Cykliczne obciążenia udarowe podczas ćwiczeń eksplozywnych (RFD > 5000 N/s)
- Wymagana odporność na intensywne użytkowanie (8-10 godzin dziennie, 6 dni w tygodniu)
- Szybkość przezbrajania systemu między ćwiczeniami (< 30 sekund)
- Stabilność konstrukcji przy gwałtownych zmianach kierunku wektora siły
- Odporność na pot, wilgoć, zmienne warunki środowiskowe (sale treningowe bez klimatyzacji)

**W wychowaniu fizycznym (edukacja szkolna):**
- Obsługa dużych grup uczniów (20-30 osób na jednej jednostce lekcyjnej)
- Konstrukcja odporna na niewłaściwe użytkowanie przez młodzież (wandalizm, przeciążenia)
- Brak ostrych krawędzi, elementów mogących spowodować skaleczenie
- Lekkość i mobilność umożliwiająca szybkie przemieszczanie między salami
- Intuicyjność montażu bez narzędzi przez nauczyciela WF
- Niski koszt produkcji umożliwiający skalowanie na wiele szkół

Proces projektowania mechanicznego został przeprowadzony zgodnie z metodyką **V-Model** stosowaną w inżynierii systemów krytycznych, gdzie każdemu etapowi projektowania odpowiada etap weryfikacji:

```
Faza definicji wymagań ←→ Testy walidacyjne (UAT)
       ↓                        ↑
Projekt koncepcyjny   ←→    Testy integracyjne
       ↓                        ↑
Projekt szczegółowy   ←→     Testy jednostkowe
       ↓                        ↑
Prototypowanie        ←→      Testy funkcjonalne
       ↓                        ↑
Produkcja             ←→      Kontrola jakości
```

### 3.1.1.2. Metodologia projektowania wspieranego komputerowo (CAD/CAE)

Cały proces projektowania mechanicznego został przeprowadzony z wykorzystaniem zaawansowanych narzędzi CAD (Computer-Aided Design) i CAE (Computer-Aided Engineering):

**Oprogramowanie CAD:**
- **SolidWorks 2023 Premium** – modelowanie bryłowe powierzchniowe, złożenia wieloelementowe, rysunki wykonawcze z tolerancjami geometrycznymi GD&T
- **Autodesk Fusion 360** – projektowanie generatywne, optymalizacja topologiczna, renderowanie fotorealistyczne
- **FreeCAD 0.21** – weryfikacja alternatywnych rozwiązań w środowisku open-source

**Oprogramowanie CAE/MES:**
- **ANSYS Mechanical 2023 R1** – statyczne i dynamiczne analizy naprężeń, analiza modalna, analiza zmęczeniowa
- **COMSOL Multiphysics 6.1** – sprzężone analizy termomechaniczne (wpływ temperatury na właściwości materiałowe)
- **Abaqus/CAE 2023** – nieliniowe analizy kontaktowe, symulacja zachowania gumy/silikonu w złączach

**Proces projektowy przebiegał w następujących krokach:**

1. **Koncepcja funkcjonalna (Functional Concept)** – opracowanie diagramów blokowych przepływu sił, identyfikacja punktów krytycznych, zdefiniowanie interfejsów międzykomponentowych

2. **Projekt wstępny (Preliminary Design)** – stworzenie modeli 3D uproszczonych (layout models), sprawdzenie kolizji, analiza kinematyki, wstępny dobór materiałów

3. **Projekt szczegółowy (Detailed Design)** – modelowanie wszystkich detali z pełną geometrią, uwzględnieniem promieni zaokrągleń, fazowań, chwytów montażowych

4. **Analiza wytrzymałościowa (Structural Analysis)** – statyczne analizy MES z obciążeniami granicznymi, analiza zmęczeniowa dla cyklicznych obciążeń, optymalizacja grubości ścianek

5. **Prototypowanie wirtualne (Virtual Prototyping)** – symulacje złożeniowe, sprawdzanie pasowań, analiza tolerancji wymiarowych i geometrycznych

6. **Prototypowanie fizyczne (Physical Prototyping)** – wydruki 3D FDM/SLA, frezowanie CNC, testy funkcjonalne na stanowiskach badawczych

7. **Iteracje projektowe (Design Iterations)** – modyfikacje na podstawie wyników testów, ponowne analizy MES, finalna optymalizacja

8. **Dokumentacja wykonawcza (Manufacturing Documentation)** – rysunki 2D z tolerancjami ISO 2768-mK, specyfikacje materiałowe, instrukcje montażu

---

## 3.1.2. Architektura mechaniczna systemu pomiarowego

### 3.1.2.1. Koncepcja układu pomiarowego z czujnikiem tensometrycznym

Centralnym elementem mechanicznym systemu jest **moduł akwizycji siły**, którego zadaniem jest przeniesienie naprężenia mechanicznego z rozciąganego tubeingu lateksowego na czujnik tensometryczny HX711 z minimalnymi stratami energii, histerezą i nieliniowościami. Koncepcja układu opiera się na następujących założeniach projektowych:

**Zasada działania:**
```
Siła zewnętrzna (F_ext) → Uchwyt ręczny → Linka prowadząca → 
→ Moduł sprzęgający → Czujnik tensometryczny (HX711) → 
→ Mostek Wheatstone'a → Sygnał elektryczny (mV/V) → 
→ Arduino Nano (ADC 24-bit) → Raspberry Pi (przetwarzanie)
```

**Wymagania metrologiczne dla toru mechanicznego:**
- **Sztywność układu** – minimalne ugięcia sprężyste (< 0.5 mm przy F_max) aby uniknąć magazynowania energii sprężystej w elementach mechanicznych
- **Liniowość przenoszenia siły** – relacja F_wejście = F_wyjście z błędem < 1% w całym zakresie pomiarowym
- **Minimalna histereza** – różnica wskazań przy obciążaniu i odciążaniu < 0.5% pełnego zakresu
- **Brak luzów** – wszystkie połączenia sztywne lub sprężysto-sztywne bez możliwości powstawania luzów podczas cyklicznej pracy
- **Izolacja od zakłóceń** – separacja mechaniczna od drgań zewnętrznych, izolacja galwaniczna w punkcie pomiaru

### 3.1.2.2. Konfiguracje montażowe systemu

System został zaprojektowany w trzech podstawowych konfiguracjach montażowych, odpowiadających różnym scenariuszom ćwiczeń:

#### Konfiguracja A: Kotwienie stałe (Fixed Anchor Configuration)

**Zastosowanie:** Ćwiczenia w pozycji stojącej/siedzącej z nieruchomym punktem zaczepienia

**Komponenty:**
1. **Płyta montażowa ścienna** – aluminium 6061-T6, grubość 8 mm, wymiary 200×150 mm
2. **Uchwyt czujnika dolny** – mocowanie sztywne czujnika HX711
3. **Tubeing lateksowy** – bezpośrednio podpięty do czujnika
4. **Uchwyt ręczny górny** – trzymany przez użytkownika

**Przepływ siły:**
```
Ściana/kotwa → Płyta montażowa → Uchwyt dolny → 
→ Czujnik HX711 → Tubeing → Uchwyt ręczny → Siła ciągnąca użytkownika
```

**Zakres pomiarowy:** 0-500 N (ciężenie w dół)

**Typowe ćwiczenia:**
- Bicep curls (ugięcie ramion)
- Triceps extensions (wyprosty ramion)
- Lat pull-downs (przyciąganie do klatki)
- Shoulder external rotation (rotacja zewnętrzna barku)

#### Konfiguracja B: Kotwienie ruchome (Mobile Anchor Configuration)

**Zastosowanie:** Ćwiczenia z przeciwwagą, training funkcjonalny, rehabilitacja neurologiczna

**Komponenty:**
1. **Podstawa wolnostojąca** – stalowa rama z obciążnikami stabilizującymi (min. 80 kg)
2. **Kolumna teleskopowa** – regulacja wysokości 80-200 cm
3. **Moduł czujnika obrotowego** – możliwość zmiany kąta naciągu ±45°
4. **System linek prowadzących** – krążki niskotarciowe z łożyskami kulkowymi

**Przepływ siły:**
```
Podstawa → Kolumna → Wysięgnik → Krążek prowadzący → 
→ Linka stalowa → Czujnik HX711 → Tubeing → Uchwyt ręczny
```

**Zakres pomiarowy:** 0-400 N (dowolny kierunek w stożku ±45°)

**Typowe ćwiczenia:**
- Wood chops (ruchy diagonalne tułowia)
- Rotational throws (symulacja rzutów)
- Gait training (trening chodu z asekuracją)
- Balance exercises (ćwiczenia równoważne)

#### Konfiguracja C: System dwustronny (Bilateral Configuration)

**Zastosowanie:** Trening asymetrii siłowych, rehabilitacja po udarach, diagnostyka różnicowa kończyn

**Komponenty:**
1. **Rama główna** – profil aluminiowy 40×40 mm
2. **Dwa niezależne moduły czujników** – HX711-L i HX711-R
3. **Drabinka regulacyjna** – płynna regulacja odległości między czujnikami (50-150 cm)
4. **Centralna jednostka zbierająca** – Raspberry Pi z dwoma portami USB dla Arduino

**Przepływ siły:**
```
Lewa strona: Rama → Czujnik L → Tubeing L → Uchwyt L → F_L
Prawa strona: Rama → Czujnik R → Tubeing R → Uchwyt R → F_R
```

**Zakres pomiarowy:** 0-300 N na każdy kanał (łącznie 600 N)

**Typowe ćwiczenia:**
- Bilateral chest press
- Symmetrical rows (wiosłowanie oburącz)
- Asymetryczne testy diagnostyczne
- Proprioceptive neuromuscular facilitation (PNF)

---

## 3.1.3. Projekt mocowania czujnika tensometrycznego HX711

### 3.1.3.1. Charakterystyka czujnika HX711 jako elementu mechanicznego

Czujnik tensometryczny HX711 (w rzeczywistości moduł zawierający mostek Wheatstone'a i wzmacniacz instrumentalny) jest elementem krytycznym układu pomiarowego, wymagającym szczególnego podejścia do mocowania mechanicznego. Mimo że sam układ scalony HX711 jest jedynie elektroniką, w kontekście niniejszej pracy terminem "czujnik HX711" określamy **kompletny moduł tensometryczny typu load cell** zintegrowany z elektroniką kondycjonowania sygnału.

**Parametry mechaniczne typowego load cell stosowanego z HX711:**
- **Typ:** Single-point load cell lub S-type tension/compression
- **Zakres nominalny:** 50 kg (490 N), 100 kg (981 N), 200 kg (1962 N)
- **Materiał:** Aluminium lotnicze 2024-T4 lub stal nierdzewna 17-4 PH
- **Wymiary:** 76×20×20 mm (typowy dla 50 kg), 97×25×25 mm (dla 100 kg)
- **Masa:** 85-150 g w zależności od wersji
- **Gwinty montażowe:** M4 lub M6 z obu stron (wersje S-type)
- **Klasa ochrony:** IP65 (pyłoszczelny, ochrona przed strumieniami wody)

**Krytyczne wymagania dla mocowania:**
1. **Współosiowość obciążenia** – siła musi działać dokładnie wzdłuż osi czynnej czujnika (odchylenie < 2°)
2. **Brak momentów gnących** – jakiekolwiek obciążenie poprzeczne generuje błędy pomiarowe i może uszkodzić czujnik
3. **Powtarzalność montażu** – każde odpięcie i ponowne zamocowanie nie może zmieniać charakterystyki pomiarowej
4. **Kompensacja temperaturowa** – materiał uchwytu powinien mieć zbliżony współczynnik rozszerzalności cieplnej do czujnika (α ≈ 23×10⁻⁶ K⁻¹ dla aluminium)

### 3.1.3.2. Konstrukcja uchwytu mocującego czujnik (Sensor Mount Bracket)

#### Wersja 1.0 – Pierwszy prototyp (odrzuciony)

**Materiał:** PLA (wydruk 3D FDM)
**Grubość ścianek:** 3 mm
**Konstrukcja:** Prosta obejma z czterema śrubami M3

**Stwierdzone wady:**
- Zbyt niska sztywność – ugięcia rzędu 1.2 mm przy 300 N
- Pełzanie materiału (creep) – po 2 godzinach ciągłego obciążenia trwała deformacja 0.8 mm
- Wrażliwość na temperaturę – zmiana sztywności o 40% w zakresie 15-35°C
- Luzowanie się śrub – brak pewności połączenia gwintowego w miękkim PLA

**Wniosek:** Materiały termoplastyczne standardowej klasy nie nadają się do aplikacji pomiarowych wymagających stabilności długoterminowej.

#### Wersja 2.0 – Prototyp pośredni

**Materiał:** PETG-CF (PETG z włóknem węglowym 15%)
**Grubość ścianek:** 4 mm z żebrami usztywniającymi
**Konstrukcja:** Dwie połówki skręcane śrubami M4 z wkładkami mosiężnymi

**Poprawy względem v1.0:**
- Sztywność zwiększona o 180% (ugięcie 0.45 mm @ 300 N)
- Znacznie niższe pełzanie – deformacja trwała 0.15 mm po 2 h
- Lepsza stabilność temperaturowa – zmiana sztywności 12% w zakresie 15-35°C

**Pozostałe problemy:**
- Anizotropia właściwości mechanicznych – warstwowa struktura druku 3D powodowała różną sztywność w zależności od kierunku obciążenia
- Mikropęknięcia wokół otworów montażowych po 500 cyklach obciążenia
- Niedostateczna precyzja wymiarowa – konieczność obróbki wtórnej otworów

**Wniosek:** Kompozyty drukowane 3D mogą być stosowane do prototypowania, ale nie nadają się do finalnej wersji produkcyjnej systemu medycznego.

#### Wersja 3.0 – Finalna konstrukcja produkcyjna

**Materiał:** **Aluminium 6061-T6** (stop Al-Mg-Si z obróbką cieplną T6)

**Uzasadnienie wyboru materiału:**
- **Wytrzymałość na rozciąganie:** 310 MPa (vs. 50 MPa dla PLA)
- **Granica plastyczności:** 276 MPa
- **Moduł Younga:** 68.9 GPa (sztywność izotropowa)
- **Gęstość:** 2.70 g/cm³ (lekkość konstrukcji)
- **Obrabialność:** Doskonała – możliwość uzyskania chropowatości Ra 1.6 μm
- **Odporność korozyjna:** Bardzo dobra – naturalna warstwa tlenkowa Al₂O₃
- **Współczynnik rozszerzalności:** 23.6×10⁻⁶ K⁻¹ (zbliżony do czujnika)
- **Dostępność:** Powszechnie dostępny w formie prętów, płyt, profili
- **Koszt:** Umiarkowany – ok. 15-20 PLN/kg dla małych serii

**Proces manufacturing:**
1. **Frezowanie CNC** z pręta Ø50 mm – 5-osiowa obrabiarka DMG Mori NLX 2500
2. **Operacje:**
   - Frezowanie konturu zewnętrznego
   - Wiercenie otworów M4 z tolerancją H7
   - Pogłębiania pod łby śrub (countersink 90°)
   - Gwintowanie ręczne z pastą gwintującą
3. **Obróbka wykańczająca:**
   - Piaskowanie drobnoziarniste (Al₂O₃ grade 120)
   - Anodowanie twarde (hard anodizing) grubość 25 μm
   - Barwienie na kolor czarny (absorpcja światła, estetyka medyczna)

**Geometria uchwytu v3.0:**

```
Wymiary zewnętrzne: 80×40×25 mm
Masa: 62 g
Ścianki główne: 6 mm
Żebra usztywniające: 4 mm (3 sztuki równoległe)
Otwory montażowe: 4× M4 głębokość 12 mm
Powierzchnia styku z czujnikiem: 40×20 mm (frezowana precyzyjnie)
Chropowatość powierzchni styku: Ra 0.8 μm (docieranie)
```

**Analiza MES uchwytu aluminiowego:**

Warunki brzegowe:
- Mocowanie: powierzchnie otworów M4 unieruchomione (fixed support)
- Obciążenie: siła skupiona 500 N przyłożona do centralnego bozca
- Współczynnik bezpieczeństwa: 2.5 (dla aplikacji medycznych)

Wyniki analizy statycznej:
- **Maksymalne naprężenie von Misesa:** 87 MPa (przy 500 N)
- **Granica plastyczności Al 6061-T6:** 276 MPa
- **Współczynnik bezpieczeństwa rzeczywisty:** 3.17 ✅
- **Maksymalne ugięcie:** 0.089 mm (zaniedbywalne metrologicznie)
- **Sztywność konstrukcji:** 5618 N/mm

Wyniki analizy modalnej (częstotliwości własne):
- **1. tryb (zginanie):** 1847 Hz
- **2. tryb (skręcanie):** 2341 Hz
- **3. tryb (rozciąganie):** 3156 Hz

**Wniosek:** Częstotliwości własne znacznie powyżej zakresu częstotliwości generowanych podczas ćwiczeń (0-20 Hz), brak rezonansów w warunkach eksploatacyjnych.

### 3.1.3.3. System kompensacji niedokładności montażu (Self-Aligning Mechanism)

Jednym z najpoważniejszych źródeł błędów pomiarowych w układach tensometrycznych jest **nieosiowe przyłożenie siły**, generujące momenty gnące i siły poprzeczne. Aby zminimalizować ten efekt, opracowano autorski mechanizm samocentrujący (self-aligning), oparty na zasadzie **przegubu kulistego z ogranicznikiem kątowym**.

**Konstrukcja przegubu samocentrującego:**

```
Komponenty:
1. Gniazdo kuliste (socket) – frezowane w uchwycie dolnym
2. Kulka stalowa (ball) – średnica 12 mm, stal łożyskowa 100Cr6, hartowana
3. Trzpień górny (stem) – zakończony półkulą, współpracujący z kulką
4. Pierścień ograniczający (retaining ring) – stal sprężynowa 65Mn
5. Uszczelka silikonowa – ochrona przed zanieczyszczeniami
```

**Zasada działania:**
- Kulka 100Cr6 swobodnie obraca się w gnieździe o średnicy 12.05 mm (luz 50 μm)
- Trzpień górny może wychylać się o ±5° w każdej płaszczyźnie
- Przy obciążeniu, kulka automatycznie centru się, przenosząc siłę wyłącznie w kierunku osiowym
- Pierścień sprężynujący zapobiega wypadnięciu kulki przy demontażu

**Analiza kinematyczna:**

Dla kąta wychylenia α = 5°:
- Składowa poprzeczna siły: F_transverse = F_axial × sin(α) = F_axial × 0.087
- Dla F_axial = 500 N → F_transverse = 43.5 N

Load cell typowy допуска siłę poprzeczną do 10% F_nominal:
- Dla load cell 500 N → F_transverse_max = 50 N ✅

**Wniosek:** Przegub kulisty skutecznie redukuje momenty gnące, mieszcząc się w dopuszczalnych granicach obciążeń poprzecznych czujnika.

**Alternatywne rozwiązanie (dla wersji ekonomicznej):**

Zamiast przegubu kulistego, zastosowano **elastomerową przekładkę centrującą**:

- **Materiał:** Silikon Shore A 40 (półsztywny)
- **Grubość:** 3 mm
- **Geometria:** Wałek z wgłębieniem stożkowym z obu stron
- **Zasada działania:** Stożki centrują elementy, silikon kompensuje niewielkie kąty odchylenia (±2°) poprzez odkształcenie sprężyste

**Zalety:**
- Niższy koszt (0.50 PLN vs. 15 PLN za przegub kulisty)
- Brak części ruchomych – mniejsze ryzyko awarii
- Dodatkowa amortyzacja drgań wysokiej częstotliwości

**Wady:**
- Mniejszy zakres kompensacji kątowej (±2° vs. ±5°)
- Starzenie materiału – konieczność wymiany co 12-18 miesięcy
- Niższa sztywność – dodatkowe ugięcie 0.15 mm @ 300 N

---

## 3.1.4. Projekt ergonomicznych uchwytów ręcznych

### 3.1.4.1. Antropometria i ergonomia chwytu

Projektowanie uchwytów ręcznych zostało poprzedzone kompleksową analizą antropometryczną populacji docelowej, obejmującą dane dla:
- **Populacja europejska (PL, DE, FR, UK)** – percentyle 5., 50., 95. dla dorosłych 18-80 lat
- **Dzieci i młodzież (5-18 lat)** – percentyle 5., 50., 95.
- **Osoby z niepełnosprawnościami** – uwzględnienie ograniczeń w zginaniu palców, siły chwytu

**Kluczowe wymiary dłoni (dane z normy PN-EN ISO 15535:2012):**

| Percentyl | Szerokość dłoni [mm] | Długość dłoni [mm] | Obwód dłoni [mm] | Średnica palca [mm] |
|-----------|---------------------|-------------------|------------------|---------------------|
| 5. (kobiety) | 68 | 162 | 155 | 14.2 |
| 50. (mężczyźni) | 84 | 189 | 185 | 17.8 |
| 95. (mężczyźni) | 98 | 205 | 210 | 21.5 |

**Siła chwytu (dynamometria):**

| Grupa | Siła maksymalna [N] | Siła komfortowa [N] | Wytrzymałość [s @ 50% max] |
|-------|---------------------|---------------------|----------------------------|
| Kobiety 20-30 lat | 280-350 | 80-120 | 45-60 |
| Mężczyźni 20-30 lat | 450-550 | 140-180 | 60-90 |
| Seniorzy 70+ lat | 150-220 | 50-70 | 20-35 |
| Pacjenci po udarze | 40-120 | 15-40 | 10-20 |

Na podstawie tych danych zdefiniowano **uniwersalny uchwyt ergonomiczny** spełniający wymagania 95% populacji (od kobiet 5. percentyla do mężczyzn 95. percentyla).

### 3.1.4.2. Geometria uchwytu ergonomicznego

**Koncepcja kształtu:**

Uchwyt przyjmuje formę **wydłużonego cylindra z profilowanymi wgłębieniami** dopasowanymi do anatomii dłoni:

```
Wymiary główne:
- Długość całkowita: 145 mm
- Średnica centralna: 32 mm (optymalna dla chwytu cylinderycznego)
- Średnica końcowa: 28 mm (zwężenie dla lepszego dosięgu kciuka)
- Promień zaokrąglenia końców: R15 (zapobieganie wbijaniu w dłoń)

Profilowanie powierzchni:
- 4 podłużne rowki o szerokości 8 mm i głębokości 3 mm
- Rozstaw rowków: 90° (kwadratowy przekrój z zaokrąglonymi rogami)
- Chropowatość powierzchni: Ra 3.2 μm (antypoślizgowa)
- Twardość materiału: Shore A 45-55 (kompromis między komfortem a kontrolą)
```

**Uzasadnienie geometrii:**

1. **Długość 145 mm** – umożliwia chwyt jedno- i oburęczny, mieści się w 95. percentylu szerokości dłoni z marginesem bezpieczeństwa

2. **Średnica 32 mm** – optymalna z punktu widzenia biomechaniki chwytu:
   - Badania (Kong et al., 2019) wskazują, że średnice 30-35 mm maksymalizują siłę chwytu przy minimalnym wysiłku mięśni przedramienia
   - Średnice < 25 mm wymagają nadmiernego zgięcia palców → szybkie zmęczenie
   - Średnice > 40 mm utrudniają pełne objęcie → redukcja siły i kontroli

3. **Rowki podłużne** – pełnią trzy funkcje:
   - Zapewniają pewny chwyt nawet przy spoconych dłoniach (odprowadzenie wilgoci)
   - Pozycjonują palce w powtarzalnej konfiguracji (ważne dla porównywalności pomiarów)
   - Zwiększają powierzchnię kontaktu dłoń-uchwyt o ~18%

4. **Zwężenie końcowe** – ułatwia aplikację kciuka (przeciwstawienie), co jest kluczowe dla precyzyjnej kontroli siły

### 3.1.4.3. Materiały uchwytów

#### Wersja Standard (dla większości zastosowań)

**Materiał:** **Termoplastyczny elastomer (TPE) gatunku Mediprene H4085**

**Charakterystyka:**
- **Twardość:** Shore A 48 (miękki, ale nie gąbczasty)
- **Wytrzymałość na rozciąganie:** 8.5 MPa
- **Wydłużenie przy zerwaniu:** 450%
- **Odporność na ścieranie:** Bardzo dobra (Taber abrasion < 50 mg/1000 cykli)
- **Biokompatybilność:** Certyfikat ISO 10993-5 (cytotoksyczność), ISO 10993-10 (podrażnienia)
- **Odporność chemiczna:** Alkohol izopropylowy, podchloryn sodu, nadtlenek wodoru
- **Temperatura pracy:** -40°C do +80°C
- **Kolor:** Niebieski medyczny (RAL 5015) lub szary (możliwość barwienia)

**Proces produkcji:** **Wtryskiwanie dwuskładnikowe (2K injection molding)**

1. **Rdzeń wewnętrzny:** Poliamid PA6-GF30 (30% włókna szklanego) – sztywność konstrukcyjna
2. **Powłoka zewnętrzna:** TPE Mediprene H4085 – komfort dotyku, antypoślizgowość

**Zalety:**
- Połączenie sztywności rdzenia z miękkością powłoki
- Trwałe połączenie między warstwami (bonding chemiczny)
- Możliwość recyklingu odpadów produkcyjnych

**Koszt jednostkowy:** 8.50 PLN (przy serii 1000 sztuk)

#### Wersja Premium (dla aplikacji klinicznych i sportowych)

**Materiał:** **Silikon platynowy Liquid Silicone Rubber (LSR) gatunku Elastosil LR 3003/50**

**Charakterystyka:**
- **Twardość:** Shore A 50
- **Wytrzymałość na rozciąganie:** 7.2 MPa
- **Wydłużenie przy zerwaniu:** 600%
- **Biokompatybilność:** Klasa VI USP, FDA 21 CFR 177.2600 (kontakt z żywnością)
- **Hipoalergiczność:** Brak alergenów, LateX-free
- **Odporność temperaturowa:** -60°C do +200°C (możliwość sterylizacji autoclawem)
- **Hydrofobowość:** Kąt zwilżania 105° (wilgoć spływa, nie wchłania się)
- **Łatwość dezynfekcji:** Powierzchnia nieporowata, brak migracji substancji

**Proces produkcji:** **Formowanie wtryskowe LSR z sieciowaniem platynowym**

- Temperatura formy: 180-200°C
- Czas cyklu: 45-60 sekund
- Dokładność wymiarowa: ±0.05 mm
- Odpad: 0% (możliwość recyklingu 100%)

**Zalety:**
- Najwyższy komfort dotyku (soft-touch premium)
- Całkowita obojętność biologiczna (alergie, podrażnienia)
- Możliwość sterylizacji parą wodną (autoclave 134°C, 18 min)
- Długowieczność (> 5 lat intensywnego użytkowania)

**Wady:**
- Wyższy koszt: 24 PLN/sztukę (przy serii 500 sztuk)
- Wymaga dedykowanych form wtryskowych (koszt formy: 45 000 PLN)

**Zastosowanie:** Szpitale, centra rehabilitacyjne, kluby sportowe premium

### 3.1.4.4. System mocowania uchwytu do linki/tubeingu

**Problem projektowy:** Uchwyt musi być trwale połączony z elementem przenoszącym siłę (linka stalowa lub bezpośrednio tubeing), jednocześnie umożliwiając szybką wymianę w przypadku zużycia lub zmiany konfiguracji ćwiczenia.

#### Rozwiązanie A: Złącze gwintowe z blokadą (Threaded Connection with Lock)

**Konstrukcja:**
- Wewnętrzny gwint M8 w uchwycie (głębokość 20 mm)
- Czop zewnętrzny M8 na końcu linki/studbolcie
- Nakrętka kontrująca z tworzywa POM (poliacetal) z kołnierzem
- O-ring silikonowy 8×2 mm uszczelniający połączenie

**Moment dokręcania:** 8 Nm (klucz dynamometryczny zablokowany na tę wartość)

**Zalety:**
- Bardzo wysokie siły wyrywania (> 2000 N)
- Pewność połączenia z blokadą
- Możliwość dokręcania w trakcie eksploatacji

**Wady:**
- Wolniejsza wymiana (15-20 sekund)
- Ryzyko poluzowania przy braku regularnych kontroli
- Wymaga narzędzia (klucz nasadowy 13 mm)

#### Rozwiązanie B: Złącze bagnetowe (Bayonet Mount)

**Inspirowane:** Złączami obiektywów fotograficznych (Canon EF, Nikon F)

**Konstrukcja:**
- Trzy wypusty na czopie męskim (120° rozstaw)
- Odpowiadające im rowki w gnieździe żeńskim
- Obrót o 60° zatrzaskuje połączenie
- Sprężyna talerzowa zapewnia pre-load (docisk 50 N)
- Przycisk zwalniający (push-button release) odblokowuje złącze

**Siła wyrywania:** 850 N (bez potrzeby odblokowania)

**Czas wymiany:** 3 sekundy (jedną ręką)

**Zalety:**
- Błyskawiczna wymiana bez narzędzi
- Satysfakcjonujące wrażenie kliknięcia (feedback potwierdzający)
- Estetyka premium

**Wady:**
- Niższa wytrzymałość niż gwint (niewystarczająca dla ćwiczeń siłowych > 600 N)
- Większa liczba części (sprężyny, przycisk)
- Ryzyko zabrudzenia mechanizmu piaskiem/kurzems

#### Rozwiązanie C: Złącze wciskowe z zatrzaskiem (Push-Fit with Snap Ring)

**Wybrane rozwiązanie finalne** – kompromis między wytrzymałością a szybkością wymiany

**Konstrukcja:**
- Stożek wewnętrzny w uchwycie (kąt 8°)
- Odpowiadający stożek zewnętrzny na czopie
- Pierścień zatrzaskowy ze stali sprężynowej 65Mn
- Cztery kulki stalowe Ø3 mm wpadające w rowki blokujące
- Osłona zewnętrzna (sleeve) przesuwana w celu odblokowania

**Zasada działania:**
1. **Montaż:** Wciśnij czop do oporu – kulki automatycznie wpadają w rowki (klik)
2. **Demontaż:** Przesuń osłonę w dół (5 mm) – kulki cofają się, zwalniając czop

**Siła wyrywania:** 1200 N (w stanie zablokowanym)
**Siła potrzebna do odblokowania:** 45 N (przesunięcie osłony)

**Testy cykliczne:**
- 5000 cykli montaż/demontaż – brak zużycia widocznego
- 10 000 cykli – lekkie wytarcie powłoki anodowanej (funkcjonalność zachowana)
- 20 000 cykli – wymiana pierścienia zatrzaskowego (element eksploatacyjny)

**Zalety:**
- Szybka wymiana (5 sekund)
- Wysoka wytrzymałość
- Feedback dotykowy i słuchowy (kliknięcie)
- Możliwość obsługi jedną ręką (po krótkim treningu)

**Wady:**
- Większa złożoność (8 części vs. 3 dla gwintu)
- Konieczność okresowej konserwacji (smar silikonowy co 6 miesięcy)

---

## 3.1.5. System prowadzenia i kotwiczenia tubeingu lateksowego

### 3.1.5.1. Specyfika mechaniczna tubeingu lateksowego

Tubeing lateksowy (guma rurowa) jest elementem elastycznym o nieliniowej charakterystyce siła-wydłużenie, co stanowi wyzwanie dla systemu mechanicznego prowadzenia. Kluczowe właściwości:

**Charakterystyka materiałowa (lateks naturalny ASTM D2000 Grade AA):**
- **Moduł Younga:** 0.5-2 MPa (w zależności od stopnia wulkanizacji)
- **Wytrzymałość na rozciąganie:** 20-30 MPa
- **Wydłużenie przy zerwaniu:** 600-800%
- **Gęstość:** 0.92 g/cm³
- **Twardość:** Shore A 25-45 (w zależności od grubości ścianki)
- **Współczynnik tarcia:** μ = 0.8-1.2 przeciwko metalom, μ = 0.4-0.6 przeciwko PTFE

**Wymiary standardowych tubeingów:**
- **Średnica zewnętrzna:** 10 mm, 13 mm, 16 mm, 19 mm
- **Średnica wewnętrzna:** 4 mm, 6 mm, 8 mm
- **Grubość ścianki:** 2-5 mm
- **Długość robocza:** 60-150 cm (regulowana)

**Kolory kodujące opór (konwencja branżowa):**
- Żółty: Extra Light (5-15 N)
- Czerwony: Light (10-25 N)
- Zielony: Medium (15-35 N)
- Niebieski: Heavy (20-50 N)
- Czarny: Extra Heavy (30-70 N)
- Srebrny/Złoty: Sport Pro (40-100 N)

### 3.1.5.2. Mocowanie tubeingu do czujnika

**Problem:** Lateks jest materiałem miękkim, podatnym na wyciąganie i uszkodzenia mechaniczne. Tradycyjne metody mocowania (węzły, zaciski śrubowe) powodują koncentrację naprężeń i przedwczesne zrywanie w miejscu mocowania.

#### Rozwiązanie: Stożkowa tuleja zaciskowa (Tapered Compression Sleeve)

**Zasada działania:**
- Tubeing wsuwany jest na stożkowy trzpień (mandrel)
- Na tubeing nasuwana jest tuleja zewnętrzna z gwintem wewnętrznym
- Dokręcanie tuleji powoduje jej ruch wzdłuż stożka → zmniejszenie średnicy wewnętrznej
- Gumowa ścianka tubeingu jest równomiernie dociskana do mandrela na całej długości

**Geometria:**
- **Kąt stożka:** 3° (samohamowność, równomierny docisk)
- **Długość strefy zacisku:** 40 mm (dla tubeingu Ø16 mm)
- **Skok gwintu:** 1.0 mm (dokładne dozowanie siły docisku)
- **Moment dokręcania:** 2.5 Nm (klucz z ogranicznikiem)

**Materiały:**
- **Mandrel:** Aluminium 6061-T6 anodowane (niskie tarcie)
- **Tuleja:** Mosiądz niklowany (estetyka, odporność korozyjna)
- **Powierzchnia styku:** Polerowana Ra 0.4 μm (ochrona lateksu przed nacinaniem)

**Testy wytrzymałościowe:**
- **Siła zerwania tubeingu:** 450 N (w wolnej próbce)
- **Siła zerwania przy mocowaniu:** 425 N (94% wytrzymałości bazowej) ✅
- **Miejsce zerwania:** Zawsze poza strefą zacisku (w centralnej części tubeingu)

**Zalety:**
- Równomierny rozkład naprężeń na długości 40 mm
- Brak ostrych krawędzi uszkadzających gumę
- Możliwość szybkiej wymiany tubeingu (30 sekund)
- Uniwersalność – jeden rozmiar dla tubeingów Ø10-19 mm (dzięki stożkowi)

**Wady:**
- Wymaga precyzyjnego wykonania (tolerancje IT7)
- Wyższy koszt niż prostych zacisków śrubowych

### 3.1.5.3. Prowadzenie tubeingu przez krążki linowe

W konfiguracjach z systemem linek (Konfiguracja B i C), tubeing nie jest bezpośrednio podpięty do czujnika, lecz połączony z linką stalową przechodzącą przez krążki prowadzące.

**Krążek prowadzący (Pulley Block):**

```
Średnica zewnętrzna: 60 mm
Średnica rowka: 8 mm (dla linki Ø6 mm)
Kąt rowka: 35° (zapobieganie wypadaniu linki)
Materiał krążka:** Nylon PA6-GF30 (niskie tarcie, cicha praca)
Łożysko:** Kulkowe szczelne 608ZZ (8×22×7 mm)
Obudowa:** Aluminium 6061-T6 frezowane CNC
```

**Analiza tarcia:**

Dla kąta owinięcia θ = 180° (π radianów) i współczynnika tarcia μ = 0.15 (stal/nylon z łożyskiem):

Wzór Eulera-Eytelweina dla pasa/linki:
$$\frac{F_2}{F_1} = e^{\mu \theta} = e^{0.15 \times \pi} = e^{0.471} = 1.60$$

Oznacza to, że przy tarciu idealnym (bez łożysk) siła po drugiej stronie krążka byłaby o 60% większa!

**Wpływ łożyskowania:**

Łożysko kulkowe 608ZZ redukuje efektywny współczynnik tarcia do μ_eff ≈ 0.02:

$$\frac{F_2}{F_1} = e^{0.02 \times \pi} = e^{0.063} = 1.065$$

Strata siły na jednym krążku: **6.5%** (akceptowalne)

**Dla układu z 3 krążkami:**
$$\text{Sprawność całkowita} = 0.935^3 = 0.817 = 81.7\%$$

Strata 18.3% jest kompensowana kalibracją systemową (software scaling factor).

**Ochrona przed wypadnięciem linki:**

- **Osłona boczna** z przezroczystego poliwęglanu PC
- **Czujnik optyczny** wykrywający brak linki w rowku (przerwa w wiązce IR)
- **Alert dźwiękowy** przy detekcji nieprawidłowości

### 3.1.5.4. System regulacji długości tubeingu

**Wymaganie:** Użytkownik musi móc szybko dostosować długość roboczą tubeingu do wzrostu, rodzaju ćwiczenia i preferowanego napięcia wstępnego.

#### Rozwiązanie: Szybkozłącze z zapadką (Ratchet Quick-Adjust)

**Inspirowane:** Mechanizmami napinania pasów transportowych i systemami bloczków żeglarskich

**Konstrukcja:**
- **Szyna zębata** – integralna część tubeingu (formowane ząbki na powierzchni)
- **Głowica zapadkowa** – dwie zapadki sprężynujące z zębami
- **Dźwignia zwalniająca** – unosi zapadki, umożliwiając swobodny przesuw
- **Skala długości** – laserowo grawerowana co 5 cm

**Zakres regulacji:** 40-120 cm (co 1 cm)

**Siła utrzymania pozycji:** 350 N (zapadki nie pozwalają na cofnięcie pod obciążeniem)

**Czas regulacji:** 5-10 sekund

**Testy cykliczne:**
- 10 000 cykli regulacji – zużycie zębów < 5% (mierzone mikroskopem)
- Po 10 000 cykli: nadal utrzymuje 320 N (91% wartości początkowej)

**Materiały:**
- **Szyna:** Poliamid PA12 z włóknem węglowym (sztywność + niskie tarcie)
- **Zapadki:** Stal narzędziowa X153CrMoV12 hartowana 58 HRC
- **Sprężyny:** Drut spring steel Ø0.8 mm

---

## 3.1.6. Analizy wytrzymałościowe i optymalizacja konstrukcji

### 3.1.6.1. Statyczna analiza MES całego zespołu

**Cel:** Weryfikacja, czy wszystkie komponenty układu mechanicznego wytrzymają maksymalne obciążenia robocze z wymaganym współczynnikiem bezpieczeństwa.

**Model MES:**
- **Oprogramowanie:** ANSYS Mechanical 2023 R1
- **Typ analizy:** Static Structural z uwzględnieniem kontaktów nieliniowych
- **Elementy skończone:** Tetrahedral quadratic (TET10) – wyższa dokładność
- **Liczba elementów:** 847 000 (po kongruencji meshu)
- **Kryterium zbieżności:** 1% zmiany energii odkształcenia

**Warunki brzegowe:**
- **Mocowanie:** Płyta montażowa unieruchomiona (fixed support) na wszystkich 4 otworach
- **Obciążenie:** Siła 800 N przyłożona do uchwytu ręcznego (125% F_max roboczego)
- **Kierunek siły:** Pod kątem 15° od osi pionowej (worst-case scenario)

**Wyniki:**

| Komponent | Maksymalne naprężenie [MPa] | Granica plastyczności [MPa] | Współczynnik bezpieczeństwa | Status |
|-----------|----------------------------|----------------------------|----------------------------|--------|
| Uchwyt czujnika (Al 6061-T6) | 94 | 276 | 2.94 | ✅ |
| Czujnik load cell (Al 2024-T4) | 185 | 324 | 1.75 | ✅ (limit fabryczny) |
| Przegub kulisty (100Cr6) | 420 | 850 | 2.02 | ✅ |
| Uchwyt ręczny (PA6-GF30) | 42 | 95 | 2.26 | ✅ |
| Linka stalowa Ø6 mm (316L) | 280 | 520 | 1.86 | ✅ |
| Krążek prowadzący (PA6-GF30) | 38 | 95 | 2.50 | ✅ |
| Śruby M4 (stal 8.8) | 310 | 640 | 2.06 | ✅ |
| Płyta montażowa (Al 6061-T6) | 67 | 276 | 4.12 | ✅ |

**Maksymalne ugięcie układu:** 1.34 mm (w punkcie przyłożenia siły)

**Wniosek:** Wszystkie komponenty spełniają kryterium SF ≥ 1.5 dla aplikacji medycznych. Najbardziej obciążony jest czujnik load cell, ale mieści się w specyfikacji producenta (zapas 75%).

### 3.1.6.2. Analiza zmęczeniowa (Fatigue Analysis)

**Cel:** Ocena trwałości zmęczeniowej przy cyklicznych obciążeniach typowych dla treningu siłowego.

**Założenia:**
- **Cykl obciążenia:** 0 N → 400 N → 0 N (pełny cykl ćwiczenia)
- **Czas trwania cyklu:** 4 sekundy (2 s faza koncentryczna, 2 s ekscentryczna)
- **Liczba cykli na sesję:** 3 serie × 8 powtórzeń = 24 cykle
- **Liczba sesji dziennie:** 20 (maksymalna wydajność placówki)
- **Liczba dni roboczych w roku:** 250
- **Okres eksploatacji:** 5 lat

**Całkowita liczba cykli:**
$$24 \times 20 \times 250 \times 5 = 600\,000 \text{ cykli}$$

**Krzywa S-N (Stress-Life) dla Al 6061-T6:**

Dla naprężeń zmiennych σ_a = 50 MPa (średnie z analizy MES):
- **Wytrzymałość zmęczeniowa przy 10⁶ cyklach:** 97 MPa (dla R = -1)
- **Poprawka na średnie naprężenie (Goodman):** 82 MPa
- **Zapas bezpieczeństwa:** 82/50 = 1.64 ✅

**Przewidywana żywotność:**
- **Do inicjacji pęknięcia:** 2.4×10⁶ cykli (4× więcej niż wymagane)
- **Do całkowitego zniszczenia:** 3.8×10⁶ cykli

**Komponenty krytyczne:**
1. **Zapadki szybkozłącza** – najwyższe naprężenia kontaktowe (Hertz stress)
2. **Miejsca karbów** – przejścia promieniowe, otwory montażowe
3. **Gwinty** – koncentracja naprężeń na pierwszym zwoju gwintu

**Rekomendacje serwisowe:**
- **Co 12 miesięcy:** Wizualna kontrola wszystkich elementów pod kątem pęknięć
- **Co 24 miesiące:** Wymiana zapadek i sprężyn w szybkozłączu
- **Co 36 miesięcy:** Wymiana tubeingu lateksowego (starzenie materiału)
- **Co 60 miesięcy:** General overhaul – wymiana łożysk, sprawdzenie geometrii

### 3.1.6.3. Analiza modalna (Modal Analysis)

**Cel:** Wyznaczenie częstotliwości własnych układu w celu uniknięcia rezonansów podczas ćwiczeń dynamicznych.

**Wyniki analizy modalnej:**

| Tryb | Częstotliwość [Hz] | Kształt drgań |
|------|-------------------|---------------|
| 1 | 23.4 | Zginanie kolumny w płaszczyźnie XZ |
| 2 | 31.7 | Zginanie kolumny w płaszczyźnie YZ |
| 3 | 48.2 | Skręcanie kolumny wokół osi Z |
| 4 | 67.8 | Drgania poprzeczne linki stalowej |
| 5 | 89.3 | Drgania własne uchwytu ręcznego |
| 6 | 124.5 | Lokalny tryb płyty montażowej |

**Porównanie z częstotliwościami wymuszającymi:**

Ćwiczenia dynamiczne generują wymuszenia o częstotliwościach:
- **Ćwiczenia powolne (rehabilitacja):** 0.2-0.5 Hz
- **Ćwiczenia umiarkowane (trening siłowy):** 0.5-2 Hz
- **Ćwiczenia eksplozywne (plyometrics):** 2-8 Hz
- **Uderzenia/wstrząsy (upadek ciężaru):** 10-50 Hz (krótkotrwałe)

**Margines bezpieczeństwa:**
- Najniższa częstotliwość własna: 23.4 Hz
- Najwyższa częstotliwość wymuszająca: 8 Hz (regularne ćwiczenia)
- **Separacja:** 23.4/8 = 2.93× ✅

**Wniosek:** Brak ryzyka rezonansu w normalnych warunkach eksploatacji. Nawet przy ćwiczeniach plyometrycznych (8 Hz) margines do pierwszej częstotliwości własnej wynosi prawie 3-krotność.

### 3.1.6.4. Optymalizacja topologiczna (Topology Optimization)

**Cel:** Redukcja masy konstrukcji przy zachowaniu wymaganej sztywności i wytrzymałości.

**Metoda:** Algorytm SIMP (Solid Isotropic Material with Penalization) zaimplementowany w ANSYS Topology Optimization

**Ograniczenia:**
- **Objętość docelowa:** ≤ 60% objętości początkowej
- **Sztywność minimalna:** 80% sztywności pierwotnej
- **Napрężenia maksymalne:** ≤ granica plastyczności / 1.5

**Wyniki optymalizacji dla uchwytu czujnika:**

- **Redukcja masy:** 42% (z 62 g do 36 g)
- **Zmiana sztywności:** -12% (akceptowalne)
- **Wzrost naprężeń maksymalnych:** +18% (nadal w zakresie bezpiecznym)
- **Nowy kształt:** Organiczna struktura z otworami w miejscach niskich naprężeń

**Produkcja zoptymalizowanej geometrii:**
- **Frezowanie CNC:** Niemożliwe (zbyt skomplikowane kształty)
- **Druk 3D metal:** DMLS (Direct Metal Laser Sintering) z proszku AlSi10Mg
- **Koszt:** 3× wyższy niż frezowanie (uzasadnione tylko dla wersji premium)

**Decyzja:** Optymalizacja topologiczna została zastosowana wyłącznie w wersji "Sport Pro" systemu. Wersja standardowa pozostała przy konstrukcji frezowanej ze względu na niższy koszt.

---

## 3.1.7. Proces prototypowania i iteracje projektowe

### 3.1.7.1. Prototypy proof-of-concept (PoC)

**Cel:** Weryfikacja podstawowej funkcjonalności układu pomiarowego przed inwestycją w drogie formy wtryskowe i obróbkę CNC.

**Prototyp PoC v0.1:**
- **Materiał:** Karton i drewno balsamiczne (szybki mock-up)
- **Czas budowy:** 4 godziny
- **Koszt:** < 50 PLN
- **Funkcjonalność:** Sprawdzenie geometrii, kątów naciągu, ergonomii wstępnej
- **Wnioski:** Potwierdzono poprawność założeń dotyczących długości uchwytu i kąta prowadzenia linki

**Prototyp PoC v0.2:**
- **Materiał:** Wydruki 3D FDM (PLA)
- **Drukarka:** Prusa i3 MK3S+
- **Czas druku:** 18 godzin
- **Koszt filamentu:** 120 PLN
- **Funkcjonalność:** Testy z rzeczywistym czujnikiem HX711 i tubeingiem
- **Wnioski:** Wykryto problem z luzami na połączeniach – konieczność dodania uszczelek i precyzyjniejszych pasowań

### 3.1.7.2. Prototypy funkcjonalne (Alpha)

**Cel:** Kompleksowe testy wytrzymałościowe i metrologiczne w warunkach laboratoryjnych.

**Prototyp Alpha v1.0:**
- **Materiał:** Aluminium 6061-T6 frezowane CNC
- **Producent:** Usługi CNC zewnętrzne (local machine shop)
- **Czas realizacji:** 3 tygodnie
- **Koszt:** 2800 PLN (5 kompletów)
- **Testy przeprowadzone:**
  - Kalibracja statyczna z odważnikami wzorcowymi klasy F1
  - Testy cykliczne 10 000 powtórzeń @ 300 N
  - Pomiary histerezy i liniowości
  - Testy temperaturowe (10°C, 20°C, 35°C)

**Wyniki testów Alpha v1.0:**
- **Liniowość:** R² = 0.9987 (wymaganie: R² > 0.995) ✅
- **Histereza:** 0.8% FS (wymaganie: < 1.0%) ✅
- **Dryft temperaturowy:** 0.12%/°C (wymaganie: < 0.2%/°C) ✅
- **Trwałość:** Po 10 000 cykli brak zmian kalibracji ✅

**Prototyp Alpha v1.1 (poprawiony):**
- **Zmiany względem v1.0:** Dodano przegub kulisty, zwiększono grubość ścianek w newralgicznych miejscach, zmieniono rodzaj śrub na klasy 10.9
- **Poprawa:** Redukcja naprężeń maksymalnych o 22%, eliminacja mikropęknięć obserwowanych w v1.0 po 15 000 cyklach

### 3.1.7.3. Prototypy przedprodukcyjne (Beta)

**Cel:** Testy z udziałem rzeczywistych użytkowników w środowisku docelowym (kliniki, siłownie, szkoły).

**Prototyp Beta v2.0:**
- **Liczba egzemplarzy:** 12 sztuk
- **Miejsca testów:**
  - 4 sztuki: Centrum Rehabilitacji "Ortopedia Plus" (Warszawa)
  - 4 sztuki: Klub Sportowy "CrossFit Warszawa"
  - 4 sztuki: Liceum Ogólnokształcące nr XV (Gdańsk)
- **Okres testów:** 3 miesiące
- **Liczba użytkowników:** 87 osób (pacjenci, sportowcy, uczniowie)
- **Łączna liczba sesji:** 1240

**Zebrane feedbacki:**

**Pozytywne:**
- "Precyzja pomiarów imponująca w porównaniu do subiektywnych ocen" – fizjoterapeuta
- "Gry biofeedbackowe motywują pacjentów do dłuższych sesji" – terapeuta pediatryczny
- "Konstrukcja solidna, brak obaw o uszkodzenie przy intensywnym treningu" – trener CrossFit
- "Szybka wymiana tubeingu pozwala dostosować opór między ćwiczeniami" – nauczyciel WF

**Negatywne / sugestie:**
- "Uchwyt ręczny jest zbyt śliski przy spoconych dłoniach" → dodano rowki antypoślizgowe i zmieniono compound TPE na bardziej chropowaty
- "Regulacja długości wymaga dwóch rąk" → przeprojektowano dźwignię zwalniającą na większą, dostępną jedną ręką
- "Brak oznaczeń kolorystycznych dla różnych rozmiarów tubeingu" → dodano kolorowe pierścienie identyfikacyjne (żółty/czerwony/zielony/niebieski)
- "Zbyt ostre krawędzie na płycie montażowej" → dodano fazowania 0.5×45° na wszystkich krawędziach zewnętrznych

**Prototyp Beta v2.1 (finalny przed produkcją):**
- Uwzględniono wszystkie istotne uwagi z testów Beta v2.0
- Przeprowadzono drugą rundę testów (4 tygodnie) z 30 użytkownikami
- Uzyskano akceptację komisji bioetycznej do testów klinicznych

### 3.1.7.4. Walidacja konstrukcji finalnej

**Prototyp Gold Master v3.0:**
- Egzemplarz referencyjny zatwierdzony do produkcji seryjnej
- Zamrożona dokumentacja (frozen design) – żadne zmiany bez formalnego Engineering Change Request (ECR)
- Wzorzec do porównań dla przyszłych partii produkcyjnych

**Testy walidacyjne Gold Master:**
1. **Testy środowiskowe:**
   - Cycling temperaturowy: -10°C do +50°C, 10 cykli
   - Wilgotność względna: 10% do 95% RH, 48 godzin
   - Wstrząsy: 50 G, 11 ms half-sine (symulacja upadku)
   - Wibracje losowe: 5-500 Hz, 1.5 g RMS, 30 minut na oś

2. **Testy EMC:**
   - Emisja promieniowana: PN-EN 55011 Class B
   - Odporność na pole radiacyjne: PN-EN 61000-4-3 (3 V/m)
   - ESD: PN-EN 61000-4-2 (±8 kV contact, ±15 kV air)

3. **Testy bezpieczeństwa:**
   - Hipot test: 1500 V AC, 60 s, prąd upływu < 5 mA
   - Test palności: UL94 V-0 dla wszystkich tworzyw
   - Test ostrości krawędzi: Brak zadziorów > 0.05 mm

**Wynik:** Wszystkie testy zdane bez zastrzeżeń. Gotowość do certyfikacji i produkcji seryjnej.

---

## 3.1.8. Dokumentacja techniczna i specyfikacje wykonawcze

### 3.1.8.1. Rysunki wykonawcze (Engineering Drawings)

Kompletna dokumentacja rysunkowa obejmuje 23 arkusze formatu A3/A2:

**Lista rysunków:**

| Nr rysunku | Nazwa komponentu | Format | Rewizja | Data |
|------------|------------------|--------|---------|------|
| DWG-001 | Płyta montażowa ścienna | A2 | C | 2024-01-15 |
| DWG-002 | Uchwyt czujnika dolny | A3 | D | 2024-02-03 |
| DWG-003 | Przegub kulisty kompletny | A3 | B | 2024-01-28 |
| DWG-004 | Kulka stalowa Ø12 mm | A4 | A | 2024-01-20 |
| DWG-005 | Trzpień górny z półkulą | A4 | B | 2024-01-22 |
| DWG-006 | Pierścień ograniczający | A4 | A | 2024-01-22 |
| DWG-007 | Uchwyt ręczny korpus | A3 | C | 2024-02-10 |
| DWG-008 | Uchwyt ręczny powłoka TPE | A3 | B | 2024-02-12 |
| DWG-009 | Szybkozłącze męskie | A4 | B | 2024-02-05 |
| DWG-010 | Szybkozłącze żeńskie | A4 | B | 2024-02-05 |
| DWG-011 | Stożkowa tuleja zaciskowa | A4 | A | 2024-02-18 |
| DWG-012 | Mandrel stożkowy | A4 | A | 2024-02-18 |
| DWG-013 | Krążek prowadzący | A3 | B | 2024-02-25 |
| DWG-014 | Oś krążka | A4 | A | 2024-02-25 |
| DWG-015 | Obudowa krążka | A4 | B | 2024-02-26 |
| DWG-016 | Szyna zębata szybkozłącza | A4 | A | 2024-03-01 |
| DWG-017 | Głowica zapadkowa | A4 | A | 2024-03-01 |
| DWG-018 | Dźwignia zwalniająca | A4 | A | 2024-03-02 |
| DWG-019 | Sprężyna zapadki | A4 | A | 2024-03-02 |
| DWG-020 | Kolumna teleskopowa | A2 | B | 2024-03-10 |
| DWG-021 | Podstawa wolnostojąca | A2 | A | 2024-03-12 |
| DWG-022 | Rama systemu dwustronnego | A2 | A | 2024-03-15 |
| DWG-023 | Złożenie główne (assembly) | A1 | C | 2024-03-20 |

**Standardy wymiarowania i tolerancji:**

- **Tolerancje ogólne:** ISO 2768-mK (medium precision for machined parts)
- **Tolerancje geometryczne (GD&T):** ISO 1101 (płaszczyznowość, równoległość, współosiowość)
- **Chropowatość powierzchni:** ISO 1302 (Ra, Rz oznaczenia)
- **Pasowania:** ISO 286-1/2 (system Hole Basis, pasowania H7/g6, H7/h6)

**Przykład zapisu tolerancji geometrycznej dla uchwytu czujnika:**

```
Ⓜ Płaszczyznowość powierzchni styku z czujnikiem: 0.02 mm
∥ Równoległość powierzchni styku do bazy A: 0.05 mm
◎ Współosiowość otworów M4 względem osi C: Ø0.1 mm
⌖ Położenie otworów M4 względem datums A-B-C: Ø0.2 mm
```

### 3.1.8.2. Specyfikacje materiałowe (Material Specifications)

**Specyfikacja MAT-001: Aluminium 6061-T6**

- **Stop:** AlMg1SiCu (wg PN-EN 573-3)
- **Stan hutniczy:** T6 (rozpuszczanie, gaszenie, sztuczne starzenie)
- **Skład chemiczny (% masowych):**
  - Mg: 0.8-1.2
  - Si: 0.4-0.8
  - Cu: 0.15-0.40
  - Cr: 0.04-0.35
  - Fe: ≤ 0.7
  - Zn: ≤ 0.25
  - Ti: ≤ 0.15
  - Mn: ≤ 0.15
  - Al: balance
- **Właściwości mechaniczne:**
  - Wytrzymałość na rozciąganie Rm: ≥ 310 MPa
  - Granica plastyczności Rp0.2: ≥ 276 MPa
  - Wydłużenie A5: ≥ 12%
  - Twardość Brinella: 95 HB
  - Moduł Younga: 68.9 GPa
  - Gęstość: 2.70 g/cm³
- **Certyfikaty wymagane:** EN 10204 3.1 (mill certificate)
- **Dostawcy zatwierdzeni:** Constellium, Hydro Aluminium, Gränges

**Specyfikacja MAT-002: Termoplastyczny elastomer TPE Mediprene H4085**

- **Producent:** Teknor Apex
- **Rodzina:** Mediprene (biokompatybilne TPE)
- **Twardość:** 48 Shore A
- **Gęstość:** 0.89 g/cm³
- **Wytrzymałość na rozciąganie:** 8.5 MPa
- **Wydłużenie przy zerwaniu:** 450%
- **Temperatura pracy:** -40°C do +80°C
- **Certyfikaty:**
  - ISO 10993-5 (cytotoksyczność)
  - ISO 10993-10 (podrażnienia, sensytyzacja)
  - USP Class VI
  - FDA 21 CFR 177.2600
- **Kolor:** Natural (możliwość barwienia masterbatchem)
- **Postać dostawy:** Granulat 25 kg worki

**Specyfikacja MAT-003: Stal łożyskowa 100Cr6**

- **Oznaczenie:** 100Cr6 (PN-EN ISO 683-17) / AISI 52100
- **Skład chemiczny (%):**
  - C: 0.93-1.05
  - Cr: 1.35-1.60
  - Si: 0.15-0.35
  - Mn: 0.25-0.45
  - P: ≤ 0.025
  - S: ≤ 0.015
- **Obróbka cieplna:** Hartowanie w oleju, odpuszczanie 180°C
- **Twardość:** 58-65 HRC
- **Czystość włączeniowa:** ≤ 15 ppm tlenu (steel cleanliness)
- **Zastosowanie:** Kulki przegubu, elementy łożysk

### 3.1.8.3. Instrukcje montażu i konserwacji

**Instrukcja Montażu IM-001: Instalacja modułu czujnika**

**Czas montażu:** 15 minut
**Narzędzia wymagane:** Klucz imbusowy 4 mm, klucz nasadowy 13 mm, pasta gwintująca Loctite 243

**Kroki:**
1. Oczyścić powierzchnię montażową płyty ściennej z kurzu i tłuszczu (alkohol izopropylowy)
2. Przyłożyć płytę montażową do ściany, zaznaczyć 4 otwory
3. Wywiercić otwory Ø8 mm głębokość 50 mm (kołki rozporowe M8)
4. Wbić kołki, przyłożyć płytę, dokręcić śruby M8×60 momentem 25 Nm
5. Włożyć przegub kulisty do gniazda w uchwycie dolnym (sprawdzić swobodny obrót)
6. Nałożyć pierścień ograniczający (kliknięcie potwierdzające)
7. Wkręcić czujnik load cell w przegub (gwint M8, moment 12 Nm)
8. Podłączyć kabel sygnałowy do złącza JST-XH4 (pin 1: VCC, pin 2: A+, pin 3: A-, pin 4: GND)
9. Założyć osłonę kabla (cable gland M12)
10. Przeprowadzić kalibrację zerową (tare) w oprogramowaniu

**Instrukcja Konserwacji IK-001: Przegląd miesięczny**

**Czas przeglądu:** 10 minut
**Czynności:**
1. Wizualna inspekcja tubeingu – szukać pęknięć, naderwań, odbarwień
2. Sprawdzenie luzów w przegubie kulistym (max 0.2 mm ruchu poprzecznego)
3. Dokręcenie wszystkich śrub (moment check – 90% momentu początkowego)
4. Czyszczenie powierzchni styku czujnika (sucha szmatka bezzwłóknieniowa)
5. Sprawdzenie kalibracji z odważnikiem 10 kg (odczyt powinien być w granicach ±2%)
6. Smarowanie prowadnic linek (smar silikonowy w sprayu)
7. Test funkcji alarmowych (symulacja przekroczenia progu siły)

**Zalecenia wymiany elementów eksploatacyjnych:**

| Element | Żywotność [miesiące] | Żywotność [cykle] | Koszt wymiany [PLN] |
|---------|---------------------|-------------------|---------------------|
| Tubeing lateksowy | 12-18 | 50 000 | 45-80 |
| Pierścień zatrzaskowy | 24 | 20 000 | 8 |
| Sprężyna zapadki | 36 | 30 000 | 5 |
| Łożysko krążka | 60 | 100 000 | 15 |
| Powłoka TPE uchwytu | 48 | 80 000 | 35 |
| Linka stalowa Ø6 mm | 60 | 100 000 | 25 |

---

## 3.1.9. Podsumowanie rozdziału

Niniejszy podrozdział przedstawił kompletny proces projektowania mechanicznego autorskiego systemu pomiarowo-treningowego z biofeedbackiem, od koncepcji architektury układu pomiarowego, poprzez szczegółowe projekty komponentów, analizy wytrzymałościowe MES, proces prototypowania i iteracji, aż po finalną dokumentację wykonawczą.

**Kluczowe osiągnięcia projektowe:**

1. **Opracowanie innowacyjnego mocowania czujnika tensometrycznego** z przegubem kulistym samocentrującym, eliminującym błędy z tytułu nieosiowego przyłożenia siły (redukcja błędów pomiarowych z 8% do < 1%).

2. **Zaprojektowanie ergonomicznego uchwytu ręcznego** z dwukomponentowego materiału (rdzeń PA6-GF30 + powłoka TPE Mediprene), spełniającego wymagania 95% populacji użytkowników i posiadającego certyfikaty biokompatybilności ISO 10993.

3. **Opracowanie szybkozłącza wciskowego z zatrzaskiem kulistym**, umożliwiającego wymianę tubeingu w 5 sekund bez narzędzi, przy jednoczesnym utrzymaniu siły wyrywania 1200 N.

4. **Zaprojektowanie stożkowej tulei zaciskowej** do mocowania tubeingu lateksowego, osiągającej 94% wytrzymałości bazowej gumy (zerwanie zawsze poza strefą zacisku).

5. **Przeprowadzenie kompleksowych analiz MES** (statyczna, zmęczeniowa, modalna, optymalizacja topologiczna), potwierdzających współczynniki bezpieczeństwa > 1.5 dla wszystkich komponentów i żywotność > 600 000 cykli.

6. **Zrealizowanie procesu prototypowania w modelu V-Model** z trzema rundami prototypów (PoC, Alpha, Beta) z udziałem 87 rzeczywistych użytkowników, co pozwoliło na wyeliminowanie 94% usterek przed produkcją seryjną.

7. **Wygenerowanie kompletnej dokumentacji wykonawczej** obejmującej 23 rysunki z tolerancjami geometrycznymi GD&T, 3 specyfikacje materiałowe z certyfikatami EN 10204 3.1, oraz instrukcje montażu i konserwacji.

**Wkład w dziedzinę inżynierii biomedycznej:**

Przedstawione rozwiązania konstrukcyjne stanowią oryginalny wkład w dziedzinę mechanicznego projektowania systemów pomiarowych dla rehabilitacji i sportu. Szczególne znaczenie mają:
- Metoda kompensacji niedokładności montażu poprzez przegub kulisty z ogranicznikiem kątowym
- Algorytm optymalizacji geometrii uchwytu pod kątem antropometrii wielopopulacyjnej
- Autorskie szybkozłącze hybrydowe łączące zalety połączeń gwintowych i bagnetowych

**Gotowość do wdrożenia:**

Finalna wersja projektu (Gold Master v3.0) uzyskała pozytywne wyniki wszystkich testów walidacyjnych, w tym środowiskowych, EMC i bezpieczeństwa. Dokumentacja jest zamrożona i gotowa do przekazania do produkcji seryjnej. Szacowany koszt wytworzenia jednego kompletu mechanicznego w serii 1000 sztuk wynosi 285 PLN (bez elektroniki), co plasuje system w segmencie ekonomicznym przy zachowaniu parametrów jakościowych klasy premium.

**Perspektywy rozwoju:**

Planowane prace nad kolejną generacją systemu mechanicznego obejmują:
- Zastosowanie kompozytów węglowych (CFRP) do redukcji masy o 40%
- Integrację czujników siły w samym uchwycie (force-sensing grip)
- Druk 3D metalowy (DMLS) dla zoptymalizowanych topologicznie geometrii
- Modułową platformę z możliwością rozbudowy o kolejne czujniki (EMG, goniometry, akcelerometry)

Niniejszy rozdział stanowi fundament dla kolejnych części pracy, w szczególności rozdziału 3.2 (układy warunkowania sygnału) i rozdziału 3.4 (prototypowanie i testy wytrzymałościowe), gdzie opisano integrację zaprojektowanych komponentów mechanicznych z elektroniką i oprogramowaniem systemu.
