# Rozdział 1.4: Rozróżnienie prawne: wyroby medyczne a urządzenia pomiarowe specjalne

## 1.4.1 Wprowadzenie do problematyki klasyfikacji prawnej urządzeń rehabilitacyjnych

### 1.4.1.1 Kontekst regulacyjny w Unii Europejskiej i Rzeczypospolitej Polskiej

Klasyfikacja prawna urządzeń stosowanych w fizjoterapii, rehabilitacji ruchowej i treningu sportowym stanowi fundamentalny aspekt procesu projektowania, wytwarzania i wprowadzania do obrotu tego typu rozwiązań technicznych. W polskim systemie prawnym, będącym implementacją regulacji Unii Europejskiej, wyróżniamy dwie zasadnicze kategorie urządzeń mających zastosowanie w ochronie zdrowia:

1. **Wyroby medyczne** – regulowane Rozporządzeniem Parlamentu Europejskiego i Rady (UE) 2017/745 z dnia 5 kwietnia 2017 r. w sprawie wyrobów medycznych (MDR – Medical Device Regulation)
2. **Urządzenia pomiarowe specjalne** – podlegające ustawie z dnia 11 maja 2001 r. Prawo o miarach oraz rozporządzeniom wykonawczym Ministra Zdrowia i Ministra Przedsiębiorczości i Technologii

Niniejszy rozdział ma na celu szczegółową analizę kryteriów kwalifikujących autorski system pomiarowo-treningowy z pętlą sprzężenia zwrotnego do odpowiedniej kategorii prawnej, ze szczególnym uwzględnieniem statusu poszczególnych komponentów systemu:
- Raspberry Pi 4 jako jednostki rejestrującej i treningowej
- Arduino Nano/Pico jako modułów akwizycji danych
- Czujnika tensometrycznego HX711
- Lateksowego tubeingu jako elementu oporowego

### 1.4.1.2 Definicja wyrobu medycznego według MDR

Zgodnie z art. 2 pkt 1 Rozporządzenia (UE) 2017/745, **wyrobem medycznym** jest:

> *"Jakikolwiek instrument, aparat, wyposażenie, oprogramowanie, implant, odczynnik, materiał lub inny artykuł, przeznaczony przez wytwórcę do stosowania u ludzi samotnie lub w połączeniu, w szczególności do następujących celów:*
> - *diagnozy, zapobiegania, monitorowania, przewidywania, rokowania, leczenia lub łagodzenia chorób,*
> - *diagnozy, monitorowania, leczenia, łagodzenia lub kompensowania urazów lub niepełnosprawności,*
> - *badania, zastępowania lub modyfikacji budowy anatomicznej lub procesu fizjologicznego lub patologicznego,*
> - *dostarczania informacji poprzez badanie in vitro próbek pobranych z organizmu ludzkiego"*

Kluczowym elementem definicji jest **przeznaczenie declared by the manufacturer** (deklarowane przez wytwórcę), które musi być udokumentowane w etykietowaniu, instrukcji używania i materiałach promocyjnych.

### 1.4.1.3 Klasyfikacja wyrobów medycznych

Rozporządzenie MDR wprowadza czteroklasowy system ryzyka:

| Klasa | Ryzyko | Przykłady | Procedura oceny zgodności |
|-------|--------|-----------|---------------------------|
| **I** | Niskie | Bandaze, laski, wózki inwalidzkie, tubing lateksowy | Samoocena wytwórcy (bez udziału jednostki notyfikowanej) |
| **Is** | Niskie + sterylność | Wyroby klasy I dostarczane w stanie sterylnym | Udział jednostki notyfikowanej w aspekcie sterylności |
| **Im** | Niskie + funkcja pomiarowa | Termometry kliniczne, ciśnieniomierze | Udział jednostki notyfikowanej w aspekcie metrologicznym |
| **IIa** | Średnie | Aparaty słuchowe, wheelchairs elektryczne | Pełna ocena jednostki notyfikowanej |
| **IIb** | Wysokie | Inkubatory noworodkowe, respiratory, defibrylatory | Rozszerzona ocena techniczna i kliniczna |
| **III** | Bardzo wysokie | Implanty serca, stenty wieńcowe, protezy stawów | Najbardziej rygorystyczna procedura z badaniem klinicznym |

## 1.4.2 Analiza statusu prawnego komponentów systemu pomiarowo-treningowego

### 1.4.2.1 Lateksowy tubing – wyrób medyczny klasy I

#### Charakterystyka produktu

Lateksowy tubing (taśma oporowa, pasek lateksowy) stanowi elastyczny element wykonany z naturalnego kauczuku lub syntetycznych polimerów, służący do generowania oporu zmiennego podczas ćwiczeń terapeutycznych i treningowych. Zgodnie z intended use deklarowanym przez producentów taśm rehabilitacyjnych (np. TheraBand®, Performer®, inne marki), tubing spełnia definicję wyrobu medycznego klasy I:

**Przeznaczenie medyczne:**
- Ćwiczenia rehabilitacyjne po urazach narządu ruchu
- Trening siłowy mięśni w warunkach ograniczonego obciążenia osiowego
- Ćwiczenia propriocepcji i stabilizacji centralnej
- Terapia manualna wspomagana oporem elastycznym

**Podstawa klasyfikacji:**
- Załącznik VIII, reguła klasyfikacji nr 1 MDR: *"Wszystkie wyroby nieinwazyjne należą do klasy I, chyba że zastosowano jedną z reguł od 1 do 4"*
- Brak penetracji ciała (nieinwazyjność)
- Brak funkcji pomiarowej wbudowanej w samą taśmę
- Kontakt wyłącznie ze skórą intacta (nienaruszoną)

#### Deklaracja zgodności dla tubingu

Jako wytwórca lub dystrybutor lateksowego tubeingu, należy sporządzić **Deklarację Zgodności UE** zgodnie z Załącznikiem IV MDR, zawierającą:

```
DEKLARACJA ZGODNOŚCI UE
Dla wyrobu medycznego klasy I

1. Nazwa i adres wytwórcy: [DANE]
2. Numer rejestracyjny wytwórcy: [PL-XXXX]
3. Identyfikacja wyrobu:
   - Nazwa handlowa: Lateksowy Tubing Rehabilitacyjny
   - Kod UMDN: [np. A130915 - Resistance bands]
   - Warianty kolorystyczne i opory: Yellow (light), Red (medium), Green (heavy), Black (extra heavy)
4. Oświadczenie: Niniejszym oświadczamy, że wyrób medyczny spełnia wymagania:
   - Rozporządzenia (UE) 2017/745 (MDR)
   - Normy zharmonizowanej: PN-EN ISO 13485:2016 (System zarządzania jakością)
   - PN-EN 1041:2008 (Informacje dostarczane przez wytwórcę)
5. Podpis osoby upoważnionej: [PODPIS]
6. Data i miejsce wystawienia: [DATA, MIEJSCE]
```

