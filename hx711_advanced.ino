/*
 * Advanced Arduino HX711 Scale with High-Speed Serial, Watchdog, Pseudo-threading, CRC
 * 
 * Features:
 * - Max serial throughput (115200 baud)
 * - Hardware watchdog timer
 * - Cooperative pseudo-threading (non-blocking multitasking)
 * - CRC-8 checksum for data integrity
 * - Tare command via serial
 * - Binary data transmission at 80Hz with timestamp, value, CRC
 * - Device identification on command
 * 
 * Connections:
 * HX711      Arduino Nano/Uno
 * VCC   ---> 5V (or 3.3V)
 * GND   ---> GND
 * DT    ---> D2 (Data)
 * SCK   ---> D3 (Clock)
 * 
 * Commands (send via Serial):
 * - "ID" or "id": Get device identification
 * - "TARE" or "tare": Tare the scale
 * - "DATA" or "data": Request binary data packet
 * - "CONT" or "cont": Toggle continuous data mode
 * - "STOP" or "stop": Stop continuous mode
 */

#include <avr/wdt.h>  // Hardware Watchdog Timer
#include <util/crc16.h> // CRC utilities

// Pin definitions
#define HX711_DT_PIN  2   // Data pin
#define HX711_SCK_PIN 3   // Clock pin

// Number of samples for averaging (1 sample for 80Hz output)
#define HX711_SAMPLES 1

// Serial baud rate for maximum throughput
#define SERIAL_BAUD_RATE 115200

// Binary packet structure
#pragma pack(push, 1)
struct DataPacket {
    uint32_t timestamp;  // Milliseconds since start
    int32_t value;       // Averaged HX711 reading
    uint8_t crc;         // CRC-8 checksum
};
#pragma pack(pop)

// Global variables
volatile bool hx711_data_ready = false;
volatile unsigned long last_hx711_read = 0;
int32_t current_weight = 0;
int32_t tare_offset = 0;
bool continuous_mode = false;
unsigned long last_data_send = 0;

// Pseudo-thread timing
unsigned long thread_timer_hx711 = 0;
unsigned long thread_timer_serial = 0;
unsigned long thread_timer_watchdog = 0;

#define HX711_THREAD_INTERVAL 10    // Read HX711 every 10ms
#define SERIAL_THREAD_INTERVAL 5    // Check serial every 5ms
#define WATCHDOG_THREAD_INTERVAL 500 // Feed watchdog every 500ms

// Device identification
const char DEVICE_ID[] = "HX711-SCALE-V1.0";
const char DEVICE_NAME[] = "Advanced HX711 Weight Scale";

// Forward declarations
void setupWatchdog();
void feedWatchdog();
void resetWatchdog();
long readHX711Raw();
int32_t readHX711Averaged(int samples);
void processSerialCommand();
void sendBinaryData();
void sendDeviceID();
void calculateCRC(uint8_t *data, size_t len, uint8_t *crc);
uint8_t crc8(const uint8_t *data, size_t length);

// ============================================================
// HARDWARE WATCHDOG FUNCTIONS
// ============================================================

void setupWatchdog() {
    // Disable interrupts during configuration
    cli();
    
    // Reset watchdog
    wdt_reset();
    
    // Configure watchdog timer
    // Set WDTCSR register for 1 second timeout
    WDTCSR |= (1 << WDCE) | (1 << WDE);  // Enable change flag
    WDTCSR = (1 << WDE) | (1 << WDIE) | (1 << WDP3) | (1 << WDP0);  // 1s timeout, interrupt mode
    
    // Enable interrupts
    sei();
}

void feedWatchdog() {
    wdt_reset();
}

// Watchdog Timer Interrupt Service Routine
ISR(WDT_vect) {
    // Watchdog interrupt - just reset and continue
    // This prevents system reset while allowing us to detect hangs
    static volatile uint8_t wdt_count = 0;
    wdt_count++;
}

void resetWatchdog() {
    // Force system reset using watchdog
    cli();
    WDTCSR |= (1 << WDCE) | (1 << WDE);
    WDTCSR = (1 << WDE);  // System reset mode, no interrupt
    while(1);  // Wait for reset
}

// ============================================================
// HX711 FUNCTIONS
// ============================================================

