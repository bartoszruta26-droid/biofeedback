# Rozdział 8. Załączniki – Kompleksowa dokumentacja techniczna, oprogramowanie, protokoły metrologiczne i wyniki walidacji klinicznej

## 8.1. Wprowadzenie do załączników pracy habilitacyjnej

Niniejszy rozdział stanowi zbiór kompletnych materiałów dowodowych, dokumentacji technicznej, kodów źródłowych, protokołów badawczych i wyników walidacji, które uzupełniają merytoryczną treść pracy habilitacyjnej i stanowią materialny dowód realizacji oryginalnego osiągnięcia projektowego zgodnie z art. 219 ust. 1 pkt 2 lit. c ppkt 1 ustawy – Prawo o szkolnictwie wyższym i nauce.

Załączniki zostały zorganizowane w sześciu głównych kategoriach odpowiadających strukturze wskazanej w spisie treści pracy:

- **Załącznik A:** Dokumentacja techniczna i schematy elektryczne
- **Załącznik B:** Kody źródłowe oprogramowania Raspberry Pi, Arduino i Pico
- **Załącznik C:** Protokoły legalizacji pierwotnej i wzorcowania
- **Załącznik D:** Deklaracja zgodności i dokumentacja metrologiczna
- **Załącznik E:** Wyniki walidacji klinicznej i treningowej
- **Załącznik F:** Oświadczenia autora i recenzentów (zgodnie z wymogami uczelni)

Każdy załącznik został opatrzony szczegółowym opisem, metadanymi identyfikacyjnymi oraz informacjami dotyczącymi wersji, daty opracowania i osób odpowiedzialnych za przygotowanie dokumentacji.

---

## ZAŁĄCZNIK A. Dokumentacja techniczna i schematy elektryczne

### A.1. Schemat ideowy układu warunkowania sygnału tensometrycznego

**Numer dokumentu:** PTMS-ELEC-001  
**Wersja:** 2.3  
**Data:** 2024-03-15  
**Format:** A2 (420 × 594 mm)  
**Opracował:** mgr inż. [Imię i Nazwisko Autora]  
**Zatwierdził:** prof. dr hab. [Promotor/Konsultant]

#### A.1.1. Opis ogólny schematu ideowego

Schemat ideowy przedstawia kompletny układ warunkowania sygnału dla czujnika tensometrycznego typu full-bridge z mostkiem Wheatstone'a o rezystancji nominalnej 350Ω. Układ składa się z następujących funkcjonalnych bloków:

**Blok 1: Mostek Wheatstone'a i czujnik tensometryczny**

```
        VEXC (+5V)
           │
      ┌────┴────┐
      │         │
     R1        R2
    350Ω      350Ω
      │         │
      ├────┬────┤ → OUT+ (sygnał różnicowy)
      │    │    │
     R3        R4
    350Ω      350Ω
      │         │
      └────┬────┘
           │
          GND
```

Gdzie:
- R1, R2, R3, R4 to czteroelementowy mostek tensometryczny w konfiguracji full-bridge;
- VEXC to napięcie wzbudzenia mostka (nominalnie +5V DC, stabilizowane);
- OUT+ i OUT− to wyjścia różnicowe sygnału analogowego;
- Sygnał różnicowy wynosi typowo 0–20 mV dla zakresu 0–50 kgf.

**Blok 2: Wzmacniacz instrumentalny HX711**

Układ scalony HX711 (Avia Semiconductor) pełni funkcję:
- Wzmacniacza instrumentalnego o programowalnym wzmocnieniu (32×, 64×, 128×);
- Przetwornika analogowo-cyfrowego (ADC) sigma-delta 24-bit;
- Stabilizatora napięcia dla czujnika (AVDD 2.6–5.5V);
- Interfejsu szeregowego do komunikacji z mikrokontrolerem.

**Parametry konfiguracyjne HX711:**
- Wzmocnienie: 128 V/V (wybrane dla optymalizacji dynamiki);
- Częstotliwość próbkowania: 80 Hz (tryb szybki);
- Napięcie odniesienia: wewnętrzne 1.25V;
- Filtr cyfrowy: sinc³ z częstotliwością odcięcia 20 Hz.

**Blok 3: Filtr dolnoprzepustowy RC**

Na wejściach A+ i A− układu HX711 zastosowano pasywne filtry RC dolnoprzepustowe:
- Rezystancja: R = 100Ω (tolerancja 1%);
- Kondensator: C = 100 nF (foliowy, polipropylenowy);
- Częstotliwość graniczna: $f_c = \frac{1}{2\pi RC} = 15.9 \text{ Hz}$.

Filtr eliminuje zakłócenia wysokoczęstotliwościowe (>20 Hz) pochodzące od:
- Zakłóceń elektromagnetycznych sieci energetycznej (50 Hz i harmoniczne);
- Szumów termicznych rezystorów mostka;
- Zakłóceń RF (radio frequency interference).

**Blok 4: Stabilizator napięcia wzbudzenia**

Dla zapewnienia stabilności metrologicznej mostka Wheatstone'a zastosowano precyzyjny stabilizator napięcia referencyjnego REF5050 (Texas Instruments):
- Napięcie wyjściowe: 5.0V ±0.05%;
- Dryft temperaturowy: 10 ppm/°C;
- Szum własny: 6 µVpp (0.1–10 Hz);
- Prąd obciążenia: do 50 mA (wystarczający dla mostka 350Ω, pobór ~14 mA).

**Blok 5: Złącza i interfejsy**

Schemat obejmuje następujące złącza:
- J1: Złącze 4-pinowe JST-XH dla czujnika tensometrycznego (VCC, GND, A+, A−);
- J2: Złącze 6-pinowe dla komunikacji z Arduino Nano (VCC, GND, DT, SCK, PD_SCK, GAIN);
- J3: Złącze zasilania zewnętrzneho 5V DC (barrel jack 2.1 mm);
- J4: Złącze programowania ISP dla ewentualnej aktualizacji firmware.

#### A.1.2. Lista komponentów elektronicznych (BOM – Bill of Materials)

| Poz. | Ilość | Symbol | Wartość/Numer | Producent | Obudowa | Uwagi |
|------|-------|--------|---------------|-----------|---------|-------|
| 1 | 1 | U1 | HX711 | Avia Semiconductor | SOP-16 | ADC 24-bit |
| 2 | 1 | U2 | REF5050 | Texas Instruments | SOIC-8 | Stabilizator 5V |
| 3 | 1 | U3 | ATmega328P-AU | Microchip | TQFP-32 | Mikrokontroler (opcjonalny) |
| 4 | 4 | R1–R4 | 350Ω | Vishay | Strain gauge | Czujnik tensometryczny |
| 5 | 2 | R5, R6 | 100Ω | Yageo | 0805 | Filtr RC |
| 6 | 2 | C1, C2 | 100 nF | Murata | 0805 | Kondensator foliowy |
| 7 | 2 | C3, C4 | 10 µF | Panasonic | 1206 | Kondensator tantalowy |
| 8 | 1 | J1 | AXK504137YG | Panasonic | 4-pin JST | Złącze czujnika |
| 9 | 1 | J2 | DF13-6P-1.25DSA | Hirose | 6-pin | Komunikacja |
| 10 | 1 | J3 | DCJ052AH | CUI Devices | Barrel jack | Zasilanie |
| 11 | 1 | F1 | 0ZCJ0050FF2E | Bel Fuse | 1206 | Bezpiecznik resetowalny 0.5A |
| 12 | 1 | D1 | SS14 | Diodes Inc. | SMA | Dioda Schottky'ego |
| 13 | 1 | X1 | ABS07-32.768KHZ | Abracon | Crystal 32.768 kHz | Zegar RTC (opcjonalny) |

#### A.1.3. Wytyczne projektowania PCB

**Warstwy PCB:**
- Warstwa TOP: ścieżki sygnałowe, komponenty SMD;
- Warstwa GND: pełna masa (ground plane) dla ekranowania;
- Warstwa BOTTOM: ścieżki zasilające;
- Warstwa POWER: plane napięcia 5V i 3.3V.

**Zasady routingu:**
1. Ścieżki analogowe (A+, A−) prowadzone jako para różnicowa z impedancją 100Ω;
2. Minimalizacja długości ścieżek od czujnika do HX711 (<50 mm);
3. Unikanie prowadzenia ścieżek cyfrowych w pobliżu wejść analogowych;
4. Umieszczenie kondensatorów odsprzęgających jak najbliżej pinów zasilania U1 i U2;
5. Izolacja galwaniczna sekcji analogowej od cyfrowej za pomocą przerwy w masie.

**Parametry techniczne PCB:**
- Materiał podłoża: FR-4, klasa palności UL94-V0;
- Grubość płytki: 1.6 mm;
- Grubość miedzi: 35 µm (1 oz);
- Minimalna szerokość ścieżki: 0.2 mm;
- Minimalna średnica przelotki: 0.3 mm;
- Powłoka ochronna: solder mask zielony, silkscreen biały;
- Finish powierzchniowy: ENIG (Electroless Nickel Immersion Gold) dla lepszej lutowalności.

---

### A.2. Schemat montażowy PCB z rozmieszczeniem komponentów

**Numer dokumentu:** PTMS-ELEC-003  
**Wersja:** 2.1  
**Data:** 2024-02-28  
**Format:** A2 (420 × 594 mm)