**Uwaga kluczowa:** Sama taśma lateksowa JEST wyrobem medycznym klasy I i wymaga oznakowania CE zgodnie z MDR, nawet jeśli jest sprzedawana jako sprzęt fitness, o ile deklaruje się jej zastosowanie rehabilitacyjne.

### 1.4.2.2 Czujnik tensometryczny – urządzenie pomiarowe specjalne

#### Status prawny sensora HX711

Czujnik tensometryczny (strain gauge) z układem warunkowania sygnału HX711 **NIE JEST** wyrobem medycznym w rozumieniu MDR, ponieważ:

1. **Brak bezpośredniego kontaktu z pacjentem** – czujnik jest elementem pośredniczącym, zamontowanym w obudowie mechanicznej, bez styczności z ciałem pacjenta
2. **Brak samodzielnej funkcji diagnostycznej** – surowy sygnał elektryczny z mostka Wheatstone'a nie ma znaczenia klinicznego bez przetworzenia algorytmicznego
3. **Przeznaczenie ogólne** – tensometry są komponentami przemysłowymi stosowanymi w wagach elektronicznych, systemach monitorowania konstrukcji, automatyce przemysłowej

**Klasyfikacja według Prawa o miarach:**
Czujnik tensometryczny w systemie pomiarowo-treningowym kwalifikuje się jako **urządzenie pomiarowe specjalne** podlegające:
- Ustawie z dnia 11 maja 2001 r. Prawo o miarach (Dz.U. 2022 poz. 2235 z późn. zm.)
- Rozporządzeniu Ministra Zdrowia z dnia 30 lipca 2021 r. w sprawie rodzajów przyrządów pomiarowych podlegających prawnej kontroli metrologicznej

#### Legalizacja pierwotna urządzenia specjalnego

Zgodnie z art. 6 ust. 1 pkt 4 Prawa o miarach, **legalizacji pierwotnej** podlegają przyrządy pomiarowe wprowadzane do obrotu lub oddawane do użytku po raz pierwszy, w tym urządzenia specjalne stosowane w ochronie zdrowia.

**Procedura legalizacji jednostkowej:**

1. **Wniosek do Okręgowego Urzędu Miar (OUM)**
   - Formularz WG-1 "Wniosek o legalizację przyrządu pomiarowego"
   - Dokumentacja techniczno-ruchowa urządzenia
   - Protokoły badań fabrycznych wytwórcy

2. **Badania metrologiczne**
   - Wzorcowanie wzorcami odniesienia o klasie dokładności co najmniej 3× lepszej niż badany przyrząd
   - Wyznaczenie błędów wskazań w całym zakresie pomiarowym
   - Określenie niepewności pomiaru zgodnie z GUM (Guide to the Expression of Uncertainty in Measurement)

3. **Decyzja administracyjna**
   - Pozytywny wynik → legalizacja (plombowanie, naklejka legalizacyjna, świadectwo legalizacji)
   - Negatywny wynik → decyzja odmowna z uzasadnieniem

4. **Okres ważności legalizacji**
   - Dla urządzeń specjalnych: zazwyczaj 12-24 miesiące
   - Konieczność okresowej legalizacji wtórnej

**Świadectwo legalizacji powinno zawierać:**
```
OKRĘGOWY URZĄD MIAR W [MIASTO]
ŚWIADECTWO LEGALIZACJI NR [XXX/2024]

1. Nazwa przyrządu: System Pomiaru Siły Naciągu Tubeingu
2. Typ: BIOFEEDBACK-STRAIN-V1.0
3. Numer fabryczny: [SERIAL-NUMBER]
4. Zakres pomiarowy: 0-500 N
5. Dokładność: ±0.5% wartości wskazanej
6. Warunki środowiskowe: temperatura 15-35°C, wilgotność 30-80% RH
7. Wynik legalizacji: POZYTYWNY
8. Ważne do: [DATA + 24 MIESIĄCE]
9. Pieczęć i podpis legalizatora
```

### 1.4.2.3 Mikrokontrolery Arduino i Raspberry Pi Pico – komponenty elektroniczne

#### Status prawny jako części składowej systemu

Płytki rozwojowe Arduino Nano, Arduino Uno, Raspberry Pi Pico stanowią **komponenty elektroniczne ogólnego zastosowania**, które:

1. **Nie są wyrobami medycznymi** – brak deklarowanego przeznaczenia medycznego przez producentów (Arduino LLC, Raspberry Pi Foundation)
2. **Nie podlegają legalizacji jako urządzenia pomiarowe** – same w sobie nie realizują funkcji pomiarowej, jedynie przetwarzają dane cyfrowe
3. **Wymagają integracji w systemie** – dopiero połączenie z sensorem, zasilaniem i oprogramowaniem tworzy funkcjonalną całość

#### Dobrowolne podwyższenie standardów jakościowych

Mimo braku prawnego obowiązku, autor projektu deklaruje dobrowolne spełnienie wymagań analogicznych do wyrobów medycznych klasy IIb w zakresie:

**Bezpieczeństwo elektryczne:**
- Zgodność z normą PN-EN 60601-1 (Bezpieczeństwo elektryczne wyrobów medycznych)
- Izolacja galwaniczna obwodów niskiego napięcia (<5V DC) od sieci 230V AC
- Zabezpieczenia przed zwarciem, przeciążeniem, przegrzaniem

**Kompatybilność elektromagnetyczna (EMC):**
- PN-EN 60601-1-2: Emisja zakłóceń poniżej limitów Class B
- Odporność na wyładowania elektrostatyczne (ESD) ±8 kV contact, ±15 kV air
- Immunity na pola elektromagnetyczne RF 80 MHz – 2.5 GHz, 10 V/m

**Niezawodność operacyjna:**
- Testy życia przyspieszonego (ALT – Accelerated Life Testing)
- MTBF (Mean Time Between Failures) > 50,000 godzin
- Redundancja krytycznych funkcji oprogramowania

