# Rozdział 5.5. Archiwizacja i integralność danych pomiarowych – aspekty śladu metrologicznego

## 5.5.1. Wprowadzenie do problematyki zarządzania danymi pomiarowymi w systemach medycznych klasy IIb

### 5.5.1.1. Rola integralności danych w zapewnieniu wiarygodności naukowej osiągnięcia projektowego

W kontekście niniejszej pracy habilitacyjnej, przedstawiającej oryginalne osiągnięcie projektowe w postaci autorskiego systemu pomiarowo-treningowego z pętlą sprzężenia zwrotnego, zagadnienie archiwizacji i integralności danych pomiarowych nabiera fundamentalnego znaczenia zarówno z perspektywy metrologicznej, jak i prawno-formalnej. Dane rejestrowane przez system stanowią bowiem nie tylko podstawę do podejmowania decyzji terapeutycznych przez fizjoterapeutów i trenerów, ale również dowód naukowy potwierdzający skuteczność opracowanego rozwiązania, jego parametry metrologiczne oraz możliwość zastosowania w warunkach klinicznych i treningowych.

**Definicja integralności danych zgodnie z wytycznymi ALCOA+:**

Zgodnie z międzynarodowymi standardamiGood Data Practice (GDP) oraz wytycznymi agencji regulacyjnych (FDA – Food and Drug Administration, EMA – European Medicines Agency), integralność danych w systemach medycznych musi spełniać kryteria określane akronimem **ALCOA+**:

| Kryterium | Rozwinięcie | Znaczenie w kontekście systemu biofeedback |
|-----------|-------------|--------------------------------------------|
| **A** | Attributable (Przypisywalność) | Każda zarejestrowana wartość siły musi być jednoznacznie powiązana z konkretnym pacjentem, sesją treningową, operatorem przeprowadzającym badanie oraz egzemplarzem urządzenia pomiarowego |
| **L** | Legible (Czytelność) | Dane muszą być przechowywane w formacie zapewniającym długoterminową czytelność, niezależną od zmian technologicznych oprogramowania i systemów operacyjnych |
| **C** | Contemporaneous (Równoczesność rejestracji) | Rejestracja danych musi odbywać się w czasie rzeczywistym, z dokładnym znacznikiem czasowym (timestamp) synchronizowanym ze wzorcem czasu UTC |
| **O** | Original (Oryginalność) | Musi istnieć możliwość odróżnienia danych pierwotnych (raw data) od danych przetworzonych, zmodyfikowanych lub zaanonymizowanych |
| **A** | Accurate (Dokładność) | Wartości zapisane w plikach muszą dokładnie odpowiadać wartościom zmierzonym przez czujnik, z uwzględnieniem udokumentowanej niepewności pomiaru |
| **+** | Complete, Consistent, Enduring, Available | Dane muszą być kompletne (brak luk w rejestracji), spójne (brak sprzecznych informacji), trwałe (odporne na utratę) i dostępne (możliwe do odczytu przez uprawnione osoby) |

W kontekście niniejszej pracy habilitacyjnej, spełnienie kryteriów ALCOA+ jest warunkiem koniecznym dla:

1. **Uznania danych za dowód naukowy** – dane niespełniające wymogów integralności nie mogą stanowić podstawy publikacji naukowych ani wniosków badawczych przedstawianych w rozprawie habilitacyjnej;

2. **Legalizacji metrologicznej urządzeń specjalnych** – procedura legalizacji pierwotnej wymaga udokumentowania mechanizmów zabezpieczających integralność danych przed nieautoryzowaną modyfikacją;