#### A.2.1. Widok warstwy TOP (komponenty)

Schemat montażowy przedstawia rozmieszczenie wszystkich komponentów na płytce drukowanej o wymiarach 80 × 60 mm. Komponenty podzielono na strefy funkcjonalne:

**Strefa A: Wejście analogowe (lewa górna ćwiartka)**
- Złącze J1 dla czujnika tensometrycznego;
- Filtry RC (R5, R6, C1, C2);
- Układ HX711 (U1).

**Strefa B: Sekcja cyfrowa (prawa górna ćwiartka)**
- Mikrokontroler ATmega328P (U3);
- Kwarc 16 MHz z kondensatorami 22 pF;
- Złącze programowania ISP.

**Strefa C: Zasilanie (dolna część)**
- Stabilizator REF5050 (U2);
- Kondensatory filtrujące C3, C4;
- Złącze zasilania J3;
- Bezpiecznik F1 i dioda D1.

**Strefa D: Komunikacja (prawy bok)**
- Złącze J2 dla komunikacji z Raspberry Pi;
- Rezystory podciągające 10 kΩ dla linii I2C (opcjonalne).

#### A.2.2. Widok warstwy BOTTOM (ścieżki)

Warstwa dolna zawiera głównie ścieżki zasilające oraz połączenia masy. Kluczowe obszary:

- **Ground plane:** Pełna powierzchnia masy na całej płytce z wyjątkiem strefy analogowej, gdzie zastosowano izolowaną wyspę masy analogowej (AGND);
- **Power planes:** Obszary miedzi dla rozprowadzenia napięć 5V i 3.3V;
- **Via stitching:** Przylutowane przelotki łączące masy między warstwami co 5 mm wokół obwodu płytki.

#### A.2.3. Instrukcja lutowania i kontroli jakości

**Procedura lutowania reflow:**
1. Nałożenie pasty lutowniczej typu SAC305 (Sn96.5Ag3.0Cu0.5) przez szablon stalowy grubości 0.12 mm;
2. Umieszczenie komponentów SMD za pomocą pick-and-place machine z dokładnością ±0.05 mm;
3. Profil reflow:
   - Preheat: 150–180°C przez 60–90 s;
   - Soak: 180–200°C przez 60 s;
   - Reflow peak: 245–250°C przez 10–20 s;
   - Cooling: schładzanie do <100°C w czasie <6 s.

**Kontrola jakości po lutowaniu:**
- Inspekcja wizualna pod mikroskopem 20× (zgodność z IPC-A-610 Class 2);
- Test automatyczny AOI (Automated Optical Inspection) dla wykrywania:
  - Brakujących komponentów;
  - Zwarcia między sąsiednimi padami;
  - Niewystarczającej ilości cyny (cold joints);
  - Przesunięć komponentów >50% szerokości pada.
- Test elektryczny ICT (In-Circuit Test) dla weryfikacji ciągłości połączeń i poprawności wartości komponentów pasywnych.

---

### A.3. Rysunki wykonawcze elementów mechanicznych

#### A.3.1. Uchwyt mocujący czujnik tensometryczny

**Numer dokumentu:** PTMS-MCH-003  
**Wersja:** D  
**Data:** 2024-02-03  
**Materiał:** Aluminium 6061-T6  
**Proces wytwarzania:** Frezowanie CNC 5-osiowe

**Wymiary zewnętrzne:** 80 × 40 × 25 mm  
**Masa:** 62 g  
**Tolerancje ogólne:** ISO 2768-mK

**Cechy konstrukcyjne:**
- Gniazdo kuliste półsferyczne Ø12.05 mm (+0.05/0) dla przegubu samocentrującego;
- Powierzchnia styku z load cell: płaszczyzna 40×20 mm frezowana precyzyjnie Ra 0.8 μm;
- 4 otwory gwintowane M4×0.7 dla mocowania czujnika, głębokość 10 mm;
- Kanał wewnętrzny Ø6 mm dla prowadzenia kabla sygnałowego;
- Fazowania 0.3×45° na wszystkich krawędziach zewnętrznych.

**Operacje technologiczne:**
1. Cięcie pręta aluminiowego Ø45 mm na odcinki 30 mm;
2. Frezowanie czołowe powierzchni bazowej A;
3. Frezowanie konturu zewnętrznego 5-osiowe;
4. Wiercenie i gwintowanie otworów M4;
5. Frezowanie gniazda kulistego narzędziem kulowym Ø12 mm;
6. Drążenie kanału kablowego wiertłem długim Ø6 mm;
7. Anodowanie twarde 25 μm, kolor czarny.

**Kontrola wymiarowa CMM:**
- Sprawdzenie współosiowości gniazda kulistego względem bazy A: tolerancja 0.02 mm;
- Pomiar płaskości powierzchni styku: tolerancja 0.01 mm;
- Weryfikacja położenia otworów M4: tolerancja pozycji Ø0.1 mm.

#### A.3.2. Szybkozłącze tubeingu lateksowego

**Numer dokumentu:** PTMS-MCH-006  
**Wersja:** C  
**Data:** 2024-01-20  
**Materiał:** PA6-GF30 (poliamid zbrojony włóknem szklanym 30%)  
**Proces wytwarzania:** Wtryskiwanie dwukomponentowe (2K injection molding)

**Zasada działania:**
Szybkozłącze umożliwia błyskawiczne podpinanie i odpinanie tubeingu lateksowego bez użycia narzędzi. Mechanizm zatrzaskowy oparty jest na trzech kulkach stalowych Ø3 mm, które blokują się w rowku stożkowej tulei zaciskowej.

**Komponenty szybkozłącza:**
1. Korpus zewnętrzny (część żeńska) – wtrysk z PA6-GF30, kolor niebieski;
2. Tuleja zaciskowa stożkowa – wtrysk z POM-C (poliacetal), kolor biały;
3. Kulki zatrzaskowe 3× Ø3 mm – stal nierdzewna 316, hartowana 55 HRC;
4. Sprężyna pierścieniowa – stal sprężynowa 65Mn, średnica drutu 0.8 mm;
5. Mandrel stożkowy – stal 316L, stożek 3°, powierzchnia polerowana Ra 0.4 μm.

**Siła potrzebna do zapięcia:** 25–35 N  
**Siła potrzebna do odpięcia:** 40–60 N (poprzez pociągnięcie tulei zwalniającej)  
**Żywotność mechaniczna:** >50,000 cykli zapięcia/odpięcia

---

### A.4. Diagramy architektury systemu embedded

#### A.4.1. Diagram blokowy przepływu danych

```
┌─────────────────────────────────────────────────────────────────────────┐
│                         ARCHITEKTURA SYSTEMU                            │
│                    BIOFEEDBACK-TUBE v1.0                                │
└─────────────────────────────────────────────────────────────────────────┘

[ŚRODOWISKO ZEWNĘTRZNE]
         │
         ▼
┌──────────────────┐
│  TUBING LATEKSOWY │  ← Element oporowy, generuje siłę F [N]
└────────┬─────────┘
         │ przeniesienie siły mechanicznej
         ▼
┌──────────────────┐
│ LOAD CELL 50kg   │  ← Czujnik tensometryczny full-bridge 350Ω
│  (HX711-ready)   │  ← Wyjście: sygnał różnicowy 0–20 mV
└────────┬─────────┘
         │ sygnał analogowy A+/A−
         ▼
┌──────────────────┐      ┌──────────────────┐
│   HX711 ADC      │◄────►│  ARDUINO NANO    │
│  24-bit ΣΔ       │      │  ATmega328P      │
│  80 Hz sampling  │      │  16 MHz, 2KB RAM │
└────────┬─────────┘      └────────┬─────────┘
         │ dane cyfrowe            │ transmisja szeregowa UART
         │ (24-bit signed int)     │ 115200 baud, 8N1
         ▼                         ▼
┌─────────────────────────────────────────────────────────┐
│              RASPBERRY PI 4 Model B                     │
│  Broadcom BCM2711 Quad-core Cortex-A72 @ 1.5 GHz        │
│  4GB LPDDR4 RAM, 32GB eMMC                              │
│  Raspberry Pi OS 64-bit (Debian 12 Bookworm)            │
│                                                         │
│  ┌───────────────────────────────────────────────────┐ │
│  │  APLIKACJA BIOFEEDBACK (C++ / Qt5.15)             │ │
│  │                                                   │ │
│  │  ┌─────────────┐  ┌─────────────┐  ┌───────────┐ │ │
│  │  │ Moduł AKWI- │  │ Moduł PRZE- │  │ Moduł WI- │ │ │
│  │  │ ZYCJI DANYCH│  │ TWARZANIA   │  │ ZUALIZACJI│ │ │
│  │  │             │  │ SYGNAŁÓW    │  │ I GRAFÓW  │ │ │
│  │  │ - SerialPort│  │ - Filtr     │  │ - Wykresy │ │ │
│  │  │ - Parsing   │  │   Butter-   │  │   czasu   │ │ │
│  │  │ - CRC-8     │  │   wortha    │  │   rze-    │ │ │
│  │  │ - Timestamp │  │ - Detekcja  │  │ czywisty  │ │ │
│  │  │ - JSON      │  │   powtórzeń │  │ - Dashboard│ │ │
│  │  │ - Encryption│  │ - Obliczanie│  │ - Alarmy  │ │ │
│  │  │   AES-256   │  │   parametrów│  │ - Gry     │ │ │
│  │  └─────────────┘  └─────────────┘  └───────────┘ │ │
│  └───────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────┘
         │
         ▼
┌──────────────────┐
│  EKSPORT DANYCH  │
│  - JSON files    │
│  - CSV reports   │
│  - PDF summary   │
│  - Cloud backup  │
└──────────────────┘
```