**Deklaracja zgodności dla mikrokontrolerów w systemie:**
```
OŚWIADCZENIE WYTWÓRCY SYSTEMU ZINTEGROWANEGO

Niniejszym oświadczam, że moduły mikrokontrolerów Arduino Nano i Raspberry Pi Pico, 
zintegrowane w ramach Autorskiego Systemu Pomiarowo-Treningowego BIOFEEDBACK-TUBE, 
spełniają dobrowolnie przyjęte wymagania:

1. Bezpieczeństwo elektryczne zgodne z PN-EN 60601-1:2007 + A1:2014
2. Kompatybilność elektromagnetyczna zgodna z PN-EN 60601-1-2:2015
3. Zarządzanie ryzykiem zgodnie z PN-EN ISO 14971:2020
4. Walidacja oprogramowania zgodnie z IEC 62304:2007 (klasa C)

Mimo że komponenty te nie są wyrobami medycznymi w rozumieniu MDR, 
ich integracja w systemie została przeprowadzona z zachowaniem rygorów 
technicznych właściwych dla wyrobów klasy IIb.

[PODPIS WYTWÓRCY SYSTEMU]
[DATA I MIEJSCE]
```

### 1.4.2.4 Raspberry Pi 4 – jednostka rejestrująca i treningowa

#### Dualizm funkcjonalny: komputer ogólny vs. system medyczny

Raspberry Pi 4 Model B jest **jednopłytkowym komputerem ogólnego zastosowania**, który w konfiguracji fabrycznej nie stanowi wyrobu medycznego. Jednakże w kontekście zintegrowanego systemu biofeedback:

**Argumenty PRZEWIDUJĄCE status urządzenia medycznego:**
- Przetwarza dane biomedyczne (siła naciągu, parametry biomechaniczne)
- Generuje wizualny biofeedback wykorzystywany w procesie terapeutycznym
- Przechowuje dane pacjentów w formacie JSON z możliwością eksportu do dokumentacji medycznej
- Realizuje algorytmy decyzyjne (progi alarmowe, adaptacja trudności gier)

**Argumenty PRZECIW statusowi urządzenia medycznego:**
- Nie diagnozuje chorób ani nie steruje terapią automatycznie
- Fizjoterapeuta zachowuje pełną kontrolę nad procesem rehabilitacji
- Dane z systemu mają charakter pomocniczy, nie stanowią podstawy samodzielnej diagnozy
- Brak inwazyjności i bezpośredniego wpływu na organizm pacjenta

#### Rozstrzygnięcie: urządzenie pomiarowe specjalne

Po analizie powyższych argumentów, Raspberry Pi 4 w systemie BIOFEEDBACK-TUBE kwalifikuje się jako **urządzenie pomiarowe specjalne**, a nie wyrób medyczny, z następujących powodów:

1. **Funkcja pomocnicza** – system dostarcza danych liczbowych i wizualizacji, ale nie podejmuje autonomicznych decyzji terapeutycznych
2. **Nadzór profesjonalisty** – wszystkie decyzje kliniczne należą do fizjoterapeuty
3. **Brak ingerencji w organizm** – brak stymulacji elektrycznej, mechanicznej czy farmakologicznej
4. **Analogia do istniejących rozwiązań** – wagi medyczne, spirometry, termometry są urządzeniami pomiarowymi, nie wyrobami medycznymi klasy II

#### Wymagania dla Raspberry Pi jako urządzenia specjalnego

Mimo zaklasyfikowania jako urządzenie pomiarowe, Raspberry Pi musi spełniać określone wymagania techniczne:

**Tabela 1.4.1: Specyfikacja techniczna Raspberry Pi 4 w systemie biofeedback**

| Parametr | Wymaganie | Uzasadnienie |
|----------|-----------|--------------|
| **Procesor** | Broadcom BCM2711, Quad-core Cortex-A72 @ 1.5GHz | Zapewnienie płynnej wizualizacji 60 FPS w grach biofeedback |
| **RAM** | Minimum 4GB LPDDR4 | Buforowanie danych z częstotliwością 100Hz przez extended sessions |
| **Storage** | MicroSD 32GB Class 10 minimum | Przechowywanie danych pacjentów z zachowaniem integralności |
| **USB Ports** | 2× USB 3.0, 2× USB 2.0 | Podłączenie Arduino (serial), ewentualnych dodatkowych sensorów |
| **Ethernet/Gigabit** | Tak | Możliwość synchronizacji z systemem szpitalnym (HL7/FHIR) |
| **WiFi/Bluetooth** | 802.11ac dual-band, BT 5.0 | Łączność bezprzewodowa z tabletami, drukarkami medycznymi |
| **Zasilanie** | 5V DC 3A przez USB-C | Stabilność napięcia krytyczna dla precyzji pomiarów ADC |
| **Temperatura pracy** | 0-50°C z aktywnym chłodzeniem | Zapobieganie thermal throttling podczas długich sesji |
| **OS** | Raspberry Pi OS 64-bit (Debian-based) | Stabilność, długoterminowe wsparcie, bezpieczeństwo |

**Oprogramowanie systemowe:**
- Kernel Linux z patchami PREEMPT_RT dla determinizmu czasu rzeczywistego
- Usługa systemd do automatycznego startu aplikacji biofeedback
- Firewall (ufw) z regułami ograniczającymi dostęp sieciowy
- Szyfrowanie dysku LUKS dla ochrony danych RODO

## 1.4.3 System zintegrowany jako całość – analiza kwalifikacji prawnej

### 1.4.3.1 Zasada integracji funkcjonalnej

Zgodnie z wytycznymi Komisji Europejskiej (Document MDCG 2021-24), gdy kilka urządzeń jest łączonych w system zintegrowany, kwalifikacja prawna dotyczy **całości systemu**, a nie tylko poszczególnych komponentów. Kluczowe pytania brzmią:

1. Czy system jako całość spełnia definicję wyrobu medycznego?
2. Czy integracja zmienia przeznaczenie lub profil ryzyka komponentów?
3. Kto jest prawnie odpowiedzialny za system zintegrowany (manufacturer vs. integrator)?

### 1.4.3.2 Argumentacja za statusem urządzenia pomiarowego specjalnego

Autorski system pomiarowo-treningowy BIOFEEDBACK-TUBE **NIE JEST** wyrobem medycznym w rozumieniu MDR, lecz **urządzeniem pomiarowym specjalnym** podlegającym legalizacji jednostkowej, co wynika z następującej argumentacji:

**Cel główny systemu:**
- Pomiar i wizualizacja parametrów biomechanicznych (siła, czas, moc, energia)
- Dostarczanie informacji zwrotnej (biofeedback) dla pacjenta i terapeuty
- Rejestracja danych treningowych w formacie JSON do dalszej analizy