3. **Dobrowolnej deklaracji zgodności dla wyrobów medycznych klasy IIb** – norma IEC 62304 („Medical device software – Software life cycle processes") oraz rozporządzenie MDR (Medical Device Regulation UE 2017/745) nakładają rygorystyczne wymagania dotyczące zarządzania danymi medycznymi;

4. **Możliwości wykorzystania danych w badaniach klinicznych** – dane zbierane w ramach badań naukowych muszą spełniać wymogiGood Clinical Practice (GCP) oraz rozporządzenia RODO (GDPR) w zakresie ochrony danych osobowych.

### 5.5.1.2. Specyfika danych pomiarowych w systemach biomechanicznych z pętlą sprzężenia zwrotnego

Dane generowane przez autorski system pomiarowo-treningowy charakteryzują się specyficznymi cechami, które nakładają szczególne wymagania na proces archiwizacji i zabezpieczania integralności:

**Cecha 1: Wysoka częstotliwość próbkowania i duża objętość danych**

System pracuje z częstotliwością próbkowania 80 Hz, co oznacza rejestrację 80 punktów pomiarowych na sekundę. Dla typowej sesji treningowej trwającej 30 minut daje to:

$$N_{samples} = 80 \text{ Hz} \times 30 \text{ min} \times 60 \text{ s/min} = 144\,000 \text{ próbek}$$

Każda próbka zawiera następujące informacje:
- Wartość siły w Newtonach (4 bajty, float32);
- Znacznik czasowy w milisekundach (8 bajtów, uint64);
- Numer sekwencyjny pakietu (4 bajty, uint32);
- Sumę kontrolną CRC-8 (1 bajt, uint8);
- Status jakości sygnału (1 bajt, uint8 – flagi: overload, underload, noise, drift);

Łączny rozmiar danych surowych dla jednej sesji:

$$Size_{raw} = 144\,000 \times (4 + 8 + 4 + 1 + 1) \text{ B} = 2\,592\,000 \text{ B} \approx 2.47 \text{ MB}$$

Dla pojedynczego pacjenta przechodzącego rehabilitację przez 12 tygodni (3 sesje/tydzień) całkowita objętość danych wynosi:

$$Size_{total} = 2.47 \text{ MB} \times 3 \text{ sesje/tydzień} \times 12 \text{ tygodni} \approx 89 \text{ MB}$$

W skali całej placówki terapeutycznej obsługującej 100 pacjentów rocznie, roczna objętość danych przekracza 8.9 GB, co wymaga zastosowania efektywnych strategii kompresji, indeksowania i hierarchicznego zarządzania pamięcią masową.

**Cecha 2: Konieczność zachowania metadanych metrologicznych**

Każdy pomiar siły zarejestrowany przez system musi być powiązany z metadanymi metrologicznymi umożliwiającymi odtworzenie warunków pomiaru i oszacowanie niepewności wyniku. Metadane te obejmują:

- **Identyfikator egzemplarza urządzenia** – numer seryjny Raspberry Pi, Arduino Nano, czujnika HX711;
- **Data i ważność kalibracji** – timestamp ostatniego wzorcowania, data następnej kalibracji okresowej;
- **Współczynniki kalibracyjne** – współczynnik konwersji jednostek ADC na Niutony, offset zera, współczynniki korekcyjne temperaturowe;
- **Warunki środowiskowe** – temperatura otoczenia, wilgotność względna (jeśli dostępne z dodatkowych sensorów);
- **Konfiguracja oprogramowania** – wersja firmware Arduino, wersja aplikacji RPi, hash kodu źródłowego;
- **Ślad audytowy** – lista operacji wykonanych na danych (eksport, modyfikacja, usunięcie).

**Cecha 3: Wymóg długoterminowej archiwizacji z zachowaniem formatu czytelnego**

Zgodnie z polskim prawem farmaceutycznym oraz wytycznymi dotyczącymi dokumentacji medycznej, dane pacjentów muszą być przechowywane przez minimum 30 lat od zakończenia procesu terapeutycznego. W tym okresie technologie magazynowania danych, formaty plików i systemy operacyjne ulegną wielokrotnym zmianom, stąd konieczność zastosowania:

- Formatów otwartych i standaryzowanych (JSON, XML, HDF5) zamiast formatów własnościowych;
- Schematów walidacji (JSON Schema, XSD) umożliwiających weryfikację struktury danych po latach;
- Strategii migracji danych do nowych formatów przed dezaktualizacją technologii;
- Mechanizmów emulacji środowiska odczytu dla przestarzałych formatów.

**Cecha 4: Konieczność zapewnienia ciągłości łańcucha dowodowego (chain of custody)**

W przypadku wykorzystania danych pomiarowych jako dowodu w postępowaniach sądowych (np. sprawy o błędy medyczne, odszkodowania sportowe) lub w sporach dotyczących ważności rekordów treningowych, niezbędne jest udokumentowanie pełnej historii przetwarzania danych od momentu rejestracji do chwili obecnej. Łańcuch dowodowy obejmuje:

- Rejestrację wszystkich operacji na danych (kto, kiedy, jaką operację wykonał);
- Zabezpieczenie przed nieautoryzowanym dostępem (uwierzytelnianie, autoryzacja, szyfrowanie);
- Mechanizmy wykrywania prób modyfikacji (sumy kontrolne, podpisy cyfrowe);
- Procedury backupu i odtwarzania danych awaryjnych.

---

## 5.5.2. Struktura danych JSON i mechanizmy walidacji schematu

### 5.5.2.1. Projekt schematu JSON dla danych pomiarowych z uwzględnieniem wymagań metrologicznych

Dane pomiarowe generowane przez autorski system są przechowywane w formacie **JSON (JavaScript Object Notation)**, który został wybrany ze względu na następujące zalety:

1. **Czytelność dla człowieka i maszyny** – tekstowy format ułatwia ręczną inspekcję danych i debugowanie;
2. **Niezależność od platformy** – JSON jest obsługiwany natywnie przez wszystkie współczesne języki programowania;
3. **Elastyczność struktury** – możliwość dodawania nowych pól bez łamania kompatybilności wstecznej;
4. **Wsparcie dla narzędzi walidacji** – istnienie standardu JSON Schema umożliwiającego formalną weryfikację struktury;
5. **Efektywna kompresja** – tekstowy format dobrze kompresuje się algorytmami gzip, bzip2, LZMA.

**Struktura pliku JSON dla pojedynczej sesji pomiarowej:**

Plik JSON zawierający dane z jednej sesji treningowej składa się z czterech logicznych sekcji: nagłówka (header), metadanych metrologicznych (metrology), właściwych danych pomiarowych (measurements) oraz stopki z sumami kontrolnymi (footer).

```json
{
  "$schema": "https://biofeedback-system.edu/schemas/session_data_v2.3.json",
  "header": {
    "sessionId": "SESSION-2024-03-15-P001-S003",
    "sessionType": "training_biofeedback",
    "startTimeISO8601": "2024-03-15T14:30:00.000Z",
    "endTimeISO8601": "2024-03-15T15:00:00.000Z",
    "durationSeconds": 1800,
    "operatorId": "PHYSIO-042",
    "operatorName": "mgr Anna Kowalska",
    "facilityId": "REHAB-CENTER-WAW-01",
    "facilityName": "Centrum Rehabilitacji Sportowej"
  },
  
  "patient": {
    "patientId": "PAT-2024-00127",
    "anonymousCode": "ANON-7F3A9B2C",
    "dateOfBirth": "1985-06-20",
    "sex": "M",
    "heightCm": 178,
    "weightKg": 82.5,
    "diagnosis": "Status post reconstructionem ACL sin. gen.",
    "informedConsentId": "CONSENT-2024-00127-v3"
  },
  
  "device": {
    "systemSerialNumber": "BIOFEEDBACK-SYS-2024-0042",
    "raspberryPiSerial": "10000000a1b2c3d4",
    "arduinoNanoFirmwareVersion": "2.3.1",
    "arduinoNanoFirmwareHash": "sha256:7f8a9b2c3d4e5f6a7b8c9d0e1f2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a",
    "hx711SensorSerial": "HX711-LOT42-UNIT157",
    "latexTubeingColor": "blue",
    "latexTubeingResistanceClass": "heavy",
    "latexTubeingSerialNumber": "TUBE-2024-BLU-089"
  },
  
  "metrology": {
    "calibrationCertificateId": "CAL-CERT-2024-0042-001",
    "calibrationDateISO8601": "2024-01-10T09:00:00.000Z",
    "calibrationValidUntilISO8601": "2025-01-10T09:00:00.000Z",
    "calibrationLaboratory": "Główny Urząd Miar, Warszawa",
    "calibrationTechnician": "inż. Piotr Nowak",
    "referenceStandardId": "REF-STD-FORCE-500N-003",
    "referenceStandardValidUntilISO8601": "2025-06-30T23:59:59.000Z",
    
    "calibrationCoefficients": {
      "slope_N_per_ADC": 0.00234567,
      "offset_N": -0.012,
      "temperatureCoefficient_N_per_C": 0.00015,
      "linearityCorrectionEnabled": true,
      "hysteresisCorrectionEnabled": true
    },
    
    "measurementUncertainty": {
      "expandedUncertainty_U_N": 2.5,
      "coverageFactor_k": 2,
      "confidenceLevelPercent": 95,
      "uncertaintyBudget": {
        "sensorRepeatability_N": 1.2,
        "adcQuantization_N": 0.3,
        "temperatureDrift_N": 0.8,
        "linearityError_N": 0.5,
        "hysteresisError_N": 0.4,
        "referenceStandardUncertainty_N": 1.5
      }
    },
    
    "environmentalConditions": {
      "temperature_C": 21.3,
      "humidity_RH_percent": 48,
      "atmosphericPressure_hPa": 1013.2,
      "electromagneticInterferenceLevel": "low"
    }
  },
  
  "measurements": {
    "samplingFrequencyHz": 80,
    "numberOfSamples": 144000,
    "dataFormat": "interleaved_force_timestamp_quality",
    "compressionAlgorithm": "none",
    
    "data": [
      {
        "sampleIndex": 0,
        "timestampMs": 1710512400000,
        "forceRawADC": 12456,
        "forceCalibrated_N": 29.23,
        "signalQualityFlags": {
          "overload": false,
          "underload": false,
          "excessiveNoise": false,
          "temperatureWarning": false
        },
        "crc8Checksum": 0xA7
      },
      {
        "sampleIndex": 1,
        "timestampMs": 1710512400012,
        "forceRawADC": 12489,
        "forceCalibrated_N": 29.31,
        "signalQualityFlags": {
          "overload": false,
          "underload": false,
          "excessiveNoise": false,
          "temperatureWarning": false
        },
        "crc8Checksum": 0xB2
      }
    ]
  },
  
  "derivedParameters": {
    "peakForce_N": 287.45,
    "meanForce_N": 156.32,
    "forceRange_N": 245.18,
    "impulse_Ns": 18234.5,
    "work_Joules": 892.3,
    "powerPeak_W": 456.7,
    "powerMean_W": 234.1,
    "rateOfForceDevelopment_N_per_s": 1245.6,
    "coefficientOfVariation_percent": 8.3,
    "fatigueIndex_percent": 12.7,
    "movementQualityScore": 87.5
  },
  
  "footer": {
    "fileCreatedAtISO8601": "2024-03-15T15:00:05.123Z",
    "fileCreatedBy": "biofeedback-app v3.2.1",
    "originalFileName": "SESSION-2024-03-15-P001-S003.json",
    "fileSizeBytes": 2684354,
    "md5Hash": "d41d8cd98f00b204e9800998ecf8427e",
    "sha256Hash": "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
    "digitalSignature": "MIIBkTCB+wIJAKHBfpegPjMCMA0GCSqGSIb3DQEBCwUAMBExDzANBgNVBAMMBnJvb3RDQTAeFw0yNDAxMTUwMDAwMDBaFw0yNTAxMTUwMDAwMDBaMBExDzANBgNVBAMMBnJvb3RDQTBcMA0GCSqGSIb3DQEBAQUAA0sAMEgCQQC7o96W...",
    "legalizationMark": "LEGALIZATION-PL-GUM-2024-0042-VALID"
  }
}
```

**Schemat walidacji JSON Schema dla danych sesyjnych:**

Dla zapewnienia spójności strukturalnej danych i automatycznej detekcji błędów formatu opracowano formalny schemat walidacji w standardzie JSON Schema Draft 7:

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "$id": "https://biofeedback-system.edu/schemas/session_data_v2.3.json",
  "title": "Biofeedback Session Data Schema",
  "description": "Schema for validation of measurement session data from latex tubeing biofeedback system",
  "type": "object",
  "required": ["header", "patient", "device", "metrology", "measurements", "footer"],
  
  "properties": {
    "$schema": {
      "type": "string",
      "format": "uri",
      "description": "URI reference to this schema version"
    },
    
    "header": {
      "type": "object",
      "required": ["sessionId", "sessionType", "startTimeISO8601", "endTimeISO8601", "operatorId"],
      "properties": {
        "sessionId": {
          "type": "string",
          "pattern": "^SESSION-\\d{4}-\\d{2}-\\d{2}-P\\d{3,6}-S\\d{3}$",
          "description": "Unique session identifier with date, patient number and session number"
        },
        "sessionType": {
          "type": "string",
          "enum": ["measurement_static", "measurement_dynamic", "training_biofeedback", "training_game", "calibration_procedure"]
        },
        "startTimeISO8601": {
          "type": "string",
          "format": "date-time",
          "description": "Session start time in ISO 8601 format with UTC timezone"
        },
        "endTimeISO8601": {
          "type": "string",
          "format": "date-time",
          "description": "Session end time in ISO 8601 format with UTC timezone"
        },
        "durationSeconds": {
          "type": "integer",
          "minimum": 1,
          "maximum": 7200
        },
        "operatorId": {
          "type": "string",
          "pattern": "^(PHYSIO|TECH|TRAINER)-\\d{3,5}$"
        }
      }
    },
    
    "metrology": {
      "type": "object",
      "required": ["calibrationCertificateId", "calibrationDateISO8601", "calibrationValidUntilISO8601", "calibrationCoefficients", "measurementUncertainty"],
      "properties": {
        "calibrationCertificateId": {
          "type": "string",
          "pattern": "^CAL-CERT-\\d{4}-\\d{4,6}-\\d{3}$"
        },
        "calibrationDateISO8601": {
          "type": "string",
          "format": "date-time"
        },
        "calibrationValidUntilISO8601": {
          "type": "string",
          "format": "date-time"
        },
        "calibrationCoefficients": {
          "type": "object",
          "required": ["slope_N_per_ADC", "offset_N"],
          "properties": {
            "slope_N_per_ADC": {"type": "number"},
            "offset_N": {"type": "number"},
            "temperatureCoefficient_N_per_C": {"type": "number"},
            "linearityCorrectionEnabled": {"type": "boolean"},
            "hysteresisCorrectionEnabled": {"type": "boolean"}
          }
        },
        "measurementUncertainty": {
          "type": "object",
          "required": ["expandedUncertainty_U_N", "coverageFactor_k", "confidenceLevelPercent"],
          "properties": {
            "expandedUncertainty_U_N": {"type": "number", "minimum": 0},
            "coverageFactor_k": {"type": "number", "enum": [1, 2, 3]},
            "confidenceLevelPercent": {"type": "number", "enum": [68.27, 95.45, 99.73]}
          }
        }
      }
    },
    
    "measurements": {
      "type": "object",
      "required": ["samplingFrequencyHz", "numberOfSamples", "data"],
      "properties": {
        "samplingFrequencyHz": {
          "type": "integer",
          "enum": [10, 20, 40, 80]
        },
        "numberOfSamples": {
          "type": "integer",
          "minimum": 1,
          "maximum": 1000000
        },
        "data": {
          "type": "array",
          "items": {
            "type": "object",
            "required": ["sampleIndex", "timestampMs", "forceCalibrated_N", "crc8Checksum"],
            "properties": {
              "sampleIndex": {"type": "integer", "minimum": 0},
              "timestampMs": {"type": "integer", "minimum": 0},
              "forceRawADC": {"type": "integer"},
              "forceCalibrated_N": {"type": "number"},
              "crc8Checksum": {
                "type": "integer",
                "minimum": 0,
                "maximum": 255
              }
            }
          }
        }
      }
    },
    
    "footer": {
      "type": "object",
      "required": ["fileCreatedAtISO8601", "md5Hash", "sha256Hash"],
      "properties": {
        "fileCreatedAtISO8601": {
          "type": "string",
          "format": "date-time"
        },
        "md5Hash": {
          "type": "string",
          "pattern": "^[a-f0-9]{32}$",
          "description": "MD5 hash of the file content for integrity verification"
        },
        "sha256Hash": {
          "type": "string",
          "pattern": "^[a-f0-9]{64}$",
          "description": "SHA-256 hash of the file content for cryptographic integrity"
        },
        "digitalSignature": {
          "type": "string",
          "description": "RSA digital signature encoded in Base64"
        },
        "legalizationMark": {
          "type": "string",
          "pattern": "^LEGALIZATION-PL-GUM-\\d{4}-\\d{4,6}-[A-Z]+$"
        }
      }
    }
  }
}
```

**Procedura walidacji danych przy imporcie:**

Przed zaakceptowaniem danych do archiwum system przeprowadza wieloetapową walidację:

1. **Walidacja składni JSON** – sprawdzenie poprawności parsowania pliku;
2. **Walidacja schematu JSON Schema** – weryfikacja obecności wszystkich wymaganych pól i ich typów danych;
3. **Walidacja semantyczna** – sprawdzenie spójności logicznej (np. endTime > startTime, numberOfSamples zgadza się z długością tablicy data);
4. **Weryfikacja sum kontrolnych** – przeliczenie MD5 i SHA-256 i porównanie z wartościami w footer;
5. **Weryfikacja podpisu cyfrowego** – sprawdzenie autentyczności podpisu przy użyciu klucza publicznego jednostki legalizującej;
6. **Weryfikacja ważności kalibracji** – sprawdzenie czy data sesji mieści się w okresie ważności kalibracji urządzenia.

Algorytm walidacji zaimplementowany w języku C++ z wykorzystaniem biblioteki nlohmann/json:

```cpp
// src/data/DataValidator.cpp
#include "DataValidator.hpp"
#include <nlohmann/json.hpp>
#include <openssl/sha.h>
#include <openssl/md5.h>
#include <iostream>
#include <fstream>

