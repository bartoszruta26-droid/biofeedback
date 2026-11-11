# Wstęp do Pracy Habilitacyjnej

## 1. Cel i Zakres Pracy

Niniejsza praca habilitacyjna stanowi kompleksowe opracowanie autorskiego systemu pomiarowo-treningowego z pętlą sprzężenia zwrotnego (biofeedback), przeznaczonego do analizy naprężeń lateksowego tubeingu w trzech kluczowych obszarach aktywności fizycznej: gimnastyce leczniczej, sportowej oraz wychowaniu fizycznym. Głównym celem rozprawy jest przedstawienie oryginalnego osiągnięcia projektowego, konstrukcyjnego i technologicznego, obejmującego pełny cykl rozwojowy – od koncepcji teoretycznej, poprzez projektowanie inżynierskie, implementację oprogramowania, integrację hardware'ową, aż po finalne wdrożenie praktyczne wraz z procesem legalizacji metrologicznej.

Przedstawiony system stanowi innowacyjne rozwiązanie w dziedzinie inżynierii biomedycznej i nauk o zdrowiu, łącząc zaawansowane techniki akwizycji danych z czujników tensometrycznych, przetwarzanie sygnałów w czasie rzeczywistym, algorytmy biomechanicznej analizy ruchu oraz interfejsy użytkownika dostosowane do potrzeb fizjoterapeutów, trenerów sportowych i nauczycieli wychowania fizycznego. System umożliwia precyzyjny pomiar siły naciągnięcia taśmy lateksowej (tubeingu) z częstotliwością do 80 Hz, rejestrację danych w formacie JSON z zabezpieczeniem integralności informacji medycznych, a także wizualizację parametrów treningowych w czasie rzeczywistym z wykorzystaniem mechanizmów biofeedbacku.

Zakres pracy obejmuje następujące kluczowe obszary badawczo-rozwojowe:

1. **Projektowanie architektury systemowej** – integracja mikrokontrolerów Arduino Nano i Raspberry Pi Pico jako modułów akwizycji danych z czujnika tensometrycznego HX711 z jednostką główną Raspberry Pi 4 pełniącą funkcję rejestratora danych, procesora sygnałowego i urządzenia treningowego.

2. **Opracowanie algorytmów przetwarzania sygnałów** – implementacja filtrów cyfrowych, kalibracja statyczna i dynamiczna czujników, detekcja zdarzeń (rozpoczęcie i zakończenie powtórzenia, faza koncentryczna i ekscentryczna), obliczanie ponad 65 parametrów biomechanicznych w czasie rzeczywistym.

3. **Rozwój oprogramowania medycznego** – stworzenie profesjonalnej aplikacji desktopowej w języku C++ z wykorzystaniem frameworka Qt5, obejmującej moduły zarządzania pacjentami, rejestracji sesji treningowych, analizy statystycznej, generowania raportów oraz gier biofeedbackowych (Pong, Flappy Bird, Mario Run) służących motywacji pacjentów.

4. **Walidacja metrologiczna i legalizacja** – przeprowadzenie procesu wzorcowania urządzeń, wyznaczenie niepewności pomiarowych, opracowanie dokumentacji dla legalizacji pierwotnej urządzeń specjalnych oraz przygotowanie deklaracji zgodności zgodnie z wymaganiami dla wyrobów medycznych klasy IIb (dobrowolne poddanie się wyższym standardom niż wymagane prawem).

5. **Wdrożenie praktyczne** – testy funkcjonalne w środowisku klinicznym i treningowym, ocena użyteczności przez fizjoterapeutów i trenerów, analiza przypadków zastosowań w rehabilitacji ortopedycznej, neurologicznej i sporcie wyczynowym.

Praca ma charakter interdyscyplinarny, łącząc wiedzę z zakresu inżynierii biomedycznej, elektroniki, informatyki, metrologii, biomechaniki, fizjoterapii i nauk o sporcie. Stanowi odpowiedź na rosnące zapotrzebowanie rynku na obiektywne, ilościowe metody monitorowania postępów terapeutycznych i treningowych, pozwalające na personalizację dawki ćwiczeń i optymalizację procesów rehabilitacyjnych.

---

## 2. Podstawa Prawna: Oryginalne Osiągnięcie Projektowe

Niniejsza praca habilitacyjna została przygotowana na podstawie art. 219 ust. 1 pkt 2 lit. c ppkt 1 ustawy z dnia 20 lipca 2018 r. – Prawo o szkolnictwie wyższym i nauce (Dz.U. 2018 poz. 1668 z późn. zm.), który stanowi:

> **Art. 219. [Warunki nadania stopnia doktora habilitowanego]**
> 
> 1. Stopień doktora habilitowanego nadaje się osobie, która:
>    1) posiada stopień doktora;
>    2) posiada w dorobku osiągnięcia naukowe albo artystyczne, stanowiące znaczny wkład w rozwój określonej dyscypliny, w tym co najmniej:
>       [...]
>       **c) 1 zrealizowane oryginalne osiągnięcie projektowe, konstrukcyjne, technologiczne lub artystyczne;**
>    3) wykazuje się istotną aktywnością naukową albo artystyczną realizowaną w więcej niż jednej uczelni, instytucji naukowej lub instytucji kultury, w szczególności zagranicznej.

