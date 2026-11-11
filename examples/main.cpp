/**
 * @file main.cpp
 * @brief Przykładowe użycie klasy SerialCommunication
 * 
 * Demonstruje wszystkie funkcjonalności:
 * - Identyfikacja portu i Arduino
 * - CRC verification
 * - Watchdog monitoring
 * - Timestamp synchronization
 * - Odczyt wartości z czujnika
 * - Tarowanie (kalibracja)
 */

#include "sensor/SerialCommunication.hpp"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>

using namespace sensor;

void printSeparator() {
    std::cout << "\n========================================\n" << std::endl;
}

void printArduinoInfo(const ArduinoInfo& info) {
    std::cout << "=== Informacje o Arduino ===" << std::endl;
    std::cout << "Nazwa urządzenia: " << info.deviceName << std::endl;
    std::cout << "Firmware: " << info.firmwareVersion << std::endl;
    std::cout << "Board ID: 0x" << std::hex << info.boardId << std::dec << std::endl;
    std::cout << "Wykryto: " << (info.isConnected ? "TAK" : "NIE") << std::endl;
    printSeparator();
}

void printSensorData(const SensorData& data) {
    std::cout << "=== Dane z Czujnika ===" << std::endl;
    std::cout << "Wartość surowa: " << std::fixed << std::setprecision(4) 
              << data.value << std::endl;
    std::cout << "Wartość skalibrowana: " << data.calibratedValue << std::endl;
    std::cout << "Timestamp: " << data.timestamp << " ms" << std::endl;
    std::cout << "CRC: 0x" << std::hex << static_cast<int>(data.crc) << std::dec << std::endl;
    std::cout << "Poprawność: " << (data.isValid ? "TAK" : "NIE") << std::endl;
    printSeparator();
}