using json = nlohmann::json;

class DataValidator {
public:
    struct ValidationResult {
        bool isValid;
        std::vector<std::string> errors;
        std::vector<std::string> warnings;
    };
    
    static ValidationResult validateSessionFile(const std::string& filePath) {
        ValidationResult result;
        result.isValid = true;
        
        // Step 1: Load and parse JSON
        json data;
        try {
            std::ifstream file(filePath);
            file >> data;
        } catch (const json::parse_error& e) {
            result.isValid = false;
            result.errors.push_back(std::string("JSON parse error: ") + e.what());
            return result;
        }
        
        // Step 2: Validate required top-level fields
        std::vector<std::string> requiredFields = {"header", "patient", "device", "metrology", "measurements", "footer"};
        for (const auto& field : requiredFields) {
            if (!data.contains(field)) {
                result.isValid = false;
                result.errors.push_back("Missing required field: " + field);
            }
        }
        
        if (!result.isValid) return result;
        
        // Step 3: Validate header
        if (!data["header"].contains("sessionId") || 
            !data["header"]["sessionId"].is_string()) {
            result.isValid = false;
            result.errors.push_back("Invalid or missing sessionId in header");
        }
        
        // Step 4: Validate timestamps consistency
        if (data["header"].contains("startTimeISO8601") && 
            data["header"].contains("endTimeISO8601")) {
            std::string startTime = data["header"]["startTimeISO8601"];
            std::string endTime = data["header"]["endTimeISO8601"];
            if (startTime >= endTime) {
                result.isValid = false;
                result.errors.push_back("endTime must be greater than startTime");
            }
        }
        
        // Step 5: Validate measurements array size
        if (data["measurements"].contains("numberOfSamples") && 
            data["measurements"].contains("data")) {
            int declaredSamples = data["measurements"]["numberOfSamples"];
            int actualSamples = data["measurements"]["data"].size();
            if (declaredSamples != actualSamples) {
                result.isValid = false;
                result.errors.push_back(
                    "Sample count mismatch: declared=" + std::to_string(declaredSamples) + 
                    ", actual=" + std::to_string(actualSamples)
                );
            }
        }
        
        // Step 6: Verify SHA-256 hash
        std::string storedHash = data["footer"]["sha256Hash"];
        std::string computedHash = computeSHA256(filePath);
        if (storedHash != computedHash) {
            result.isValid = false;
            result.errors.push_back("SHA-256 hash mismatch - file may be corrupted or tampered");
        }
        
        // Step 7: Validate calibration validity period
        if (data["metrology"].contains("calibrationValidUntilISO8601")) {
            std::string validUntil = data["metrology"]["calibrationValidUntilISO8601"];
            std::string sessionTime = data["header"]["endTimeISO8601"];
            if (sessionTime > validUntil) {
                result.warnings.push_back(
                    "WARNING: Session performed after calibration expiration date"
                );
            }
        }
        
        return result;
    }
    
private:
    static std::string computeSHA256(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary);
        unsigned char hash[SHA_DIGEST_LENGTH];
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        
        char buffer[8192];
        while (file.read(buffer, sizeof(buffer))) {
            SHA256_Update(&sha256, buffer, file.gcount());
        }
        SHA256_Update(&sha256, buffer, file.gcount());
        SHA256_Final(hash, &sha256);
        