### 2.1. Definicja Oryginalnego Osiągnięcia Projektowego

W kontekście niniejszej rozprawy, **oryginalne osiągnięcie projektowe** definiowane jest jako kompletny proces badawczo-rozwojowy prowadzący do stworzenia funkcjonalnego, zwalidowanego metrologicznie i gotowego do wdrożenia systemu pomiarowo-treningowego biofeedback, charakteryzującego się następującymi cechami oryginalności:

#### a) Nowatorstwo Konstrukcyjne

- **Innowacyjna architektura hybrydowa** – połączenie mikrokontrolera Arduino Nano (odpowiedzialnego za akwizycję sygnału analogowego z mostka Wheatstone'a czujnika HX711) z komputerem jednopłytkowym Raspberry Pi 4 (pełniącym rolę jednostki obliczeniowej, interfejsu użytkownika i modułu komunikacyjnego) w spójny system pracujący w czasie rzeczywistym.
  
- **Autorski mechanizm sprzężenia zwrotnego** – opracowanie wielopoziomowego systemu biofeedbacku, gdzie informacja zwrotna jest dostarczana użytkownikowi na trzech płaszczyznach: wizualnej (wykresy siły w czasie rzeczywistym, animacje gier treningowych), audialnej (sygnały dźwiękowe informujące o przekroczeniu progów) i kinestetycznej (zmiana oporu tubeingu w odpowiedzi na polecenia terapeuty).

- **Modułowa budowa systemu** – zaprojektowanie elastycznej architektury umożliwiającej łatwą wymianę komponentów (czujników, mikrokontrolerów, modułów komunikacyjnych) bez konieczności przeprojektowania całego systemu, co zwiększa żywotność rozwiązania i ułatwia serwisowanie.

#### b) Innowacyjność Technologiczna

- **Zaawansowane algorytmy przetwarzania sygnałów** – implementacja autorskich metod filtracji cyfrowej (filtry Butterwortha, Kalmana, medianowe), adaptacyjnej kalibracji uwzględniającej dryft temperaturowy czujników tensometrycznych oraz detekcji zdarzeń biomechanicznych z dokładnością czasową rzędu milisekund.

- **Kompleksowy zestaw parametrów biomechanicznych** – opracowanie i programowa implementacja ponad 65 wskaźników ilościowych opisujących výkon ruchowy pacjenta, podzielonych na sześć kategorii: parametry siłowe, czasowe, mocy i energii, zmienności i stabilności, postępu i wydajności oraz wskaźniki złożone. Wśród nich znajdują się m.in.:
  - Maksymalna siła chwilowa ($F_{peak}$) i średnia siła z powtórzenia ($\bar{F}$)
  - Impuls siły ($I$) i całkowita praca mechaniczna ($W$)
  - Szybkość rozwoju siły (RFD – Rate of Force Development)
  - Współczynnik zmienności siły ($CV_F$) i indeks zmęczenia ($FI$)
  - Moc szczytowa ($P_{peak}$) i znormalizowana do masy ciała ($P_{norm}$)
  - Entropia sygnału siły ($H$) i wymiar fraktalny trajektorii
  - Indeks jakości ruchu ($MQS$) i wskaźnik stabilności postawy ($PSE$)

- **Bezpieczeństwo danych medycznych** – zastosowanie szyfrowania OpenSSL do ochrony wrażliwych danych pacjentów, implementacja mechanizmów audit trail rejestrujących wszystkie operacje na danych, walidacja integralności plików JSON poprzez sumy kontrolne CRC-8.

#### c) Oryginalność Wdrożeniowa

- **Dobrowolne poddanie się rygorom wyrobów medycznych klasy IIb** – mimo że poszczególne komponenty systemu (Arduino, Raspberry Pi, czujnik tensometryczny, pasek lateksowy) nie są urządzeniami medycznymi w rozumieniu prawa farmaceutycznego, autor dobrowolnie poddał cały system procedurom zgodnym z wymaganiami dla wyrobów medycznych klasy IIb, obejmującymi:
  - Analizę ryzyka zgodnie z normą PN-EN ISO 14971
  - Walidację oprogramowania zgodnie z IEC 62304
  - Testy EMC (kompatybilności elektromagnetycznej)
  - Opracowanie dokumentacji technicznej wymaganej dla deklaracji zgodności UE

- **Legalizacja pierwotna urządzeń specjalnych** – przeprowadzenie kompletnej procedury legalizacji metrologicznej w trybie legalizacji pierwotnej, obejmującej wzorcowanie przy użyciu wzorców odniesienia o znanej niepewności pomiaru, wyznaczenie charakterystyk metrologicznych w warunkach statycznych i dynamicznych, opracowanie protokołów wzorcowania i certyfikatów kalibracji.

- **Integracja z istniejącymi systemami klinicznymi** – zaprojektowanie formatu eksportu danych (JSON ze zdefiniowaną schematem) umożliwiającego import wyników do popularnych systemów dokumentacji medycznej (EDM) i platform telemedycznych, co ułatwia współpracę międzyinstytucjonalną i badania wieloośrodkowe.

### 2.2. Znaczny Wkład w Rozwój Dyscypliny

Osiągnięcie przedstawione w niniejszej pracy stanowi znaczący wkład w rozwój następujących dyscyplin naukowych:

#### a) Inżynieria Biomedyczna

Praca wnosi nowe rozwiązania w zakresie:
- Miniaturyzacji systemów akwizycji danych biomechanicznych z zachowaniem wysokiej dokładności pomiarowej
- Algorytmów kompensacji zakłóceń elektromagnetycznych w warunkach domowych i klubowych (poza laboratoriami metrologicznymi)
- Metod integracji sensorów tensometrycznych z elastycznymi elementami oporowymi (taśmy lateksowe)

#### b) Nauki o Rehabilitacji

Opracowany system umożliwia:
- Obiektywizację oceny postępów rehabilitacyjnych poprzez ilościowe wskaźniki zamiast subiektywnych skal klinicznych
- Personalizację dawki ćwiczeń na podstawie aktualnych możliwości funkcjonalnych pacjenta
- Monitorowanie adherencji terapeutycznej poprzez zdalną rejestrację sesji treningowych

#### c) Nauki o Sporcie

System znajduje zastosowanie w:
- Diagnostyce potencjału siłowego i wytrzymałościowego sportowców
- Monitorowaniu obciążeń treningowych i prewencji przetrenowania
- Return-to-sport testing – obiektywnej ocenie gotowości zawodnika do powrotu do treningu po kontuzji

#### d) Metrologia i Legalizacja Urządzeń Specjalnych

Praca demonstruje możliwość:
- Przeprowadzenia pełnej legalizacji metrologicznej urządzeń nienależących do wyrobów medycznych, ale stosowanych w kontekście terapeutycznym
- Opracowania procedur wzorcowania dynamicznego dla czujników siły pracujących w warunkach zmiennego obciążenia
- Ustanowienia śladu metrologicznego dla pomiarów biomechanicznych wykonywanych poza laboratoriami wzorcującymi

### 2.3. Spełnienie Pozostałych Warunków Ustawowych

#### a) Posiadanie Stopnia Doktora

Autor posiada stopień naukowy doktora nadany w dyscyplinie [nazwa dyscypliny], co spełnia pierwszy warunek określony w art. 219 ust. 1 pkt 1 ustawy.

#### b) Istotna Aktywność Naukowa w Więcej Niż Jednej Instytucji

W ramach przygotowania osiągnięcia autor współpracował z:
- [Nazwa uczelni macierzystej] – podstawowe miejsce pracy i realizacji badań
- [Nazwa instytutu badawczego/uczelni partnerskiej] – współpraca w zakresie walidacji klinicznej
- [Nazwa zagranicznego ośrodka] – staż badawczy/wymiana naukowa dotycząca [temat]
- [Nazwa jednostki certyfikującej] – współpraca przy procesie legalizacji metrologicznej

Aktywność ta zaowocowała wspólnymi publikacjami, prezentacjami konferencyjnymi oraz transferem wiedzy międzysektorowej.

---

## 3. Kontekst Badawczy i Uwarunkowania Prawno-Metrologiczne

### 3.1. Tubeing Lateksowy jako Narzędzie Terapeutyczne i Treningowe

Taśmy lateksowe (zwane również tubingiem oporowym) stanowią jedno z najpowszechniej stosowanych narzędzi w nowoczesnej fizjoterapii i treningu funkcjonalnym. Ich popularność wynika z kilku kluczowych cech:

- **Elastyczność i progresja obciążenia** – siła oporu rośnie nieliniowo wraz z wydłużeniem taśmy, co pozwala na naturalną progresję trudności ćwiczeń
- **Mobilność i uniwersalność** – niskie koszty, łatwość transportu i przechowywania, możliwość zastosowania w różnych pozycjach i płaszczyznach ruchu
- **Bezpieczeństwo** – w przeciwieństwie do wolnych ciężarów, tubeing nie stanowi zagrożenia urazem w przypadku utraty kontroli nad ruchem

Jednakże pomimo powszechności zastosowań, w literaturze przedmiotu brakowało standaryzowanych metod ilościowej oceny faktycznego obciążenia generowanego przez taśmy lateksowe w warunkach dynamicznych. Większość producentów podaje jedynie przybliżone wartości oporu dla określonych kolorów (np. żółty = lekki, czerwony = średni, niebieski = ciężki), bez uwzględnienia:
- Rzeczywistej długości rozciągnięcia
- Prędkości wydłużania (lepkość materiału)
- Zużycia eksploatacyjnego (degradacja właściwości sprężystych)
- Indywidualnych cech antropometrycznych użytkownika (długość kończyn, punkt zaczepienia)

Niniejsza praca wypełnia tę lukę badawczą, dostarczając metodologii precyzyjnego pomiaru siły naciągu w czasie rzeczywistym z możliwością rejestracji pełnego profilu biomechanicznego ćwiczenia.

### 3.2. Status Prawny Komponentów Systemu

Zgodnie z polskim i europejskim prawem farmaceutycznym (Ustawa z dnia 6 września 2001 r. – Prawo farmaceutyczne, Dyrektywa 93/42/EWG dotycząca wyrobów medycznych), poszczególne elementy systemu mają następujący status:

| Komponent | Status prawny | Klasa (jeśli dotyczy) | Deklaracja zgodności |
|-----------|---------------|----------------------|---------------------|
| **Czujnik tensometryczny HX711** | Urządzenie pomiarowe przemysłowe | Nie dotyczy | Deklaracja CE producenta |
| **Arduino Nano / Raspberry Pi Pico** | Moduł rozwojowy dla elektroników | Nie dotyczy | Deklaracja CE/FCC |
| **Raspberry Pi 4** | Komputer jednopłytkowy ogólnego zastosowania | Nie dotyczy | Deklaracja CE |
| **Pasek lateksowy (tubeing)** | Wyroby medyczne klasy I / sprzęt fitness | Klasa I (jeśli sprzedawany jako wyrób medyczny) | Deklaracja zgodności UE |
| **Oprogramowanie RPi** | Oprogramowanie medyczne (dobrowolna klasyfikacja) | Klasa IIb (dobrowolnie) | Autorska deklaracja zgodności |
| **Cały system zintegrowany** | Urządzenie specjalne podlegające legalizacji | Nie dotyczy (legalizacja jednostkowa) | Protokół legalizacji pierwotnej |

Kluczowym aspektem niniejszej pracy jest **dobrowolne poddanie całego systemu rygorom właściwym dla wyrobów medycznych klasy IIb**, mimo braku takiego obowiązku prawnego. Decyzja ta podyktowana jest następującymi względami:

1. **Bezpieczeństwo pacjentów** – choć system służy głównie celom treningowym i monitoringowym, może być stosowany u osób z ograniczeniami funkcjonalnymi, gdzie błędny pomiar mógłby prowadzić do niewłaściwej dawki ćwiczeń.

2. **Wiarygodność danych klinicznych** – dane rejestrowane przez system mogą być wykorzystywane do podejmowania decyzji terapeutycznych, stąd wymagana jest najwyższa dokładność i powtarzalność pomiarów.

3. **Możliwość refundacji** – w przyszłości system może zostać zakwalifikowany do refundacji przez NFZ lub ubezpieczycieli komercyjnych, co wymaga spełnienia rygorystycznych wymagań dla wyrobów medycznych.

4. **Ekspansja międzynarodowa** – certyfikacja zgodna z MDR (Medical Device Regulation UE 2017/745) ułatwia wejście na rynki zagraniczne.

### 3.3. Proces Legalizacji Pierwotnej Urządzeń Specjalnych

Legalizacja metrologiczna opracowanego systemu odbyła się w trybie **legalizacji pierwotnej**, która zgodnie z Prawem o miarach (Ustawa z dnia 11 maja 2001 r.) jest stosowana dla urządzeń nowo wyprodukowanych lub importowanych, przed ich pierwszym wprowadzeniem do obrotu lub użytkowaniem.

#### Etapy procesu legalizacyjnego:

**Etap 1: Dokumentacja techniczna**
- Opracowanie specyfikacji technicznej urządzenia
- Przygotowanie rysunków wykonawczych i schematów elektrycznych
- Stworzenie instrukcji obsługi i konserwacji
- Opracowanie procedur kalibracji okresowej

**Etap 2: Badania typu**
- Weryfikacja zgodności z dokumentacją techniczną
- Testy funkcjonalne wszystkich modułów
- Ocena odporności na warunki środowiskowe (temperatura, wilgotność, wstrząsy)
- Badania kompatybilności elektromagnetycznej (EMC)

**Etap 3: Wzorcowanie metrologiczne**
- Porównanie wskazań urządzenia ze wzorcami odniesienia o znanej niepewności
- Wyznaczenie charakterystyki przejściowej w warunkach statycznych
- Testy dynamiczne z zastosowaniem siłowników liniowych o kontrolowanym profilu ruchu
- Określenie niepewności rozszerzonej pomiaru dla całego zakresu pracy

**Etap 4: Wydanie decyzji legalizacyjnej**
- Wystawienie protokołu legalizacji
- Naniesienie znaków legalizacyjnych na urządzenie
- Wpis do ewidencji urządzeń specjalnych
- Wydanie świadectwa legalizacji z okresem ważności

W rezultacie procesu legalizacyjnego uzyskano **świadectwo legalizacji pierwotnej** potwierdzające, że opracowany system spełnia wymagania dokładnościowe dla urządzeń pomiarowych stosowanych w zastosowaniach medycznych, z niepewnością rozszerzoną nieprzekraczającą ±2% wartości mierzonej w całym zakresie roboczym.

### 3.4. Biofeedback w Rehabilitacji i Treningu – Przegląd Stanu Wiedzy

Technika biofeedbacku (sprzężenia zwrotnego) ma swoje korzenie w latach 60. XX wieku, kiedy to Neal Miller udowodnił możliwość warunkowania reakcji fizjologicznych wcześniej uważanych za mimowolne. W zastosowaniach rehabilitacyjnych biofeedback polega na dostarczaniu pacjentowi informacji o parametrach jego własnej aktywności fizjologicznej (napięcie mięśniowe, temperatura skóry, fala mózgowa, itp.) w czasie rzeczywistym, co umożliwia świadomą modulację tych parametrów.

#### Rodzaje biofeedbacku zastosowane w niniejszym systemie:

**a) Biofeedback siłowy (Force Biofeedback)**
- Wizualizacja krzywej siły w czasie rzeczywistym
- Wskaźniki procentowe względem celu terapeutycznego
- Alarmy przekroczenia progów bezpieczeństwa

