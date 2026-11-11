# Rozdział 1.3: Systemy wbudowane i rejestracji danych w zastosowaniach fizjoterapeutycznych i treningowych

## 1.3.1 Wprowadzenie do systemów wbudowanych w rehabilitacji i sporcie

### 1.3.1.1 Definicja i charakterystyka systemów wbudowanych

Systemy wbudowane (ang. *embedded systems*) stanowią wyspecjalizowane układy komputerowe zaprojektowane do wykonywania określonych zadań w ramach większego systemu technicznego lub urządzenia. W kontekście fizjoterapii, rehabilitacji ruchowej i treningu sportowego, systemy te pełnią funkcję kluczowych modułów akwizycji, przetwarzania i wizualizacji danych biomechanicznych w czasie rzeczywistym. Zgodnie z definicją IEEE, system wbudowany charakteryzuje się ograniczonymi zasobami obliczeniowymi, deterministycznym czasem reakcji oraz wysokim stopniem integracji z otoczeniem fizycznym poprzez sensory i aktuatory.

W zastosowaniach medycznych i treningowych systemy wbudowane muszą spełniać rygorystyczne wymagania dotyczące:
- **Niezawodności operacyjnej** – ciągłość działania przez extended periods bez awarii
- **Bezpieczeństwa użytkownika** – izolacja galwaniczna, ochrona przed porażeniem elektrycznym
- **Precyzji pomiarowej** – zdolność do rejestracji subtelnych zmian parametrów biomechanicznych
- **Czasu rzeczywistego** – gwarancja opóźnień poniżej progów percepcyjnych człowieka (<100 ms)
- **Łatwości obsługi** – interfejsy dostosowane do personelu medycznego i pacjentów

### 1.3.1.2 Ewolucja systemów pomiarowych w fizjoterapii

Historycznie, pomiary biomechaniczne w rehabilitacji opierały się na metodach subiektywnych (skale obserwacyjne, testy manualne) lub громоздkich systemach laboratoryjnych (platformy siłowe, systemy motion capture). Przełom nastąpił wraz z miniaturyzacją elektroniki i pojawieniem się mikrokontrolerów o niskim poborze mocy, co umożliwiło budowę przenośnych, kosztowo efektywnych systemów wbudowanych dedykowanych dla gabinetów fizjoterapii i sal treningowych.

Współczesne systemy wbudowane w fizjoterapii można sklasyfikować według:
1. **Lokalizacji pomiaru**: systemy stacjonarne vs. wearable devices
2. **Rodzaju sygnału**: EMG (elektromiografia), MMG (mechanomiografia), tensometria, akcelerometria, żyroskopia
3. **Architektury przetwarzania**: centralized (single-unit) vs. distributed (multi-node)
4. **Stopnia autonomii**: standalone devices vs. networked systems z cloud integration

## 1.3.2 Architektura sprzętowa systemów akwizycji danych

### 1.3.2.1 Hierarchia warstw sprzętowych

Typowy system wbudowany do analizy napięć taśm lateksowych w gimnastyce leczniczej składa się z trzech zasadniczych warstw sprzętowych:

**Warstwa 1: Sensory pierwotne (Primary Sensors)**
- Czujniki tensometryczne (strain gauges) w konfiguracji mostka Wheatstone'a
- Przetworniki analogowo-cyfrowe (ADC) o wysokiej rozdzielczości (24-bit HX711)
- Układy warunkowania sygnału: wzmacniacze instrumentalne, filtry dolnoprzepustowe

**Warstwa 2: Mikrokontrolery akwizycji (Acquisition Microcontrollers)**
- Arduino Nano / Uno jako lokalne jednostki próbkowania
- Raspberry Pi Pico z architekturą ARM Cortex-M0+
- FPGA dla aplikacji wymagających równoległego przetwarzania wielu kanałów

**Warstwa 3: Jednostki nadrzędne (Host Systems)**
- Raspberry Pi 4 jako komputer jednopłytkowy klasy industrialnej
- Embedded PC z systemami Windows IoT lub Linux embedded
- Urządzenia mobilne (tablety) z interfejsem Bluetooth Low Energy

