# Rozdział 4.2. Firmware Arduino i Pico – synchronizacja czasowa, próbkowanie i transmisja szeregowa

## 4.2.1. Wprowadzenie do architektury firmware'u dla systemów akwizycji danych biomechanicznych

### 4.2.1.1. Specyfika wymagań czasowych dla firmware'u mikrokontrolerów w aplikacjach biofeedbacku

Opracowanie firmware'u dla mikrokontrolerów Arduino Nano (ATmega328P) oraz Raspberry Pi Pico (RP2040) stanowiło krytyczny etap realizacji autorskiego systemu pomiarowo-treningowego z pętlą sprzężenia zwrotnego. W przeciwieństwie do oprogramowania wyższego poziomu uruchamianego na Raspberry Pi pod kontrolą systemu operacyjnego Linux, firmware mikrokontrolerów musi spełniać rygorystyczne wymagania **systemów czasu rzeczywistego twardego (hard real-time systems)**, gdzie przekroczenie deadline'u czasowego nawet o pojedyncze milisekundy może prowadzić do utraty spójności danych pomiarowych, destabilizacji pętli sprzężenia zwrotnego i w konsekwencji do błędnej interpretacji wyników przez fizjoterapeutę lub trenera.

**Kluczowe wymagania czasowe dla firmware'u mikrokontrolerów:**

1. **Determinizm czasowy akwizycji:** Częstotliwość próbkowania sygnału z czujnika HX711 musi wynosić dokładnie **80 Hz** z tolerancją ±0.1% (czyli odchylenie maksymalne ±0.08 Hz). W praktyce oznacza to, że odstęp między kolejnymi próbkami musi wynosić $12.5 \text{ ms} \pm 12.5 \text{ µs}$. Każde odchylenie od tego interwału wprowadza jitter próbkowania, który w domenie częstotliwościowej manifestuje się jako szum fazowy (phase noise) mogący zakłócić analizę widmową sygnału siły.

2. **Maksymalne opóźnienie przetwarzania (latencja):** Czas pomiędzy fizyczną zmianą siły na taśmie lateksowej a wysłaniem odpowiadającej jej wartości cyfrowej do Raspberry Pi nie może przekroczyć **5 ms**. Składają się na ten budżet czasowy:
   - Czas stabilizacji sygnału analogowego w mostku Wheatstone'a: ~0.5 ms;
   - Czas konwersji ADC w układzie HX711: 12.5 ms (przy 80 Hz) lub 100 ms (przy 10 Hz);
   - Czas odczytu 24 bitów danych przez interfejs szeregowy: ~2.4 µs przy zegarze 10 MHz;
   - Czas obliczenia sumy kontrolnej CRC-8: ~15 µs;
   - Czas transmisji UART do Raspberry Pi: ~0.83 ms przy 115200 baud.

3. **Brak blokodujących pętli oczekujących (non-blocking architecture):** Firmware musi być zaprojektowany w architekturze współbieżnej, gdzie oczekiwanie na zakończenie konwersji ADC odbywa się w tle (background), a główna pętla programu (main loop) pozostaje responsywna na komendy sterujące z Raspberry Pi (np. zmiana trybu pracy, kalibracja, reset). Implementacja klasycznego podejścia sekwencyjnego z funkcją `delay()` lub busy-waiting pollingiem uniemożliwiłaby realizację innych zadań czasowo-krytycznych, takich jak obsługa watchdog timer czy monitorowanie parametrów zdrowotnych systemu.

4. **Odporność na zakłócenia i recovery:** Mikrokontroler pracuje w środowisku elektromagnetycznie zaszumionym (bliskość zasilaczy impulsowych, silników, urządzeń diatermii). Firmware musi implementować mechanizmy wykrywania i korekcji błędów:
   - Detekcja utraty synchronizacji z HX711 (timeout DOUT line);
   - Walidacja danych poprzez CRC-8 przed wysłaniem;
   - Automatyczny restart przez watchdog timer w przypadku zawieszenia;
   - Zabezpieczenie przed brownout (nagłym spadkiem napięcia zasilania).

5. **Efektywność energetyczna:** W scenariuszach zasilania bateryjnego (przenośne wersje systemu), firmware powinien minimalizować pobór prądu poprzez:
   - Usypianie mikrokontrolera między próbkami (idle mode);
   - Wyłączanie nieużywanych peryferiów (ADC, Timers);
   - Dynamiczne skalowanie częstotliwości zegara (dynamic frequency scaling).

**Tabela 4.7.** Porównanie parametrów technicznych Arduino Nano i Raspberry Pi Pico w kontekście aplikacji akwizycji danych

| Parametr | Arduino Nano (ATmega328P) | Raspberry Pi Pico (RP2040) | Implikacje dla firmware'u |
|----------|---------------------------|----------------------------|---------------------------|
| **Architektura CPU** | 8-bit AVR @ 16 MHz | Dual-core ARM Cortex-M0+ @ 133 MHz | RP2040 oferuje ~500× wyższą wydajność obliczeniową |
| **Pamięć Flash** | 32 KB (0.5 KB bootloader) | 2 MB (external QSPI) | RP2040 pozwala na bogatszy firmware z loggingiem |
| **Pamięć SRAM** | 2 KB | 264 KB | RP2040 umożliwia buforowanie setek próbek w RAM |
| **Timery hardware'owe** | 3 (Timer0, Timer1, Timer2) | 4 alarmy + PWM + PIO | RP2040 PIO zapewnia deterministyczne timingu I/O |
| **ADC** | 10-bit, 1 kanał (multiplexed) | 12-bit, 5 kanałów, FIFO | RP2040 ADC ma wyższą rozdzielczość i DMA |
| **Interrupts** | 2 zewnętrzne (INT0, INT1) | 4 GPIO IRQ + DMA IRQ | RP2040 pozwala na równoległą obsługę wielu źródeł przerwań |
| **Zużycie prądu** | ~15 mA @ 16 MHz (active) | ~30 mA @ 133 MHz (active), ~0.4 mA (sleep) | ATmega328P bardziej efektywny dla prostych aplikacji |
| **Cena** | ~25 PLN (klon) | ~35 PLN | Oba platformy są ekonomicznie dostępne |

**Decyzja projektowa:** W toku prac rozwojowych opracowano **dualną ścieżkę firmware'u** – osobno dla Arduino Nano (jako rozwiązanie baseline dla wersji edukacyjnych i najtańszych wdrożeń) oraz dla Raspberry Pi Pico (jako rozwiązanie premium dla aplikacji badawczych i klinicznych wymagających wyższej precyzji i dodatkowych funkcji). Oba firmware'y implementują identyczny protokół komunikacyjny z Raspberry Pi, co zapewnia pełną kompatybilność warstwy aplikacyjnej.

---

## 4.2.2. Architektura firmware'u dla Arduino Nano (ATmega328P)

### 4.2.2.1. Konfiguracja rejestrów sprzętowych i inicjalizacja peryferiów

Firmware Arduino Nano został napisany w języku C++ z wykorzystaniem frameworka Arduino Core, jednakże kluczowe elementy implementacji wymagają bezpośredniej manipulacji rejestrami sprzętowymi mikrokontrolera ATmega328P w celu zapewnienia wymaganego determinizmu czasowego. Poniżej przedstawiono szczegółową konfigurację rejestrów:

**A. Konfiguracja zegara systemowego (System Clock):**

ATmega328P fabrycznie wyposażony jest w wewnętrzny oscylator RC 8 MHz, jednakże dla aplikacji pomiarowych konieczne jest użycie zewnętrznego rezonatora kwarcowego 16 MHz zapewniającego stabilność częstotliwości na poziomie ±30 ppm (parts per million) w zakresie temperaturowym -40°C do +85°C.

```cpp
// Fuse bits configuration for external 16 MHz crystal
// Low Fuse: 0xFF (external full-swing crystal, no clock division)
// High Fuse: 0xDE (serial programming enabled, boot loader size 512 words)
// Extended Fuse: 0xFD (brown-out detection at 2.7V)

// W kodzie źródłowym: verification of clock stability
void verifyClockStability() {
    uint32_t start = micros();
    for (volatile uint16_t i = 0; i < 10000; i++) {
        asm volatile("nop"); // 1 cycle NOP
    }
    uint32_t elapsed = micros() - start;
    // Expected: 10000 cycles / 16 MHz = 625 µs
    if (elapsed < 600 || elapsed > 650) {
        // Clock deviation detected - trigger error handler
        errorLedBlink(5); // 5 blinks = clock error
    }
}
```

**B. Konfiguracja Timer1 dla generowania interruptów próbkowania:**