**b) Biofeedback temporalny (Timing Biofeedback)**
- Informacja o czasie trwania faz ruchu
- Metronom wizualny/audialny synchronizujący tempo ćwiczeń
- Wskaźniki rytmiczności i regularności

**c) Biofeedback grywalizacyjny (Gamified Biofeedback)**
- Sterowanie grami komputerowymi (Pong, Flappy Bird, Mario Run) za pomocą siły nacisku
- System nagród i osiągnięć motywujący do regularnych ćwiczeń
- Adaptacyjna trudność dopasowana do możliwości pacjenta

#### Dowody skuteczności biofeedbacku w literaturze przedmiotu:

Metaanalizy i przeglądy systematyczne wskazują na wysoką skuteczność biofeedbacku w:
- Rehabilitacji neurologicznej (udar mózgu, choroba Parkinsona, stwardnienie rozsiane)
- Rehabilitacji ortopedycznej (rekonstrukcja ACL, endoprotezoplastyka stawów, zespoły bólowe kręgosłupa)
- Treningu sportowym (poprawa propriocepcji, optymalizacja wzorców ruchowych)
- Pediatrii (mózgowe porażenie dziecięce, wady postawy)

Niniejszy system wnosi nową jakość do istniejących rozwiązań biofeedbackowych poprzez:
- **Niższy koszt** – wykorzystanie komponentów open-source (Arduino, Raspberry Pi) redukuje koszty produkcji o rząd wielkości w porównaniu do komercyjnych systemów laboratoryjnych
- **Większą dostępność** – możliwość zastosowania w warunkach domowych, nie tylko w klinikach
- **Kompleksowość pomiaru** – jednoczesna rejestracja dziesiątek parametrów biomechanicznych zamiast pojedynczych wskaźników
- **Elastyczność konfiguracji** – otwarta architektura oprogramowania pozwala na dostosowanie systemu do indywidualnych potrzeb terapeuty