long readHX711Raw() {
    long count = 0;
    bool data_ready = false;
    
    // Wait for data ready (DT pin goes LOW)
    pinMode(HX711_DT_PIN, INPUT);
    
    unsigned long timeout = millis() + 100;  // 100ms timeout
    while (digitalRead(HX711_DT_PIN) == HIGH) {
        if (millis() > timeout) {
            return 0;  // Timeout
        }
        feedWatchdog();
    }
    
    // Send 24 clock pulses to read data
    pinMode(HX711_SCK_PIN, OUTPUT);
    digitalWrite(HX711_SCK_PIN, LOW);
    
    for (uint8_t i = 0; i < 24; i++) {
        digitalWrite(HX711_SCK_PIN, HIGH);
        delayMicroseconds(1);
        count = count << 1;
        digitalWrite(HX711_SCK_PIN, LOW);
        delayMicroseconds(1);
        if (digitalRead(HX711_DT_PIN)) {
            count++;
        }
    }
    
    // Send 25th pulse to set channel for next reading (Channel A, gain 128)
    digitalWrite(HX711_SCK_PIN, HIGH);
    delayMicroseconds(1);
    digitalWrite(HX711_SCK_PIN, LOW);
    delayMicroseconds(1);
    
    // Convert from 24-bit to 32-bit signed
    count ^= 0x800000;  // Flip MSB to get signed value
    
    return count;
}

int32_t readHX711Averaged(int samples) {
    // Read single sample for 80Hz output (no averaging needed)
    long reading = readHX711Raw();
    feedWatchdog();
    
    if (reading != 0) {
        return reading;
    }
    return 0;
}

// ============================================================
// CRC FUNCTIONS
// ============================================================

uint8_t crc8(const uint8_t *data, size_t length) {
    uint8_t crc = 0xFF;  // Initial value
    
    while (length--) {
        crc ^= *data++;
        for (uint8_t i = 0; i < 8; i++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x07;  // CRC-8 polynomial: x^8 + x^2 + x + 1
            } else {
                crc <<= 1;
            }
        }
    }
    
    return crc;
}

void calculateCRC(uint8_t *data, size_t len, uint8_t *crc) {
    *crc = crc8(data, len);
}

// ============================================================
// SERIAL COMMUNICATION FUNCTIONS
// ============================================================

void sendDeviceID() {
    Serial.print(F("DEVICE: "));
    Serial.println(DEVICE_NAME);
    Serial.print(F("ID: "));
    Serial.println(DEVICE_ID);
    Serial.print(F("VERSION: 1.0"));
    Serial.println();
    Serial.print(F("BAUD: "));
    Serial.println(SERIAL_BAUD_RATE);
    Serial.print(F("SAMPLES: "));
    Serial.println(HX711_SAMPLES);
    Serial.println(F("---"));
}

void sendBinaryData() {
    DataPacket packet;
    
    // Fill packet data
    packet.timestamp = millis();
    packet.value = current_weight - tare_offset;
    
    // Calculate CRC over timestamp and value (exclude CRC byte itself)
    uint8_t *data_ptr = (uint8_t *)&packet;
    packet.crc = crc8(data_ptr, sizeof(DataPacket) - 1);
    
    // Send binary packet
    Serial.write((uint8_t *)&packet, sizeof(DataPacket));
    
    // Send end marker for framing
    Serial.write(0xAA);
    Serial.write(0x55);
}