### 1.3.2.2 Specyfikacja techniczna czujników tensometrycznych

Czujniki tensometryczne stanowią fundament systemu pomiarowego, przekształcając odkształcenia mechaniczne taśmy lateksowej na sygnały elektryczne. Kluczowe parametry metrologiczne obejmują:

| Parametr | Symbol | Wartość typowa | Jednostka |
|----------|--------|----------------|-----------|
| Czułość nominalna | $S$ | 2.0 ± 0.5% | mV/V |
| Rezystancja mostka | $R_{bridge}$ | 350 | Ω |
| Zakres pomiarowy | $F_{range}$ | 0–500 | N |
| Nieliniowość | $NL$ | < 0.05 | % FS |
| Histereza | $H$ | < 0.05 | % FS |
| Pełzanie | $Cr$ | < 0.05 | % FS/30 min |
| Klasa ochrony | IP | IP65 | - |
| Zakres temperaturowy | $T_{op}$ | -20 do +80 | °C |

Mostek Wheatstone'a w konfiguracji czteroelementowej zapewnia kompensację temperaturową i zwiększa czułość układu. Napięcie wyjściowe mostka opisuje zależność:

$$V_{out} = V_{ex} \cdot \frac{\Delta R}{R} = V_{ex} \cdot GF \cdot \varepsilon$$

gdzie:
- $V_{ex}$ – napięcie wzbudzenia mostka (typowo 5V DC)
- $\Delta R$ – zmiana rezystancji czujnika
- $R$ – rezystancja nominalna czujnika
- $GF$ – współczynnik tensometryczny (Gauge Factor, ~2.0 dla metalowych folii)
- $\varepsilon$ – odkształcenie względne ($\Delta L / L$)

### 1.3.2.3 Przetworniki analogowo-cyfrowe wysokiej rozdzielczości

Do precyzyjnej konwersji sygnałów tensometrycznych niezbędne są przetworniki ADC o parametrach:

- **Rozdzielczość**: minimum 24 bity dla detekcji zmian rzędu 0.1 N
- **Częstotliwość próbkowania**: 10–80 Hz wystarcza dla ruchu ludzkiego
- **CMRR (Common Mode Rejection Ratio)**: > 100 dB dla eliminacji zakłóceń sieciowych
- **PGA (Programmable Gain Amplifier)**: wzmocnienie 32–128x

Układ HX711 (firma Avia Semiconductor) reprezentuje standard branżowy w aplikacjach wagowych i tensometrycznych, integrując:
- 24-bitowy ADC z sigma-delta modulacją
- Dwa kanały różnicowe z programowalnym wzmocnieniem
- Wewnętrzny oscylator 4.5 MHz z synchronizacją cykli konwersji
- Interfejs szeregowy kompatybilny z mikrokontrolerami 3.3V i 5V

## 1.3.3 Platformy mikrokontrolerowe w systemach biofeedback

### 1.3.3.1 Rodzina Arduino jako moduły akwizycji

Mikrokontrolery Arduino (ATmega328P, ATmega2560) dominują w aplikacjach edukacyjnych i prototypowych ze względu na:
- **Otwartą architekturę hardware/software** – dostępność bibliotek i społeczności
- **Łatwość programowania** – IDE z abstrakcją niskopoziomowych rejestrów
- **Bogaty ekosystem shieldów** – moduły rozszerzeń funkcjonalnych
- **Niski koszt** – cena jednostkowa < 15 PLN dla klonów

W systemie pomiarowo-treningowym Arduino Nano pełni rolę **węzła brzegowego (edge node)** odpowiedzialnego za:
1. Cykliczne odczyty z przetwornika HX711 (80 Hz w trybie continuous)
2. Wstępną filtrację cyfrową (moving average, median filter)
3. Pakietowanie danych binarnych z CRC-8 dla integralności transmisji
4. Komunikację szeregową UART z jednostką nadrzędną (115200 baud)

Protokół komunikacyjny Arduino↔Raspberry Pi definiuje strukturę ramki danych:

```cpp
struct DataPacket {
    uint32_t timestamp;  // 4 bajty - czas w milisekundach
    int32_t force_raw;   // 4 bajty - surowa wartość z ADC
    uint8_t crc8;        // 1 bajt - suma kontrolna
};
// Ramka zakończona markerami: 0xAA 0x55
```

### 1.3.3.2 Raspberry Pi jako jednostka nadrzędna

Komputer jednopłytkowy Raspberry Pi 4 Model B reprezentuje klasę **embedded SBC (Single Board Computer)** o parametrach zbliżonych do desktopów:

| Komponent | Specyfikacja |
|-----------|--------------|
| CPU | Broadcom BCM2711, Quad-core Cortex-A72 @ 1.5 GHz |
| GPU | VideoCore VI, OpenGL ES 3.0 |
| RAM | 2/4/8 GB LPDDR4-3200 |
| Storage | microSD (do 512 GB), USB 3.0 SSD |
| Connectivity | Gigabit Ethernet, Wi-Fi 5 (802.11ac), BT 5.0 |
| I/O | 4× USB (2× 3.0), 2× micro-HDMI, GPIO 40-pin |
| OS | Raspberry Pi OS (Debian-based), Ubuntu Core |

W kontekście aplikacji biofeedback Raspberry Pi realizuje funkcje:
- **Akwizycji wielowątkowej** – równoczesna obsługa portów szeregowych, GUI, zapisu dyskowego
- **Przetwarzania sygnałów** – implementacja algorytmów FFT, filtracji Kalmana, detekcji zdarzeń
- **Wizualizacji czasu rzeczywistego** – rendering wykresów 60 FPS przy użyciu Qt5 Charts
- **Zarządzania danymi pacjentów** – baza JSON z enkrypcją AES-256
- **Sterowania pętlą sprzężenia zwrotnego** – adaptacja trudności gier biofeedback w zależności od wyników

### 1.3.3.3 Porównanie platform mikrokontrolerowych

| Kryterium | Arduino Nano | Raspberry Pi Pico | Raspberry Pi 4 |
|-----------|--------------|-------------------|----------------|
| Architektura | AVR 8-bit | ARM Cortex-M0+ | ARM Cortex-A72 |
| Taktowanie | 16 MHz | 133 MHz | 1500 MHz |
| RAM | 2 KB SRAM | 264 KB SRAM | 2–8 GB DDR4 |
| Flash | 32 KB | 2 MB QSPI | microSD (external) |
| System operacyjny | Bare-metal | Bare-metal / FreeRTOS | Linux |
| Pobór mocy | ~20 mA | ~30 mA | 300–700 mA |
| Cena | ~15 PLN | ~25 PLN | ~250–400 PLN |
| Zastosowanie | Akwizycja sensorów | Edge computing | Host + GUI + AI |

## 1.3.4 Oprogramowanie systemów wbudowanych

### 1.3.4.1 Firmware mikrokontrolerów akwizycyjnych

Oprogramowanie Arduino (firmware) implementuje maszynę stanów skończonych (FSM) z następującymi stanami:

1. **INIT** – inicjalizacja UART, HX711, timerów
2. **IDLE** – oczekiwanie na komendy hosta (`ID`, `TARE`, `CONT`)
3. **CALIBRATION** – procedura zerowania i kalibracji ze znanym obciążeniem
4. **STREAMING** – ciągła transmisja pakietów binarnych 80 Hz
5. **ERROR** – obsługa wyjątków (CRC mismatch, timeout)

Kluczowe fragmenty kodu firmware:

```cpp
void loop() {
  if (serialAvailable()) {
    parseCommand(readSerial());
  }
  
  if (mode == CONTINUOUS) {
    if (hx711Ready()) {
      int32_t raw = hx711Read();
      uint32_t ts = millis();
      uint8_t crc = calculateCRC(ts, raw);
      sendBinaryPacket(ts, raw, crc);
    }
  }
}
```

### 1.3.4.2 Aplikacja nadrzędna w środowisku Qt5/C++