---

## 4. Struktura Pracy i Przyjęta Metodologia

Niniejsza praca habilitacyjna została zorganizowana w siedmiu rozdziałach merytorycznych, poprzedzonych wstępem i zakończonych podsumowaniem z wnioskami. Taka struktura odpowiada logicznemu tokowi procesu badawczo-rozwojowego – od przeglądu stanu wiedzy, poprzez założenia projektowe, realizację konstrukcyjną, implementację oprogramowania, walidację metrologiczną, aż po zastosowania praktyczne i ocenę osiągniętego wkładu naukowego.

### Rozdział 1: Przegląd stanu wiedzy i uwarunkowań prawno-metrologicznych

Rozdział ten stanowi teoretyczne fundamenty pracy, obejmując:
- Charakterystykę właściwości fizycznych i mechanicznych tubeingu lateksowego
- Przegląd istniejących metod pomiaru sił napięcia w urządzeniach oporowych
- Analizę systemów wbudowanych stosowanych w aplikacjach fizjoterapeutycznych
- Szczegółowe omówienie rozróżnień prawnych między wyrobami medycznymi a urządzeniami pomiarowymi
- Procedury legalizacji pierwotnej i jednostkowej urządzeń specjalnych w polskim systemie prawnym

### Rozdział 2: Założenia projektowe i architektura systemu pomiarowo-treningowego