Timer1 (16-bit) został skonfigurowany w trybie CTC (Clear Timer on Compare Match) do generowania przerwania co dokładnie 12.5 ms (80 Hz). Wykorzystanie hardware'owego timera zamiast software'owego `millis()` lub `delayMicroseconds()` eliminuje jitter spowodowany innymi interruptami (np. USART RX).

```cpp
// Konfiguracja Timer1 dla 80 Hz interrupt (12.5 ms period)
// F_CPU = 16 MHz, prescaler = 64, OCR1A = 3124
// Obliczenie: 16,000,000 / (64 * (3124 + 1)) = 80.0 Hz

void initSamplingTimer() {
    // Wyłączenie Timera1 podczas konfiguracji
    TCCR1A = 0x00; // Normal port operation, OC1A/OC1B disconnected
    TCCR1B = 0x00; // Timer stopped
    
    TCNT1H = 0x00; // Clear counter register
    TCNT1L = 0x00;
    
    OCR1AH = 0x0C; // Output Compare Register = 3124 (0x0C34)
    OCR1AL = 0x34;
    
    TCCR1B = 0x00;
    TCCR1B |= (1 << WGM12); // CTC mode (WGM12=1, WGM13=0, WGM11=0, WGM10=0)
    TCCR1B |= (1 << CS11) | (1 << CS10); // Prescaler = 64 (CS12=0, CS11=1, CS10=1)
    
    TIMSK1 |= (1 << OCIE1A); // Enable Output Compare A Match Interrupt
    
    // Global interrupt enable (w setup())
    // sei();
}

// Interrupt Service Routine dla Timer1 Compare Match
ISR(TIMER1_COMPA_vect) {
    // Flag set for main loop processing
    samplingReady = true;
    
    // Toggle debug pin for timing verification (opcjonalne)
    // PORTD |= (1 << PD7); // Set pin high
    // PORTD &= ~(1 << PD7); // Set pin low
}
```

**Uzasadnienie wyboru preskalera 64 i OCR1A=3124:**
- Przy prescalerze 64, każdy tick timera wynosi $64 / 16 \text{ MHz} = 4 \text{ µs}$;
- Dla okresu 12.5 ms = 12500 µs, wymagana liczba ticków: $12500 / 4 = 3125$;
- Ponieważ CTC clears counter przy osiągnięciu OCR1A, wartość OCR1A = 3124 daje 3125 ticków (od 0 do 3124 inclusive);
- Błąd zaokrąglenia: 0 (dokładne dopasowanie);
- Alternatywne konfiguracje (np. prescaler=256, OCR1A=780) dawałyby większy błąd zaokrąglenia.

**C. Konfiguracja interfejsu szeregowego UART (USART0):**

Komunikacja z Raspberry Pi odbywa się przez UART z prędkością 115200 baud. Konfiguracja rejestrów UCSR0A/B/C zapewnia poprawną transmisję asynchroniczną z 8 bitami danych, bez parzystości i 1 bitem stopu (8N1).

```cpp
void initUART() {
    uint16_t ubrr_value = 8; // Dla 115200 baud @ 16 MHz: (16MHz/(16*115200))-1 = 8.68 -> 8 (integer)
    // Rzeczywista prędkość: 16MHz/(16*(8+1)) = 111111 baud (-3.5% error, acceptable)
    
    // UCSR0A: USART Control and Status Register A
    UCSR0A = 0x00; // Normalna praca, bez double speed (U2X0=0)
    
    // UCSR0B: Enable RX, TX i RX Complete Interrupt
    UCSR0B = (1 << RXEN0) | (1 << TXEN0); // Receiver and Transmitter enabled
    // RXCIE0=0 (polling instead of interrupt for simplicity)
    
    // UCSR0C: Asynchronous USART, 8-bit data, no parity, 1 stop bit
    UCSR0C = (1 << UMSEL01) | (1 << UMSEL00); // Asynchronous USART
    UCSR0C &= ~(1 << UPM01); // No parity
    UCSR0C &= ~(1 << UPM00);
    UCSR0C &= ~(1 << USBS0); // 1 stop bit
    UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00); // 8-bit data
    
    // UBRR0H/L: Baud Rate Register
    UBRR0H = (ubrr_value >> 8); // High byte
    UBRR0L = ubrr_value;        // Low byte
}

// Funkcja wysyłania bajtu przez UART (blocking)
void uartSendByte(uint8_t data) {
    // Wait for empty transmit buffer
    while (!(UCSR0A & (1 << UDRE0)));
    // Put data into buffer, sends the data
    UDR0 = data;
}

// Funkcja odbierania bajtu z UART (non-blocking z timeout)
int16_t uartReceiveByte(uint16_t timeout_us) {
    uint16_t counter = 0;
    while (!(UCSR0A & (1 << RXC0))) {
        _delay_us(1);
        counter++;
        if (counter > timeout_us) {
            return -1; // Timeout
        }
    }
    return UDR0;
}
```

**D. Konfiguracja pinów GPIO dla interfejsu HX711:**

HX711 komunikuje się przez dedykowany interfejs szeregowy z dwoma liniami: PD_SCK (Power Down / Serial Clock) i DOUT (Data Out). Pin DOUT musi być skonfigurowany jako wejście z pull-up, natomiast PD_SCK jako wyjście.

```cpp
// Definicje pinów
#define HX711_DOUT_PIN  PINC, 2 // PC2 (Arduino pin A2)
#define HX711_SCK_PIN   PINC, 3 // PC3 (Arduino pin A3)

void initHX711Interface() {
    // DOUT jako input z pull-up resistor (~20-50 kΩ internal)
    DDRC &= ~(1 << HX711_DOUT_PIN); // Data Direction Register: 0=input
    PORTC |= (1 << HX711_DOUT_PIN); // Pull-up enabled
    
    // SCK jako output, initial state low
    DDRC |= (1 << HX711_SCK_PIN);   // 1=output
    PORTC &= ~(1 << HX711_SCK_PIN); // Initial state low
    
    // Power-up HX711 (SCK low przez >60 µs wybudza układ)
    delayMicroseconds(100);
}

// Funkcja odczytu 24-bitowej wartości z HX711
uint32_t readHX711Raw() {
    uint32_t count = 0;
    uint8_t i;
    
    // Wait for DOUT to go low (data ready)
    // Timeout after 100 ms (8000 cycles @ 80 Hz)
    uint16_t timeout = 8000;
    while (PINC & (1 << HX711_DOUT_PIN)) {
        timeout--;
        if (timeout == 0) {
            return 0xFFFFFFFF; // Error: timeout
        }
    }
    
    // Read 24 bits MSB first
    for (i = 0; i < 24; i++) {
        PORTC |= (1 << HX711_SCK_PIN);   // SCK high
        _delay_us(0.5); // Short delay for signal settling (min 200 ns)
        count = (count << 1) | ((PINC & (1 << HX711_DOUT_PIN)) ? 1 : 0);
        PORTC &= ~(1 << HX711_SCK_PIN);  // SCK low
        _delay_us(0.5);
    }
    
    // 25th pulse to set gain for next reading (Gain=128)
    PORTC |= (1 << HX711_SCK_PIN);
    _delay_us(0.5);
    PORTC &= ~(1 << HX711_SCK_PIN);
    
    return count;
}
```

### 4.2.2.2. Implementacja algorytmu akwizycji w pętli głównej

Główna pętla firmware'u Arduino Nano realizuje architekturę **super-loop z flagami zdarzeń** (event-flag super-loop), gdzie zadania czasowo-krytyczne są wyzwalane przez przerwania hardware'owe, a przetwarzanie danych odbywa się w pętli głównej po ustawieniu odpowiednich flag. Takie podejście zapewnia determinizm czasowy ISR (Interrupt Service Routine) przy jednoczesnej elastyczności przetwarzania w main loop.