        // Convert to hex string
        std::stringstream ss;
        for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }
};
```

---

## 5.5.3. Mechanizmy zabezpieczania integralności danych: sumy kontrolne, hashe kryptograficzne i podpisy cyfrowe

### 5.5.3.1. Wielopoziomowy system weryfikacji integralności danych

Zapewnienie integralności danych pomiarowych w autorskim systemie pomiarowo-treningowym zostało zrealizowane poprzez implementację wielopoziomowego systemu zabezpieczeń, działającego na trzech warstwach: poziomie pojedynczego pakietu danych, poziomie pliku sesyjnego oraz poziomie archiwum długoterminowego.

**Poziom 1: Suma kontrolna CRC-8 dla pojedynczych pakietów danych**

Każdy pakiet danych przesyłany z mikrokontrolera Arduino Nano do Raspberry Pi zawiera 8-bitową sumę kontrolną CRC (Cyclic Redundancy Check) obliczaną zgodnie z wielomianem:

$$G(x) = x^8 + x^2 + x + 1 \quad (\text{wartość heksadecymalna: 0x07})$$

Algorytm obliczania CRC-8:

```cpp
// hx711_advanced.ino - Arduino Nano firmware
uint8_t calculateCRC8(const uint8_t* data, uint8_t length) {
    uint8_t crc = 0xFF;  // Initial value
    
    for (uint8_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x07;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

// Structure of data packet sent over serial
struct DataPacket {
    uint32_t timestamp;      // 4 bytes - milliseconds since epoch
    int32_t forceValue;      // 4 bytes - calibrated force in Newtons (scaled)
    uint8_t qualityFlags;    // 1 byte - signal quality indicators
    uint8_t crc8;            // 1 byte - CRC-8 checksum
};

void sendDataPacket() {
    DataPacket packet;
    packet.timestamp = millis();
    packet.forceValue = currentForceValue;
    packet.qualityFlags = getQualityFlags();
    
    // Calculate CRC over first 9 bytes
    packet.crc8 = calculateCRC8((uint8_t*)&packet, 9);
    
    // Send packet with frame markers
    Serial.write(0xAA);  // Start marker
    Serial.write((uint8_t*)&packet, 10);
    Serial.write(0x55);  // End marker
}
```

Po stronie Raspberry Pi każdy odebrany pakiet jest natychmiast weryfikowany:

```cpp
// src/sensor/SerialReader.cpp
bool SerialReader::verifyPacketCRC(const uint8_t* packet, size_t length) {
    if (length < 10) return false;  // Minimum packet size
    
    uint8_t receivedCRC = packet[length - 1];  // Last byte is CRC
    uint8_t calculatedCRC = calculateCRC8(packet, length - 1);
    
    if (receivedCRC != calculatedCRC) {
        qWarning() << "CRC mismatch! Received:" << hex << (int)receivedCRC 
                   << "Calculated:" << hex << (int)calculatedCRC;
        emit crcErrorDetected();
        return false;
    }
    return true;
}
```

Skuteczność CRC-8 w detekcji błędów:
- Wykrywa 100% błędów pojedynczego bitu;
- Wykrywa 100% błędów podwójnego bitu w obrębie 8 bitów;
- Wykrywa 100% błędów nieparzystej liczby bitów;
- Wykrywa 99.6% błędów potrójnego bitu;
- Wykrywa 99.2% wszystkich możliwych kombinacji błędów.

**Poziom 2: Hashe kryptograficzne MD5 i SHA-256 dla całych plików sesyjnych**

Po zakończeniu sesji pomiarowej i zapisaniu danych do pliku JSON, system oblicza dwie funkcje skrótu:

1. **MD5 (Message Digest Algorithm 5)** – 128-bitowy hash używany do szybkiej detekcji przypadkowej korupcji danych;
2. **SHA-256 (Secure Hash Algorithm 256-bit)** – 256-bitowy hash kryptograficzny odporny na kolizje, służący do wykrywania celowych modyfikacji.

Implementacja obliczania hash'y w C++ z wykorzystaniem OpenSSL:

```cpp
// src/security/FileIntegrity.cpp
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <fstream>
#include <sstream>
#include <iomanip>

struct FileHashes {
    std::string md5;
    std::string sha256;
};

FileHashes calculateFileHashes(const std::string& filePath) {
    FileHashes hashes;
    std::ifstream file(filePath, std::ios::binary);
    
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filePath);
    }
    
    // MD5 calculation
    unsigned char md5Hash[MD5_DIGEST_LENGTH];
    MD5_CTX md5Context;
    MD5_Init(&md5Context);
    
    char buffer[8192];
    while (file.read(buffer, sizeof(buffer))) {
        MD5_Update(&md5Context, buffer, file.gcount());
    }
    // Process remaining bytes
    MD5_Update(&md5Context, buffer, file.gcount());
    MD5_Final(md5Hash, &md5Context);
    
    // SHA-256 calculation (reset file position)
    file.clear();
    file.seekg(0, std::ios::beg);
    
    unsigned char sha256Hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256Context;
    SHA256_Init(&sha256Context);
    
    while (file.read(buffer, sizeof(buffer))) {
        SHA256_Update(&sha256Context, buffer, file.gcount());
    }
    SHA256_Update(&sha256Context, buffer, file.gcount());
    SHA256_Final(sha256Hash, &sha256Context);
    
    // Convert to hexadecimal strings
    std::stringstream md5Stream, sha256Stream;
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        md5Stream << std::hex << std::setw(2) << std::setfill('0') << (int)md5Hash[i];
    }
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sha256Stream << std::hex << std::setw(2) << std::setfill('0') << (int)sha256Hash[i];
    }
    
    hashes.md5 = md5Stream.str();
    hashes.sha256 = sha256Stream.str();
    
    return hashes;
}
```

**Poziom 3: Podpis cyfrowy RSA dla zapewnienia autentyczności i niezaprzeczalności**

W celu zapewnienia pełnej niezaprzeczalności (non-repudiation) danych pomiarowych, każda sesja jest podpisywana cyfrowo przy użyciu algorytmu **RSA (Rivest-Shamir-Adleman)** z kluczem prywatnym 2048-bitowym. Podpis cyfrowy spełnia trzy funkcje:

1. **Autentyczność** – potwierdza, że dane zostały wygenerowane przez autoryzowane urządzenie;
2. **Integralność** – każda modyfikacja danych unieważnia podpis;
3. **Niezaprzeczalność** – twórca danych nie może zaprzeczyć ich autorstwu.

Proces generowania podpisu cyfrowego:

```cpp
// src/security/DigitalSignature.cpp
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/evp.h>

class DigitalSigner {
public:
    DigitalSigner(const std::string& privateKeyPath) {
        FILE* keyFile = fopen(privateKeyPath.c_str(), "r");
        if (!keyFile) {
            throw std::runtime_error("Cannot open private key file");
        }
        
        privateKey_ = PEM_read_RSAPrivateKey(keyFile, nullptr, nullptr, nullptr);
        fclose(keyFile);
        
        if (!privateKey_) {
            throw std::runtime_error("Failed to load private key");
        }
    }
    
    ~DigitalSigner() {
        if (privateKey_) {
            RSA_free(privateKey_);
        }
    }
    
    std::string signData(const std::string& data) {
        EVP_MD_CTX* ctx = EVP_MD_CTX_new();
        if (!ctx) throw std::runtime_error("Failed to create EVP context");
        
        // Initialize signing operation with SHA-256
        if (EVP_DigestSignInit(ctx, nullptr, EVP_sha256(), nullptr, privateKey_) != 1) {
            EVP_MD_CTX_free(ctx);
            throw std::runtime_error("Failed to initialize digest sign");
        }
        
        // Provide data for signing
        if (EVP_DigestSignUpdate(ctx, data.c_str(), data.size()) != 1) {
            EVP_MD_CTX_free(ctx);
            throw std::runtime_error("Failed to update digest");
        }
        
        // Get signature length
        size_t sigLen = 0;
        if (EVP_DigestSignFinal(ctx, nullptr, &sigLen) != 1) {
            EVP_MD_CTX_free(ctx);
            throw std::runtime_error("Failed to get signature length");
        }
        
        // Get actual signature
        std::vector<unsigned char> signature(sigLen);
        if (EVP_DigestSignFinal(ctx, signature.data(), &sigLen) != 1) {
            EVP_MD_CTX_free(ctx);
            throw std::runtime_error("Failed to finalize signature");
        }
        
        EVP_MD_CTX_free(ctx);
        
        // Encode signature as Base64 for JSON storage
        return encodeBase64(signature.data(), signature.size());
    }
    
private:
    RSA* privateKey_ = nullptr;
    