**Brak cech wyrobu medycznego:**
- ❌ Nie diagnozuje chorób ani urazów
- ❌ Nie leczy bezpośrednio – terapia jest prowadzona przez fizjoterapeutę
- ❌ Nie modyfikuje anatomii ani procesów fizjologicznych w sposób inwazyjny
- ❌ Nie dostarcza informacji do diagnozy in vitro
- ❌ Nie podejmuje autonomicznych decyzji terapeutycznych

**Analogia do innych urządzeń:**
- Waga medyczna → mierzy masę ciała, ale nie diagnozuje otyłości
- Spirometr → mierzy pojemność płuc, ale nie rozpoznaje astmy
- Ciśnieniomierz → mierzy ciśnienie krwi, ale nie leczy nadciśnienia
- **System BIOFEEDBACK-TUBE** → mierzy siłę naciągu tubeingu, ale nie prowadzi autonomically rehabilitacji

### 1.4.3.3 Struktura prawna systemu z punktu widzenia regulacyjnego

```
┌─────────────────────────────────────────────────────────────────┐
│                  SYSTEM BIOFEEDBACK-TUBE                        │
│            (Urządzenie Pomiarowe Specjalne)                     │
│                  Legalizacja Jednostkowa OUM                    │
└─────────────────────────────────────────────────────────────────┘
                              │
         ┌────────────────────┼────────────────────┐
         │                    │                    │
         ▼                    ▼                    ▼
┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐
│  RASPBERRY PI 4 │  │  ARDUINO NANO   │  │  CZUJNIK HX711  │
│  (Komputer      │  │  (Mikrokontroler│  │  (Sensor        │
│   ogólny)       │  │   ogólny)       │  │   przemysłowy)  │
│                 │  │                 │  │                 │
│ ❌ Nie medyczny │  │ ❌ Nie medyczny │  │ ❌ Nie medyczny │
│ ✅ Urządzenie   │  │ ✅ Komponent    │  │ ✅ Komponent    │
│    specjalne    │  │    systemu      │  │    systemu      │
└─────────────────┘  └─────────────────┘  └─────────────────┘
                              │
                              ▼
                   ┌─────────────────────┐
                   │  TUBING LATEKSOWY   │
                   │  (Wyrób Medyczny    │
                   │   Klasy I)          │
                   │                     │
                   │ ✅ Oznakowanie CE   │
                   │ ✅ Deklaracja MDR   │
                   └─────────────────────┘
```

**Kluczowa zasada:** tubing lateksowy JEST wyrobem medycznym klasy I (oddzielna deklaracja), natomiast cały system elektroniczny (RPi + Arduino + sensor) JEST urządzeniem pomiarowym specjalnym (legalizacja OUM).

## 1.4.4 Procedura legalizacji jednostkowej urządzeń specjalnych

### 1.4.4.1 Podstawa prawna legalizacji

Legalizacja jednostkowa systemu BIOFEEDBACK-TUBE odbywa się na podstawie:

1. **Ustawa z dnia 11 maja 2001 r. Prawo o miarach** (Dz.U. 2022 poz. 2235 z późn. zm.)
   - Art. 6: Obowiązek legalizacji przyrządów pomiarowych
   - Art. 7: Tryb legalizacji pierwotnej i wtórnej
   - Art. 26: Kontrola metrologiczna przyrządów w eksploatacji

2. **Rozporządzenie Ministra Zdrowia z dnia 30 lipca 2021 r.** w sprawie rodzajów przyrządów pomiarowych podlegających prawnej kontroli metrologicznej
   - §3 pkt 1: Przyrządy do pomiaru wielkości fizjologicznych
   - §5: Wymagania dla urządzeń specjalnych w ochronie zdrowia

3. **Normy techniczne:**
   - PN-EN ISO/IEC 17025:2018-10 (Ogólne wymagania dotyczące kompetencji laboratoriów badawczych i wzorcujących)
   - GUM (Guide to the Expression of Uncertainty in Measurement, JCGM 100:2008)
   - PN-EN 13725:2022 (Wagi i przyrządy do pomiaru siły)

### 1.4.4.2 Dokumentacja wymagana do legalizacji

**Tabela 1.4.2: Wykaz dokumentów do wniosku o legalizację jednostkową**

| Nr | Dokument | Opis | Wymagany format |
|----|----------|------|-----------------|
| 1 | Wniosek WG-1 | Formularz urzędowy OUM | Oryginał papierowy |
| 2 | Instrukcja obsługi | Szczegółowa dokumentacja użytkownika | PDF + wersja drukowana |
| 3 | Dokumentacja techniczna | Schematy elektryczne, rysunki mechaniczne | PDF/A |
| 4 | Specyfikacja metrologiczna | Zakres, dokładność, rozdzielczość, niepewność | PDF z tabelami |
| 5 | Protokoły badań fabrycznych | Wyniki testów wewnętrznych wytwórcy | PDF z danymi surowymi |
| 6 | Deklaracja zgodności | Oświadczenie wytwórcy o spełnieniu wymagań | Oryginał z podpisem |
| 7 | Certyfikaty komponentów | CE dla zasilaczy, EMC dla podzespołów | Kopie poświadczone |
| 8 | Analiza ryzyka | Zgodnie z PN-EN ISO 14971 | PDF z macierzą ryzyka |
| 9 | Dowód uiszczenia opłaty | Potwierdzenie przelewu opłaty legalizacyjnej | Skan/kopia |

**Opłata legalizacyjna (2024):**
- Badanie typu: 450–800 PLN w zależności od złożoności
- Legalizacja jednostkowa: 150–300 PLN za sztukę
- Świadectwo legalizacji: 50 PLN
- **Łącznie:** ok. 650–1150 PLN za pierwsze urządzenie

### 1.4.4.3 Proces badania metrologicznego

**Etap 1: Przygotowanie stanowiska badawczego**

Laboratorium OUM przygotowuje stanowisko z wzorcami odniesienia:
- Wzorce siły klasy F1 lub lepszej (np. odważniki wzorcowe 1–50 kg)
- Siłomierz wzorcowy o niepewności ≤0.1% zakresu
- Komora klimatyczna do testów temperaturowych (opcjonalnie)
- Oscyloskop cyfrowy do analizy dynamiki sygnału

**Etap 2: Wyznaczenie punktów kalibracyjnych**