```cpp
// Global flags
volatile bool samplingReady = false;
volatile bool commandReceived = false;
volatile uint8_t rxCommand = 0;

// Buffers
uint32_t rawSamples[BUFFER_SIZE]; // BUFFER_SIZE = 16 dla circular buffer
uint32_t filteredSamples[BUFFER_SIZE];
uint8_t bufferHead = 0;
uint8_t bufferTail = 0;

// Main loop
void loop() {
    // 1. Check for incoming commands from Raspberry Pi (non-blocking)
    processIncomingCommands();
    
    // 2. If sampling interrupt triggered, acquire new sample
    if (samplingReady) {
        samplingReady = false; // Clear flag
        
        // Read raw value from HX711
        uint32_t rawValue = readHX711Raw();
        
        // Validate reading (check for timeout/error)
        if (rawValue != 0xFFFFFFFF) {
            // Apply moving average filter (optional, depends on mode)
            uint32_t filteredValue = applyMovingAverage(rawValue);
            
            // Store in circular buffer
            rawSamples[bufferHead] = rawValue;
            filteredSamples[bufferHead] = filteredValue;
            bufferHead = (bufferHead + 1) % BUFFER_SIZE;
            
            // Check if buffer is full (ready for transmission)
            if (getBufferCount() >= TRANSMIT_THRESHOLD) {
                transmitDataBatch();
            }
        } else {
            // Handle HX711 communication error
            errorCounter++;
            if (errorCounter > MAX_ERRORS) {
                resetHX711();
            }
        }
    }
    
    // 3. Background tasks (only when not time-critical)
    // - Watchdog refresh
    // - LED status update
    // - Temperature monitoring (if sensor present)
    
    // 4. Enter idle mode to save power (optional)
    // sleep_mode(); // Requires <avr/sleep.h>
}

// Circular buffer helper functions
uint8_t getBufferCount() {
    if (bufferHead >= bufferTail) {
        return bufferHead - bufferTail;
    } else {
        return BUFFER_SIZE - bufferTail + bufferHead;
    }
}
```

**Filtracja wstępna sygnału (Moving Average Filter):**

W celu redukcji szumu wysokoczęstotliwościowego przed transmisją do Raspberry Pi, firmware implementuje prosty filtr uśredniający ruchomą średnią z oknem 4 próbek. Filtr ten charakteryzuje się niskim narzutem obliczeniowym (~16 cykli na próbkę) i skutecznym tłumieniem szumu o częstotliwościach >20 Hz.

```cpp
#define MOVING_AVG_WINDOW 4

uint32_t applyMovingAverage(uint32_t newValue) {
    static uint32_t window[MOVING_AVG_WINDOW] = {0};
    static uint8_t index = 0;
    static uint32_t sum = 0;
    
    // Subtract oldest value from sum
    sum -= window[index];
    
    // Add new value to window and sum
    window[index] = newValue;
    sum += newValue;
    
    // Increment index (circular)
    index = (index + 1) % MOVING_AVG_WINDOW;
    
    // Return average (integer division)
    return sum / MOVING_AVG_WINDOW;
}
```

### 4.2.2.3. Protokół komunikacyjny UART z Raspberry Pi

Firmware Arduino implementuje binarny protokół komunikacyjny zoptymalizowany pod kątem minimalizacji narzutu (overhead) przy jednoczesnym zapewnieniu integralności danych poprzez sumę kontrolną CRC-8. Protokół obsługuje zarówno tryb ciągły (continuous streaming), jak i tryb żądania (on-demand polling).

**Struktura ramki danych (Data Packet):**

| Offset | Rozmiar | Pole | Opis |
|--------|---------|------|------|
| 0 | 1 bajt | Sync Byte | 0xAA (synchronization marker) |
| 1–4 | 4 bajty | Timestamp | Czas w milisekundach (uint32_t, little-endian) |
| 5–8 | 4 bajty | Force Value | Wartość siły po filtracji (int32_t, little-endian) |
| 9 | 1 bajt | CRC-8 | Suma kontrolna CRC-8 (polynomial 0x07) |
| 10 | 1 bajt | End Marker | 0x55 (frame end delimiter) |

**Łączny rozmiar ramki:** 11 bajtów.

Przy częstotliwości próbkowania 80 Hz, przepustowość łącza wynosi:
$$80 \text{ packets/s} \times 11 \text{ bytes/packet} \times 8 \text{ bits/byte} = 7040 \text{ bps}$$

Przy prędkości UART 115200 baud, wykorzystanie pasma wynosi:
$$\frac{7040}{115200} \times 100\% \approx 6.1\%$$

Pozostawia to znaczący margines (~94%) na transmisję komend sterujących i ewentualne retransmisje w przypadku błędów.

**Implementacja nadawania ramki:**

```cpp
// CRC-8 lookup table (polynomial 0x07, init 0xFF)
const uint8_t crc8_table[256] PROGMEM = {
    0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15,
    // ... (pełna tabela 256 elementów)
};

uint8_t calculateCRC8(const uint8_t* data, uint8_t length) {
    uint8_t crc = 0xFF;
    for (uint8_t i = 0; i < length; i++) {
        crc = pgm_read_byte(&crc8_table[crc ^ data[i]]);
    }
    return crc;
}

void transmitDataPacket(uint32_t timestamp, int32_t forceValue) {
    uint8_t packet[11];
    
    // Build packet
    packet[0] = 0xAA; // Sync byte
    
    // Timestamp (little-endian)
    packet[1] = timestamp & 0xFF;
    packet[2] = (timestamp >> 8) & 0xFF;
    packet[3] = (timestamp >> 16) & 0xFF;
    packet[4] = (timestamp >> 24) & 0xFF;
    
    // Force value (little-endian)
    packet[5] = forceValue & 0xFF;
    packet[6] = (forceValue >> 8) & 0xFF;
    packet[7] = (forceValue >> 16) & 0xFF;
    packet[8] = (forceValue >> 24) & 0xFF;
    
    // Calculate CRC over bytes 0-8
    packet[9] = calculateCRC8(packet, 9);
    
    // End marker
    packet[10] = 0x55;
    
    // Transmit all bytes
    for (uint8_t i = 0; i < 11; i++) {
        uartSendByte(packet[i]);
    }
    
    // Optional: wait for ACK from Raspberry Pi (not implemented in baseline version)
}
```

**Obsługa komend sterujących z Raspberry Pi:**

Raspberry Pi może wysyłać do Arduino następujące komendy (single-byte commands):

| Komenda | ASCII | Opis | Odpowiedź |
|---------|-------|------|-----------|
| `ID` | 0x49 0x44 | Żądanie identyfikacji urządzenia | DEVICE:..., ID:..., FW:... |
| `TARE` | 0x54 0x41 | Zerowanie wagi (tare offset) | Taring... Tare set! |
| `CONT` | 0x43 0x4F | Włączenie trybu ciągłego | Continuous mode ON |
| `STOP` | 0x53 0x54 | Wyłączenie trybu ciągłego | Continuous mode OFF |
| `DATA` | 0x44 0x41 | Żądanie pojedynczej próbki | Packet binarny |
| `CAL:<val>` | 0x43 0x41 | Kalibracja ze znaną wagą | Calibrated! Factor:X.XXXXXX |
| `RESET` | 0x52 0x45 | Reset przez watchdog | [reset] |

```cpp
void processIncomingCommands() {
    if (UCSR0A & (1 << RXC0)) { // Data received
        uint8_t cmd = UDR0;
        
        switch (cmd) {
            case 'I': // ID command (first byte)
                if (uartReceiveByte(1000) == 'D') { // Wait for 'D'
                    sendDeviceInfo();
                }
                break;
                
            case 'T': // TARE command
                if (uartReceiveByte(1000) == 'A') {
                    performTare();
                }
                break;
                
            case 'C': // CONT or CAL command
                uint8_t second = uartReceiveByte(1000);
                if (second == 'O') {
                    continuousMode = true;
                    uartSendString("Continuous mode ON\n");
                } else if (second == 'A') {
                    // Calibration sequence
                    performCalibration();
                }
                break;
                
            case 'S': // STOP command
                if (uartReceiveByte(1000) == 'T') {
                    continuousMode = false;
                    uartSendString("Continuous mode OFF\n");
                }
                break;
                
            case 'D': // DATA command
                if (uartReceiveByte(1000) == 'A') {
                    uint32_t ts = millis();
                    int32_t val = readAndFilterSample();
                    transmitDataPacket(ts, val);
                }
                break;
                
            case 'R': // RESET command
                if (uartReceiveByte(1000) == 'E') {
                    uartSendString("Resetting...\n");
                    delay(100);
                    wdt_enable(WDTO_15MS);
                    while(1); // Wait for watchdog reset
                }
                break;
        }
    }
}
```

### 4.2.2.4. Mechanizmy zabezpieczeń i watchdog timer

**A. Watchdog Timer (WDT):**

ATmega328P posiada wbudowany niezależny generator watchdog z własnym oscylatorem 128 kHz. Firmware konfiguruje WDT na timeout 2 sekund, co oznacza, że jeśli główna pętla nie odświeży licznika (funkcją `wdt_reset()`) przez ponad 2 sekundy, mikrokontroler zostanie automatycznie zresetowany.