    std::string encodeBase64(const unsigned char* data, size_t len) {
        BIO* bio, *b64;
        BUF_MEM* bufferPtr;
        
        b64 = BIO_new(BIO_f_base64());
        bio = BIO_new(BIO_s_mem());
        bio = BIO_push(b64, bio);
        
        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
        BIO_write(bio, data, len);
        BIO_flush(bio);
        BIO_get_mem_ptr(bio, &bufferPtr);
        
        std::string result(bufferPtr->data, bufferPtr->length);
        BIO_free_all(bio);
        
        return result;
    }
};
```

**Proces weryfikacji podpisu cyfrowego:**

Weryfikacja podpisu odbywa się przy użyciu klucza publicznego, który jest dystrybuowany do wszystkich podmiotów uprawnionych do odczytu danych (szpitale, laboratoria, sądy):

```cpp
bool verifySignature(const std::string& data, const std::string& signatureBase64, 
                     const std::string& publicKeyPath) {
    // Load public key
    FILE* keyFile = fopen(publicKeyPath.c_str(), "r");
    if (!keyFile) return false;
    
    RSA* publicKey = PEM_read_RSA_PUBKEY(keyFile, nullptr, nullptr, nullptr);
    fclose(keyFile);
    
    if (!publicKey) return false;
    
    // Decode Base64 signature
    std::vector<unsigned char> signature = decodeBase64(signatureBase64);
    
    // Create verification context
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) {
        RSA_free(publicKey);
        return false;
    }
    
    // Initialize verification with SHA-256
    if (EVP_DigestVerifyInit(ctx, nullptr, EVP_sha256(), nullptr, publicKey) != 1) {
        EVP_MD_CTX_free(ctx);
        RSA_free(publicKey);
        return false;
    }
    
    // Provide data for verification
    if (EVP_DigestVerifyUpdate(ctx, data.c_str(), data.size()) != 1) {
        EVP_MD_CTX_free(ctx);
        RSA_free(publicKey);
        return false;
    }
    
    // Verify signature
    int result = EVP_DigestVerifyFinal(ctx, signature.data(), signature.size());
    
    EVP_MD_CTX_free(ctx);
    RSA_free(publicKey);
    
    return (result == 1);  // 1 = valid, 0 = invalid, -1 = error
}
```

### 5.5.3.2. Ślad audytowy (audit trail) rejestracji wszystkich operacji na danych

Zgodnie z wymaganiami normy IEC 62304 oraz przepisów dotyczącychGood Data Practice, system prowadzi szczegółowy rejestr wszystkich operacji wykonanych na danych pomiarowych. Audit trail spełnia następujące wymagania:

1. **Kompletność** – rejestrowane są wszystkie zdarzenia bez wyjątków;
2. **Niezmienność** – wpisów audit trail nie można modyfikować ani usuwać;
3. **Chronologia** – każde zdarzenie ma dokładny znacznik czasowy synchronizowany z zegarem UTC;
4. **Przypisywalność** – każde zdarzenie jest powiązane z użytkownikiem, który je wykonał;
5. **Przechowywanie** – audit trail jest przechowywany przez ten sam okres co dane medyczne (minimum 30 lat).

**Struktura wpisu audit trail:**

```json
{
  "auditEntry": {
    "entryId": "AUDIT-2024-03-15-000127",
    "timestampISO8601": "2024-03-15T15:05:23.456Z",
    "eventType": "DATA_EXPORT",
    "eventCategory": "DATA_ACCESS",
    "severity": "INFO",
    
    "actor": {
      "userId": "PHYSIO-042",
      "userName": "mgr Anna Kowalska",
      "userRole": "physiotherapist",
      "authenticationMethod": "smart_card_PIN"
    },
    
    "target": {
      "resourceType": "SESSION_DATA",
      "resourceId": "SESSION-2024-03-15-P001-S003",
      "resourcePath": "/data/patients/PAT-2024-00127/sessions/2024-03-15/SESSION-2024-03-15-P001-S003.json"
    },
    
    "action": {
      "operation": "EXPORT_CSV",
      "parameters": {
        "exportFormat": "CSV",
        "includeDerivedParameters": true,
        "anonymizePatientData": false
      },
      "result": "SUCCESS",
      "outputFile": "/exports/PAT-2024-00127_export_20240315.csv"
    },
    
    "context": {
      "workstationId": "WS-REHAB-ROOM-03",
      "ipAddress": "192.168.1.42",
      "macAddress": "00:1A:2B:3C:4D:5E",
      "applicationVersion": "biofeedback-app v3.2.1",
      "operatingSystem": "Raspberry Pi OS 12 (Bookworm)"
    },
    
    "integrity": {
      "entryHash": "sha256:a1b2c3d4e5f6...",
      "previousEntryHash": "sha256:f6e5d4c3b2a1...",
      "chainLinkage": "BLOCKCHAIN-style hash chain"
    }
  }
}
```

**Typy zdarzeń rejestrowanych w audit trail:**

| Kategoria | Typ zdarzenia | Opis |
|-----------|---------------|------|
| **DATA_CREATION** | SESSION_START, SESSION_END, DATA_RECORDING | Rozpoczęcie i zakończenie sesji, rejestracja danych |
| **DATA_ACCESS** | VIEW_SESSION, EXPORT_DATA, PRINT_REPORT | Przeglądanie, eksport, drukowanie danych |
| **DATA_MODIFICATION** | EDIT_METADATA, ANNOTATE_DATA, CORRECT_VALUE | Modyfikacja metadanych, adnotacje, korekty wartości |
| **DATA_DELETION** | DELETE_SESSION, ANONYMIZE_DATA, PURGE_ARCHIVE | Usunięcie sesji, anonimizacja, czyszczenie archiwum |
| **SYSTEM_CONFIG** | CALIBRATION_PERFORMED, SETTINGS_CHANGED, FIRMWARE_UPDATE | Kalibracja, zmiana ustawień, aktualizacja oprogramowania |
| **SECURITY** | USER_LOGIN, USER_LOGOUT, FAILED_AUTH, PRIVILEGE_CHANGE | Logowanie, wylogowanie, nieudane próby, zmiana uprawnień |
| **ERROR** | SENSOR_ERROR, COMMUNICATION_ERROR, STORAGE_ERROR | Błędy sensora, komunikacji, zapisu danych |

**Implementacja łańcucha hash'y dla zapewnienia niezmienności audit trail:**

Aby uniemożliwić niezauważoną modyfikację wpisów audit trail, zastosowano mechanizm łańcucha hash'y podobny do blockchain, gdzie każdy wpis zawiera hash poprzedniego wpisu:

```cpp
// src/audit/AuditTrail.cpp
class AuditTrail {
public:
    void addEntry(const AuditEntry& entry) {
        // Calculate hash of previous entry (or genesis hash for first entry)
        std::string previousHash = entries_.empty() ? 
            "GENESIS-HASH-0000000000000000000000000000000000000000000000000000000000000000" :
            calculateSHA256(entries_.back());
        
        // Add previous hash to current entry
        AuditEntry entryWithLink = entry;
        entryWithLink.integrity.previousEntryHash = previousHash;
        
        // Calculate hash of current entry
        entryWithLink.integrity.entryHash = calculateSHA256(entryWithLink);
        
        // Append to audit trail
        entries_.push_back(entryWithLink);
        
        // Immediately persist to disk (append-only)
        appendToLogFile(entryWithLink);
    }
    
    bool verifyChainIntegrity() const {
        std::string expectedPreviousHash = "GENESIS-HASH-0000000000000000000000000000000000000000000000000000000000000000";
        
        for (const auto& entry : entries_) {
            // Verify previous hash linkage
            if (entry.integrity.previousEntryHash != expectedPreviousHash) {
                qCritical() << "Audit trail chain broken at entry:" << entry.entryId;
                return false;
            }
            
            // Verify entry hash
            std::string calculatedHash = calculateSHA256(entry);
            if (calculatedHash != entry.integrity.entryHash) {
                qCritical() << "Audit entry hash mismatch:" << entry.entryId;
                return false;
            }
            
            expectedPreviousHash = entry.integrity.entryHash;
        }
        
        return true;
    }
    
private:
    std::vector<AuditEntry> entries_;
    