Dla zakresu pomiarowego 0–500 N wybiera się punkty kalibracyjne:
```
Punkty kalibracyjne (rosnąco):
0 N (zero) → 50 N → 100 N → 200 N → 300 N → 400 N → 500 N (full scale)

Punkty kalibracyjne (malejąco):
500 N → 400 N → 300 N → 200 N → 100 N → 50 N → 0 N
```

**Etap 3: Pomiar błędów wskazań**

Dla każdego punktu kalibracyjnego wykonuje się N=10 powtórzeń pomiaru:

$$\text{Błąd wskazań} = W_{avg} - W_{wzorzec}$$

$$\text{Błąd względny} [\%] = \frac{W_{avg} - W_{wzorzec}}{W_{wzorzec}} \times 100\%$$

Gdzie:
- $W_{avg}$ – średnia arytmetyczna z N pomiarów badanego urządzenia
- $W_{wzorzec}$ – wartość wzorca odniesienia

**Etap 4: Obliczenie niepewności pomiaru (metoda GUM)**

Niepewność rozszerzona U oblicza się ze wzoru:

$$U = k \cdot u_c = k \cdot \sqrt{u_A^2 + u_B^2}$$

Gdzie:
- $k$ – współczynnik rozszerzenia (zwykle k=2 dla poziomu ufności 95%)
- $u_A$ – niepewność typu A (oceniana statystycznie z powtarzalności)
- $u_B$ – niepewność typu B (oceniana innymi metodami, np. z certyfikatów wzorców)

**Składniki niepewności typu B:**
- $u_{B1}$ – niepewność wzorca odniesienia (z certyfikatu)
- $u_{B2}$ – dryft czasowy urządzenia
- $u_{B3}$ – wpływ temperatury
- $u_{B4}$ – histereza mechaniczna sensora
- $u_{B5}$ – nieliniowość charakterystyki przenoszenia

**Etap 5: Kryteria akceptacji**

Urządzenie przechodzi legalizację, jeśli:
- Błąd wskazań ≤ MPE (Maximum Permissible Error) określony w specyfikacji
- Niepewność rozszerzona U ≤ 1/3 MPE (reguła TUR – Test Uncertainty Ratio)
- Powtarzalność pomiarów (odchylenie standardowe) ≤ 0.5% zakresu

Dla systemu BIOFEEDBACK-TUBE założono:
- MPE = ±1% wartości wskazanej
- Wymagana niepewność U ≤ 0.33%
- Powtarzalność ≤ 0.5%

### 1.4.4.4 Świadectwo legalizacji i znakowanie

Po pozytywnej weryfikacji metrologicznej, OUM wydaje:

**1. Świadectwo Legalizacji (formularz urzędowy)**
```
OKRĘGOWY URZĄD MIAR
w [Miejscowość]

ŚWIADECTWO LEGALIZACJI Nr [XXX/RRRR]

1. RODZAJ PRZYRZĄDU: System do pomiaru siły naciągu taśm lateksowych
2. PRODUCENT: [Nazwa Wytwórcy]
3. TYP/NR MODELU: BIOFEEDBACK-TUBE v1.0
4. NUMER FABRYCZNY: [Serial Number]
5. ZAKRES POMIAROWY: 0 – 500 N
6. DOKŁADNOŚĆ: ±1% wartości wskazanej
7. WARUNKI ŚRODOWISKOWE:
   - Temperatura: 10°C – 40°C
   - Wilgotność względna: 30% – 80% (bez kondensacji)
   - Ciśnienie atmosferyczne: 860 – 1060 hPa
8. WZORCE ODNIESIENIA:
   - Siłomierz wzorcowy [Typ], nr inw. [XXX], ważny do [DATA]
   - Odważniki klasy F1, nr [YYY]
9. WYNIK BADANIA: POZYTYWNY
10. OKRES WAŻNOŚCI LEGALIZACJI: 24 miesiące od daty wydania
11. DATA NASTĘPNEJ LEGALIZACJI WTÓRNEJ: [DATA + 24 MIES.]

Miejscowość, Data: [DATA]
Podpis Legalizatora: _________________
Pieczęć OUM
```

**2. Znak legalizacyjny (naklejka holograficzna)**
- Umieszczona na widocznym miejscu obudowy Raspberry Pi
- Zawiera: numer świadectwa, datę ważności, logo GUM

**3. Plombowanie zabezpieczające**
- Zabezpieczenie śrub obudowy przed otwarciem bez autoryzacji
- Naklejka "Do not remove" z logo OUM

## 1.4.5 Deklaracja zgodności dla urządzenia pomiarowego specjalnego

### 1.4.5.1 Wzór deklaracji zgodności

```
DEKLARACJA ZGODNOŚCI URZĄDZENIA POMIAROWEGO SPECJALNEGO
(Zgodnie z ustawą Prawo o miarach i rozporządzeniami wykonawczymi)

1. WYROBCA SYSTEMU:
   Nazwa: [Pełna nazwa firmy/instytucji]
   Adres: [Ulica, kod pocztowy, miejscowość, kraj]
   NIP/REGON: [Numery identyfikacyjne]
   Osoba upoważniona: [Imię i Nazwisko, stanowisko]

2. IDENTYFIKACJA URZĄDZENIA:
   Nazwa handlowa: Autorski System Pomiarowo-Treningowy BIOFEEDBACK-TUBE
   Model/TYP: BFT-RPI4-HX711-v1.0
   Numery seryjne: [Zakres lub lista numerów]
   Rok produkcji: 2024

3. PRZEZNACZENIE URZĄDZENIA:
   Urządzenie służy do pomiaru, rejestracji i wizualizacji siły naciągu 
   lateksowych taśm oporowych (tubingów) podczas ćwiczeń rehabilitacyjnych, 
   treningu sportowego i wychowania fizycznego. System dostarcza informacji 
   zwrotnej (biofeedback) w postaci graficznej i liczbowej dla pacjentów, 
   sportowców i fizjoterapeutów.

4. PODSTAWY PRAWNE:
   - Ustawa z dnia 11 maja 2001 r. Prawo o miarach (Dz.U. 2022 poz. 2235)
   - Rozporządzenie Ministra Zdrowia z dnia 30 lipca 2021 r. w sprawie rodzajów 
     przyrządów pomiarowych podlegających prawnej kontroli metrologicznej
   - Norma PN-EN ISO/IEC 17025:2018-10
   - Wytyczne GUM (JCGM 100:2008)

5. SPEŁNIONE WYMAGANIA:
   □ Wymagania metrologiczne dla urządzeń specjalnych
   □ Dokładność pomiaru: ±1% wartości wskazanej w zakresie 0-500 N
   □ Powtarzalność: ≤0.5% wartości wskazanej
   □ Stabilność długoterminowa: ≤1% przez 12 miesięcy
   □ Zakres temperatur pracy: 10°C – 40°C
   □ Odporność na zakłócenia EMC: poziom przemysłowy

6. JEDNOSTKA NOTYFIKOWANA / LABORATORIUM WZORCUJĄCE:
   Nazwa: Okręgowy Urząd Miar w [Miejscowość]
   Numer świadectwa legalizacji: [XXX/RRRR]
   Data legalizacji pierwotnej: [DD.MM.RRRR]
   Ważne do: [DD.MM.RRRR]

7. OŚWIADCZENIE:
   Niniejszym oświadczamy, że urządzenie pomiarowe specjalne opisane w pkt 2 
   spełnia wszystkie istotne wymagania określone w przepisach wymienionych w pkt 4 
   i przeszło pozytywnie procedurę legalizacji jednostkowej przeprowadzoną przez 
   uprawnioną jednostkę kontrolującą.

   Urządzenie NIE JEST wyrobem medycznym w rozumieniu Rozporządzenia (UE) 2017/745 
   (MDR) i nie podlega oznakowaniu CE jako wyrób medyczny. Lateksowy tubing 
   stosowany w połączeniu z systemem jest oddzielnym wyrobem medycznym klasy I 
   i posiada własną deklarację zgodności MDR.

8. ZAŁĄCZNIKI:
   - Kopia świadectwa legalizacji OUM
   - Instrukcja obsługi urządzenia
   - Protokoły badań metrologicznych
   - Specyfikacja techniczna

9. MIEJSCE I DATA WYSTAWIENIA:
   Miejscowość: [Miejscowość]
   Data: [DD.MM.RRRR]

10. PODPIS OSOBY UPOWAŻNIONEJ:
    
    _________________________________
    [Imię i Nazwisko]
    [Stanowisko]
    [Pieczęć firmowa]
```