void processSerialCommand() {
    static char buffer[32];
    static uint8_t buffer_index = 0;
    
    while (Serial.available()) {
        char c = Serial.read();
        
        // Handle newline or carriage return as command terminator
        if (c == '\n' || c == '\r') {
            buffer[buffer_index] = '\0';
            
            if (buffer_index > 0) {
                // Convert to uppercase for comparison
                char upper_buffer[32];
                for (uint8_t i = 0; i < buffer_index && i < 31; i++) {
                    upper_buffer[i] = (buffer[i] >= 'a' && buffer[i] <= 'z') ? 
                                      buffer[i] - 32 : buffer[i];
                    upper_buffer[i+1] = '\0';
                }
                
                // Process commands
                if (strcmp(upper_buffer, "ID") == 0) {
                    sendDeviceID();
                }
                else if (strcmp(upper_buffer, "TARE") == 0) {
                    Serial.println(F("Taring..."));
                    tare_offset = current_weight;
                    Serial.println(F("Tare set!"));
                }
                else if (strcmp(upper_buffer, "DATA") == 0) {
                    sendBinaryData();
                    Serial.println(F("[DATA_SENT]"));
                }
                else if (strcmp(upper_buffer, "CONT") == 0) {
                    continuous_mode = true;
                    Serial.println(F("Continuous mode ON"));
                }
                else if (strcmp(upper_buffer, "STOP") == 0) {
                    continuous_mode = false;
                    Serial.println(F("Continuous mode OFF"));
                }
                else if (strcmp(upper_buffer, "RESET") == 0) {
                    Serial.println(F("Resetting..."));
                    delay(100);
                    resetWatchdog();
                }
                else {
                    Serial.print(F("Unknown command: "));
                    Serial.println(buffer);
                    Serial.println(F("Commands: ID, TARE, DATA, CONT, STOP, RESET"));
                }
            }
            
            buffer_index = 0;
        }
        else if (buffer_index < 31) {
            buffer[buffer_index++] = c;
        }
    }
}

// ============================================================
// PSEUDO-THREADING (Cooperative Multitasking)
// ============================================================

// Thread: HX711 Reader - reads at 80Hz (every ~12.5ms)
void threadHX711() {
    if (millis() - thread_timer_hx711 >= 12) {  // ~80Hz = 12.5ms interval
        thread_timer_hx711 = millis();
        
        // Check if HX711 is ready
        pinMode(HX711_DT_PIN, INPUT);
        if (digitalRead(HX711_DT_PIN) == LOW) {
            // Read single sample at 80Hz
            long raw = readHX711Raw();
            if (raw != 0) {
                current_weight = raw;
            }
        }
    }
}

// Thread: Serial Command Processor
void threadSerial() {
    if (millis() - thread_timer_serial >= SERIAL_THREAD_INTERVAL) {
        thread_timer_serial = millis();
        processSerialCommand();
    }
}

// Thread: Watchdog Feeder
void threadWatchdog() {
    if (millis() - thread_timer_watchdog >= WATCHDOG_THREAD_INTERVAL) {
        thread_timer_watchdog = millis();
        feedWatchdog();
    }
}

// Thread: Continuous Data Sender - sends at 80Hz
void threadContinuousData() {
    if (continuous_mode && (millis() - last_data_send >= 12)) {  // ~80Hz
        last_data_send = millis();
        sendBinaryData();
    }
}

// ============================================================
// ARDUINO SETUP AND LOOP
// ============================================================

void setup() {
    // Initialize pins
    pinMode(HX711_DT_PIN, INPUT);
    pinMode(HX711_SCK_PIN, OUTPUT);
    digitalWrite(HX711_SCK_PIN, LOW);
    
    // Initialize Serial at maximum baud rate
    Serial.begin(SERIAL_BAUD_RATE);
    while (!Serial) {
        ;  // Wait for serial port to connect (needed for some boards)
    }
    
    // Setup hardware watchdog
    setupWatchdog();
    
    // Initial delay for stabilization
    delay(100);
    
    // Perform initial HX711 reading
    Serial.println(F("Initializing HX711..."));
    
    // Take multiple readings to stabilize
    for (int i = 0; i < 10; i++) {
        readHX711Averaged(10);
        delay(10);
        feedWatchdog();
    }
    
    // Set initial tare offset
    tare_offset = readHX711Averaged(HX711_SAMPLES);
    
    Serial.println(F("System Ready!"));
    Serial.println(F("Send 'ID' for device info, 'TARE' to zero, 'DATA' for reading"));
    Serial.println(F("Send 'CONT' for continuous mode, 'STOP' to disable"));
    Serial.println();
}

void loop() {
    // Cooperative multitasking - run all pseudo-threads
    threadHX711();
    threadSerial();
    threadWatchdog();
    threadContinuousData();
    
    // Main watchdog feed (safety net)
    feedWatchdog();
    
    // Small delay to prevent CPU hogging
    delayMicroseconds(100);
}