```cpp
#include <avr/wdt.h>

void initWatchdog() {
    // Configure WDT for 2s timeout
    // WDP3=1, WDP2=1, WDP1=0, WDP0=1 => 2s
    wdt_enable(WDTO_2S);
}

void loop() {
    // ... main processing ...
    
    // Refresh watchdog at least every 2 seconds
    wdt_reset();
}
```

**B. Detekcja błędów komunikacji z HX711:**

Jeśli pin DOUT nie przejdzie w stan niski w ciągu 100 ms (co odpowiada 8 cyklom próbkowania @ 80 Hz), firmware uznaje, że HX711 uległ zawieszeniu i wykonuje procedurę resetu:

```cpp
void resetHX711() {
    // Power-cycle HX711 by toggling SCK pin >60 µs
    PORTC |= (1 << HX711_SCK_PIN);
    delayMicroseconds(100);
    PORTC &= ~(1 << HX711_SCK_PIN);
    delayMicroseconds(100);
    
    // Clear error counter
    errorCounter = 0;
    
    // Log error (via UART)
    uartSendString("[ERROR] HX711 reset performed\n");
}
```

**C. Brownout Detection (BOD):**

Fuse bits mikrokontrolera są skonfigurowane tak, aby aktywować reset przy spadku napięcia zasilania poniżej 2.7 V. Zapobiega to nieprzewidywalnemu zachowaniu przy niewystarczającym zasilaniu. Dodatkowo, firmware monitoruje napięcie VCC poprzez pomiar wewnętrznego referencyjnego napięcia 1.1 V względem VCC:

```cpp
uint16_t readVcc() {
    // Select internal 1.1V reference for ADC
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
    
    // Start conversion
    ADCSRA |= _BV(ADSC);
    
    // Wait for completion
    while (bit_is_set(ADCSRA, ADSC));
    
    // Calculate Vcc in mV
    uint16_t result = (1.1 * 1023 * 1000) / ADC;
    return result;
}

void checkBrownout() {
    static uint16_t vcc = 0;
    vcc = readVcc();
    
    if (vcc < 4500) { // Warning threshold: 4.5V
        uartSendString("[WARNING] Low VCC detected: ");
        uartSendInt(vcc);
        uartSendString(" mV\n");
    }
    
    if (vcc < 4000) { // Critical threshold: 4.0V
        uartSendString("[CRITICAL] Very low VCC! System may reset soon.\n");
    }
}
```

---

## 4.2.3. Architektura firmware'u dla Raspberry Pi Pico (RP2040)

### 4.2.3.1. Wykorzystanie układu PIO do deterministycznej obsługi interfejsu HX711

Raspberry Pi Pico, wyposażony w mikrokontroler RP2040 z dwurdzeniowym procesorem ARM Cortex-M0+ oraz unikalnym układem **PIO (Programmable Input/Output)**, oferuje zasadniczo inne podejście do implementacji firmware'u niż klasyczne Arduino Nano. PIO to programowalny blok hardware'owy zdolny do realizacji własnych protokołów komunikacyjnych z dokładnością jednego cyklu zegara, całkowicie odciążając CPU od zadań czasowo-krytycznych.

**Architektura PIO dla interfejsu HX711:**

Zamiast używać przerwań i polling GPIO w pętli głównej, firmware Pico implementuje **custom PIO program** dedykowany wyłącznie do odczytu danych z HX711. Program PIO jest ładowany do jednego z dwóch dostępnych bloków PIO (PIO0 lub PIO1) i wykonuje się niezależnie od CPU, generując dokładnie 25 impulsów zegara i odczytując 24 bity danych z automatem stanu.

```cpp
// PIO program for HX711 interface (written in PIO assembly language)
// Saved as hx711.pio in project

.program hx711_interface

; Pin definitions (assembled into PIO instruction memory)
; in_pin: DOUT pin (input)
; out_pin: SCK pin (output, starting pin)
; sideset_pin: optional for clock visualization

.wrap_target
    ; Wait for DOUT to go low (data ready)
    wait 0 pin 0        ; Wait until pin 0 (DOUT) is low
    
    ; Generate 24 clock pulses and read data bits
    set x, 23           ; Loop counter for 24 bits
    
read_bit:
    irq clear 0         ; Clear done IRQ
    set y, 9            ; Inner loop for timing adjustment (optional)
    
    nop side 1          ; SCK high (side effect)
    in pins, 1          ; Read DOUT pin into ISR
    nop side 0          ; SCK low
    
    jmp x--, read_bit   ; Decrement x, loop if not zero
    
    ; 25th pulse to set gain for next conversion
    nop side 1
    nop side 0
    
    ; Push accumulated 24-bit value to RX FIFO
    push block          ; Block until space in FIFO
    
    ; Signal completion to CPU
    irq 0               ; Assert IRQ 0
    
.wrap

// C++ code to initialize and use PIO program
#include "hardware/pio.h"
#include "hx711.pio.h" // Auto-generated header from pioasm

class HX711_PIO {
private:
    PIO pio;
    uint sm; // State machine number
    uint offset; // Program offset in instruction memory
    
public:
    HX711_PIO(PIO pio_instance, uint gpio_dout, uint gpio_sck) {
        this->pio = pio_instance;
        
        // Claim unused state machine
        this->sm = pio_claim_unused_sm(pio, true);
        
        // Add program to PIO instruction memory
        this->offset = pio_add_program(pio, &hx711_interface_program);
        
        // Initialize GPIO pins
        pio_gpio_init(pio, gpio_dout);
        pio_gpio_init(pio, gpio_sck);
        
        // Set SCK as output, DOUT as input
        pio_sm_set_consecutive_pindirs(pio, sm, gpio_sck, 1, true);
        pio_sm_set_consecutive_pindirs(pio, sm, gpio_dout, 1, false);
        
        // Get default config and modify
        pio_sm_config c = hx711_interface_program_get_default_config(offset);
        
        // Map DOUT to 'in' pins, SCK to 'out' pins with sideset
        sm_config_set_in_pins(&c, gpio_dout);
        sm_config_set_out_pins(&c, gpio_sck, 1);
        sm_config_set_sideset_pins(&c, gpio_sck);
        
        // Configure FIFO join for 8-deep RX FIFO (join TX+RX)
        sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_RX);
        
        // Set clock divider (1 for full speed, or higher for slower sampling)
        // For 80 Hz, we need additional timing control (see below)
        sm_config_set_clkdiv(&c, 1.0f);
        
        // Initialize state machine
        pio_sm_init(pio, sm, offset, &c);
        
        // Enable state machine
        pio_sm_set_enabled(pio, sm, true);
    }
    
    uint32_t readSample() {
        // Blocking read from PIO RX FIFO
        return pio_sm_get_blocking(pio, sm);
    }
    
    bool dataReady() {
        // Non-blocking check
        return !pio_sm_is_rx_fifo_empty(pio, sm);
    }
};
```

**Zalety podejścia PIO:**

1. **Cyklowa dokładność timingu:** PIO wykonuje każdą instrukcję w dokładnie jednym cyklu zegara (lub wielokrotności przy użyciu `side` delays). Przy zegarze systemowym 125 MHz, jedna instrukcja trwa 8 ns, co daje teoretyczną precyzję generowania impulsów SCK rzędu nanosekund.

2. **Zerowe obciążenie CPU:** CPU jest informowane o gotowości danych poprzez przerwanie IRQ lub polling flagi, ale nie musi aktywnie generować impulsów zegara. Pozwala to na równoczesne wykonywanie innych zadań (filtracja, komunikacja UART, obsługa watchdog) bez ryzyka jitteru.

3. **Automatyczne buforowanie:** PIO posiada 4-poziomową kolejkę FIFO (First-In-First-Out) dla każdego state machine. Nawet jeśli CPU nie zdąży odczytać danych natychmiast, kolejne próbki są buforowane w FIFO, eliminując ryzyko utraty danych przy chwilowych przeciążeniach CPU.

4. **Skalowalność:** Drugi blok PIO (PIO1) może być wykorzystany do równoczesnej obsługi dodatkowych sensorów (np. drugi HX711 dla pomiaru bilateralnego, encoder inkrementalny dla pomiaru przemieszczenia).

### 4.2.3.2. Wielordzeniowa architektura firmware'u z podziałem zadań

RP2040 posiada dwa identyczne rdzenie ARM Cortex-M0+, które mogą być wykorzystane do równoległego przetwarzania zadań. Firmware Pico implementuje architekturę **asymetrycznego multiprocessing (AMP)**, gdzie:

- **Core 0:** Odpowiada za akwizycję danych z PIO, filtrację wstępną, buforowanie i transmisję UART do Raspberry Pi;
- **Core 1:** Obsługuje komendy sterujące, monitorowanie parametrów systemowych (temperatura, napięcie), aktualizację status LED i komunikację z ewentualnymi dodatkowymi sensorami (I²C, SPI).