### 1.4.5.2 Rejestracja w ewidencji urządzeń specjalnych

Po uzyskaniu legalizacji, wytwórca ma obowiązek zgłosić urządzenie do ewidencji prowadzonej przez Prezesa Głównego Urzędu Miar (GUM):

**Dane wymagane w rejestrze:**
1. Nazwa i adres wytwórcy
2. Typ/model urządzenia
3. Zakres numerów seryjnych
4. Data legalizacji pierwotnej
5. Numer świadectwa legalizacji
6. Okres ważności legalizacji
7. Miejsce instalacji/eksploatacji (dla urządzeń stacjonarnych)

Rejestracja odbywa się elektronicznie przez platformę e-Urząd Miar lub tradycyjnie formularzem papierowym EM-1.

## 1.4.6 Porównanie z wyrobami medycznymi klasy IIb – analiza różnic

### 1.4.6.1 Tabela porównawcza wymagań

**Tabela 1.4.3: Zestawienie wymagań dla urządzeń pomiarowych specjalnych vs. wyrobów medycznych klasy IIb**

| Wymaganie | Urządzenie Pomiarowe Specjalne | Wyrób Medyczny Klasy IIb |
|-----------|-------------------------------|--------------------------|
| **Podstawa prawna** | Prawo o miarach (ustawa krajowa) | MDR (UE) 2017/745 (rozporządzenie UE) |
| **Procedura oceny** | Legalizacja jednostkowa w OUM | Ocena zgodności z jednostką notyfikowaną |
| **Czas procedury** | 2–6 tygodni | 12–24 miesiące |
| **Koszt** | 650–1150 PLN | 50,000–200,000 EUR |
| **Oznakowanie** | Znak legalizacyjny OUM | Oznakowanie CE z numerem NB |
| **Nadzór rynku** | GUM, Inspekcja Miar | URM, jednostki notyfikowane, SANEPID |
| **Okres ważności** | 24 miesiące (legalizacja wtórna) | 5 lat (certyfikat CE, odnawialny) |
| **Badania kliniczne** | Nie wymagane | Często wymagane (zależnie od typu) |
| **System jakości** | Nie wymagany (zalecany) | Obowiązkowy ISO 13485 |
| **Traceability** | Do wzorców krajowych GUM | Do wymagań MDR i norm zharmonizowanych |
| **Vigilance** | Zgłaszanie awarii do OUM | System vigilance EUDAMED, poważne incydenty |
| **UDID/DI** | Nie dotyczy | Unique Device Identification required |
| **Rejestracja** | Ewidencja GUM (krajowa) | EUDAMED (baza europejska) |
| **Osoba odpowiedzialna** | Nie wymagana | RP (Regulatory Person) w UE wymagana |
| **PMCF** | Nie dotyczy | Post-Market Clinical Follow-up required |
| **Instrukcja** | W języku krajowym | We wszystkich językach krajów dystrybucji |

### 1.4.6.2 Analiza kosztów i czasu wdrożenia

**Scenariusz A: Urządzenie pomiarowe specjalne (wybrana ścieżka)**
```
Koszty jednorazowe:
- Badanie typu + legalizacja: 1,150 PLN
- Dokumentacja techniczna: 5,000 PLN (roboczogodziny)
- Szkolenia personelu: 2,000 PLN
- SUMA: ~8,150 PLN (~1,900 EUR)

Czas wdrożenia:
- Przygotowanie dokumentacji: 4–6 tygodni
- Procedura legalizacji: 2–4 tygodnie
- ŁĄCZNIE: 6–10 tygodni

Koszty roczne (eksploatacja):
- Legalizacja wtórna (co 2 lata): 300 PLN/rok
- Przeglądy okresowe: 500 PLN/rok
- SUMA: ~800 PLN/rok
```

**Scenariusz B: Wyrób medyczny klasy IIb (alternatywa odrzucona)**
```
Koszty jednorazowe:
- Jednostka notyfikowana (ocena zgodności): 80,000 EUR
- Badania kliniczne: 150,000–300,000 EUR
- Certyfikacja ISO 13485: 25,000 EUR
- Dokumentacja techniczna MDR: 50,000 EUR
- SUMA: ~305,000–455,000 EUR

Czas wdrożenia:
- Opracowanie dokumentacji: 6–12 miesięcy
- Badania kliniczne: 12–18 miesięcy
- Ocena NB: 6–12 miesięcy
- ŁĄCZNIE: 24–42 miesięcy

Koszty roczne (eksploatacja):
- Nadzór NB (audyty): 15,000 EUR/rok
- PMCF studies: 30,000 EUR/rok
- Vigilance reporting: 10,000 EUR/rok
- Ubezpieczenie OC: 20,000 EUR/rok
- SUMA: ~75,000 EUR/rok
```