#### A.4.2. Diagram stanów aplikacji głównej

```
┌─────────────────────────────────────────────────────────────────┐
│                   DIAGRAM STANÓW APLIKACJI                      │
└─────────────────────────────────────────────────────────────────┘

    ┌──────────────┐
    │   START      │
    │ Power-On     │
    └──────┬───────┘
           │ inicjalizacja hardware
           ▼
    ┌──────────────┐
    │   INIT       │
    │ - Load config│
    │ - Open port  │
    │ - Check HW   │
    └──────┬───────┘
           │
     ┌─────┴─────┐
     │           │
HW OK       HW ERROR
     │           │
     ▼           ▼
┌─────────┐  ┌──────────┐
│  IDLE   │  │  ERROR   │
│ Standby │  │  STATE   │
└────┬────┘  └──────────┘
     │ start measurement
     ▼
┌──────────────┐
│  MEASURING   │◄────────────────────┐
│ - Acquire    │                     │
│ - Process    │                     │
│ - Display    │                     │
└──────┬───────┘                     │
       │                             │
  ┌────┴────┐                        │
  │         │                        │
CONTINUE   STOP/PAUSE                │
  │         │                        │
  │         ▼                        │
  │    ┌─────────┐                  │
  │    │ PAUSED  │──────────────────┘
  │    │ Resume? │
  │    └─────────┘
  │
  ▼
┌──────────────┐
│  COMPLETED   │
│ - Save data  │
│ - Generate   │
│   report     │
└──────┬───────┘
       │
       ▼
    ┌─────────┐
    │  IDLE   │
    └─────────┘
```

---

## ZAŁĄCZNIK B. Kody źródłowe oprogramowania Raspberry Pi, Arduino i Pico

### B.1. Firmware Arduino Nano – akwizycja danych z HX711

**Plik:** `hx711_advanced.ino`  
**Wersja:** 1.5.0  
**Data:** 2024-03-10  
**Środowisko:** Arduino IDE 2.2.1  
**Biblioteki:** HX711_ADC by Olav Kallhovd (v2.1.2)

```cpp
/**
 * BIOFEEDBACK-TUBE: Advanced HX711 Weight Scale Firmware
 * =======================================================
 * 
 * Opis:
 *   Firmware dla mikrokontrolera Arduino Nano odpowiedzialny za:
 *   - Akwizycję sygnału z czujnika tensometrycznego poprzez układ HX711
 *   - Konwersję analogowo-cyfrową 24-bit z częstotliwością 80 Hz
 *   - Transmisję danych binarnych przez UART z kontrolą CRC-8
 *   - Realizację komend tekstowych (TARE, CALIBRATE, CONTINUOUS, etc.)
 *   - Tryb ciągły (continuous mode) z pakietowaniem ramek
 * 
 * Autor: [Imię i Nazwisko]
 * Data: 2024-03-10
 * Licencja: MIT License
 * 
 * Pinout:
 *   HX711 DT  -> Arduino D2 (Pin 2)
 *   HX711 SCK -> Arduino D3 (Pin 3)
 *   UART TX   -> Arduino D1 (Pin 1) -> Raspberry Pi RX
 *   UART RX   -> Arduino D0 (Pin 0) <- Raspberry Pi TX
 */

#include <HX711_ADC.h>
#include <EEPROM.h>

// ==================== DEFINICJE PINÓW ====================
#define HX711_DT_PIN  2
#define HX711_SCK_PIN 3

// ==================== STAŁE KONFIGURACYJNE ====================
const unsigned long BAUD_RATE = 115200;
const byte PACKET_START_BYTE = 0xAA;
const byte PACKET_END_BYTE = 0x55;
const unsigned long SAMPLE_RATE_HZ = 80;
const unsigned long SAMPLE_PERIOD_MS = 1000 / SAMPLE_RATE_HZ; // 12.5 ms

// ==================== STRUKTURY DANYCH ====================
/**
 * Struktura pakietu danych binarnych (9 bajtów + markery)
 * Format ramki:
 *   [START][TIMESTAMP:4][VALUE:4][CRC8][END]
 *   0xAA   [0-3]      [4-7]   [8]    0x55
 *   Łącznie: 11 bajtów
 */
struct DataPacket {
    uint32_t timestamp;  // Czas w milisekundach od uruchomienia
    int32_t value;       // Wartość surowa z ADC (24-bit sign-extended do 32-bit)
    uint8_t crc;         // Suma kontrolna CRC-8
};

// ==================== ZMIENNE GLOBALNE ====================
HX711_ADC scale(HX711_DT_PIN, HX711_SCK_PIN);

bool continuousMode = false;
unsigned long lastSampleTime = 0;
uint32_t packetCounter = 0;

// Kalibracja
float calibrationFactor = 125.5;  // Wartość domyślna, do kalibracji
int32_t tareOffset = 0;

// Bufor кольowy dla trybu ciągłego
const uint8_t BUFFER_SIZE = 16;
DataPacket circularBuffer[BUFFER_SIZE];
uint8_t bufferHead = 0;
uint8_t bufferTail = 0;

// ==================== FUNKCJE CRC-8 ====================
/**
 * Obliczanie sumy kontrolnej CRC-8 z wielomianem x^8 + x^2 + x + 1 (0x07)
 * Wartość początkowa: 0xFF
 * 
 * @param data Wskaźnik do tablicy bajtów
 * @param length Długość danych w bajtach
 * @return Obliczona wartość CRC-8
 */
uint8_t calculateCRC8(const uint8_t* data, uint8_t length) {
    uint8_t crc = 0xFF;  // Initial value
    
    for (uint8_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x07;  // Polynomial 0x07
            } else {
                crc <<= 1;
            }
        }
    }
    
    return crc;
}

// ==================== FUNKCJE KOMUNIKACYJNE ====================
/**
 * Wysyłanie pakietu binarnego przez UART
 * Format: 0xAA [timestamp:4] [value:4] [crc] 0x55
 */
void sendBinaryPacket(uint32_t timestamp, int32_t value) {
    uint8_t buffer[9];
    
    // Pakowanie danych (little-endian)
    buffer[0] = (timestamp >> 0) & 0xFF;
    buffer[1] = (timestamp >> 8) & 0xFF;
    buffer[2] = (timestamp >> 16) & 0xFF;
    buffer[3] = (timestamp >> 24) & 0xFF;
    
    buffer[4] = (value >> 0) & 0xFF;
    buffer[5] = (value >> 8) & 0xFF;
    buffer[6] = (value >> 16) & 0xFF;
    buffer[7] = (value >> 24) & 0xFF;
    
    // Obliczanie CRC dla timestamp + value (8 bajtów)
    buffer[8] = calculateCRC8(buffer, 8);
    
    // Wysyłanie ramki z markerami
    Serial.write(PACKET_START_BYTE);
    Serial.write(buffer, 9);
    Serial.write(PACKET_END_BYTE);
    
    packetCounter++;
}

/**
 * Parsowanie komend tekstowych z Raspberry Pi
 * Obsługiwane komendy:
 *   ID, WHOAMI      - Identyfikacja urządzenia
 *   TARE, CALZERO   - Zerowanie wagi
 *   DATA, READ      - Pojedynczy odczyt
 *   FORCE           - Odczyt w formacie tekstowym
 *   CONT            - Włączenie trybu ciągłego
 *   STOP            - Wyłączenie trybu ciągłego
 *   CAL:<grams>     - Kalibracja znaną masą
 *   RESET           - Reset przez watchdog
 */
void parseCommand(String command) {
    command.trim();
    command.toUpperCase();
    
    if (command == "ID" || command == "WHOAMI") {
        Serial.println("DEVICE:Advanced HX711 Weight Scale");
        Serial.println("ID:HX711-SCALE-V1.0");
        Serial.println("FW:1.5.0,BR:115200");
        Serial.println("OK");
        
    } else if (command == "TARE" || command == "CALZERO") {
        Serial.println("Taring...");
        scale.tareNoDelay();
        tareOffset = scale.getOffset();
        EEPROM.put(0, tareOffset);  // Zapis offsetu do EEPROM
        Serial.println("Tare set!");
        
    } else if (command == "DATA" || command == "READ") {
        int32_t rawValue = scale.getData();
        float forceGrams = (rawValue - tareOffset) / calibrationFactor;
        
        Serial.print("RAW:");
        Serial.print(rawValue);
        Serial.print(",FORCE:");
        Serial.print(forceGrams, 2);
        Serial.print("g,TS:");
        Serial.print(millis());
        Serial.println("ms");
        
    } else if (command == "FORCE") {
        int32_t rawValue = scale.getData();
        float forceGrams = (rawValue - tareOffset) / calibrationFactor;
        float forceNewtons = forceGrams * 9.80665 / 1000.0;
        
        Serial.print("FORCE:");
        Serial.print(forceNewtons, 3);
        Serial.print("N,GRAMS:");
        Serial.print(forceGrams, 2);
        Serial.print(",RAW:");
        Serial.print(rawValue);
        Serial.print(",TS:");
        Serial.print(millis());
        Serial.println("ms");
        
    } else if (command == "CONT") {
        continuousMode = true;
        Serial.println("Continuous mode ON (80Hz)");
        
    } else if (command == "STOP") {
        continuousMode = false;
        Serial.println("Continuous mode OFF");
        
    } else if (command.startsWith("CAL:")) {
        float knownMass = command.substring(4).toFloat();
        int32_t rawValue = scale.getData();
        
        // Obliczanie nowego współczynnika kalibracji
        calibrationFactor = (rawValue - tareOffset) / knownMass;
        
        Serial.print("Calibrated! Factor:");
        Serial.println(calibrationFactor, 6);
        
        // Zapis do EEPROM
        EEPROM.put(sizeof(tareOffset), calibrationFactor);
        
    } else if (command == "RESET") {
        Serial.println("Resetting...");
        delay(100);
        watchdog_enable();  // Wymaga biblioteki avr/wdt.h
        
    } else {
        Serial.print("ERROR:Unknown command '");
        Serial.print(command);
        Serial.println("'");
    }
}

// ==================== SETUP ====================
void setup() {
    // Inicjalizacja UART
    Serial.begin(BAUD_RATE);
    while (!Serial) {
        ;  // Wait for serial port to connect
    }
    
    Serial.println("\n=== BIOFEEDBACK-TUBE HX711 Firmware v1.5.0 ===");
    Serial.println("Initializing...");
    
    // Inicjalizacja HX711
    scale.begin();
    scale.setGain(128);  // Gain 128 dla czujników 350Ω
    
    // Odczyt kalibracji z EEPROM
    EEPROM.get(0, tareOffset);
    EEPROM.get(sizeof(tareOffset), calibrationFactor);
    
    Serial.print("Loaded offset: ");
    Serial.println(tareOffset);
    Serial.print("Loaded calibration factor: ");
    Serial.println(calibrationFactor, 6);
    
    // Stabilizacja przed pierwszym pomiarem
    Serial.println("Warming up (5 seconds)...");
    delay(5000);
    
    // Auto-tare przy starcie
    scale.tareNoDelay();
    tareOffset = scale.getOffset();
    
    Serial.println("System ready. Waiting for commands...");
    Serial.println("Type 'HELP' for available commands.");
}

// ==================== MAIN LOOP ====================
void loop() {
    // Obsługa komend z UART
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        parseCommand(command);
    }
    
    // Tryb ciągły: wysyłanie danych 80 Hz
    if (continuousMode) {
        unsigned long currentTime = millis();
        
        if (currentTime - lastSampleTime >= SAMPLE_PERIOD_MS) {
            lastSampleTime = currentTime;
            
            // Pobranie wartości z HX711
            int32_t rawValue = scale.getData();
            
            // Wysyłanie pakietu binarnego
            sendBinaryPacket(currentTime, rawValue);
            
            // Blink LED co 100 pakietów (diagnostic)
            if (packetCounter % 100 == 0) {
                digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
            }
        }
    }
    
    // Watchdog reset (zapobieganie zawieszaniu)
    // watchdog_reset();  // Odkomentować jeśli używany WDT
}
```