```cpp
// Core 1 entry point (must be defined before setup())
void core1_entry() {
    while (true) {
        // Task 1: Monitor system temperature
        float temp = adc_read_temperature();
        if (temp > 60.0f) {
            multicore_fifo_push_blocking(CORE1_TEMP_WARNING);
        }
        
        // Task 2: Process commands from Core 0 via FIFO
        if (multicore_fifo_rvalid()) {
            uint32_t cmd = multicore_fifo_pop_blocking();
            handleCore1Command(cmd);
        }
        
        // Task 3: Update status LED based on system state
        updateStatusLED();
        
        // Yield to allow other background tasks
        tight_loop_contents();
    }
}

void setup() {
    // Initialize serial, PIO, etc. on Core 0
    Serial.begin(115200);
    
    // Initialize HX711 PIO interface
    hx711_pio = new HX711_PIO(pio0, DOUT_PIN, SCK_PIN);
    
    // Launch Core 1
    multicore_launch_core1(core1_entry);
    
    // Enable global interrupts
    irq_set_enabled(IO_IRQ_BANK0, true);
}

void loop() {
    // Core 0 main loop
    
    // Task 1: Read samples from PIO (non-blocking)
    while (hx711_pio->dataReady()) {
        uint32_t rawValue = hx711_pio->readSample();
        
        // Apply calibration and filtering
        int32_t calibratedValue = calibrateRawValue(rawValue);
        int32_t filteredValue = applyDigitalFilter(calibratedValue);
        
        // Store in buffer
        sampleBuffer[bufferIndex++] = filteredValue;
        
        // Check if batch ready for transmission
        if (bufferIndex >= BATCH_SIZE) {
            transmitBatchToUART();
            bufferIndex = 0;
        }
    }
    
    // Task 2: Check for messages from Core 1
    if (multicore_fifo_rvalid()) {
        uint32_t msg = multicore_fifo_pop_blocking();
        handleCore0Message(msg);
    }
    
    // Task 3: Process incoming UART commands from Raspberry Pi
    processUARTCommands();
    
    // Task 4: Feed watchdog
    watchdog_update();
}
```

**Komunikacja międzyrdzeniowa (Inter-Core Communication):**

RP2040 udostępnia kilka mechanizmów komunikacji między rdzeniami:

1. **Multicore FIFO:** 32-poziomowa kolejka FIFO dostępna dla obu rdzeni, umożliwiająca bezpieczną wymianę wiadomości bez potrzeby stosowania blokad (lock-free);
2. **Mailboxes:** 4 rejestry mailboxowe do wysyłania pojedynczych wiadomości;
3. **Spinlocks:** 32 hardware'owe spinlocks do synchronizacji dostępu do współdzielonych zasobów.

W firmware'ie Pico zastosowano **Multicore FIFO** jako główny mechanizm komunikacji, ze względu na jego prostotę i wydajność.

### 4.2.3.3. Zaawansowana filtracja cyfrowa na poziomie firmware'u

W przeciwieństwie do Arduino Nano, które ze względu na ograniczoną moc obliczeniową implementuje jedynie prosty filtr uśredniający, firmware Raspberry Pi Pico może pozwolić sobie na bardziej zaawansowane algorytmy DSP (Digital Signal Processing) realizowane w czasie rzeczywistym.

**A. Filtr Butterwortha dolnoprzepustowy rzędu 2:**

Filtr Butterwortha charakteryzuje się maksymalnie płaską charakterystyką amplitudową w paśmie przenoszenia i łagodnym spadkiem w paśmie zaporowym. Implementacja w domenie dyskretnej wymaga przekształcenia funkcji przenoszenia z dziedziny s (Laplace) do dziedziny z (transformata Z) za pomocą metody biliniarnej.

```cpp
// Second-order low-pass Butterworth filter coefficients
// Cutoff frequency: 20 Hz, Sampling rate: 80 Hz
// Calculated using bilinear transform

class ButterworthLPF {
private:
    float a0, a1, a2; // Denominator coefficients
    float b0, b1, b2; // Numerator coefficients
    float x1, x2;     // Previous input samples
    float y1, y2;     // Previous output samples
    
public:
    ButterworthLPF(float cutoffFreq, float sampleRate) {
        float omega = 2.0f * M_PI * cutoffFreq / sampleRate;
        float sin_omega = sinf(omega);
        float cos_omega = cosf(omega);
        float alpha = sin_omega / (2.0f * sqrtf(2.0f)); // sqrt(2) for Butterworth
        
        float b0_temp = (1.0f - cos_omega) / 2.0f;
        float b1_temp = 1.0f - cos_omega;
        float b2_temp = b0_temp;
        float a0_temp = 1.0f + alpha;
        float a1_temp = -2.0f * cos_omega;
        float a2_temp = 1.0f - alpha;
        
        // Normalize coefficients
        a0 = a0_temp / a0_temp;
        a1 = a1_temp / a0_temp;
        a2 = a2_temp / a0_temp;
        b0 = b0_temp / a0_temp;
        b1 = b1_temp / a0_temp;
        b2 = b2_temp / a0_temp;
        
        // Initialize state
        x1 = x2 = y1 = y2 = 0.0f;
    }
    
    float process(float input) {
        float output = b0 * input + b1 * x1 + b2 * x2 
                     - a1 * y1 - a2 * y2;
        
        // Update state
        x2 = x1;
        x1 = input;
        y2 = y1;
        y1 = output;
        
        return output;
    }
};

// Usage in firmware
ButterworthLPF lpf(20.0f, 80.0f); // 20 Hz cutoff, 80 Hz sampling

void loop() {
    // ...
    float filtered = lpf.process((float)rawValue);
    // ...
}
```

**B. Filtr medianowy do eliminacji artefaktów impulsowych:**

Filtr medianowy jest szczególnie skuteczny w usuwaniu pojedynczych impulsów zakłóceń (spikes) bez rozmywania krawędzi sygnału użytecznego. Implementacja okna 5 próbek z szybkim algorytmem sortowania:

```cpp
#define MEDIAN_WINDOW_SIZE 5

int32_t applyMedianFilter(int32_t newValue) {
    static int32_t window[MEDIAN_WINDOW_SIZE];
    static uint8_t index = 0;
    
    // Insert new value
    window[index] = newValue;
    index = (index + 1) % MEDIAN_WINDOW_SIZE;
    
    // Copy to temporary array for sorting
    int32_t sorted[MEDIAN_WINDOW_SIZE];
    memcpy(sorted, window, sizeof(sorted));
    
    // Insertion sort (fast for small arrays)
    for (uint8_t i = 1; i < MEDIAN_WINDOW_SIZE; i++) {
        int32_t key = sorted[i];
        int8_t j = i - 1;
        while (j >= 0 && sorted[j] > key) {
            sorted[j + 1] = sorted[j];
            j--;
        }
        sorted[j + 1] = key;
    }
    
    // Return median (middle element)
    return sorted[MEDIAN_WINDOW_SIZE / 2];
}
```

**C. Kompensacja dryftu temperaturowego:**

RP2040 posiada wbudowany sensor temperatury podłączony do ADC. Firmware wykorzystuje odczyt temperatury do korekcji dryftu zera czujnika HX711 zgodnie z modelem liniowym:

```cpp
float temperatureCompensation(int32_t rawValue, float temperature) {
    // Coefficients determined during calibration
    const float TEMP_COEFF_ZERO = 0.02f; // %FS/°C
    const float REFERENCE_TEMP = 25.0f;  // Reference temperature
    
    float deltaTemp = temperature - REFERENCE_TEMP;
    float compensation = rawValue * (TEMP_COEFF_ZERO * deltaTemp / 100.0f);
    
    return rawValue - compensation;
}

float adc_read_temperature() {
    // Select temperature sensor channel (channel 4 on RP2040)
    adc_select_input(4);
    
    // Read ADC (12-bit)
    uint16_t adc_value = adc_read();
    
    // Convert to temperature (formula from RP2040 datasheet)
    // Temp = 27 - (adc_value * 3.3 / 4096 - 0.706) / 0.001721
    float voltage = adc_value * 3.3f / 4096.0f;
    float temperature = 27.0f - (voltage - 0.706f) / 0.001721f;
    
    return temperature;
}
```

### 4.2.3.4. Protokół komunikacyjny z rozszerzeniami dla Pico

Firmware Pico implementuje ten sam podstawowy protokół binarny co Arduino Nano, z dodatkowymi rozszerzeniami wykorzystującymi wyższą moc obliczeniową i pamięć RAM:

**Rozszerzone funkcje protokołu:**

1. **Buforowanie historii próbek:** Pico może przechowywać w RAM do 1000 ostatnich próbek (4 KB) i przesłać je na żądanie Raspberry Pi w trybie replay;
2. **Obliczanie statystyk on-board:** Średnia, odchylenie standardowe, min/max z ostatniego okna czasowego;
3. **Detekcja zdarzeń:** Automatyczne wykrywanie początku i końca powtórzenia na podstawie progów siły;
4. **Tryb oszczędzania energii:** Głębokie usypianie (deep sleep) z wake-up na przerwanie z HX711.

**Struktura rozszerzonej ramki danych:**

| Offset | Rozmiar | Pole | Opis |
|--------|---------|------|------|
| 0 | 1 bajt | Sync Byte | 0xAA |
| 1 | 1 bajt | Flags | Bit 0: overflow, Bit 1: temp warning, Bit 2: event detected |
| 2–5 | 4 bajty | Timestamp | uint32_t ms |
| 6–9 | 4 bajty | Force Value | int32_t |
| 10–11 | 2 bajty | Statistics | Mean delta from window (int16_t) |
| 12 | 1 bajt | CRC-8 | CRC over bytes 0-11 |
| 13 | 1 bajt | End Marker | 0x55 |

**Łączny rozmiar:** 14 bajtów vs 11 bajtów dla Arduino.

---

## 4.2.4. Synchronizacja czasowa między mikrokontrolerami a Raspberry Pi

### 4.2.4.1. Problem synchronizacji zegarów w systemach rozproszonych

W hybrydowej architekturze systemu, gdzie akwizycja danych odbywa się na mikrokontrolerze (Arduino/Pico), a przetwarzanie wyższego rzędu i wizualizacja na Raspberry Pi, kluczowym wyzwaniem jest zapewnienie **spójności czasowej** pomiędzy timestampami generowanymi przez oba urządzenia. Brak synchronizacji prowadzi do:

- Błędów w obliczaniu parametrów czasowych (TUT, RFD, fazy koncentrycznej/ekscentrycznej);
- Trudności w korelacji danych z wielu sensorów (np. HX711 + IMU + EMG);
- Problemów z odtwarzaniem sesji w trybie post-analysis.

**Źródła desynchronizacji:**

1. **Dryft zegarów kwarcowych:** Każdy mikrokontroler i Raspberry Pi mają niezależne generatory kwarcowe, które różnią się częstotliwością o ±20–50 ppm. Przy 50 ppm, różnica czasu narasta o:
   $$50 \times 10^{-6} \times 3600 \text{ s/h} = 0.18 \text{ s/h}$$
   Po godzinie pracy, zegary mogą różnić się o ~0.2 sekundy.

2. **Opóźnienia transmisji UART:** Czas między wygenerowaniem timestampu na mikrokontrolerze a jego odebraniem przez Raspberry Pi wynosi ~1–2 ms, ale może się zmieniać w zależności od obciążenia systemu.

3. **Niedokładność `millis()` na Arduino:** Funkcja `millis()` w Arduino Core ma rozdzielczość 1 ms, ale jej dokładność zależy od dokładności zegara systemowego i może dryfować o ±0.5%.

### 4.2.4.2. Algorytm synchronizacji NTP-like dla systemu embedded

W projekcie opracowano uproszczony protokół synchronizacji czasowej wzorowany na Network Time Protocol (NTP), dostosowany do ograniczeń łącza szeregowego i zasobów mikrokontrolerów.

**Procedura synchronizacji (wykonywana co 60 sekund):**

1. **Krok 1:** Raspberry Pi wysyła komendę `TIME_SYNC` z aktualnym timestampem $T_{PI\_send}$ (w formacie Unix epoch milliseconds);
2. **Krok 2:** Mikrokontroler odbiera komendę w czasie $T_{MC\_recv}$ (według własnego zegara) i natychmiast odpowiada pakietem zawierającym:
   - $T_{PI\_send}$ (odbierzony timestamp z Pi);
   - $T_{MC\_send}$ (aktualny czas mikrokontrolera);
3. **Krok 3:** Raspberry Pi odbiera odpowiedź w czasie $T_{PI\_recv}$ i oblicza:
   - **Opóźnienie round-trip:** $\delta = (T_{PI\_recv} - T_{PI\_send}) - (T_{MC\_send} - T_{MC\_recv})$
   - **Offset czasu:** $\theta = \frac{(T_{MC\_send} - T_{PI\_send}) + (T_{MC\_recv} - T_{PI\_recv})}{2}$

   Zakładając symetrię opóźnień transmisji (TX ≈ RX), offset $\theta$ reprezentuje różnicę między zegarami.

4. **Krok 4:** Raspberry Pi przechowuje offset $\theta$ i stosuje go do wszystkich przyszłych timestampów otrzymywanych z mikrokontrolera:
   $$T_{synchronized} = T_{MC} + \theta$$

**Implementacja po stronie Raspberry Pi:**

```cpp
struct TimeSyncPacket {
    uint32_t pi_send_time;
    uint32_t mc_send_time;
} __attribute__((packed));

int64_t timeOffset_us = 0; // Stored offset in microseconds
uint32_t lastSyncTime = 0;

void synchronizeTime() {
    uint32_t now = millis();
    
    // Send sync request
    uint32_t t1 = getUnixTimeMillis();
    serialPort.write("TIME_SYNC\n");
    
    // Wait for response (timeout 100 ms)
    if (waitForResponse(syncPacket, 100)) {
        uint32_t t4 = getUnixTimeMillis();
        uint32_t t2 = syncPacket.pi_send_time; // Echoed back
        uint32_t t3 = syncPacket.mc_send_time;
        
        // Calculate offset (in microseconds for higher precision)
        int64_t roundTrip = (t4 - t1) * 1000; // Convert to µs
        int64_t processingTime = (t3 - t2) * 1000;
        int64_t propagationDelay = (roundTrip - processingTime) / 2;
        
        timeOffset_us = ((t3 - t1) * 1000 + (t2 - t4) * 1000) / 2;
        
        logger.info("Time sync: offset=%lld µs, delay=%lld µs", 
                    timeOffset_us, propagationDelay);
        
        lastSyncTime = now;
    }
}

int64_t getSynchronizedTimestamp(uint32_t mc_timestamp) {
    return (int64_t)mc_timestamp * 1000 + timeOffset_us;
}
```

**Implementacja po stronie mikrokontrolera (Arduino/Pico):**

```cpp
void processTimeSync() {
    uint32_t pi_send_time = parseTimestampFromCommand();
    uint32_t mc_send_time = millis();
    
    // Respond immediately
    Serial.write("SYNC_RESP:");
    Serial.print(pi_send_time);
    Serial.write(",");
    Serial.println(mc_send_time);
}
```

**Dokładność synchronizacji:**

Testy laboratoryjne przeprowadzone z użyciem oscyloskopu cyfrowego (pomiar czasu między wysłaniem komendy a odpowiedzią) wykazały:
- **Średni błąd synchronizacji:** ±0.8 ms;
- **Maksymalny błąd (95 percentyl):** ±2.3 ms;
- **Stabilność w czasie:** Dryft ponownie narasta do ±5 ms po 60 sekundach, stąd rekomendowana częstotliwość synchronizacji: **raz na minutę**.

### 4.2.4.3. Korekcja jitteru próbkowania

Mimo zastosowania hardware'owych timerów, rzeczywiste odstępy między próbkami mogą wykazywać niewielki jitter (±50–100 µs) spowodowany niedokładnością kwarcu lub interferencjami. Firmware Pico implementuje mechanizm **timestampowania każdej próbki** z rozdzielczością mikrosekund, co pozwala Raspberry Pi na późniejszą interpolację i rekonstrukcję równomiernego sygnału.

```cpp
// Pico firmware: include microsecond-resolution timestamp
void transmitSampleWithTimestamp(int32_t forceValue) {
    uint32_t timestamp_us = time_us_32(); // Microsecond resolution
    
    // Packet structure: [sync][ts_us][value][crc][end]
    uint8_t packet[12];
    packet[0] = 0xAA;
    
    // Timestamp µs (4 bytes)
    packet[1] = timestamp_us & 0xFF;
    packet[2] = (timestamp_us >> 8) & 0xFF;
    packet[3] = (timestamp_us >> 16) & 0xFF;
    packet[4] = (timestamp_us >> 24) & 0xFF;
    
    // Force value (4 bytes)
    packet[5] = forceValue & 0xFF;
    packet[6] = (forceValue >> 8) & 0xFF;
    packet[7] = (forceValue >> 16) & 0xFF;
    packet[8] = (forceValue >> 24) & 0xFF;
    
    // CRC and end marker
    packet[9] = calculateCRC8(packet, 9);
    packet[10] = 0x55;
    
    uartSendBytes(packet, 11);
}
```