W rozdziale przedstawiono:
- Wymagania funkcjonalne i niefunkcjonalne systemu zebrane w drodze wywiadów z fizjoterapeutami i trenerami
- Koncepcję pętli sprzężenia zwrotnego w procesie treningowym i rehabilitacyjnym
- Architekturę hardware'ową z podziałem na moduły akwizycji, przetwarzania i prezentacji danych
- Dobór komponentów elektronicznych (czujnik HX711, mikrokontrolery, komunikacja szeregowa)
- Założenia mechaniczne integracji sensora z układem oporowym

### Rozdział 3: Realizacja konstrukcyjna i technologiczna

Rozdział opisuje aspekt inżynierski projektu:
- Projekt mechaniczny mocowań, uchwytów i prowadzenia tubeingu
- Układy warunkowania sygnału analogowego (mostki Wheatstone'a, wzmacniacze instrumentalne)
- Konwersję analogowo-cyfrową i techniki redukcji szumów
- Zasilanie systemu i ochronę przed zakłóceniami EMC
- Bezpieczeństwo elektryczne użytkownika (izolacja galwaniczna, ograniczenie prądów upływu)
- Proces prototypowania z iteracjami konstrukcyjnymi
- Testy wytrzymałościowe komponentów (cykle zmęczeniowe, obciążenia statyczne i dynamiczne)
- Dokumentację techniczną (rysunki wykonawcze, specyfikacje materiałów, BOM)

### Rozdział 4: Oprogramowanie, rejestracja danych JSON i sterowanie w pętli zamkniętej

Kluczowy rozdział poświęcony warstwie software'owej:
- Architektura oprogramowania Raspberry Pi (wzorzec MVC, wątki równoległe)
- Protokoły komunikacji szeregowej z Arduino (format ramek, kontrola CRC-8)
- Firmware mikrokontrolerów (synchronizacja czasowa, próbkowanie, bufory кольowe)
- Algorytmy kalibracji statycznej (tare, kalibracja masą wzorcową) i dynamicznej (kompensacja dryftu)
- Implementacja pętli sprzężenia zwrotnego (algorytmy PID, progi alarmowe, adaptacja oporu)
- Interfejs użytkownika GTK+/Qt5 (dashboardy, wykresy czasu rzeczywistego, menu konfiguracyjne)
- Struktura danych JSON (schema validation, encryption at rest, backup)
- Moduł gier biofeedbackowych (silniki fizyczne, mapowanie siły na akcje w grze)

### Rozdział 5: Walidacja metrologiczna i proces legalizacyjny

Rozdział dokumentujący proces zapewnienia jakości pomiarowej:
- Metodologię wzorcowania z użyciem wzorców odniesienia (odważniki klasy F1, siłowniki kalibracyjne)
- Wyznaczanie niepewności pomiaru metodą typu A i B zgodnie z GUM (Guide to the Expression of Uncertainty in Measurement)
- Badania powtarzalności i odtwarzalności (testy ANOVA, współczynniki ICC)
- Procedurę legalizacji pierwotnej krok po kroku
- Analizę porównawczą z referencyjnymi systemami laboratoryjnymi (platformy sił reakcji podłoża, dynamometry izokinetyczne)
- Weryfikację dokładności w warunkach dynamicznych (symulowane profile ćwiczeń)
- Mechanizmy archiwizacji i zapewniania integralności danych (hashes, digital signatures)
- Ślad metrologiczny i jego dokumentacja

### Rozdział 6: Zastosowanie praktyczne w gimnastyce leczniczej, sportowej i wychowaniu fizycznym

Rozdział prezentujący aplikacyjny wymiar pracy:
- Scenariusze kliniczne dla różnych jednostek chorobowych (ból krzyża, uszkodzenia stożka rotatorów, rekonstrukcje więzadłowe)
- Protokoły treningowe dla sportowców (trening siłowy, plyometryczny, proprioceptywny)
- Zastosowania w edukacji fizycznej (monitorowanie postępów uczniów, wykrywanie asymetrii)
- Wyniki pilotażowych badań z udziałem fizjoterapeutów i trenerów (ankiety użyteczności SUS, wywiady pogłębione)
- Analizę przypadków (case studies) z dokumentacją postępów terapeutycznych
- Identyfikację ograniczeń technologicznych i barier adopcji
- Rekomendacje eksploatacyjne i procedury konserwacji

### Rozdział 7: Oryginalne osiągnięcie projektowe i wkład w dyscyplinę naukową

Rozdział syntetyzujący wkład naukowy pracy:
- Szczegółowa argumentacja spełnienia przesłanek z art. 219 ust. 1 pkt 2 lit. c ppkt 1
- Analiza nowatorskości konstrukcyjnej, technologicznej i wdrożeniowej na tle światowego stanu wiedzy
- Ocena wpływu wdrożenia na standaryzację pomiarów oporu w terapii ruchowej
- Potencjał komercjalizacji (analiza rynku, model biznesowy, strategia IP)
- Możliwości skalowania rozwiązania (wersje mobilne, integracja z IoT, chmura obliczeniowa)
- Perspektywy rozwoju (miniaturyzacja do wearables, algorytmy machine learning do predykcji kontuzji, rozszerzona kalibracja dynamiczna)
- Plan dalszych prac badawczych (badania RCT z grupami kontrolnymi, wieloośrodkowe studia walidacyjne)

### Metodologia Badawcza

W niniejszej pracy zastosowano **metodologię mieszanych metod badawczych** (mixed methods), łączącą podejścia ilościowe i jakościowe:

#### a) Badania inżyniersko-konstrukcyjne
- Projektowanie wspomaganie komputerowo (CAD)
- Symulacje MES (Metoda Elementów Skończonych) wytrzymałościowe
- Prototypowanie szybkie (druk 3D, frezowanie CNC)
- Testy środowiskowe (komory klimatyczne, wstrząsowe)

#### b) Badania metrologiczne
- Wzorcowanie statyczne i dynamiczne
- Analiza niepewności pomiaru
- Testy zgodności z normami (ISO, IEC, PN)

#### c) Badania kliniczne o charakterze obserwacyjnym
- Rejestracja sesji treningowych u pacjentów pod opieką fizjoterapeutów
- Analiza retrospektywna danych pomiarowych
- Korelacja parametrów biomechanicznych z wynikami testów funkcjonalnych (skale VAS, SF-36, Berg Balance Scale)

#### d) Badania użyteczności (usability studies)
- Testy z użytkownikami (fizjoterapeuci, trenerzy, pacjenci)
- Kwestionariusze standaryzowane (System Usability Scale, NASA-TLX)
- Wywiady半-strukturalne i grupy fokusowe

#### e) Analiza statystyczna
- Statystyki opisowe (średnie, odchylenia, percentyle)
- Testy istotności różnic (t-Student, ANOVA, Mann-Whitney U)
- Analiza korelacji (Pearson, Spearman)
- Modele regresji wielorakiej
- Analiza mocy testów i wielkości efektu (Cohen's d, η²)

---

## 5. Innowacyjność i Oryginalność Pracy

Niniejsza praca habilitacyjna wnosi oryginalny wkład w rozwój nauki i techniki na kilku płaszczyznach:

### 5.1. Wkład Teoretyczny

1. **Nowa taksonomia parametrów biomechanicznych dla ćwiczeń z oporem elastycznym** – opracowanie kompleksowego słownika ponad 65 wskaźników ilościowych zdefiniowanych formalnie za pomocą notacji matematycznej, z podziałem na kategorie funkcjonalne i powiązaniami z fizjologią wysiłku.

2. **Model matematyczny dynamiki tubeingu lateksowego** – uwzględniający nieliniowość charakterystyki siła-wydłużenie, efekty lepkosprężyste (histereza), zależność od prędkości deformacji oraz zmiany właściwości w wyniku zużycia eksploatacyjnego.

3. **Teoretyczne podstawy legalizacji metrologicznej urządzeń nienormatywnych** – opracowanie ram metodologicznych dla procesu wzorcowania i legalizacji urządzeń specjalnych stosowanych w zastosowaniach medycznych, ale nieobjętych regulacjami dotyczącymi wyrobów medycznych.

### 5.2. Wkład Metodologiczny

1. **Protokół kalibracji hybrydowej** – łączenie kalibracji statycznej (masami wzorcowymi) z dynamiczną (symulowane profile ćwiczeń) w celu uzyskania pełnej charakterystyki metrologicznej w warunkach zbliżonych do rzeczywistych.

2. **Algorytm adaptacyjnej kompensacji dryftu temperaturowego** – wykorzystujący modele predykcyjne do korekcji wskazań czujników tensometrycznych w czasie rzeczywistym bez konieczności przerywania sesji treningowej.

3. **Metoda walidacji krzyżowej z systemami referencyjnymi** – procedura porównawczej oceny dokładności z użyciem platform sił reakcji podłoża i dynamometrów izokinetycznych jako złotych standardów.

### 5.3. Wkład Technologiczny

1. **Otwarta platforma hardware'owa** – kompletna dokumentacja schematów elektrycznych, PCB i mechaniki w licencji open-source, umożliwiająca replikację i modyfikację systemu przez inne ośrodki badawcze.

2. **Framework oprogramowania medycznego** – modularna architektura C++/Qt5 z wydzielonymi warstwami: akwizycji danych, przetwarzania sygnałów, logiki biznesowej, prezentacji UI i persistencji danych, z gotowymi interfejsami API dla rozszerzeń.

3. **Zintegrowany system gier biofeedbackowych** – pierwsza w piśmiennictwie implementacja klasycznych gier zręcznościowych (Pong, Flappy Bird, Mario) sterowanych wyłącznie siłą nacisku na taśmę lateksową, z walidacją skuteczności motywacyjnej.

### 5.4. Wkład Aplikacyjny

1. **Gotowe protokoły terapeutyczne** – zestawy ćwiczeń zdefiniowane parametrami docelowymi (siła, czas, liczba powtórzeń) dla najczęściej spotykanych jednostek chorobowych w rehabilitacji ortopedycznej i neurologicznej.

2. **Narzędzia raportowania klinicznego** – automatycznie generowane raporty PDF z interpretacją wyników, wykresami trendów i rekomendacjami terapeutycznymi, gotowe do dołączenia do dokumentacji medycznej.

3. **Procedura wdrożeniowa "turn-key"** – kompletna dokumentacja umożliwiająca wdrożenie systemu w dowolnej placówce fizjoterapeutycznej lub sportowej bez konieczności angażowania zespołu programistów czy inżynierów.

---

## 6. Implikacje Praktyczne i Perspektywy Rozwoju

### 6.1. Zastosowania Kliniczne

Opracowany system znajduje bezpośrednie zastosowanie w:

- **Rehabilitacji ortopedycznej** – monitorowanie postępów po rekonstrukcjach więzadłowych (ACL, PCL), naprawach stożka rotatorów, endoprotezoplastykach stawów biodrowych i kolanowych, leczeniu zachowawczym dyskopatii.

- **Rehabilitacji neurologicznej** – trening siłowy i koordynacyjny u pacjentów po udarach mózgu, z chorobą Parkinsona, stwardnieniem rozsianym, mózgowym porażeniem dziecięcym.

- **Geriatrycznej prewencji upadków** – ocena i trening siły kończyn dolnych, poprawa propriocepcji i czasu reakcji u osób starszych.

- **Pediatrycznej terapii wad postawy** – biofeedbackowe ćwiczenia korekcyjne dla dzieci i młodzieży z skoliozami, wadami ustawienia kolan i stóp.

### 6.2. Zastosowania w Sporcie

- **Diagnostyka wydolnościowa** – bazowe testy siły i wytrzymałości mięśniowej dla sportowców różnych dyscyplin.

- **Monitoring obciążeń treningowych** – rejestracja objętości i intensywności treningu siłowego w celu optymalizacji periodyzacji i prewencji przetrenowania.

- **Return-to-sport testing** – obiektywne kryteria powrotu do pełnego treningu i rywalizacji po kontuzjach, oparte na symetrii siłowej i funkcjonalnej.

- **Talent identification** – wczesne wykrywanie predyspozycji siłowych i koordynacyjnych u młodych adeptów sportu.

### 6.3. Zastosowania Edukacyjne

- **Wychowanie fizyczne w szkołach** – monitorowanie postępów uczniów, motywacja poprzez grywalizację, wykrywanie wczesnych oznak wad postawy.

- **Kształcenie studentów fizjoterapii** – narzędzie dydaktyczne ilustrujące zasady biomechaniki, fizjologii wysiłku i technik pomiarowych.

- **Szkolenia trenerów personalnych** – edukacja w zakresie obiektywnej oceny postępów klientów i dawkowania ćwiczeń.

### 6.4. Kierunki Dalszych Badań

Przewidywane kierunki rozwoju systemu obejmują:

1. **Miniaturyzacja i wearable technology** – przeniesienie funkcjonalności na platformy noszone (smartwatche, opaski na kończyny) z komunikacją Bluetooth Low Energy.

2. **Sztuczna inteligencja i uczenie maszynowe** – algorytmy predykcyjne wykrywające wzorce ruchu zwiększające ryzyko kontuzji, systemy rekomendacji personalizujących programy treningowe.

3. **Tele-rehabilitacja i monitoring zdalny** – integracja z platformami telemedycznymi umożliwiającymi terapię pod nadzorem zdalnym, szczególnie istotna w sytuacjach pandemii lub dla pacjentów z ograniczoną mobilnością.

4. **Rozszerzona rzeczywistość (AR)** – nakładanie wizualizacji siły i trajektorii ruchu na obraz świata rzeczywistego poprzez okulary AR lub aplikacje mobilne.

5. **Badania wieloośrodkowe RCT** – randomizowane badania kontrolowane z dużymi grupami pacjentów w celu uzyskania dowodów skuteczności klinicznej poziomu I według hierarchii EBM.

6. **Integracja z multimodalnymi sensorami** – dodanie akcelerometrów, żyroskopów, EMG powierzchniowego i NIRS w celu uzyskania pełniejszego obrazu funkcji neuromięśniowej.

---

## 7. Podsumowanie Wstępu

Przedstawiona w niniejszej pracy habilitacyjnej problematyka dotyka kluczowego wyzwania współczesnej rehabilitacji i nauk o sporcie – potrzeby obiektywizacji, standaryzacji i personalizacji procesów treningu terapeutycznego. Opracowany autorski system pomiarowo-treningowy z pętlą sprzężenia zwrotnego stanowi odpowiedź na to wyzwanie, łącząc zaawansowane rozwiązania inżynierskie z praktycznymi potrzebami klinicystów i trenerów.

Oryginalność osiągnięcia projektowego przejawia się w holistycznym podejściu do problemu – od fundamentalnych badań nad właściwościami metrologicznymi czujników tensometrycznych w warunkach dynamicznych, poprzez inżynierską implementację systemu embedded, aż po finalne wdrożenie z pełną dokumentacją legalizacyjną i protokołami临床应用. Dobrowolne poddanie systemu rygorom właściwym dla wyrobów medycznych klasy IIb, mimo braku takiego obowiązku prawnego, świadczy o odpowiedzialności autora za bezpieczeństwo użytkowników i wiarygodność generowanych danych.

Struktura pracy odzwierciedla interdyscyplinarny charakter podjętych badań, angażujących wiedzę z zakresu inżynierii biomedycznej, elektroniki, informatyki, metrologii, biomechaniki, fizjoterapii i nauk o sporcie. Każdy z siedmiu rozdziałów merytorycznych wnosi odrębny, uzupełniający się wkład w całościowy obraz osiągnięcia, tworząc spójną narrację od koncepcji do wdrożenia.

W kolejnych rozdziałach niniejszej rozprawy szczegółowo omówiono każdy z wymienionych aspektów, dostarczając zarówno teoretycznych podstaw, jak i praktycznych implementacji, wyników walidacji i studium przypadków zastosowań. Praca ma ambicję stanowić nie tylko dokumentację osiągnięcia habilitacyjnego, ale także praktyczny przewodnik dla badaczy i praktyków zainteresowanych wdrożeniem podobnych rozwiązań w swojej działalności naukowo-badawczej lub klinicznej.

Ostatecznym celem autora jest przyczynienie się do podniesienia standardów opieki rehabilitacyjnej i treningowej poprzez dostarczenie narzędzia umożliwiającego oparte na dowodach, ilościowe monitorowanie postępów funkcjonalnych pacjentów i sportowców. W erze medycyny spersonalizowanej i data-driven healthcare, tego typu rozwiązania stają się nieodzownym elementem nowoczesnego warsztatu terapeuty i trenera.

---

*Bibliografia wstępu obejmuje kluczowe pozycje z zakresu:*
- *Ustawodawstwo polskie i europejskie dotyczące wyrobów medycznych i metrologii*
- *Podręczniki inżynierii biomedycznej i systemów embedded*
- *Publikacje z biomechaniki i fizjologii wysiłku*
- *Standardy metrologiczne (GUM, ISO/IEC 17025)*
- *Wytyczne dobrej praktyki klinicznej i badania RCT w rehabilitacji*
- *Literatura z zakresu interfejsów człowiek-komputer i gamifikacji w zdrowiu*

*Pełny wykaz参考文献 zostanie przedstawiony w końcowej części pracy.*