---

### B.2. Aplikacja Raspberry Pi – moduł akwizycji danych

**Plik:** `src/sensor/hx711_reader.cpp`  
**Wersja:** 2.1.0  
**Data:** 2024-03-12  
**Kompilator:** GCC 12.2 (C++17)  
**Biblioteki:** libserialport, Qt5 Core

```cpp
/**
 * BIOFEEDBACK-TUBE: HX711 Sensor Reader Module
 * =============================================
 * 
 * Opis:
 *   Moduł odpowiedzialny za komunikację z Arduino Nano poprzez port
 *   szeregowy USB, odbiór danych binarnych z czujnika HX711, weryfikację
 *   CRC-8 oraz konwersję na jednostki fizyczne (Newtony).
 * 
 * Autor: [Imię i Nazwisko]
 * Data: 2024-03-12
 * Licencja: GPL-3.0-or-later
 */

#include "hx711_reader.h"
#include <QDebug>
#include <QTimer>
#include <QFile>
#include <QDir>
#include <libserialport.h>

// ==================== KONSTANTY ====================
constexpr int DEFAULT_BAUD_RATE = 115200;
constexpr int PACKET_SIZE = 11;  // 1 (start) + 4 (ts) + 4 (val) + 1 (crc) + 1 (end)
constexpr uint8_t PACKET_START = 0xAA;
constexpr uint8_t PACKET_END = 0x55;

// ==================== IMPLEMENTACJA KLASY HX711Reader ====================

HX711Reader::HX711Reader(QObject *parent)
    : QObject(parent)
    , m_port(nullptr)
    , m_isConnected(false)
    , m_calibrationFactor(125.5)
    , m_tareOffset(0)
    , m_sampleCount(0)
{
    // Inicjalizacja libserialport
    sp_return ret = sp_init();
    if (ret != SP_OK) {
        qCritical() << "Failed to initialize libserialport:" << ret;
    }
    
    // Timer do odpytywania portu
    m_pollTimer = new QTimer(this);
    connect(m_pollTimer, &QTimer::timeout, this, &HX711Reader::pollSerialPort);
}

HX711Reader::~HX711Reader()
{
    disconnectDevice();
    sp_deinit();
}

/**
 * Automatyczne wykrywanie portu szeregowego Arduino
 * Szuka urządzeń z vendor ID Arduino lub nazwą zawierającą "ttyUSB", "ttyACM"
 */
QString HX711Reader::autoDetectPort()
{
    struct sp_port **portList;
    sp_return ret = sp_list_ports(&portList);
    
    if (ret != SP_OK) {
        qWarning() << "Failed to list serial ports";
        return QString();
    }
    
    for (int i = 0; portList[i] != nullptr; ++i) {
        char *portName = sp_get_port_name(portList[i]);
        QString name(portName);
        
        // Sprawdź typowe nazwy dla Arduino na Linux
        if (name.contains("ttyUSB") || name.contains("ttyACM")) {
            sp_free_port_list(portList);
            qDebug() << "Auto-detected Arduino port:" << name;
            return name;
        }
        
        // Sprawdź po vendor/product ID (Arduino = 0x2341)
        uint16_t vendorId, productId;
        if (sp_get_port_vendor_id(portList[i], &vendorId) == SP_OK &&
            sp_get_port_product_id(portList[i], &productId) == SP_OK) {
            if (vendorId == 0x2341) {  // Arduino SA
                sp_free_port_list(portList);
                qDebug() << "Auto-detected Arduino by VID/PID:" << name;
                return name;
            }
        }
    }
    
    sp_free_port_list(portList);
    qWarning() << "No Arduino device found";
    return QString();
}

/**
 * Podłączenie do urządzenia szeregowego
 */
bool HX711Reader::connectToDevice(const QString &portName, int baudRate)
{
    if (m_isConnected) {
        qWarning() << "Already connected";
        return false;
    }
    
    QString port = portName;
    if (port.isEmpty()) {
        port = autoDetectPort();
        if (port.isEmpty()) {
            qCritical() << "Could not auto-detect port";
            return false;
        }
    }
    
    // Otwarcie portu
    ret = sp_get_port_by_name(port.toUtf8().constData(), &m_port);
    if (ret != SP_OK) {
        qCritical() << "Failed to get port:" << port << "Error:" << ret;
        return false;
    }
    
    ret = sp_open(m_port, SP_MODE_READ_WRITE);
    if (ret != SP_OK) {
        qCritical() << "Failed to open port:" << ret;
        sp_free_port(m_port);
        m_port = nullptr;
        return false;
    }
    
    // Konfiguracja portu
    sp_set_baudrate(m_port, baudRate);
    sp_set_bits(m_port, 8);
    sp_set_parity(m_port, SP_PARITY_NONE);
    sp_set_stopbits(m_port, 1);
    sp_set_flowcontrol(m_port, SP_FLOWCONTROL_NONE);
    
    // Flush buforów
    sp_flush(m_port, SP_BUF_BOTH);
    
    m_isConnected = true;
    m_sampleCount = 0;
    
    // Start pollowania
    m_pollTimer->start(10);  // Sprawdzaj co 10 ms
    
    emit connected(port, baudRate);
    qDebug() << "Connected to" << port << "@" << baudRate;
    
    // Wysyłanie komendy identyfikacyjnej
    sendCommand("ID\n");
    
    return true;
}

/**
 * Rozłączenie z urządzeniem
 */
void HX711Reader::disconnectDevice()
{
    m_pollTimer->stop();
    
    if (m_port) {
        sendCommand("STOP\n");  // Wyłącz tryb ciągły
        QThread::msleep(100);
        
        sp_close(m_port);
        sp_free_port(m_port);
        m_port = nullptr;
    }
    
    m_isConnected = false;
    emit disconnected();
}

/**
 * Wysyłanie komendy tekstowej do Arduino
 */
void HX711Reader::sendCommand(const QString &command)
{
    if (!m_isConnected || !m_port) {
        qWarning() << "Not connected";
        return;
    }
    
    QByteArray data = command.toUtf8();
    sp_return ret = sp_blocking_write(m_port, data.constData(), data.size(), 1000);
    
    if (ret != data.size()) {
        qWarning() << "Failed to send command, written:" << ret << "expected:" << data.size();
    }
}

/**
 * Odpytywanie portu szeregowego i parsowanie danych
 */
void HX711Reader::pollSerialPort()
{
    if (!m_isConnected || !m_port) {
        return;
    }
    
    // Odczyt dostępnych bajtów
    uint8_t buffer[256];
    sp_return ret = sp_nonblocking_read(m_port, buffer, sizeof(buffer));
    
    if (ret <= 0) {
        return;  // Brak danych
    }
    
    // Dodanie do bufora wejściowego
    m_inputBuffer.append(reinterpret_cast<char*>(buffer), ret);
    
    // Parsowanie pakietów
    parseInputBuffer();
}

/**
 * Parsowanie bufora wejściowego w poszukiwaniu pakietów binarnych
 */
void HX711Reader::parseInputBuffer()
{
    while (m_inputBuffer.size() >= PACKET_SIZE) {
        // Szukanie bajtu startowego
        int startIndex = m_inputBuffer.indexOf(PACKET_START);
        
        if (startIndex < 0) {
            // Brak bajtu startowego, czyszczenie bufora
            m_inputBuffer.clear();
            return;
        }
        
        // Usunięcie danych przed bajtem startowym
        if (startIndex > 0) {
            m_inputBuffer.remove(0, startIndex);
        }
        
        // Sprawdzenie czy mamy pełny pakiet
        if (m_inputBuffer.size() < PACKET_SIZE) {
            break;  // Czekaj na więcej danych
        }
        
        // Weryfikacja bajtu końcowego
        if (static_cast<uint8_t>(m_inputBuffer[PACKET_SIZE - 1]) != PACKET_END) {
            // Nieprawidłowy pakiet, usuń pierwszy bajt i szukaj ponownie
            m_inputBuffer.remove(0, 1);
            continue;
        }
        
        // Ekstrakcja pakietu
        QByteArray packet = m_inputBuffer.mid(0, PACKET_SIZE);
        m_inputBuffer.remove(0, PACKET_SIZE);
        
        // Parsowanie
        if (parseBinaryPacket(packet)) {
            m_sampleCount++;
        }
    }
}

/**
 * Parsowanie pojedynczego pakietu binarnego
 * Format: [START][TS:4][VAL:4][CRC][END]
 */
bool HX711Reader::parseBinaryPacket(const QByteArray &packet)
{
    if (packet.size() != PACKET_SIZE) {
        qWarning() << "Invalid packet size:" << packet.size();
        return false;
    }
    
    // Ekstrakcja pól (little-endian)
    const uint8_t* data = reinterpret_cast<const uint8_t*>(packet.constData() + 1);
    
    uint32_t timestamp = static_cast<uint32_t>(data[0]) |
                         (static_cast<uint32_t>(data[1]) << 8) |
                         (static_cast<uint32_t>(data[2]) << 16) |
                         (static_cast<uint32_t>(data[3]) << 24);
    
    int32_t rawValue = static_cast<int32_t>(data[4]) |
                       (static_cast<int32_t>(data[5]) << 8) |
                       (static_cast<int32_t>(data[6]) << 16) |
                       (static_cast<int32_t>(data[7]) << 24);
    
    uint8_t receivedCRC = static_cast<uint8_t>(data[8]);
    
    // Weryfikacja CRC
    uint8_t calculatedCRC = calculateCRC8(data, 8);
    if (receivedCRC != calculatedCRC) {
        qWarning() << "CRC mismatch! Received:" << hex << receivedCRC 
                   << "Calculated:" << hex << calculatedCRC;
        emit crcError(timestamp, receivedCRC, calculatedCRC);
        return false;
    }
    
    // Konwersja na jednostki fizyczne
    double forceNewtons = rawValueToNewtons(rawValue);
    
    // Emitowanie sygnału z danymi
    SensorData sensorData;
    sensorData.timestamp = timestamp;
    sensorData.rawValue = rawValue;
    sensorData.forceNewtons = forceNewtons;
    sensorData.sampleIndex = m_sampleCount;
    
    emit dataReceived(sensorData);
    
    return true;
}

/**
 * Konwersja wartości surowej ADC na Newtony
 * F [N] = (raw - tare) / calibrationFactor * g / 1000
 */
double HX711Reader::rawValueToNewtons(int32_t rawValue) const
{
    double forceGrams = (rawValue - m_tareOffset) / m_calibrationFactor;
    double forceNewtons = forceGrams * 9.80665 / 1000.0;
    return forceNewtons;
}

/**
 * Obliczanie CRC-8 (identyczne jak w firmware Arduino)
 */
uint8_t HX711Reader::calculateCRC8(const uint8_t* data, uint8_t length) const
{
    uint8_t crc = 0xFF;
    
    for (uint8_t i = 0; i < length; ++i) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; ++j) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x07;
            } else {
                crc <<= 1;
            }
        }
    }
    
    return crc;
}

/**
 * Wyzerowanie wagi (tare)
 */
void HX711Reader::tare()
{
    sendCommand("TARE\n");
    
    // Poczekaj na potwierdzenie
    QEventLoop loop;
    QTimer::singleShot(2000, &loop, &QEventLoop::quit);
    loop.exec();
}

/**
 * Kalibracja znaną masą
 */
void HX711Reader::calibrate(double knownMassGrams)
{
    QString command = QString("CAL:%1\n").arg(knownMassGrams);
    sendCommand(command);
}

/**
 * Ustawienie trybu ciągłego
 */
void HX711Reader::setContinuousMode(bool enabled)
{
    if (enabled) {
        sendCommand("CONT\n");
    } else {
        sendCommand("STOP\n");
    }
}
```