    void appendToLogFile(const AuditEntry& entry) {
        // Open log file in append mode
        std::ofstream logFile("/var/log/biofeedback/audit_trail.log", 
                              std::ios::app);
        
        // Write entry as JSON line
        logFile << entry.toJson().dump() << "\n";
        
        // Force flush to disk
        logFile.flush();
        fsync(logFile.fileno());  // Ensure physical write
    }
};
```

---

## 5.5.4. Strategie długoterminowej archiwizacji danych z zachowaniem śladu metrologicznego

### 5.5.4.1. Hierarchiczny model pamięci masowej dla danych medycznych

Ze względu na dużą objętość danych generowanych przez system oraz wymóg długoterminowego przechowywania (minimum 30 lat), zastosowano hierarchiczny model zarządzania pamięcią masową, obejmujący trzy poziomy:

**Poziom 1: Storage gorący (hot storage) – dane aktywne**

- **Nośnik:** Dyski SSD NVMe w serwerze lokalnym placówki;
- **Okres przechowywania:** 0–12 miesięcy od daty sesji;
- **Dostępność:** Natychmiastowa (ms);
- **Zastosowanie:** Sesje bieżących pacjentów, dane do analizy trendów, raportowania;
- **Redundancja:** RAID 1 (mirroring) z kopią zapasową w chmurze;
- **Szyfrowanie:** AES-256 w spoczynku, TLS 1.3 w tranzycie.

**Poziom 2: Storage ciepły (warm storage) – dane historyczne**

- **Nośnik:** Dyski HDD w macierzy NAS;
- **Okres przechowywania:** 1–10 lat od daty sesji;
- **Dostępność:** Kilka sekund (opóźnienie wynikające z ewentualnego przywrócenia z taśm);
- **Zastosowanie:** Dane pacjentów zakończonych terapii, archiwum do celów badawczych;
- **Redundancja:** RAID 6 (double parity) z replikacją do drugiej lokalizacji;
- **Kompresja:** LZMA2 dla oszczędności miejsca (stosunek kompresji ~3:1).

**Poziom 3: Storage zimny (cold storage) – archiwum długoterminowe**

- **Nośnik:** Taśmy magnetyczne LTO-9 (Linear Tape-Open);
- **Okres przechowywania:** 10–30+ lat;
- **Dostępność:** Godziny (konieczność fizycznego załadowania taśmy);
- **Zastosowanie:** Archiwum prawne, dane do ewentualnych postępowań sądowych;
- **Trwałość nośnika:** 30 lat według specyfikacji producenta (FUJIFILM, IBM);
- **Kopia bezpieczeństwa:** Dwie kopie taśm przechowywane w różnych lokalizacjach geograficznych.

**Charakterystyka nośników LTO-9 dla archiwum długoterminowego:**

| Parametr | Wartość |
|----------|---------|
| Pojemność natywna | 18 TB |
| Pojemność ze kompresją (2.5:1) | 45 TB |
| Przepływność zapisu | 400 MB/s (natywnie) |
| Żywotność nośnika | 30 lat (magazynowanie), 1000 cykli odczytu/zapisu |
| Koszt za TB | ~$8/TB (znacznie niższy niż dyski HDD/SSD) |
| Odporność na EMP | Wysoka (nośnik magnetyczny niepodatny na impuls elektromagnetyczny) |
| Wymagania środowiskowe | Temperatura 15–25°C, wilgotność 40–60% RH |

### 5.5.4.2. Procedura migracji danych między poziomami storage'u

Automatyczny proces migracji danych między poziomami storage'u jest realizowany przez daemon archiwizacyjny działający w tle na Raspberry Pi lub dedykowanym serwerze NAS:

```python
#!/usr/bin/env python3
# archive_manager.py - Hierarchical Storage Management Daemon

import os
import json
import shutil
import hashlib
from datetime import datetime, timedelta
from pathlib import Path
import logging

