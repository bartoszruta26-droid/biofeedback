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
    std::cout << "Firmware: v" << static_cast<int>(info.firmwareMajor) 
              << "." << static_cast<int>(info.firmwareMinor) << std::endl;
    std::cout << "Board ID: 0x" << std::hex << info.boardId << std::dec << std::endl;
    std::cout << "Wykryto: " << (info.isDetected ? "TAK" : "NIE") << std::endl;
    if (info.isDetected) {
        std::cout << "Czas wykrycia: " << info.detectedAt << " ms" << std::endl;
    }
    printSeparator();
}

void printSensorData(const SensorData& data) {
    std::cout << "=== Dane z Czujnika ===" << std::endl;
    std::cout << "Wartość surowa: " << std::fixed << std::setprecision(4) 
              << data.rawValue << std::endl;
    std::cout << "Wartość skalibrowana: " << data.calibratedValue << std::endl;
    std::cout << "Timestamp: " << data.timestamp << " ms" << std::endl;
    std::cout << "Status: ";
    switch (data.status) {
        case SensorStatus::OK:
            std::cout << "OK";
            break;
        case SensorStatus::ERROR:
            std::cout << "BŁĄD";
            break;
        case SensorStatus::CALIBRATING:
            std::cout << "KALIBRACJA";
            break;
        default:
            std::cout << "NIEZNANY";
            break;
    }
    std::cout << std::endl;
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
    std::string foundPort;
    ArduinoInfo arduinoInfo;
    
    if (comm.identifyPort(foundPort, arduinoInfo)) {
        std::cout << "Znaleziono Arduino na porcie: " << foundPort << std::endl;
        printArduinoInfo(arduinoInfo);
    } else {
        std::cout << "Nie udało się zidentyfikować Arduino." << std::endl;
        std::cout << "Sprawdź połączenie i spróbuj ponownie." << std::endl;
        return 1;
    }
    
    // 3. Konfiguracja watchdog
    std::cout << "3. Konfiguracja Watchdog..." << std::endl;
    comm.setWatchdogTimeout(5000); // 5 sekund timeoutu
    std::cout << "Watchdog timeout ustawiony na: " 
              << comm.getWatchdogTimeout() << " ms" << std::endl;
    printSeparator();
    
    // 4. Sprawdzenie statusu watchdog
    std::cout << "4. Sprawdzenie Watchdog..." << std::endl;
    if (comm.checkWatchdog()) {
        std::cout << "Watchdog: OK (połączenie aktywne)" << std::endl;
    } else {
        std::cout << "Watchdog: TIMEOUT (brak aktywności)" << std::endl;
    }
    printSeparator();
    
    // 5. Synchronizacja timestamp
    std::cout << "5. Synchronizacja timestamp..." << std::endl;
    int64_t arduinoTimestamp = 0;
    if (comm.syncTimestamp(arduinoTimestamp)) {
        std::cout << "Timestamp Arduino: " << arduinoTimestamp << " ms" << std::endl;
        std::cout << "Synchronizacja zakończona sukcesem." << std::endl;
    } else {
        std::cout << "Nie udało się zsynchronizować timestamp." << std::endl;
    }
    printSeparator();
    
    // 6. Tarowanie (kalibracja zera)
    std::cout << "6. Tarowanie (kalibracja zera)..." << std::endl;
    std::cout << "Upewnij się, że czujnik jest w pozycji zerowej." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    if (comm.zeroCalibrate()) {
        std::cout << "Kalibracja zera zakończona sukcesem." << std::endl;
    } else {
        std::cout << "Nie udało się przeprowadzić kalibracji zera." << std::endl;
    }
    printSeparator();
    
    // 7. Kalibracja dwupunktowa (opcjonalna)
    std::cout << "7. Kalibracja dwupunktowa (przykład)..." << std::endl;
    std::cout << "Wymaga dwóch znanych wartości referencyjnych." << std::endl;
    // Przykład: comm.twoPointCalibrate(0.0f, 100.0f);
    printSeparator();
    
    // 8. Status kalibracji
    std::cout << "8. Status kalibracji: " 
              << (comm.isCalibrated() ? "SKALIBROWANY" : "NIESKALIBROWANY") 
              << std::endl;
    printSeparator();
    
    // 9. Pętla odczytu danych
    std::cout << "9. Rozpoczynanie odczytu danych z czujnika..." << std::endl;
    std::cout << "Naciśnij Ctrl+C aby przerwać." << std::endl;
    printSeparator();
    
    int readCount = 0;
    const int maxReads = 10; // Dla przykładu czytamy 10 razy
    
    while (readCount < maxReads && comm.isConnected()) {
        // Sprawdź watchdog przed odczytem
        if (!comm.checkWatchdog()) {
            std::cout << "UWAGA: Watchdog timeout! Sprawdzam połączenie..." << std::endl;
            if (!comm.isConnected()) {
                std::cout << "BŁĄD: Utracono połączenie." << std::endl;
                break;
            }
        }
        
        SensorData data;
        if (comm.readSensorData(data)) {
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
    std::cout << "Nazwa: " << comm.getDeviceName() << std::endl;
    auto fwVersion = comm.getFirmwareVersion();
    std::cout << "Firmware: v" << static_cast<int>(fwVersion.major) 
              << "." << static_cast<int>(fwVersion.minor) << std::endl;
    printSeparator();
    
    // 11. Pobranie konfiguracji
    std::cout << "11. Aktualna konfiguracja:" << std::endl;
    auto config = comm.getConfig();
    std::cout << "Baud Rate: " << config.baudRate << std::endl;
    std::cout << "Watchdog Timeout: " << config.watchdogTimeoutMs << " ms" << std::endl;
    std::cout << "Zero Offset: " << config.zeroOffset << std::endl;
    std::cout << "Scale Factor: " << config.scaleFactor << std::endl;
    printSeparator();
    
    // Rozłączenie
    std::cout << "12. Zamykanie połączenia..." << std::endl;
    comm.disconnect();
    std::cout << "Połączenie zakończone." << std::endl;
    printSeparator();
    
    std::cout << "=== Koniec przykładu ===" << std::endl;
    
    return 0;
}