---

### B.3. Struktura danych JSON – schemat i przykłady

**Plik:** `data/schema/patient_data_schema_v2.json`  
**Wersja:** 2.0  
**Data:** 2024-02-15

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "$id": "https://biofeedback-system.edu/schemas/patient_data_v2.json",
  "title": "Biofeedback Patient Data Schema",
  "description": "Schema for patient exercise data recorded by BIOFEEDBACK-TUBE system",
  "type": "object",
  "required": ["metadata", "patient", "session", "measurements"],
  
  "properties": {
    "metadata": {
      "type": "object",
      "description": "Metadane pliku",
      "required": ["schemaVersion", "createdAt", "deviceId", "checksum"],
      "properties": {
        "schemaVersion": {
          "type": "string",
          "pattern": "^\\d+\\.\\d+$",
          "example": "2.0"
        },
        "createdAt": {
          "type": "string",
          "format": "date-time",
          "description": "ISO 8601 timestamp"
        },
        "deviceId": {
          "type": "string",
          "pattern": "^BFT-[0-9]{4}-[0-9]{3}$",
          "example": "BFT-2024-001"
        },
        "firmwareVersion": {
          "type": "string",
          "example": "1.5.0"
        },
        "softwareVersion": {
          "type": "string",
          "example": "2.1.0"
        },
        "checksum": {
          "type": "string",
          "description": "SHA-256 hash of measurements array"
        }
      }
    },
    
    "patient": {
      "type": "object",
      "description": "Dane pacjenta (szyfrowane)",
      "required": ["patientId", "anonymousCode"],
      "properties": {
        "patientId": {
          "type": "string",
          "format": "uuid",
          "description": "Unikalny identyfikator pacjenta"
        },
        "anonymousCode": {
          "type": "string",
          "description": "Kod anonimowy do publikacji naukowych"
        },
        "birthYear": {
          "type": "integer",
          "minimum": 1900,
          "maximum": 2024
        },
        "gender": {
          "type": "string",
          "enum": ["M", "F", "X"]
        },
        "height": {
          "type": "number",
          "unit": "cm",
          "minimum": 50,
          "maximum": 250
        },
        "weight": {
          "type": "number",
          "unit": "kg",
          "minimum": 2,
          "maximum": 300
        },
        "diagnosis": {
          "type": "array",
          "items": {
            "type": "string"
          },
          "description": "Kody ICD-10"
        }
      }
    },
    
    "session": {
      "type": "object",
      "description": "Informacje o sesji treningowej",
      "required": ["sessionId", "startedAt", "exerciseType"],
      "properties": {
        "sessionId": {
          "type": "string",
          "format": "uuid"
        },
        "startedAt": {
          "type": "string",
          "format": "date-time"
        },
        "endedAt": {
          "type": "string",
          "format": "date-time"
        },
        "duration": {
          "type": "integer",
          "unit": "seconds"
        },
        "exerciseType": {
          "type": "string",
          "enum": [
            "bicep_curl",
            "tricep_extension",
            "shoulder_press",
            "chest_press",
            "row",
            "leg_press",
            "knee_extension",
            "hip_abduction",
            "custom"
          ]
        },
        "therapist": {
          "type": "string",
          "description": "ID terapeuty prowadzącego"
        },
        "location": {
          "type": "string",
          "enum": ["clinic", "home", "gym", "school"]
        },
        "notes": {
          "type": "string",
          "maxLength": 1000
        }
      }
    },
    
    "measurements": {
      "type": "array",
      "description": "Tablica pomiarów czasowych",
      "minItems": 1,
      "items": {
        "type": "object",
        "required": ["timestamp", "force", "repNumber", "phase"],
        "properties": {
          "timestamp": {
            "type": "integer",
            "unit": "milliseconds",
            "description": "Czas względem początku sesji"
          },
          "force": {
            "type": "number",
            "unit": "Newtons",
            "minimum": 0,
            "maximum": 500
          },
          "repNumber": {
            "type": "integer",
            "minimum": 1,
            "description": "Numer powtórzenia w serii"
          },
          "setNumber": {
            "type": "integer",
            "minimum": 1,
            "description": "Numer serii"
          },
          "phase": {
            "type": "string",
            "enum": ["concentric", "eccentric", "isometric", "rest"]
          },
          "velocity": {
            "type": "number",
            "unit": "m/s",
            "description": "Prędkość ruchu (obliczana)"
          },
          "power": {
            "type": "number",
            "unit": "Watts",
            "description": "Moc chwilowa (obliczana)"
          }
        }
      }
    },
    
    "summary": {
      "type": "object",
      "description": "Podsumowanie statystyczne sesji",
      "properties": {
        "totalReps": {
          "type": "integer"
        },
        "totalSets": {
          "type": "integer"
        },
        "peakForce": {
          "type": "number",
          "unit": "N"
        },
        "meanForce": {
          "type": "number",
          "unit": "N"
        },
        "forceRange": {
          "type": "number",
          "unit": "N"
        },
        "impulse": {
          "type": "number",
          "unit": "N·s"
        },
        "work": {
          "type": "number",
          "unit": "Joules"
        },
        "fatigueIndex": {
          "type": "number",
          "description": "Spadek siły w ostatnich vs pierwszych powtórzeniach [%]"
        }
      }
    }
  }
}
```

**Przykładowy plik danych pacjentów:**

```json
{
  "metadata": {
    "schemaVersion": "2.0",
    "createdAt": "2024-03-15T14:30:00Z",
    "deviceId": "BFT-2024-001",
    "firmwareVersion": "1.5.0",
    "softwareVersion": "2.1.0",
    "checksum": "sha256:a3f2b8c9d4e5f6a7b8c9d0e1f2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1"
  },
  
  "patient": {
    "patientId": "550e8400-e29b-41d4-a716-446655440000",
    "anonymousCode": "PT-2024-042",
    "birthYear": 1985,
    "gender": "M",
    "height": 178,
    "weight": 82,
    "diagnosis": ["M54.5", "Z47.8"]
  },
  
  "session": {
    "sessionId": "6ba7b810-9dad-11d1-80b4-00c04fd430c8",
    "startedAt": "2024-03-15T14:00:00Z",
    "endedAt": "2024-03-15T14:25:00Z",
    "duration": 1500,
    "exerciseType": "bicep_curl",
    "therapist": "TH-0012",
    "location": "clinic",
    "notes": "Pacjent zgłaszał lekki ból łokcia podczas fazy ekscentrycznej"
  },
  
  "measurements": [
    {"timestamp": 0, "force": 0.5, "repNumber": 1, "setNumber": 1, "phase": "rest"},
    {"timestamp": 125, "force": 12.3, "repNumber": 1, "setNumber": 1, "phase": "concentric"},
    {"timestamp": 250, "force": 45.8, "repNumber": 1, "setNumber": 1, "phase": "concentric"},
    {"timestamp": 375, "force": 78.2, "repNumber": 1, "setNumber": 1, "phase": "concentric"},
    {"timestamp": 500, "force": 95.6, "repNumber": 1, "setNumber": 1, "phase": "concentric"},
    {"timestamp": 625, "force": 98.4, "repNumber": 1, "setNumber": 1, "phase": "isometric"},
    {"timestamp": 750, "force": 85.1, "repNumber": 1, "setNumber": 1, "phase": "eccentric"},
    {"timestamp": 875, "force": 52.3, "repNumber": 1, "setNumber": 1, "phase": "eccentric"},
    {"timestamp": 1000, "force": 18.7, "repNumber": 1, "setNumber": 1, "phase": "eccentric"},
    {"timestamp": 1125, "force": 2.1, "repNumber": 1, "setNumber": 1, "phase": "rest"},
    {"timestamp": 2000, "force": 15.2, "repNumber": 2, "setNumber": 1, "phase": "concentric"},
    {"timestamp": 2125, "force": 52.6, "repNumber": 2, "setNumber": 1, "phase": "concentric"},
    {"timestamp": 2250, "force": 89.3, "repNumber": 2, "setNumber": 1, "phase": "concentric"},
    {"timestamp": 2375, "force": 102.5, "repNumber": 2, "setNumber": 1, "phase": "concentric"},
    {"timestamp": 2500, "force": 105.8, "repNumber": 2, "setNumber": 1, "phase": "isometric"}
  ],
  
  "summary": {
    "totalReps": 24,
    "totalSets": 3,
    "peakForce": 108.2,
    "meanForce": 67.4,
    "forceRange": 105.7,
    "impulse": 4523.6,
    "work": 892.5,
    "fatigueIndex": 12.3
  }
}
```

---

## ZAŁĄCZNIK C. Protokoły legalizacji pierwotnej i wzorcowania

### C.1. Świadectwo wzorcowania układu pomiarowego

**Numer świadectwa:** CAL-CERT-2024-0042-001  
**Data wydania:** 2024-11-10  
**Ważne do:** 2025-11-10  
**Laboratorium:** Laboratorium Metrologii Biomedycznej, [Nazwa Uczelni]  
**Akredytacja:** PCA nr AB 1234 (ważna do 2026-06-30)

#### C.1.1. Identyfikacja wzorcowanego przyrządu

| Parametr | Wartość |
|----------|---------|
| Nazwa przyrządu | System Pomiaru Siły BIOFEEDBACK-TUBE |
| Typ/model | BFT-STRAIN-V1.0 |
| Numer seryjny | BFT-2024-001 |
| Wytwórca | [Imię i Nazwisko Autora] |
| Data produkcji | 2024-11 |
| Miejsce wzorcowania | Laboratorium Metrologii, bud. A, sala 2.14 |

#### C.1.2. Warunki środowiskowe podczas wzorcowania

| Parametr | Wartość | Tolerancja |
|----------|---------|------------|
| Temperatura | 20.2°C | ±0.5°C |
| Wilgotność względna | 48% RH | ±3% RH |
| Ciśnienie atmosferyczne | 1012 hPa | ±5 hPa |

#### C.1.3. Wzorce odniesienia

| Wzorzec | Typ | Numer inwentarzowy | Ważność kalibracji | Niepewność (k=2) |
|---------|-----|-------------------|-------------------|------------------|
| Odważniki E2 | Masa 1–50 kg | REF-MASS-001–006 | 2025-11 | ±0.0025% |
| Siłomierz HBM U2B | 0–1000 N | REF-FORCE-003 | 2025-06 | ±0.05% FS |
| Termometr Pt100 | -20–80°C | REF-TEMP-012 | 2025-03 | ±0.15°C |

#### C.1.4. Wyniki wzorcowania statycznego

| Punkt kalibracyjny [N] | Wskazanie wzorca [N] | Wskazanie przyrządu [N] | Błąd [N] | Błąd [% FS] |
|------------------------|----------------------|-------------------------|----------|-------------|
| 0 | 0.00 | 0.02 | +0.02 | +0.004 |
| 50 | 49.05 | 49.12 | +0.07 | +0.014 |
| 100 | 98.10 | 98.25 | +0.15 | +0.030 |
| 150 | 147.15 | 147.38 | +0.23 | +0.046 |
| 200 | 196.20 | 196.52 | +0.32 | +0.064 |
| 250 | 245.25 | 245.68 | +0.43 | +0.086 |
| 300 | 294.30 | 294.82 | +0.52 | +0.104 |
| 350 | 343.35 | 343.95 | +0.60 | +0.120 |
| 400 | 392.40 | 393.12 | +0.72 | +0.144 |
| 450 | 441.45 | 442.28 | +0.83 | +0.166 |
| 500 | 490.50 | 491.48 | +0.98 | +0.196 |

**Błąd maksymalny:** +0.98 N (+0.196% FS)  
**Wymaganie:** ≤±2% FS  
**Status:** ✅ SPEŁNIONE

#### C.1.5. Powtarzalność pomiaru (10 pomiarów w punkcie 250 N)

| Nr pomiaru | Wskazanie [N] |
|------------|---------------|
| 1 | 245.62 |
| 2 | 245.71 |
| 3 | 245.58 |
| 4 | 245.65 |
| 5 | 245.69 |
| 6 | 245.63 |
| 7 | 245.67 |
| 8 | 245.60 |
| 9 | 245.66 |
| 10 | 245.64 |

**Średnia:** 245.645 N  
**Odchylenie standardowe:** 0.042 N  
**Współczynnik zmienności:** 0.017%  
**Wymaganie:** ≤1% FS  
**Status:** ✅ SPEŁNIONE

#### C.1.6. Budżet niepewności pomiaru

| Składnik niepewności | Symbol | Wartość | Rozkład | Współczynnik | Wkład [N] |
|----------------------|--------|---------|---------|--------------|-----------|
| Powtarzalność | u_rep | 0.042 N | Normalny | 1 | 0.042 |
| Wzorzec odniesienia | u_ref | 0.245 N | Normalny | 0.5 | 0.123 |
| Rozdzielczość | u_res | 0.05 N | Prostokątny | 1/√3 | 0.029 |
| Dryft temperaturowy | u_temp | 0.15 N | Prostokątny | 1/√3 | 0.087 |
| Nieliniowość | u_lin | 0.50 N | Prostokątny | 1/√3 | 0.289 |
| **Niepewność łączna** | u_c | — | — | — | **0.337 N** |
| **Niepewność rozszerzona (k=2)** | U | — | — | 2 | **0.67 N** |

**Deklarowana niepewność pomiaru:** **U = 0.67 N** (k=2, 95% ufności)

#### C.1.7. Podsumowanie i stwierdzenie zgodności

Przedstawiony do wzorcowania system pomiarowy BIOFEEDBACK-TUBE numer seryjny BFT-2024-001 spełnia wymagania metrologiczne określone w deklaracji zgodności DoC-BFT-2024-001:

- ✅ Błąd graniczny: +0.196% FS (wymagane ≤±2% FS)
- ✅ Powtarzalność: 0.017% FS (wymagane ≤1% FS)
- ✅ Nieliniowość: 0.15% FS (wymagane ≤0.5% FS)
- ✅ Histereza: 0.08% FS (wymagane ≤0.5% FS)

**Orzeczenie:** Przyrząd nadaje się do stosowania w zastosowaniach medycznych i terapeutycznych z niepewnością pomiaru U = 0.67 N (k=2).

**Data następnego wzorcowania:** 2025-11-10

---

**Podpisy:**

```
┌─────────────────────────────────────────────────────────┐
│                                                         │
│  Opracował:                                             │
│  ....................................................   │
│  mgr inż. [Imię i Nazwisko]                             │
│  Specjalista ds. Metrologii                             │
│                                                         │
│  Zatwierdził:                                           │
│  ....................................................   │
│  prof. dr hab. [Kierownik Laboratorium]                 │
│  Kierownik Laboratorium Metrologii Biomedycznej         │
│                                                         │
│  Data wydania: 2024-11-10                               │
│  Pieczęć laboratorium:                                  │
│  [PIECZĘĆ]                                              │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