Oprogramowanie Raspberry Pi zostało zrealizowane w architekturze **Model-View-Controller (MVC)** z wykorzystaniem frameworka Qt5:

**Warstwa Model:**
- `HX711Sensor` – abstrakcja komunikacji szeregowej
- `DataManager` – CRUD operacje na rekordach pacjentów
- `EncryptionModule` – szyfrowanie danych wrażliwych (AES-256-CBC)

**Warstwa View:**
- `MainWindow` – główny interfejs z kartami (Tabs)
- `GraphWidget` – widget wykresu czasu rzeczywistego (QChart)
- `GameCanvas` – płótno renderujące gry biofeedback (QPainter)

**Warstwa Controller:**
- `MeasurementTab` – logika akwizycji i statystyk
- `TrainingTab` – selekcja gier i adaptacja parametrów
- `PatientTab` – zarządzanie bazą pacjentów

Struktura projektu zgodna z standardami medical software:

```
biofeedback-app/
├── src/core/              # Logika biznesowa
├── src/gui/               # Komponenty interfejsu
├── src/sensor/            # Sterowanie hardware
├── src/games/             # Silniki gier
├── src/data/              # Persistencja danych
├── include/               # Nagłówki publiczne
├── config/                # Pliki konfiguracyjne JSON
└── data/patients/         # Baza rekordów medycznych
```

### 1.3.4.3 Algorytmy przetwarzania sygnałów

Dane surowe z tensometru podlegają wieloetapowej obróbce:

**Etap 1: Filtracja wstępna (hardware)**
- Filtr dolnoprzepustowy RC z częstotliwością graniczną 50 Hz
- Ekranowanie kabli sygnałowych (shielded twisted pair)

**Etap 2: Filtracja cyfrowa (software)**
- **Moving Average Filter**: $y[n] = \frac{1}{N}\sum_{i=0}^{N-1} x[n-i]$ (N=5)
- **Median Filter**: eliminacja impulsowych zakłóceń EMI
- **Butterworth IIR**: pasmo 0.5–20 Hz dla ruchu ludzkiego

**Etap 3: Detekcja zdarzeń biomechanicznych**
- Algorytm detekcji powtórzeń (rep counting) na podstawie progów siły
- Segmentacja faz: koncentryczna (narastanie siły) vs. ekscentryczna (spadek)
- Obliczanie metryk: Peak Force, Time Under Tension, RFD

**Etap 4: Ekstrakcja parametrów klinicznych**
Zgodnie z dokumentacją `MATHEMATICAL_FORMULAS.md`, system oblicza 65+ wskaźników:

$$F_{peak}^{(s,n)} = \max_{t \in [t_{start}, t_{end}]} F(t)$$

$$RFD^{(s,n)} = \frac{F_{peak} - F_{start}}{T_{conc}}$$

$$CV_F^{(s)} = \frac{\sigma_F}{\bar{F}_{set}} \times 100\%$$

## 1.3.5 Rejestracja i struktura danych pomiarowych

### 1.3.5.1 Format JSON dla danych medycznych

Dane pacjentów przechowywane są w hierarchicznej strukturze katalogów:

```
data/patients/
└── PESEL_12345678901/
    ├── profile.json          # Dane demograficzne
    ├── measurements/
    │   ├── 2024-01-15_session_001.json
    │   └── 2024-01-17_session_002.json
    ├── exercises/
    │   └── plan_terapeutyczny.json
    └── backups/
        └── 2024-01-15_backup.zip.enc
```

Schemat rekordu sesji pomiarowej:

```json
{
  "session_id": "uuid-v4",
  "patient_id": "PESEL_12345678901",
  "timestamp_start": "2024-01-15T10:30:00Z",
  "timestamp_end": "2024-01-15T10:45:00Z",
  "therapist": "Jan Kowalski",
  "exercise_type": "tubeing_elbow_flexion",
  "sets": [
    {
      "set_number": 1,
      "reps": [
        {
          "rep_number": 1,
          "peak_force_N": 45.3,
          "mean_force_N": 38.7,
          "duration_s": 2.4,
          "concentric_time_s": 1.1,
          "eccentric_time_s": 1.3,
          "rfd_N_per_s": 125.6,
          "smoothness_index": 0.87
        }
      ],
      "set_metrics": {
        "total_work_J": 156.8,
        "average_power_W": 42.3,
        "fatigue_index_pct": 5.2
      }
    }
  ],
  "calibration": {
    "tare_value": 1024,
    "calibration_factor": 0.0448,
    "last_calibration": "2024-01-15T09:00:00Z"
  },
  "integrity": {
    "checksum_sha256": "a1b2c3...",
    "signed_by": "system_certificate_v1"
  }
}
```

### 1.3.5.2 Bezpieczeństwo i integralność danych

Zgodnie z wymogami RODO oraz normą PN-EN 62304 (Medical Device Software), system implementuje:

1. **Enkrypcję danych wrażliwych**:
   - Algorytm: AES-256-CBC z losowym IV
   - Klucz wyprowadzany z hasła użytkownika przez PBKDF2 (100 000 iteracji)
   - Szyfrowane pliki: `profile.enc`, `measurements/*.enc`

2. **Ślad audytowy (Audit Trail)**:
   - Logowanie wszystkich operacji CRUD z timestampem
   - Niezmienialne logi w formacie syslog
   - Rotacja plików logów co 7 dni

3. **Mechanizmy integralności**:
   - Sumy kontrolne SHA-256 dla każdego rekordu
   - Podpis cyfrowy RSA-2048 dla raportów medycznych
   - Walidacja CRC przy transmisji szeregowej

4. **Kontrola dostępu**:
   - Autentykacja dwuskładnikowa (hasło + token)
   - Role: Administrator, Terapeuta, Pacjent (read-only)
   - Sesje wygasające po 15 minutach nieaktywności

## 1.3.6 Pętla sprzężenia zwrotnego w systemach biofeedback

### 1.3.6.1 Teoretyczne podstawy biofeedback

Biofeedback stanowi technikę behawioralną, w której pacjent otrzymuje informację zwrotną (visual, auditory, haptic) o przebiegu procesów fizjologicznych normalnie niepodlegających świadomej kontroli. W kontekście treningu z taśmą lateksową, pętla sprzężenia obejmuje:

1. **Sensor** – tensometr mierzący siłę naciągu
2. **Przetworzenie** – ekstrakcja metryk biomechanicznych
3. **Prezentacja** – wizualizacja na ekranie (wykres, gra)
4. **Interpretacja** – pacjent identyfikuje wzorce ruchu
5. **Korekta** – świadoma modyfikacja techniki ćwiczenia
6. **Wzmocnienie** → powrót do punktu 1

Czas opóźnienia pętli (latency) musi być < 100 ms, aby mózg mógł skutecznie powiązać akcję z feedbackiem (granica percepcji kauzalności).

### 1.3.6.2 Implementacja gier biofeedback

System zawiera 9 gier terapeutycznych realizujących różne paradygmaty treningowe:

| Gra | Mechanika | Cel terapeutyczny |
|-----|-----------|-------------------|
| **Sin/Tan Wave** | Podążanie za falą sinusoidalną | Trening płynności, precyzji |
| **Flappy Bird** | Skoki poprzez przeszkody | Eksplozywność, timing |
| **Pong** | Odbijanie piłki | Koordynacja oko-ręka |
| **Arkanoid** | Rozbijanie cegieł | Wytrzymałość siłowa |
| **Mario Run** | Przeskakiwanie przeszkód | Kontrola fazy ekscentrycznej |
| **Galaga** | Strzelanie do celów | Stabilizacja, precyzja |
| **Archer** | Celowanie łukiem | Izometryczne utrzymanie napięcia |
| **Scored Earth** | Trafianie w strefy | Gradacja siły |
| **Car Race** | Wyścigi z przeszkodami | Reakcja, adaptacja |

Każda gra implementuje **dynamiczną adaptację trudności**:
- Poziom oporu (threshold siły) dostosowuje się do historii wyników
- Szerokość okna tolerancji maleje wraz z postępem pacjenta
- System rekomenduje progresję/regresję ćwiczeń na podstawie trendów