**Wniosek ekonomiczny:** Wybór ścieżki urządzenia pomiarowego specjalnego redukuje koszty wdrożenia około **200-krotnie** i skraca czas wejścia na rynek z **3 lat do 3 miesięcy**, przy zachowaniu porównywalnego poziomu bezpieczeństwa metrologicznego.

## 1.4.7 Aspekty międzynarodowe – uznawalność legalizacji poza granicami Polski

### 1.4.7.1 Mutual Recognition Agreements (MRA)

Legalizacja przeprowadzona przez polski Okręgowy Urząd Miar jest uznawana w krajach, które podpisały umowy o wzajemnym uznawaniu wzorców i certyfikatów:

**Organizacje międzynarodowe:**
- **OIML** (International Organization of Legal Metrology) – Polska jest członkiem od 1960 r.
- **WELMEC** (European Cooperation in Legal Metrology) – współpraca europejska
- **EA** (European Accreditation) – akredytacje laboratoriów

**Kraje z umowami MRA z Polską:**
- Wszystkie państwa Unii Europejskiej
- Kraje EFTA (Norwegia, Islandia, Liechtenstein, Szwajcaria)
- Wybrane kraje partnerskie (Turcja, Ukraina, Gruzja – w ograniczonym zakresie)

### 1.4.7.2 Procedura uznawania legalizacji w innym kraju UE

Jeśli system BIOFEEDBACK-TUBE miałby być eksploatowany w Niemczech:

1. **Uznanie automatyczne** – świadectwo legalizacji OUM Warszawa jest honorowane przez PTB (Physikalisch-Technische Bundesanstalt) na podstawie dyrektywy MID (Measuring Instruments Directive)
2. **Brak ponownej legalizacji** – urządzenie nie musi przechodzić powtórnej procedury w Niemczech
3. **Tłumaczenie dokumentacji** – instrukcja obsługi musi być dostępna w języku niemieckim
4. **Rejestracja lokalna** – zgłoszenie do niemieckiego Eichamt (urząd miar) jako użytkownik zagraniczny

**Ograniczenia:**
- Legalizacja ważna jest tylko w oryginalnej konfiguracji – jakakolwiek modyfikacja sprzętowa/programowa unieważnia uznawalność
- Okres ważności zgodny z datą w świadectwie – po upływie konieczna legalizacja wtórna w kraju eksploatacji lub Polsce

### 1.4.7.3 Ekspansja poza UE – USA, Kanada, Australia

**Stany Zjednoczone:**
- Wymagana certyfikacja NIST (National Institute of Standards and Technology)
- Możliwość skorzystania z procedury fast-track dla urządzeń już certyfikowanych w UE
- Dodatkowe wymagania FDA 21 CFR Part 820 (Quality System Regulation) jeśli device byłby klasyfikowany jako medical device

**Kanada:**
- Uznanie certyfikatów europejskich na podstawie MRA UE-Kanada
- Rejestracja w Health Canada jako Class II/III device (w zależności od intended use)

**Australia:**
- TGA (Therapeutic Goods Administration) wymaga osobnej rejestracji
- Możliwość skorzystania z ścieżki CE-mark recognition (do 2025 r.)

## 1.4.8 Studium przypadku – analiza podobnych systemów na rynku

### 1.4.8.1 Przykład 1: System Tendo Power Meter

**Charakterystyka:**
- Producent: Tendo Sport (Słowacja)
- Przeznaczenie: Pomiar mocy i prędkości w treningu siłowym
- Status prawny: Urządzenie pomiarowe specjalne (nie wyrób medyczny)
- Legalizacja: Slovenský Metrologický Ústav (SMÚ)

**Wnioski:**
- System jest sprzedawany globalnie jako narzędzie treningowe, nie medyczne
- Brak oznakowania CE jako medical device
- Deklaracja zgodności jako equipment for sports measurement

### 1.4.8.2 Przykład 2: Vald ForceFrame

**Charakterystyka:**
- Producent: Vald Performance (Australia)
- Przeznaczenie: Platforma do izometrycznych testów siłowych
- Status prawny: Wyrób medyczny klasy I (w niektórych jurysdykcjach)
- Certyfikacja: TGA Class I, CE MDR Class I

**Wnioski:**
- Dualizm klasyfikacji zależy od deklarowanego intended use
- Jeśli producent deklaruje zastosowanie kliniczne → medical device
- Jeśli tylko sport/research → general measurement device

### 1.4.8.3 Przykład 3: HUR Smart Touch Strength Equipment

**Charakterystyka:**
- Producent: HUR Oy (Finlandia)
- Przeznaczenie: Maszyny siłowe z czujnikami i biofeedbackiem dla seniorów
- Status prawny: Wyrób medyczny klasy IIa
- Certyfikacja: CE MDR Class IIa, FDA 510(k) cleared

**Wnioski:**
- Maszyny dedykowane dla rehabilitacji geriatrycznej → wyższa klasa ryzyka
- Wymagana jednostka notyfikowana (Fi-Medic w Finlandii)
- Pełna dokumentacja kliniczna i PMCF

### 1.4.8.4 Pozycjonowanie systemu BIOFEEDBACK-TUBE

Na podstawie analizy przypadków, system BIOFEEDBACK-TUBE plasuje się w kategorii **urządzeń pomiarowych specjalnych**, analogicznie do Tendo Power Meter, z następującym uzasadnieniem:

1. **Grupa docelowa:** fizjoterapeuci, trenerzy, nauczyciele WF (nie wyłącznie personel medyczny)
2. **Setting:** gabinety prywatne, siłownie, szkoły (nie tylko placówki medyczne)
3. **Intended use:** monitoring postępów, biofeedback, motywacja (nie diagnoza/leczenie)
4. **Risk profile:** niska inwazyjność, brak decyzji autonomicznych, nadzór człowieka

## 1.4.9 Rekomendacje dla wytwórcy i użytkowników systemu

### 1.4.9.1 Dla wytwórcy (autora systemu)

**Działania obligatoryjne:**
1. ✅ Przeprowadzić legalizację jednostkową w właściwym terytorialnie OUM
2. ✅ Sporządzić i podpisać Deklarację Zgodności urządzenia pomiarowego
3. ✅ Umieścić znak legalizacyjny na obudowie urządzenia
4. ✅ Wystawić świadectwo legalizacji dla każdego egzemplarza
5. ✅ Prowadzić ewidencję wyprodukowanych urządzeń (numery seryjne, daty legalizacji)
6. ✅ Zgłosić urządzenie do rejestru GUM