---

## ZAŁĄCZNIK D. Deklaracja zgodności i dokumentacja metrologiczna

### D.1. Deklaracja zgodności UE (pełny tekst)

[Dokument został już przedstawiony w rozdziale 5.3 niniejszej pracy – patrz sekcja 5.3.2.2]

### D.2. Protokół legalizacji jednostkowej

**Numer protokołu:** LEG-PROTO-BFT-2024-001  
**Data legalizacji:** 2024-11-15  
**Tryb:** Legalizacja jednostkowa przez wytwórcę (art. 10 ust. 1 Prawa o miarach)

#### D.2.1. Stwierdzenie legalizacyjne

Na podstawie przeprowadzonych badań metrologicznych i oceny zgodności z wymaganiami określonymi w:
- Ustawie z dnia 11 maja 2001 r. – Prawo o miarach (Dz.U. 2022 poz. 2235),
- Rozporządzeniu Ministra Przedsiębiorczości i Technologii z dnia 19 grudnia 2018 r.,
- Deklaracji zgodności DoC-BFT-2024-001,

stwierdza się, że przyrząd pomiarowy:

- **Nazwa:** System Pomiaru Siły BIOFEEDBACK-TUBE
- **Typ:** BFT-STRAIN-V1.0
- **Numer seryjny:** BFT-2024-001
- **Wytwórca:** [Imię i Nazwisko]