int main(int argc, char* argv[]) {
    std::cout << "=== Przykład użycia SerialCommunication ===" << std::endl;
    printSeparator();
    
    // Utworzenie obiektu komunikacji
    SerialCommunication comm;
    
    // 1. Skanowanie dostępnych portów
    std::cout << "1. Skanowanie dostępnych portów..." << std::endl;
    auto ports = comm.scanAvailablePorts();
    
    if (ports.empty()) {
        std::cout << "Nie znaleziono żadnych portów szeregowych." << std::endl;
        std::cout << "Podłącz Arduino NANO i spróbuj ponownie." << std::endl;
        
        // Dla celów demonstracyjnych pokazujemy jak wyglądałoby użycie
        std::cout << "\n--- Tryb demonstracyjny (bez podłączonego urządzenia) ---" << std::endl;
        std::cout << "Gdyby urządzenie było podłączone, wykonano by następujące kroki:" << std::endl;
        std::cout << "  - Połączenie z portem" << std::endl;
        std::cout << "  - Identyfikacja Arduino" << std::endl;
        std::cout << "  - Synchronizacja timestamp" << std::endl;
        std::cout << "  - Kalibracja (tarowanie)" << std::endl;
        std::cout << "  - Odczyt danych z czujnika" << std::endl;
        return 0;
    }
    
    std::cout << "Znalezione porty:" << std::endl;
    for (size_t i = 0; i < ports.size(); ++i) {
        std::cout << "  [" << i << "] " << ports[i] << std::endl;
    }
    printSeparator();
    
    // 2. Automatyczna identyfikacja portu i Arduino
    std::cout << "2. Automatyczna identyfikacja Arduino..." << std::endl;
    
    if (comm.autoConnect()) {
        ArduinoInfo arduinoInfo = comm.getArduinoInfo();
        std::cout << "Znaleziono Arduino!" << std::endl;
        printArduinoInfo(arduinoInfo);
    } else {
        std::cout << "Nie udało się zidentyfikować Arduino." << std::endl;
        std::cout << "Sprawdź połączenie i spróbuj ponownie." << std::endl;
        return 1;
    }
    
    // 3. Konfiguracja watchdog
    std::cout << "3. Konfiguracja Watchdog..." << std::endl;
    comm.configureWatchdog(5000, true); // 5 sekund timeoutu
    std::cout << "Watchdog skonfigurowany." << std::endl;
    printSeparator();
    
    // 4. Sprawdzenie statusu watchdog
    std::cout << "4. Sprawdzenie Watchdog..." << std::endl;
    if (comm.isWatchdogOK()) {
        std::cout << "Watchdog: OK (połączenie aktywne)" << std::endl;
    } else {
        std::cout << "Watchdog: TIMEOUT (brak aktywności)" << std::endl;
    }
    printSeparator();
    
    // 5. Synchronizacja timestamp
    std::cout << "5. Synchronizacja timestamp..." << std::endl;
    // Request timestamp from Arduino using TIME command
    std::string timeResponse = comm.sendCommand("TIME", true);
    uint32_t arduinoTimestamp = 0;
    
    // Parse timestamp from response (expected format: "TIME:<value>" or just numeric value)
    size_t colonPos = timeResponse.find("TIME:");
    if (colonPos != std::string::npos) {
        try {
            arduinoTimestamp = std::stoul(timeResponse.substr(colonPos + 5));
        } catch (...) {
            std::cout << "Nie udało się sparsować timestamp z odpowiedzi: " << timeResponse << std::endl;
        }
    } else {
        // Try to parse as raw number if no prefix found
        try {
            // Find first digit in response
            size_t firstDigit = timeResponse.find_first_of("0123456789");
            if (firstDigit != std::string::npos) {
                arduinoTimestamp = std::stoul(timeResponse.substr(firstDigit));
            }
        } catch (...) {
            std::cout << "Nie udało się sparsować timestamp z odpowiedzi." << std::endl;
        }
    }
    
    if (arduinoTimestamp != 0) {
        comm.syncTimestamp(arduinoTimestamp);
        std::cout << "Timestamp Arduino: " << arduinoTimestamp << " ms" << std::endl;
        std::cout << "Offset synchronizacji: " << comm.getTimestampOffset() << " ms" << std::endl;
    } else {
        std::cout << "Uwaga: Nie otrzymano valid timestamp z Arduino. Synchronizacja pominięta." << std::endl;
    }
    std::cout << "Synchronizacja zakończona." << std::endl;
    printSeparator();
    
    // 6. Tarowanie (kalibracja zera)
    std::cout << "6. Tarowanie (kalibracja zera)..." << std::endl;
    std::cout << "Upewnij się, że czujnik jest w pozycji zerowej." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    double tareValue = comm.tare(10);
    std::cout << "Kalibracja zera zakończona. Wartość tare: " << tareValue << std::endl;
    printSeparator();
    
    // 7. Kalibracja dwupunktowa (opcjonalna)
    std::cout << "7. Kalibracja dwupunktowa (przykład)..." << std::endl;
    std::cout << "Wymaga dwóch znanych wartości referencyjnych." << std::endl;
    // Przykład: comm.twoPointCalibrate(0.0f, 100.0f);
    printSeparator();
    
    // 8. Status kalibracji
    std::cout << "8. Kalibracja zakończona." << std::endl;
    printSeparator();
    
    // 9. Pętla odczytu danych
    std::cout << "9. Rozpoczynanie odczytu danych z czujnika..." << std::endl;
    std::cout << "Naciśnij Ctrl+C aby przerwać." << std::endl;
    printSeparator();
    
    int readCount = 0;
    const int maxReads = 10; // Dla przykładu czytamy 10 razy
    
    while (readCount < maxReads && comm.isConnected()) {
        // Sprawdź watchdog przed odczytem
        if (!comm.isWatchdogOK()) {
            std::cout << "UWAGA: Watchdog timeout! Sprawdzam połączenie..." << std::endl;
            if (!comm.isConnected()) {
                std::cout << "BŁĄD: Utracono połączenie." << std::endl;
                break;
            }
        }
        
        SensorData data = comm.readData(1000);
        if (data.isValid) {
            ++readCount;
            std::cout << "\n[Odczyt #" << readCount << "]" << std::endl;
            printSensorData(data);
            
            // Reset watchdog po udanym odczycie
            comm.resetWatchdog();
        } else {
            std::cout << "Błąd odczytu danych z czujnika." << std::endl;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    // 10. Pobranie informacji o urządzeniu
    std::cout << "\n10. Informacje o urządzeniu:" << std::endl;
    ArduinoInfo info = comm.getArduinoInfo();
    std::cout << "Nazwa: " << info.deviceName << std::endl;
    printSeparator();
    
    // Rozłączenie
    std::cout << "11. Zamykanie połączenia..." << std::endl;
    comm.disconnect();
    std::cout << "Połączenie zakończone." << std::endl;
    printSeparator();
    
    std::cout << "=== Koniec przykładu ===" << std::endl;
    
    return 0;
}