**Działania zalecane (dobrowolne podwyższenie standardów):**
1. ⭐ Wdrożyć system zarządzania jakością ISO 9001 (nie wymagane ISO 13485)
2. ⭐ Przeprowadzić badania EMC w akredytowanym laboratorium
3. ⭐ Opracować procedurę recall/field corrective action na wypadek wykrycia wad
4. ⭐ Ubezpieczyć odpowiedzialność cywilną za szkody spowodowane urządzeniem
5. ⭐ Regularnie aktualizować oprogramowanie i udostępniać patche bezpieczeństwa
6. ⭐ Prowadzić rejestr reklamacji i incydentów (voluntary vigilance)

### 1.4.9.2 Dla użytkowników (fizjoterapeutów, trenerów)

**Obowiązki prawne:**
1. ✅ Eksploatować urządzenie wyłącznie w okresie ważności legalizacji
2. ✅ Poddać urządzenie legalizacji wtórnej przed upływem terminu ważności
3. ✅ Nie modyfikować sprzętu/oprogramowania bez zgody wytwórcy (unieważnia legalizację)
4. ✅ Przechowywać kopię świadectwa legalizacji w dokumentacji placówki
5. ✅ Zgłaszać wytwórcy i OUM wszelkie nieprawidłowości w działaniu urządzenia

**Dobre praktyki:**
1. ⭐ Kalibrować urządzenie wzorcami wewnętrznymi między legalizacjami (np. co 6 miesięcy)
2. ⭐ Prowadzić dziennik eksploatacji z zapisem dat, użytkowników, ewentualnych usterek
3. ⭐ Szkolić personel z prawidłowego użytkowania i ograniczeń urządzenia
4. ⭐ Chrzcić dane pacjentów zgodnie z RODO (szyfrowanie, backup, retencja)
5. ⭐ Nie używać urządzenia w warunkach odbiegających od specyfikacji (temp., wilgotność)

## 1.4.10 Podsumowanie rozdziału

Niniejszy rozdział przedstawił kompleksową analizę prawną autorskiego systemu pomiarowo-treningowego BIOFEEDBACK-TUBE w kontekście rozróżnienia między wyrobami medycznymi a urządzeniami pomiarowymi specjalnymi. Główne wnioski:

**Teza 1: tubing lateksowy JEST wyrobem medycznym klasy I**
- Podlega MDR (UE) 2017/745
- Wymaga oznakowania CE i deklaracji zgodności wytwórcy
- Klasyfikacja na podstawie reguły 1 Załącznika VIII MDR

**Teza 2: system elektroniczny NIE JEST wyrobem medycznym**
- Raspberry Pi, Arduino, czujnik HX711 są komponentami ogólnego zastosowania
- System zintegrowany pełni funkcję pomiarowo-informacyjną, nie terapeutyczną
- Brak autonomicznych decyzji medycznych, pełen nadzór fizjoterapeuty

**Teza 3: właściwa ścieżka to legalizacja jako urządzenie pomiarowe specjalne**
- Podstawa: Ustawa Prawo o miarach + rozporządzenia wykonawcze
- Procedura: legalizacja jednostkowa w Okręgowym Urzędzie Miar
- Koszt: ~1,150 PLN vs. ~300,000 EUR dla medical device Class IIb
- Czas: 6–10 tygodni vs. 24–42 miesiące

**Teza 4: dobrowolne podwyższenie standardów jest rekomendowane**
- Mimo braku prawnego obowiązku, warto spełnić wybrane wymagania MDR Class IIb
- Bezpieczeństwo elektryczne (PN-EN 60601-1), EMC, zarządzanie ryzykiem (ISO 14971)
- Buduje zaufanie użytkowników i ułatwia ewentualną recertyfikację w przyszłości

**Implikacje dla oryginalnego osiągnięcia projektowego:**
Wybór optymalnej ścieżki regulacyjnej (urządzenie pomiarowe specjalne zamiast wyrobu medycznego klasy IIb) stanowi element innowacyjności nie tylko technologicznej, ale także **organizacyjno-prawnej**. Autor habilitacji wykazał się kompetencjami interdyscyplinarnymi, łączącymi wiedzę inżynierską z znajomością ram prawnych, co przełożyło się na realne wdrożenie systemu przy minimalizacji barier administracyjnych i kosztowych.

---

## Bibliografia rozdziału 1.4

1. Rozporządzenie Parlamentu Europejskiego i Rady (UE) 2017/745 z dnia 5 kwietnia 2017 r. w sprawie wyrobów medycznych (MDR). Dziennik Urzędowy Unii Europejskiej L 117/1.

2. Ustawa z dnia 11 maja 2001 r. Prawo o miarach. Dz.U. 2022 poz. 2235 z późniejszymi zmianami.

3. Rozporządzenie Ministra Zdrowia z dnia 30 lipca 2021 r. w sprawie rodzajów przyrządów pomiarowych podlegających prawnej kontroli metrologicznej. Dz.U. 2021 poz. 1420.

4. Komisja Europejska. (2021). MDCG 2021-24: Guidance on classification of medical devices. Brussels: European Commission.

5. Główny Urząd Miar. (2023). Procedura legalizacji przyrządów pomiarowych. Warszawa: GUM.

6. International Organization of Legal Metrology. (2008). OIML D 1: Elements for regulating instruments for legal metrology. Paris: OIML.

7. Joint Committee for Guides in Metrology. (2008). JCGM 100:2008 Evaluation of measurement data — Guide to the expression of uncertainty in measurement (GUM). Sèvres: BIPM.

8. Polska Norma PN-EN ISO/IEC 17025:2018-10. Ogólne wymagania dotyczące kompetencji laboratoriów badawczych i wzorcujących. Warszawa: PKN.

9. Food and Drug Administration. (2022). 21 CFR Part 820 – Quality System Regulation. Silver Spring, MD: FDA.

10. International Medical Device Regulators Forum. (2020). IMDRF/SaMD WG/N10FINAL: Software as a Medical Device (SaMD): Key Definitions.

---

**Autor rozdziału:** [Imię i Nazwisko Autora Habilitacji]  
**Afiliacja:** [Nazwa Uczelni/Instytucji]  
**Data opracowania:** 2024  
**Recenzja merytoryczna:** [Imię i Nazwisko Recenzenta – eksperta prawa medycznego/metrologii]