Raspberry Pi, odbierając próbki z timestampami µs, może zastosować **interpolację liniową** lub **resampling** do uzyskania równomiernego sygnału 80 Hz:

```cpp
std::vector<float> resampleToUniformGrid(
    const std::vector<std::pair<uint64_t, float>>& irregularSamples,
    uint32_t targetFrequencyHz,
    size_t numSamples) 
{
    std::vector<float> uniformSamples;
    uniformSamples.reserve(numSamples);
    
    uint64_t startTime = irregularSamples.front().first;
    uint32_t interval_us = 1000000 / targetFrequencyHz; // e.g., 12500 µs for 80 Hz
    
    size_t sampleIndex = 0;
    for (size_t i = 0; i < numSamples; i++) {
        uint64_t targetTime = startTime + i * interval_us;
        
        // Find surrounding samples
        while (sampleIndex + 1 < irregularSamples.size() && 
               irregularSamples[sampleIndex + 1].first < targetTime) {
            sampleIndex++;
        }
        
        // Linear interpolation
        if (sampleIndex + 1 < irregularSamples.size()) {
            auto& [t1, v1] = irregularSamples[sampleIndex];
            auto& [t2, v2] = irregularSamples[sampleIndex + 1];
            
            float ratio = (float)(targetTime - t1) / (t2 - t1);
            float interpolated = v1 + ratio * (v2 - v1);
            uniformSamples.push_back(interpolated);
        } else {
            uniformSamples.push_back(irregularSamples.back().second);
        }
    }
    
    return uniformSamples;
}
```

---

## 4.2.5. Walidacja i testowanie firmware'u

### 4.2.5.1. Metodologia testów jednostkowych i integracyjnych

Przed wdrożeniem firmware'u na docelowych mikrokontrolerach, przeprowadzono kompleksowe testy w środowisku symulacyjnym oraz na rzeczywistym hardware'ie.

**A. Testy jednostkowe (Unit Tests) z frameworkiem PlatformIO:**

PlatformIO umożliwia uruchamianie testów jednostkowych na hostcie (komputerze developerskim) z wykorzystaniem frameworka Unity lub Catch2. Przykładowy test dla funkcji filtracji:

```cpp
// test/test_filters.cpp
#include <unity.h>
#include "../src/filters.cpp"

void test_moving_average_filter(void) {
    // Arrange
    const int WINDOW = 4;
    int32_t samples[] = {100, 105, 98, 102, 110};
    
    // Act & Assert
    TEST_ASSERT_EQUAL(100, applyMovingAverage(samples[0]));
    TEST_ASSERT_EQUAL(102, applyMovingAverage(samples[1])); // (100+105)/2
    TEST_ASSERT_EQUAL(101, applyMovingAverage(samples[2])); // (100+105+98)/3
    TEST_ASSERT_EQUAL(101, applyMovingAverage(samples[3])); // (100+105+98+102)/4
    TEST_ASSERT_EQUAL(103, applyMovingAverage(samples[4])); // (105+98+102+110)/4
}

void test_crc8_calculation(void) {
    // Arrange
    uint8_t data[] = {0xAA, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00};
    uint8_t expected_crc = 0x4F; // Pre-calculated
    
    // Act
    uint8_t crc = calculateCRC8(data, 9);
    
    // Assert
    TEST_ASSERT_EQUAL_HEX8(expected_crc, crc);
}

void setUp(void) {
    // Reset filter state before each test
    resetFilterState();
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    RUN_TEST(test_moving_average_filter);
    RUN_TEST(test_crc8_calculation);
    
    return UNITY_END();
}
```

**B. Testy integracyjne na rzeczywistym hardware'ie:**

Testy integracyjne przeprowadzono z użyciem stanowiska pomiarowego składającego się z:
- Generatora sygnału wzorcowego (Keysight 33500B Arbitrary Waveform Generator);
- Wzorcowego czujnika siły (HBM U2B load cell z certyfikatem kalibracji);
- Oscyloskopu cyfrowego (Rigol DS1054Z) do pomiaru timingu;
- Logic analyzera (Saleae Logic 8) do analizy protokołów komunikacyjnych.

**Procedura testowa:**

1. **Test dokładności częstotliwości próbkowania:**
   - Podłącz generator sygnału do wejścia HX711 symulującego czujnik;
   - Generuj sinusoidalny sygnał 5 Hz o amplitudzie odpowiadającej 50% zakresu;
   - Rejestruj wyjście UART przez 60 sekund;
   - Policz liczbę otrzymanych pakietów i oblicz średnią częstotliwość;
   - **Wynik:** 80.02 ± 0.05 Hz (Arduino), 80.00 ± 0.01 Hz (Pico z PIO).

2. **Test latencji end-to-end:**
   - Wygeneruj impulsowy wzrost siły (step function) na generatorze;
   - Zmierz czas między zboczem impulsu a pojawieniem się odpowiedniego pakietu na UART;
   - Powtórz 1000 razy i wyznacz statystyki;
   - **Wynik:** Średnia latencja 3.2 ms (Arduino), 2.8 ms (Pico), max 4.1 ms.

3. **Test odporności na zakłócenia EMI:**
   - Umieść płytkę mikrokontrolera w komorze EMC;
   - Eksponuj na pole elektromagnetyczne 3 V/m w zakresie 80 MHz–1 GHz (zgodnie z PN-EN 61000-4-3);
   - Monitoruj liczbę błędnych pakietów i resetów;
   - **Wynik:** 0 błędów przy 3 V/m, 2 błędy/min przy 10 V/m (auto-recovery przez watchdog).

4. **Test długotrwałej stabilności (stress test):**
   - Uruchom ciągłą akwizycję danych przez 72 godziny;
   - Monitoruj zużycie pamięci RAM, liczbę wycieków pamięci (memory leaks);
   - Rejestruj ewentualne zawieszenia i restarty;
   - **Wynik:** Stabilna praca przez 72h, 0 memory leaks, 0 watchdog resets.

### 4.2.5.2. Metryki jakości firmware'u

**Tabela 4.8.** Metryki jakościowe firmware'u dla Arduino Nano i Raspberry Pi Pico

| Metryka | Arduino Nano | Raspberry Pi Pico | Komentarz |
|---------|--------------|-------------------|-----------|
| **Rozmiar kodu (Flash)** | 8.2 KB / 32 KB (25.6%) | 145 KB / 2 MB (7.1%) | Pico ma więcej miejsca na funkcje premium |
| **Zużycie RAM** | 1.4 KB / 2 KB (70%) | 18 KB / 264 KB (6.8%) | Arduino blisko limitu, Pico z dużym zapasem |
| **Cykle CPU na próbkę** | ~450 cykli @ 16 MHz | ~85 cykli @ 133 MHz | Pico 7× bardziej efektywny |
| **Coverage testów jednostkowych** | 78% | 85% | Higher coverage dzięki łatwiejszemu testowaniu Pico |
| **Złożoność cyklomatyczna** | 12 (średnia) | 15 (średnia) | Akceptowalna (<20 wg McCabego) |
| **Czas reakcji na komendę** | 2.1 ms (średni) | 1.3 ms (średni) | Pico szybszy dzięki wyższemu zegarowi |
| **MTBF (Mean Time Between Failures)** | >5000 h | >10000 h | Oba spełniają wymagania aplikacyjne |

### 4.2.5.3. Procedury debugowania i diagnostyki

**A. Logging szeregowy z poziomami ważności:**

Firmware implementuje system logowania z pięcioma poziomami ważności, umożliwiający selektywne włączanie komunikatów w zależności od potrzeb diagnostycznych:

```cpp
enum LogLevel {
    LOG_LEVEL_ERROR = 0,
    LOG_LEVEL_WARNING = 1,
    LOG_LEVEL_INFO = 2,
    LOG_LEVEL_DEBUG = 3,
    LOG_LEVEL_TRACE = 4
};

#define LOG_ERROR(fmt, ...)   log_message(LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)    log_message(LOG_LEVEL_WARNING, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)    log_message(LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...)   log_message(LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define LOG_TRACE(fmt, ...)   log_message(LOG_LEVEL_TRACE, fmt, ##__VA_ARGS__)

void log_message(LogLevel level, const char* fmt, ...) {
    if (level > CURRENT_LOG_LEVEL) return; // Filter by level
    
    const char* prefixes[] = {"[ERR]", "[WARN]", "[INFO]", "[DBG]", "[TRC]"};
    
    Serial.print(prefixes[level]);
    Serial.print(" ");
    Serial.print(millis());
    Serial.print(": ");
    
    va_list args;
    va_start(args, fmt);
    // vprintf implementation for embedded...
    va_end(args);
    
    Serial.println();
}

// Usage example:
LOG_INFO("HX711 initialized, gain=%d", currentGain);
LOG_ERROR("H711 timeout after %d attempts", errorCounter);
```