### 1.3.6.3 Algorytm adaptacji w czasie rzeczywistym

```cpp
void TrainingTab::adjustDifficulty(float currentPerformance) {
    float movingAvg = performanceBuffer.getAverage();
    
    if (currentPerformance > movingAvg * 1.2) {
        // Pacjent radzi sobie dobrze - zwiększ trudność
        thresholdForce *= 1.05;  // +5%
        toleranceWindow *= 0.95; // -5% tolerancji
        emit difficultyIncreased();
    }
    else if (currentPerformance < movingAvg * 0.8) {
        // Pacjent ma trudności - zmniejsz wymagania
        thresholdForce *= 0.95;
        toleranceWindow *= 1.05;
        emit difficultyDecreased();
    }
    
    // Zapisz do historii dla długoterminowej analizy
    progressDatabase.logSession(currentPerformance);
}
```

## 1.3.7 Aspekty metrologiczne i walidacja systemów

### 1.3.7.1 Kalibracja statyczna i dynamiczna

Procedura kalibracji obejmuje dwa etapy:

**Kalibracja statyczna (zero i span):**
1. Odłączenie obciążenia → odczyt wartości zerowej (tare)
2. Zawieszenie wzorca masy (np. 5 kg = 49.05 N) → odczyt wartości końcowej
3. Obliczenie współczynnika kalibracji:

$$k_{cal} = \frac{F_{known}}{ADC_{loaded} - ADC_{tare}}$$

4. Zapis współczynnika w EEPROM Arduino

**Kalibracja dynamiczna:**
1. Generowanie sygnału referencyjnego o znanej częstotliwości (1–5 Hz)
2. Rejestracja odpowiedzi systemu i analiza transmitancji
3. Korekcja charakterystyki częstotliwościowej w oprogramowaniu

### 1.3.7.2 Wyznaczanie niepewności pomiaru

Zgodnie z Guide to the Expression of Uncertainty in Measurement (GUM), całkowita niepewność pomiaru $u_c$ wyraża się wzorem:

$$u_c = \sqrt{u_{sensor}^2 + u_{adc}^2 + u_{temp}^2 + u_{cal}^2}$$

gdzie składowe to:
- $u_{sensor}$ – niepewność czujnika (z certyfikatu: 0.05% FS)
- $u_{adc}$ – kwantyzacja przetwornika (LSB/√12)
- $u_{temp}$ – dryft temperaturowy (0.01%/°C × ΔT)
- $u_{cal}$ – niepewność wzorca kalibracyjnego (klasa M2: 0.01%)

Dla systemu z czujnikiem 500 N i ADC 24-bit:
- $u_{sensor} = 0.25$ N
- $u_{adc} = 500 / 2^{24} / \sqrt{12} ≈ 0.000086$ N (zaniedbywalne)
- $u_{temp} = 500 × 0.0001 × 10°C = 0.5$ N (dla ΔT=10°C)
- $u_{cal} = 500 × 0.0001 = 0.05$ N

$$u_c = \sqrt{0.25^2 + 0.000086^2 + 0.5^2 + 0.05^2} ≈ 0.56 \text{ N}$$

Rozszerzona niepewność dla poziomu ufności 95% (k=2):

$$U = k \cdot u_c = 2 × 0.56 = 1.12 \text{ N}$$

### 1.3.7.3 Walidacja kliniczna systemu

Proces walidacji obejmuje:

1. **Testy dokładności**: porównanie z laboratorium wzorcującym (platforma Kistler)
2. **Testy powtarzalności**: 100 pomiarów tego samego obciążenia, obliczenie CV
3. **Testy odtwarzalności**: pomiary przez różnych operatorów, w różnych warunkach
4. **Testy stabilności długoterminowej**: dryft zera przez 30 dni
5. **Testy użyteczności**: ankiety SUS (System Usability Scale) wśród fizjoterapeutów

Kryteria akceptacji:
- Błąd maksymalny: < 2% zakresu pomiarowego
- Powtarzalność: CV < 1%
- Czas reakcji systemu: < 50 ms
- Dostępność (uptime): > 99.5%