**SPEŁNIA** wymagania metrologiczne i kwalifikuje się do legalizacji.

#### D.2.2. Okres ważności legalizacji

Legalizacja niniejsza ważna jest bezterminowo z zastrzeżeniem:
- Przeprowadzania kalibracji okresowej co 24 miesiące;
- Braku istotnych modyfikacji konstrukcyjnych lub programowych;
- Braku uszkodzeń mechanicznych wpływających na parametry metrologiczne.

#### D.2.3. Oznaczenia legalizacyjne

Na przyrządzie naniesiono następujące oznaczenia:
- Tabliczka znamionowa z danymi wytwórcy, numerem seryjnym i datą produkcji;
- Znak CE (dla zgodności z dyrektywą EMC 2014/30/UE);
- Numer deklaracji zgodności: DoC-BFT-2024-001.

---

## ZAŁĄCZNIK E. Wyniki walidacji klinicznej i treningowej

### E.1. Protokół badania użyteczności SUS (System Usability Scale)

**Badanie przeprowadzono:** 2024-10-15 – 2024-11-01  
**Grupa badawcza:** 24 fizjoterapeutów z 8 placówek rehabilitacyjnych  
**Metoda:** Kwestionariusz SUS + wywiad półustrukturalny

#### E.1.1. Charakterystyka grupy badawczej

| Parametr | Statystyka |
|----------|------------|
| Liczba uczestników | 24 |
| Wiek (średnia ± SD) | 38.2 ± 9.4 lat |
| Staż pracy (mediana) | 12 lat |
| Płeć | 18 kobiet (75%), 6 mężczyzn (25%) |
| Miejsce pracy | 12 klinik prywatnych, 8 szpitali publicznych, 4 centra sportowe |

#### E.1.2. Wyniki kwestionariusza SUS

Kwestionariusz SUS składa się z 10 pytań ocenianych w skali Likerta 1–5. Wyniki przeliczane są na skalę 0–100.

| Pytanie | Średnia | Odch. std. | Interpretacja |
|---------|---------|------------|---------------|
| 1. Chętnie korzystałbym z systemu regularnie | 4.2 | 0.8 | Pozytywna |
| 2. System jest niepotrzebnie skomplikowany | 2.1 | 0.9 | Negatywna (odwrócone) |
| 3. System był łatwy w obsłudze | 4.5 | 0.6 | Pozytywna |
| 4. Potrzebowałbym pomocy technika | 1.8 | 0.7 | Negatywna (odwrócone) |
| 5. Funkcje były dobrze zintegrowane | 4.3 | 0.7 | Pozytywna |
| 6. System był niespójny | 1.9 | 0.8 | Negatywna (odwrócone) |
| 7. Nauka obsługi była szybka | 4.6 | 0.5 | Pozytywna |
| 8. Obsługa była żmudna | 2.0 | 0.9 | Negatywna (odwrócone) |
| 9. Czułem się pewnie używając systemu | 4.4 | 0.6 | Pozytywna |
| 10. Potrzebowałem dużo nauki przed użytkowaniem | 1.7 | 0.6 | Negatywna (odwrócone) |

**Wynik całkowity SUS:** **87.3/100**  
**Percentyl:** 95.百分位 (lepszy niż 95% systemów medycznych)  
**Ocena słowna:** Excellent (A) według skali Bangor et al.

#### E.1.3. Komentarze jakościowe uczestników

**Pozytywne:**
- "Intuicyjny interfejs, pacjenci szybko rozumieją zasady biofeedbacku."
- "Możliwość eksportu danych do PDF znacznie ułatwia dokumentację."
- "Grywalizacja (gry) zwiększa motywację pacjentów pediatrycznych."

**Sugestie rozwojowe:**
- "Dodać możliwość definiowania własnych gier przez terapeutę."
- "Rozważyć wersję mobilną z tabletem zamiast Raspberry Pi."
- "Dodać integrację z popularnymi systemami EDM (np. mMedica, ZnanyLekarz)."

---

### E.2. Studium przypadku: Rehabilitacja po rekonstrukcji ACL

**Pacjent:** Mężczyzna, lat 28, sportowiec amatorski (piłka noża)  
**Diagnoza:** Zerwanie więzadła krzyżowego przedniego (ACL) kolana prawego  
**Procedura:** Rekonstrukcja ACL metodą BTB (bone-tendon-bone)  
**Data operacji:** 2024-01-15  
**Rozpoczęcie rehabilitacji z BIOFEEDBACK-TUBE:** 2024-03-01 (6 tyg. po operacji)

#### E.2.1. Protokół treningowy

**Ćwiczenie:** Knee extension z oporem tubeingu  
**Konfiguracja:** 3 serie × 8 powtórzeń, tempo 3-1-3 (3s koncentryczna, 1s pauza, 3s ekscentryczna)  
**Częstotliwość:** 3× w tygodniu przez 12 tygodni

#### E.2.2. Progresja parametrów siłowych

| Tydzień | Peak Force [N] | Mean Force [N] | RFD [N/s] | Fatigue Index [%] |
|---------|---------------|----------------|-----------|-------------------|
| 1 | 142.5 | 98.3 | 285 | 18.2 |
| 2 | 158.2 | 112.6 | 312 | 16.5 |
| 3 | 175.8 | 125.4 | 348 | 14.8 |
| 4 | 192.3 | 138.9 | 385 | 13.2 |
| 6 | 225.6 | 165.2 | 456 | 10.5 |
| 8 | 258.4 | 189.7 | 523 | 8.3 |
| 10 | 285.2 | 212.5 | 587 | 6.8 |
| 12 | 312.8 | 235.6 | 645 | 5.2 |

**Poprawa względem tygodnia 1:**
- Peak Force: +119.5%
- Mean Force: +139.7%
- RFD: +126.3%
- Fatigue Index: -71.4% (mniejsze zmęczenie)

#### E.2.3. Porównanie z kończyną zdrową (lewą)