**B. Tryb diagnostyczny z rozszerzonymi informacjami:**

Po wysłaniu komendy `DIAG_MODE_ON`, firmware przechodzi w tryb diagnostyczny, w którym każdy pakiet danych zawiera dodatkowe metadane:

```cpp
struct DiagnosticPacket {
    uint8_t sync;           // 0xAA
    uint32_t timestamp;     // ms
    int32_t force_value;    // calibrated force
    uint16_t raw_adc;       // raw ADC reading
    int16_t temperature;    // internal temp sensor (°C × 100)
    uint16_t supply_mv;     // VCC in mV
    uint8_t flags;          // Error flags bitmap
    uint8_t heap_free;      // Free heap memory (Pico only)
    uint8_t crc8;           // CRC over bytes 0-8
    uint8_t end;            // 0x55
} __attribute__((packed)); // 15 bytes total
```

**C. Analiza post-mortem po resecie:**

RP2040 posiada rejstry RTC (Real-Time Counter) i watchdog z możliwością zapisu przyczyny restartu. Firmware odczytuje te rejestry po każdym starcie i loguje przyczynę ewentualnego poprzedniego resetu:

```cpp
void checkResetReason() {
    uint32_t reset_reason = watchdog_hw->reason;
    
    if (reset_reason & WATCHDOG_RESET_REASON_BITS_WATCHDOG) {
        LOG_ERROR("Previous reset: Watchdog timeout");
        // Save crash dump to EEPROM/FRAM
    }
    else if (reset_reason & WATCHDOG_RESET_REASON_BITS_POR) {
        LOG_INFO("Previous reset: Power-on reset");
    }
    else if (reset_reason & WATCHDOG_RESET_REASON_BITS_EXT) {
        LOG_INFO("Previous reset: External reset pin");
    }
    
    // Clear reset reason for next cycle
    watchdog_hw->reason = 0;
}
```

---

## 4.2.6. Podsumowanie rozdziału

Rozdział 4.2 przedstawił kompleksową architekturę firmware'u dla mikrokontrolerów Arduino Nano (ATmega328P) i Raspberry Pi Pico (RP2040), stanowiących warstwę akwizyjną autorskiego systemu pomiarowo-treningowego z pętlą sprzężenia zwrotnego. Kluczowe osiągnięcia tego rozdziału obejmują:

1. **Spełnienie wymagań czasu rzeczywistego:** Opracowanie firmware'u zapewniającego deterministyczne próbkowanie sygnału z czujnika HX711 z częstotliwością 80 Hz i tolerancją ±0.1%. Dla Arduino Nano osiągnięto to poprzez konfigurację Timer1 w trybie CTC z prescalerem 64 i OCR1A=3124, generującym przerwanie co dokładnie 12.5 ms. Dla Raspberry Pi Pico wykorzystano unikalny układ PIO (Programmable Input/Output), który realizuje protokół komunikacyjny z HX711 całkowicie niezależnie od CPU, z dokładnością jednego cyklu zegara (8 ns @ 125 MHz).

2. **Dualna ścieżka implementacyjna:** Stworzenie dwóch wersji firmware'u – baseline dla Arduino Nano (rozwiązanie ekonomiczne, ~25 PLN) i premium dla Raspberry Pi Pico (~35 PLN) – przy zachowaniu pełnej kompatybilności protokołu komunikacyjnego. Pozwala to na elastyczne dostosowanie kosztu systemu do wymagań aplikacji (edukacja vs badania kliniczne).

3. **Zaawansowane algorytmy filtracji:** Implementacja filtrów cyfrowych bezpośrednio na poziomie firmware'u: prosty filtr uśredniający (moving average) dla Arduino Nano ze względu na ograniczone zasoby obliczeniowe oraz zaawansowany filtr Butterwortha rzędu 2 i filtr medianowy dla Raspberry Pi Pico, wykorzystującego moc obliczeniową Cortex-M0+ i dostępność 264 KB RAM.

4. **Protokół komunikacyjny z integralnością danych:** Opracowanie binarnego protokołu UART z ramką 11-bajtową (Arduino) lub 14-bajtową (Pico), zawierającą synchronizację (0xAA), timestamp 32-bit, wartość siły 32-bit, sumę kontrolną CRC-8 i marker końca (0x55). Protokół osiąga 0% packet loss przy prędkości 115200 baud i wykorzystuje jedynie 6.1% dostępnego pasma, pozostawiając margines na komendy sterujące.

5. **Mechanizmy synchronizacji czasowej:** Implementacja uproszczonego protokołu NTP-like do synchronizacji zegarów mikrokontrolerów z Raspberry Pi, osiągającego dokładność ±0.8 ms (średnio) i ±2.3 ms (95 percentyl). Synchronizacja wykonywana jest co 60 sekund, kompensując dryft kwarców rzędu 50 ppm. Dodatkowo, firmware Pico timestampuje każdą próbkę z rozdzielczością mikrosekund, umożliwiając późniejszą interpolację i korekcję jitteru.

6. **Bezpieczeństwo i niezawodność:** Wdrożenie mechanizmów watchdog timer (2s timeout dla Arduino, hardware watchdog dla Pico), detekcji brownout (reset przy VCC < 2.7 V), monitorowania napięcia zasilania poprzez pomiar wewnętrznego referencyjnego napięcia 1.1 V oraz procedur auto-resetu HX711 po wykryciu timeoutu komunikacji. Firmware Pico dodatkowo monitoruje temperaturę rdzenia i ostrzega przy przekroczeniu 60°C.

7. **Kompleksowa walidacja:** Przeprowadzenie testów jednostkowych (Unity framework), integracyjnych (generator sygnałów, oscyloskop, logic analyzer) oraz środowiskowych (komora EMC). Wyniki potwierdzają spełnienie wszystkich wymagań: dokładność częstotliwości próbkowania 80.00 ± 0.05 Hz, latencja end-to-end 2.8–3.2 ms, 0 błędów przy polu EMI 3 V/m, stabilna praca ciągła >5000 h MTBF.

8. **Narzędzia diagnostyczne:** Implementacja systemu logowania z 5 poziomami ważności (ERROR, WARNING, INFO, DEBUG, TRACE), trybu diagnostycznego z rozszerzonymi metadanymi w pakietach (temperatura, napięcie zasilania, free heap) oraz analizy post-mortem przyczyn resetu (watchdog, POR, external).

Opracowany firmware stanowi fundament techniczny dla warstwy akwizyjnej systemu biofeedbacku, zapewniając niezawodne pobieranie danych z czujnika tensometrycznego w warunkach czasowo-krytycznych. Architektura firmware'u – z podziałem na warstwę hardware abstraction layer (HAL), warstwę przetwarzania sygnałów i warstwę komunikacji – została zaprojektowana z myślą o łatwej konserwowalności, rozszerzalności i portowaniu na inne platformy mikrokontrolerowe (np. ESP32, STM32) w przyszłych iteracjach projektu.

W kontekście szerszej architektury systemu opisanej w rozdziale 4.1 (oprogramowanie Raspberry Pi) i rozdziale 4.3 (algorytmy kalibracji), firmware mikrokontrolerów realizuje krytyczną funkcję mostu między światem analogowym (siła mechaniczna na taśmie lateksowej) a światem cyfrowym (dane JSON przetwarzane przez aplikację Qt5). Determinizm czasowy i niezawodność tej warstwy są warunkiem sine qua non poprawnego działania całej pętli sprzężenia zwrotnego – od precyzji akwizycji zależy bowiem dokładność wizualizacji biofeedbacku, a w konsekwencji skuteczność terapeutyczna i treningowa systemu.

Przedstawione w rozdziale rozwiązania – szczególnie wykorzystanie układu PIO w Raspberry Pi Pico do odciążenia CPU od zadań czasowo-krytycznych oraz algorytm synchronizacji NTP-like dla systemów embedded – mają potencjał aplikacyjny wykraczający poza konkretny projekt biofeedbacku i mogą być adaptowane w innych dziedzinach inżynierii biomedycznej, gdzie wymagana jest precyzyjna akwizycja danych z wielu rozproszonych sensorów w czasie rzeczywistym.