## 1.3.8 Integracja z systemami zewnętrznymi i telemedycyna

### 1.3.8.1 Eksport danych do formatów medycznych

System obsługuje eksport do standardów interoperacyjności medycznej:

- **HL7 FHIR** (Fast Healthcare Interoperability Resources):
  ```json
  {
    "resourceType": "Observation",
    "status": "final",
    "category": [{"coding": [{"code": "vital-signs"}]}],
    "code": {"coding": [{"code": "8867-4", "display": "Heart rate"}]},
    "subject": {"reference": "Patient/123"},
    "valueQuantity": {"value": 45.3, "unit": "N"},
    "effectiveDateTime": "2024-01-15T10:30:00Z"
  }
  ```

- **DICOM SR** (Structured Reporting) dla archiwizacji w PACS
- **CSV/XML** dla analiz w oprogramowaniu statystycznym (SPSS, R)

### 1.3.8.2 Moduł telemedyczny

Architektura umożliwia zdalny monitoring pacjentów poprzez:
- Szyfrowane połączenie TLS 1.3 do serwera centralnego
- Synchronizacja danych w tle (background sync)
- Dashboard terapeuty z widokiem wielu pacjentów
- Alerty SMS/email przy wykryciu anomalii (np. regresja wyników)

## 1.3.9 Podsumowanie rozdziału

Rozdział 1.3 przedstawił kompleksowy przegląd systemów wbudowanych i rejestracji danych w zastosowaniach fizjoterapeutycznych i treningowych. Omówiono:

1. **Architekturę sprzętową** – od czujników tensometrycznych przez mikrokontrolery Arduino po komputery Raspberry Pi
2. **Oprogramowanie** – firmware akwizycyjny, aplikację Qt5/C++, algorytmy DSP
3. **Strukturę danych** – format JSON, bezpieczeństwo, integralność, zgodność z RODO
4. **Pętlę biofeedback** – implementację gier terapeutycznych z adaptacją trudności
5. **Aspekty metrologiczne** – kalibrację, niepewność pomiaru, walidację kliniczną
6. **Integrację** – eksport HL7/FHIR, telemedycynę

Przedstawiony system pomiarowo-treningowy z pętlą sprzężenia zwrotnego stanowi oryginalne osiągnięcie projektowe w rozumieniu art. 219 ust. 1 pkt 2 lit. c ppkt 1 ustawy Prawo o szkolnictwie wyższym i nauce, łącząc innowacje konstrukcyjne (integracja tensometrii z gamifikacją), technologiczne (algorytmy adaptacyjne) i wdrożeniowe (legalizacja jednostkowa urządzeń specjalnych).

W kolejnych rozdziałach szczegółowo opisano założenia projektowe (Rozdział 2), realizację konstrukcyjną (Rozdział 3), oprogramowanie (Rozdział 4), walidację metrologiczną (Rozdział 5) oraz zastosowania praktyczne w gimnastyce leczniczej, sportowej i wychowaniu fizycznym (Rozdział 6).

---

**Bibliografia rozdziału 1.3:**

1. IEEE Standard Definition of Embedded Systems, IEEE Std 1012-2016
2. PN-EN 62304:2008 Medical device software – Software life cycle processes
3. Guide to the Expression of Uncertainty in Measurement (GUM), JCGM 100:2008
4. Raspberry Pi Foundation, Raspberry Pi 4 Model B Technical Documentation, 2023
5. Arduino LLC, Arduino Nano Hardware Reference, 2023
6. Avia Semiconductor, HX711 24-Bit ADC Datasheet, 2022
7. Qt Company, Qt5 Charts Module Documentation, 2023
8. HL7 International, FHIR Release 4 Specification, 2023
9. Ustawa z dnia 20 lipca 2018 r. Prawo o szkolnictwie wyższym i nauce (Dz.U. 2018 poz. 1668)
10. Rozporządzenie Parlamentu Europejskiego i Rady (UE) 2017/745 w sprawie wyrobów medycznych (MDR)