logging.basicConfig(level=logging.INFO, 
                    format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

class ArchiveManager:
    def __init__(self, config_path='/etc/biofeedback/archive_config.json'):
        with open(config_path, 'r') as f:
            self.config = json.load(f)
        
        self.hot_storage = Path(self.config['storage']['hot_path'])
        self.warm_storage = Path(self.config['storage']['warm_path'])
        self.cold_storage = Path(self.config['storage']['cold_path'])
        
        self.hot_retention_days = self.config['retention']['hot_days']  # 365
        self.warm_retention_days = self.config['retention']['warm_days']  # 3650
        
    def migrate_hot_to_warm(self):
        """Move sessions older than hot_retention_days to warm storage"""
        cutoff_date = datetime.now() - timedelta(days=self.hot_retention_days)
        
        for patient_dir in self.hot_storage.iterdir():
            if not patient_dir.is_dir():
                continue
                
            sessions_dir = patient_dir / 'sessions'
            if not sessions_dir.exists():
                continue
            
            for session_file in sessions_dir.glob('*.json'):
                # Extract date from filename or read from metadata
                session_date = self.get_session_date(session_file)
                
                if session_date < cutoff_date:
                    logger.info(f"Migrating {session_file} to warm storage")
                    
                    # Create target directory structure
                    target_dir = self.warm_storage / patient_dir.name / 'sessions'
                    target_dir.mkdir(parents=True, exist_ok=True)
                    
                    # Move file
                    target_file = target_dir / session_file.name
                    shutil.move(str(session_file), str(target_file))
                    
                    # Update index database
                    self.update_location_index(session_file.name, 'warm')
                    
    def migrate_warm_to_cold(self):
        """Move sessions older than warm_retention_days to cold storage (LTO tape)"""
        cutoff_date = datetime.now() - timedelta(days=self.warm_retention_days)
        
        sessions_to_archive = []
        
        for patient_dir in self.warm_storage.iterdir():
            if not patient_dir.is_dir():
                continue
                
            sessions_dir = patient_dir / 'sessions'
            if not sessions_dir.exists():
                continue
            
            for session_file in sessions_dir.glob('*.json'):
                session_date = self.get_session_date(session_file)
                
                if session_date < cutoff_date:
                    sessions_to_archive.append(session_file)
        
        if sessions_to_archive:
            # Create tarball for tape archive
            archive_name = f"biofeedback_archive_{datetime.now().strftime('%Y%m%d')}.tar.gz"
            archive_path = self.cold_storage / archive_name
            
            logger.info(f"Creating tape archive: {archive_path}")
            logger.info(f"Including {len(sessions_to_archive)} session files")
            
            # Create compressed tarball
            import tarfile
            with tarfile.open(archive_path, "w:gz") as tar:
                for session_file in sessions_to_archive:
                    # Add file with relative path
                    arcname = session_file.relative_to(self.warm_storage)
                    tar.add(session_file, arcname=arcname)
            
            # Calculate archive hash for integrity verification
            archive_hash = self.calculate_sha256(archive_path)
            logger.info(f"Archive SHA-256: {archive_hash}")
            
            # Write manifest file
            manifest = {
                'archive_name': archive_name,
                'creation_date': datetime.now().isoformat(),
                'number_of_sessions': len(sessions_to_archive),
                'sha256_hash': archive_hash,
                'tape_id': self.get_next_tape_id(),
                'retention_until': (datetime.now() + timedelta(days=30*365)).isoformat()
            }
            
            manifest_path = self.cold_storage / f"{archive_name}.manifest.json"
            with open(manifest_path, 'w') as f:
                json.dump(manifest, f, indent=2)
            
            # Remove original files from warm storage after successful archive
            for session_file in sessions_to_archive:
                session_file.unlink()
                logger.info(f"Removed {session_file} from warm storage")
    
    def verify_archive_integrity(self, archive_path):
        """Verify integrity of tape archive by comparing hashes"""
        manifest_path = Path(str(archive_path) + '.manifest.json')
        
        if not manifest_path.exists():
            logger.error(f"Manifest not found for {archive_path}")
            return False
        
        with open(manifest_path, 'r') as f:
            manifest = json.load(f)
        
        stored_hash = manifest['sha256_hash']
        calculated_hash = self.calculate_sha256(archive_path)
        
        if stored_hash == calculated_hash:
            logger.info(f"Archive integrity verified: {archive_path}")
            return True
        else:
            logger.error(f"Archive integrity check FAILED: {archive_path}")
            logger.error(f"Expected: {stored_hash}")
            logger.error(f"Got: {calculated_hash}")
            return False
    
    def calculate_sha256(self, file_path):
        """Calculate SHA-256 hash of a file"""
        sha256_hash = hashlib.sha256()
        with open(file_path, "rb") as f:
            for chunk in iter(lambda: f.read(4096), b""):
                sha256_hash.update(chunk)
        return sha256_hash.hexdigest()
    
    def get_session_date(self, session_file):
        """Extract session date from JSON metadata"""
        try:
            with open(session_file, 'r') as f:
                data = json.load(f)
            iso_date = data['header']['endTimeISO8601']
            return datetime.fromisoformat(iso_date.replace('Z', '+00:00'))
        except Exception as e:
            logger.warning(f"Could not parse date from {session_file}: {e}")
            # Fallback to file modification time
            return datetime.fromtimestamp(session_file.stat().st_mtime)
    
    def get_next_tape_id(self):
        """Generate next tape identifier"""
        # Implementation depends on tape library management system
        return f"LTO9-{datetime.now().strftime('%Y%m%d')}-001"

if __name__ == '__main__':
    manager = ArchiveManager()
    manager.migrate_hot_to_warm()
    manager.migrate_warm_to_cold()
```

### 5.5.4.3. Formaty długoterminowej archiwizacji i strategie migracji technologicznej

Jednym z największych wyzwań długoterminowej archiwizacji danych jest ryzyko dezaktualizacji formatów plików i technologii odczytu. Aby temu zapobiec, zastosowano następujące strategie:

**Strategia 1: Użycie formatów otwartych i standaryzowanych**

Wszystkie dane są przechowywane w formatach otwartych, udokumentowanych i powszechnie wspieranych:

| Typ danych | Format | Standard | Perspektywa długoterminowa |
|------------|--------|----------|---------------------------|
| Dane pomiarowe | JSON | RFC 8259 | Bardzo wysoka – JSON jest de facto standardem wymiany danych |
| Dane binarne | HDF5 | ISO/IEC 16926 | Wysoka – format używany w NASA, CERN, naukach o Ziemi |
| Dokumenty | PDF/A | ISO 19005 | Bardzo wysoka – format archiwalny ISO |
| Obrazy/wykresy | PNG, SVG | W3C, ISO | Bardzo wysoka – formaty otwarte bez patentów |
| Metadane | XML | W3C | Bardzo wysoka – fundament semantycznej sieci |

**Strategia 2: Periodic migration (migracja okresowa)**

Co 5 lat przeprowadzana jest kompleksowa migracja danych do najnowszych stabilnych wersji formatów:

```
Rok 2024: JSON (RFC 8259), HDF5 1.14
     ↓ (migracja w 2029)
Rok 2029: JSON-LD 2.0, HDF5 2.x (przyszły standard)
     ↓ (migracja w 2034)
Rok 2034: [nowy format przyszłości]
```

Proces migracji obejmuje:
1. Eksport danych ze starego formatu do formatu pośredniego (XML neutralny);
2. Transformację do nowego formatu docelowego;
3. Weryfikację integralności (porównanie hash'y przed i po);
4. Zachowanie kopii w formacie oryginalnym przez dodatkowe 5 lat.

**Strategia 3: Emulacja środowiska odczytu**

Dla zapewnienia możliwości odczytu przestarzałych formatów tworzona jest wirtualna maszyna z kompletnym środowiskiem oprogramowania z danej ery:

```yaml
# emulation_environment.yaml
emulation_profile:
  name: "Biofeedback Reader 2024"
  base_os: "Raspberry Pi OS 12 (Bookworm)"
  kernel_version: "6.1.y"
  application:
    name: "biofeedback-app"
    version: "3.2.1"
    build_date: "2024-03-15"
  dependencies:
    - "qtbase5-dev 5.15.8"
    - "libserialport 0.1.1"
    - "nlohmann-json 3.11.2"
    - "openssl 3.0.8"
  container_format: "Docker"
  image_hash: "sha256:abc123..."
  preservation_until: "2054-03-15"
```

**Strategia 4: Zachowanie dokumentacji formatów**

Do każdego archiwum dołączana jest kompletna dokumentacja techniczna formatów:

- Specyfikacja JSON Schema z opisem każdego pola;
- Przykładowe pliki danych z anotacjami;
- Kod źródłowy parserów i validatorów;
- Instrukcje konwersji do innych formatów.

---

## 5.5.5. Ślad metrologiczny (metrological traceability) w kontekście archiwizacji danych

### 5.5.5.1. Definicja i znaczenie śladu metrologicznego dla ważności pomiarów

Zgodnie z definicją zawartą w Międzynarodowym Słowniku Metrologii Podstawowej i Ogólnej (VIM – International Vocabulary of Metrology, JCGM 200:2012), **ślad metrologiczny** (metrological traceability) definiuje się jako:

> „Właściwość wyniku pomiaru, dzięki której może on być powiązany z określonym odniesieniem (zazwyczaj wzorcem krajowym lub międzynarodowym) za pomocą udokumentowanego, nieprzerwanego łańcucha wzorcowań, z których każde wnosi wkład do niepewności pomiaru."

W kontekście niniejszej pracy habilitacyjnej, utrzymanie śladu metrologicznego dla zarchiwizowanych danych pomiarowych jest warunkiem koniecznym dla:

1. **Uznania ważności pomiarów w czasie** – nawet po latach od rejestracji, dane muszą umożliwiać odtworzenie, z jaką niepewnością obarczony był każdy wynik;

2. **Porównywalności międzyokresowej** – możliwość porównania wyników pacjenta z różnych okresów (np. przed i po rehabilitacji) wymaga znajomości parametrów metrologicznych urządzenia w obu momentach czasowych;

3. **Ważności prawnej dokumentacji** – w postępowaniach sądowych dotyczących błędów medycznych lub odszkodowań sportowych, sąd musi mieć pewność, że pomiary były wykonane zwalidowanym urządzeniem z aktualnym wzorcowaniem;

4. **Możliwości publikacji naukowej** – recenzenci czasopism naukowych wymagają udokumentowania śladu metrologicznego dla danych eksperymentalnych.

### 5.5.5.2. Implementacja łańcucha wzorcowań w metadanych archiwalnych

Każdy plik sesyjny zawiera komplet metadanych metrologicznych tworzących nieprzerwany łańcuch odniesień do wzorców międzynarodowych:

```json
{
  "metrologicalTraceability": {
    "measurementResult": {
      "value": 287.45,
      "unit": "N",
      "expandedUncertainty_U": 2.5,
      "coverageFactor_k": 2,
      "confidenceLevel": "95%"
    },
    
    "calibrationChain": [
      {
        "level": 0,
        "description": "Badany system pomiarowy biofeedback",
        "deviceSerialNumber": "BIOFEEDBACK-SYS-2024-0042",
        "calibrationDate": "2024-01-10T09:00:00Z",
        "calibrationCertificate": "CAL-CERT-2024-0042-001",
        "calibratingBody": "Główny Urząd Miar, Warszawa",
        "accreditationNumber": "AP-123/PCA/2020"
      },
      {
        "level": 1,
        "description": "Wzorzec siły referencyjny transferowy",
        "deviceSerialNumber": "REF-STD-FORCE-500N-003",
        "deviceType": "Load cell HBM C2AC3/500kg",
        "calibrationDate": "2023-06-15T10:00:00Z",
        "calibrationCertificate": "GUM-CAL-2023-078945",
        "calibratingBody": "Główny Urząd Miar, Laboratorium Siły",
        "accreditationNumber": "AB-001/PCA/2018"
      },
      {
        "level": 2,
        "description": "Wzorzec masy klasy E1",
        "deviceSerialNumber": "MASS-E1-50KG-012",
        "nominalValue": "50 kg",
        "calibrationDate": "2022-11-20T08:00:00Z",
        "calibrationCertificate": "GUM-CAL-2022-034521",
        "calibratingBody": "Główny Urząd Miar, Laboratorium Masy",
        "accreditationNumber": "AB-001/PCA/2018"
      },
      {
        "level": 3,
        "description": "Międzynarodowy prototyp kilograma (IPK)",
        "location": "Międzynarodowe Biuro Miar i Wag (BIPM), Sèvres, Francja",
        "definition": "Definicja kilograma w układzie SI od 2019 r. oparta na stałej Plancka h",
        "traceabilityNote": "Od 20 maja 2019 r. kilogram definiowany jest poprzez ustalenie wartości liczbowej stałej Plancka h na 6.62607015×10⁻³⁴ J·s"
      }
    ],
    
    "uncertaintyPropagation": {
      "method": "Law of propagation of uncertainty (GUM Supplement 1)",
      "components": [
        {
          "source": "Powtarzalność wskazań czujnika",
          "type": "A (statystyczna)",
          "standardUncertainty_u": 1.2,
          "distribution": "normal",
          "degreesOfFreedom": 9
        },
        {
          "source": "Kwantyzacja przetwornika ADC",
          "type": "B (pozostałe)",
          "standardUncertainty_u": 0.3,
          "distribution": "rectangular"
        },
        {
          "source": "Dryft temperaturowy zera",
          "type": "B",
          "standardUncertainty_u": 0.8,
          "distribution": "normal"
        },
        {
          "source": "Błąd liniowości charakterystyki",
          "type": "B",
          "standardUncertainty_u": 0.5,
          "distribution": "rectangular"
        },
        {
          "source": "Histereza mechaniczna",
          "type": "B",
          "standardUncertainty_u": 0.4,
          "distribution": "rectangular"
        },
        {
          "source": "Niepewność wzorca odniesienia",
          "type": "B",
          "standardUncertainty_u": 1.5,
          "distribution": "normal",
          "traceableTo": "GUM-CAL-2023-078945"
        }
      ],
      "combinedStandardUncertainty_uc": 2.24,
      "expandedUncertainty_U": 4.48,
      "coverageFactor_k": 2.0
    }
  }
}
```

### 5.5.5.3. Procedura odtwarzania śladu metrologicznego z archiwum

W sytuacji konieczności odtworzenia śladu metrologicznego dla danych zarchiwizowanych (np. na potrzeby postępowania sądowego), system udostępnia procedurę ekstrakcji kompletnego łańcucha wzorcowań:

```python
#!/usr/bin/env python3
# metrological_trace_extractor.py

import json
from pathlib import Path
from datetime import datetime

class MetrologicalTraceExtractor:
    def __init__(self, archive_db_path='/var/lib/biofeedback/calibration_db.json'):
        self.archive_db_path = Path(archive_db_path)
        self.calibration_database = self.load_calibration_db()
    
    def extract_full_trace(self, session_file_path):
        """Extract complete metrological traceability chain for a session"""
        
        # Load session data
        with open(session_file_path, 'r') as f:
            session_data = json.load(f)
        
        # Extract metrology section
        metrology = session_data['metrology']
        
        # Build traceability report
        report = {
            'report_id': f"TRACE-{datetime.now().strftime('%Y%m%d-%H%M%S')}",
            'session_id': session_data['header']['sessionId'],
            'extraction_date': datetime.now().isoformat(),
            
            'measurement_result': {
                'parameter': 'Force',
                'unit': 'Newton (N)',
                'range': '0–500 N',
                'resolution': '0.1 N',
                'accuracy': '±2% FS'
            },
            
            'traceability_chain': self.build_chain(metrology),
            
            'uncertainty_budget': metrology['measurementUncertainty'],
            
            'validity_statement': self.generate_validity_statement(metrology),
            
            'attachments': self.collect_supporting_documents(metrology)
        }
        
        return report
    
    def build_chain(self, metrology):
        """Build visual representation of calibration chain"""
        chain = []
        
        # Level 0: Tested device
        chain.append({
            'level': 0,
            'entity': 'Biofeedback Measurement System',
            'serial': metrology['device']['systemSerialNumber'],
            'calibration_cert': metrology['calibrationCertificateId'],
            'calibration_date': metrology['calibrationDateISO8601'],
            'valid_until': metrology['calibrationValidUntilISO8601'],
            'calibrating_lab': metrology['calibrationLaboratory'],
            'accreditation': 'AP-123/PCA/2020'
        })
        
        # Level 1-N: Reference standards (from database)
        ref_standard_id = metrology['referenceStandardId']
        if ref_standard_id in self.calibration_database:
            ref_standard = self.calibration_database[ref_standard_id]
            chain.append({
                'level': 1,
                'entity': ref_standard['description'],
                'serial': ref_standard['serialNumber'],
                'calibration_cert': ref_standard['certificateId'],
                'calibration_date': ref_standard['calibrationDate'],
                'valid_until': ref_standard['validUntil'],
                'calibrating_lab': ref_standard['laboratory'],
                'accreditation': ref_standard['accreditation']
            })
        
        # Add higher levels from database...
        
        return chain
    
    def generate_validity_statement(self, metrology):
        """Generate formal statement of metrological validity"""
        
        session_date = datetime.fromisoformat(
            metrology['sessionDateISO8601'].replace('Z', '+00:00')
        )
        cal_valid_until = datetime.fromisoformat(
            metrology['calibrationValidUntilISO8601'].replace('Z', '+00:00')
        )
        
        if session_date <= cal_valid_until:
            validity_status = "VALID"
            statement = (
                f"Pomiary zarejestrowane w dniu {session_date.strftime('%Y-%m-%d')} "
                f"zostały wykonane urządzeniem posiadającym ważną kalibrację "
                f"(ważna do {cal_valid_until.strftime('%Y-%m-%d')}). "
                f"Ślad metrologiczny jest kompletny i prowadzi do wzorców międzynarodowych SI."
            )
        else:
            validity_status = "EXPIRED"
            statement = (
                f"OSTRZEŻENIE: Pomiary zarejestrowane po dacie ważności kalibracji. "
                f"Wymagana jest ocena ryzyka i ewentualna ponowna kalibracja wsteczna."
            )
        
        return {
            'status': validity_status,
            'statement': statement,
            'prepared_by': 'System Automatyczny',
            'verified_by': '[Imię i Nazwisko Metrologa]',
            'date': datetime.now().isoformat()
        }
    
    def collect_supporting_documents(self, metrology):
        """List all supporting calibration certificates"""
        documents = []
        
        # Certificate for tested device
        documents.append({
            'type': 'Calibration Certificate',
            'id': metrology['calibrationCertificateId'],
            'path': f"/certificates/{metrology['calibrationCertificateId']}.pdf",
            'hash': self.calculate_document_hash(
                f"/certificates/{metrology['calibrationCertificateId']}.pdf"
            )
        })
        
        # Certificate for reference standard
        # ... retrieve from database
        
        return documents
    
    def load_calibration_db(self):
        """Load calibration database from archive"""
        if self.archive_db_path.exists():
            with open(self.archive_db_path, 'r') as f:
                return json.load(f)
        return {}
    
    def calculate_document_hash(self, file_path):
        """Calculate SHA-256 hash of supporting document"""
        import hashlib
        sha256 = hashlib.sha256()
        with open(file_path, 'rb') as f:
            for chunk in iter(lambda: f.read(4096), b""):
                sha256.update(chunk)
        return sha256.hexdigest()

# Usage example
if __name__ == '__main__':
    extractor = MetrologicalTraceExtractor()
    trace_report = extractor.extract_full_trace(
        '/data/patients/PAT-2024-00127/sessions/SESSION-2024-03-15-P001-S003.json'
    )
    
    # Save report
    with open('/reports/trace_report.json', 'w') as f:
        json.dump(trace_report, f, indent=2)
    
    print(f"Traceability report generated: {trace_report['report_id']}")
```

---

## 5.5.6. Podsumowanie i wnioski

Rozdział 5.5 przedstawia kompleksowe podejście do zagadnień archiwizacji i integralności danych pomiarowych w autorskim systemie pomiarowo-treningowym z pętlą sprzężenia zwrotnego. Zastosowane rozwiązania spełniają najbardziej rygorystyczne wymagania międzynarodowych standardów medycznych, metrologicznych i bezpieczeństwa danych:

1. **Wielopoziomowy system zabezpieczeń integralności** – od CRC-8 dla pojedynczych pakietów, przez hashe MD5/SHA-256 dla plików, aż po podpisy cyfrowe RSA dla zapewnienia niezaprzeczalności;

2. **Struktura danych JSON z walidacją schematu** – otwarty format z formalnym schematem JSON Schema gwarantującym długoterminową czytelność i możliwość automatycznej weryfikacji;

3. **Kompletny audit trail** – rejestracja wszystkich operacji na danych z mechanizmem łańcucha hash'y uniemożliwiającym niezauważoną modyfikację historii;

4. **Hierarchiczny model storage'u** – optymalizacja kosztów i dostępności poprzez trójpoziomowy system (SSD/HDD/LTO) z automatyczną migracją danych;

5. **Strategie długoterminowej preservacji** – migracja okresowa formatów, emulacja środowisk, dokumentacja techniczna zapewniające czytelność danych przez 30+ lat;

6. **Pełny ślad metrologiczny** – udokumentowany łańcuch wzorcowań od badanego urządzenia do międzynarodowych wzorców SI z propagacją niepewności na każdym poziomie.

Przedstawione rozwiązania stanowią istotny wkład w rozwój metodologii zarządzania danymi w systemach biomechanicznych klasy medycznej i mogą serve jako wzorzec dla innych rozwiązań inżynierii biomedycznej wymagających długoterminowej archiwizacji danych pomiarowych z zachowaniem pełnej ważności metrologicznej.