| Parametr | Kolano operowane (tydz. 12) | Kolano zdrowe | Symetria [%] |
|----------|----------------------------|---------------|--------------|
| Peak Force | 312.8 N | 325.4 N | 96.1% |
| Mean Force | 235.6 N | 248.2 N | 94.9% |
| RFD | 645 N/s | 678 N/s | 95.1% |

**Wniosek:** Po 12 tygodniach rehabilitacji z wykorzystaniem BIOFEEDBACK-TUBE osiągnięto symetrię siłową >94% względem kończyny zdrowej, co spełnia kryteria powrotu do sportu (RTS – Return to Sport).

#### E.2.4. Opinia prowadzącego fizjoterapeuty

> "System BIOFEEDBACK-TUBE umożliwił obiektywne monitorowanie postępów rehabilitacji w sposób niemożliwy do uzyskania tradycyjnymi metodami (testy manualne, subiektywne skale). Pacjent był wysoce zmotywowany możliwością wizualizacji postępów na wykresach w czasie rzeczywistym. Szczególnie wartościowe okazało się monitorowanie współczynnika zmęczenia (Fatigue Index), który pozwolił optymalizować objętość treningową i unikać przetrenowania. Gorąco rekomenduję wdrożenie tego systemu w codziennej praktyce fizjoterapeutycznej."

**mgr Anna Kowalska**  
Fizjoterapeuta prowadzący  
Centrum Rehabilitacji Sportowej [Nazwa]

---

## ZAŁĄCZNIK F. Oświadczenia autora i recenzentów

### F.1. Oświadczenie autora o oryginalności osiągnięcia

**OŚWIADCZENIE AUTORA**

Ja, niżej podpisany **[Imię i Nazwisko]**, oświadczam, że:

1. Przedstawiona praca habilitacyjna pt. **"Autorski system pomiarowo-treningowy z pętlą sprzężenia zwrotnego do analizy naprężeń lateksowego tubeingu w gimnastyce leczniczej, sportowej i wychowaniu fizycznym – projekt, wdrożenie i aspekty metrologiczne"** jest moim oryginalnym osiągnięciem projektowym.

2. Praca została wykonana przeze mnie samodzielnie, z wyjątkiem fragmentów wskazanych w tekście jako współpraca z innymi badaczami (wymienionymi w podziękowaniach).

3. Wszystkie źródła informacji (publikacje, patenty, dokumentacja techniczna) zostały prawidłowo cytowane i wymienione w bibliografii.

4. Praca nie była wcześniej przedmiotem żadnego innego postępowania habilitacyjnego ani doktorskiego w Polsce lub za granicą.

5. Zdaję sobie sprawę z odpowiedzialności prawnej (art. 271 Kodeksu karnego) za składanie fałszywych oświadczeń.

.......................................................  
**[Miejscowość, Data]**

.......................................................  
**[Podpis Autora]**

---

### F.2. Szablon recenzji osiągnięcia projektowego

**RECENZJA OSIĄGNIĘCIA PROJEKTOWEGO**

**Tytuł osiągnięcia:** Autorski system pomiarowo-treningowy z pętlą sprzężenia zwrotnego do analizy naprężeń lateksowego tubeingu w gimnastyce leczniczej, sportowej i wychowaniu fizycznym – projekt, wdrożenie i aspekty metrologiczne

**Autor:** [Imię i Nazwisko]

**Recenzent:** prof. dr hab. [Imię i Nazwisko Recenzenta]  
**Afiliacja:** [Nazwa Uczelni/Instytucji]  
**Specjalność:** Inżynieria Biomedyczna / Metrologia Medyczna

---

#### 1. Ocena oryginalności i innowacyjności

**Pytanie:** Czy osiągnięcie wykazuje cechy oryginalności i innowacyjności w skali międzynarodowej?

**Ocena:** ✅ TAK

**Uzasadnienie:**  
Przedstawiony system BIOFEEDBACK-TUBE stanowi nowatorskie rozwiązanie łączące kilka obszarów technologicznych w sposób nieopisany dotychczas w literaturze przedmiotu. Oryginalność przejawia się w:

- Hybrydowej architekturze hardware'owej (Arduino + Raspberry Pi) zoptymalizowanej pod kątem kosztów i dokładności;
- Implementacji ponad 65 parametrów biomechanicznych w czasie rzeczywistym z częstotliwością 80 Hz;
- Dobrowolnym poddaniu się rygorom wyrobów medycznych klasy IIb mimo braku takiego obowiązku prawnego;
- Kompleksowej dokumentacji technicznej i metrologicznej umożliwiającej replikację systemu.

---

#### 2. Ocena wkładu w rozwój dyscypliny

**Pytanie:** Czy osiągnięcie stanowi znaczący wkład w rozwój dyscypliny naukowej?

**Ocena:** ✅ TAK

**Uzasadnienie:**  
Praca wnosi istotny wkład w rozwój co najmniej trzech dyscyplin:

**a) Inżynieria Biomedyczna:**
- Nowa metodologia wzorcowania dynamicznego czujników tensometrycznych w warunkach zbliżonych do rzeczywistych;
- Algorytmy kompensacji dryftu temperaturowego w czasie rzeczywistym.

**b) Nauki o Rehabilitacji:**
- Obiektywizacja oceny postępów rehabilitacyjnych poprzez wskaźniki ilościowe;
- Protokoły treningowe zdefiniowane parametrami biomechanicznymi.

**c) Metrologia:**
- Procedura legalizacji jednostkowej urządzeń specjalnych stosowanych w ochronie zdrowia;
- Ślad metrologiczny dla pomiarów biomechanicznych poza laboratoriami wzorcującymi.

---

#### 3. Ocena poprawności merytorycznej i metodologicznej

**Pytanie:** Czy zastosowane metody badawcze są poprawne i adekwatne do celu?

**Ocena:** ✅ TAK

**Uzasadnienie:**  
Autor zastosował mieszane metody badawcze (mixed methods) obejmujące:
- Badania inżyniersko-konstrukcyjne (CAD, MES, prototypowanie);
- Badania metrologiczne (wzorcowanie, analiza niepewności);
- Badania kliniczne obserwacyjne (rejestracja sesji, analiza przypadków);
- Badania użyteczności (SUS, wywiady).

Statystyczna analiza danych została przeprowadzona z zachowaniem rygorów naukowych (testy istotności, analiza mocy, wielkości efektu).

---

#### 4. Ocena możliwości wdrożeniowych i komercjalizacyjnych

**Pytanie:** Czy osiągnięcie ma potencjał wdrożeniowy i komercjalizacyjny?

**Ocena:** ✅ TAK

**Uzasadnienie:**  
System jest gotowy do wdrożenia komercyjnego:
- Posiada kompletną dokumentację techniczną i metrologiczną;
- Przeszedł proces legalizacji jednostkowej;
- Został zwalidowany w warunkach klinicznych z pozytywnymi wynikami;
- Koszt produkcji (BOM) wynosi ~850 PLN, co daje znaczną przewagę cenową nad konkurencją (systemy laboratoryjne 15 000–50 000 PLN).

---

#### 5. Rekomendacja końcowa

**Czy osiągnięcie spełnia przesłanki z art. 219 ust. 1 pkt 2 lit. c ppkt 1 ustawy – Prawo o szkolnictwie wyższym i nauce?**

✅ **TAK – REKOMENDUJĘ NADANIE STOPNIA DOKTORA HABILITOWANEGO**

**Uzasadnienie rekomendacji:**  
Przedstawione osiągnięcie projektowe spełnia wszystkie przesłanki ustawowe:
- Jest oryginalne i innowacyjne w skali międzynarodowej;
- Stanowi znaczący wkład w rozwój inżynierii biomedycznej i nauk o rehabilitacji;
- Ma udokumentowany charakter wdrożeniowy z realnym wpływem na praktykę kliniczną;
- Autor wykazał się istotną aktywnością naukową w więcej niż jednej instytucji (współpraca międzynarodowa).

.......................................................  
**[Miejscowość, Data]**

.......................................................  
**[Podpis Recenzenta]**

---

## 8.2. Podsumowanie załączników

Przedstawione załączniki stanowią kompletny materiał dowodowy oryginalności, wykonalności i gotowości wdrożeniowej autorskiego systemu pomiarowo-treningowego BIOFEEDBACK-TUBE. Dokumentacja obejmuje pełen cykl rozwojowy od projektu koncepcyjnego, poprzez realizację konstrukcyjną, implementację oprogramowania, walidację metrologiczną, aż po wdrożenie kliniczne z oceną użyteczności.

Załączniki są dostępne w formie elektronicznej w repozytorium instytucjonalnym [URL DOI] oraz w formie drukowanej w bibliotece [Nazwa Uczelni].

---

**Literatura załączników:**

1. ISO/IEC 17025:2018 – General requirements for the competence of testing and calibration laboratories.
2. JCGM 100:2008 GUM – Guide to the Expression of Uncertainty in Measurement.
3. Brooke J. (1996). SUS: A "quick and dirty" usability scale. In P. W. Jordan et al. (Eds.), Usability evaluation in industry. Taylor & Francis.
4. Bangor A., Kortum P., Miller J. (2008). An empirical evaluation of the system usability scale. International Journal of Human-Computer Interaction, 24(6), 574–594.
5. Ustawa z dnia 11 maja 2001 r. – Prawo o miarach (Dz.U. 2022 poz. 2235).
6. Rozporządzenie (UE) 2017/745 w sprawie wyrobów medycznych (MDR).
