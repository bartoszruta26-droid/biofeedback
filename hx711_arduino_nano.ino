/*
 * Arduino Nano + HX711 - Przykładowy kod do wagi elektronicznej
 * 
 * Połączenia:
 * HX711      Arduino Nano
 * VCC   ---> 5V (lub 3.3V)
 * GND   ---> GND
 * DT    ---> D2 (Pin danych)
 * SCK   ---> D3 (Pin zegara)
 * 
 * Wymagana biblioteka: "HX711 Arduino Library" by Bogdan Necula
 * Instalacja: Szkic -> Dołącz bibliotekę -> Zarządzaj bibliotekami -> wyszukaj "HX711"
 */

#include "HX711.h"

// Definicja pinów
#define DT_PIN  2  // Pin danych (Data)
#define SCK_PIN 3  // Pin zegara (Clock)

// Obiekt HX711
HX711 scale;

// Kalibracja - wartość skalowania (CALIBRATION_FACTOR)
// Należy wykalibrować wagę przed użyciem (patrz funkcja calibrate())
// Przykładowa wartość, należy dostosować do własnego układu
float calibration_factor = 420.0; 

void setup() {
  Serial.begin(9600);
  Serial.println("Waga z HX711 - Start");
  Serial.println("Inicjalizacja...");

  // Inicjalizacja HX711
  scale.begin(DT_PIN, SCK_PIN);
  
  // Ustawienie współczynnika kalibracji
  scale.set_scale(calibration_factor);
  
  // Wyzerowanie wagi (upewnij się, że na wadze nic nie leży)
  scale.tare();
  
  Serial.println("Waga gotowa!");
  Serial.println("Połóż obiekt na wadze, aby odczytać masę.");
  Serial.println("");
}

void loop() {
  // Sprawdzenie, czy dostępne są dane z HX711
  if (scale.is_ready()) {
    // Odczyt wartości w gramach (po zastosowaniu kalibracji)
    float weight = scale.get_units(10); // Średnia z 10 pomiarów
    
    // Wyświetlenie wyniku
    Serial.print("Masa: ");
    Serial.print(weight, 2); // 2 miejsca po przecinku
    Serial.println(" g");
    
    delay(500); // Odczyt co 500ms
  } else {
    Serial.println("HX711 nie odpowiada - sprawdź połączenia!");
    delay(1000);
  }
}

/*
 * FUNKCJA KALIBRACJI (opcjonalna)
 * Aby wyznaczyć calibration_factor:
 * 1. Umieść znany ciężar na wadze (np. 100g)
 * 2. Odczytaj surową wartość z get_units() bez skalowania
 * 3. Oblicz: calibration_factor = odczyt_surowy / znana_masa
 * 
 * Możesz też użyć poniższej procedury:
 * 
 * void calibrate() {
 *   Serial.println("Kalibracja...");
 *   Serial.println("Upewnij się, że na wadze NIC nie leży.");
 *   delay(5000);
 *   scale.tare();
 *   Serial.println("Teraz połóż ZNANY ciężar (np. 100g).");
 *   Serial.println("Wpisz masę w gramach w monitorze portu szeregowego.");
 *   
 *   while (!Serial.available());
 *   float known_weight = Serial.parseFloat();
 *   
 *   long raw_value = scale.get_units(20);
 *   calibration_factor = raw_value / known_weight;
 *   
 *   Serial.print("Nowy calibration_factor: ");
 *   Serial.println(calibration_factor);
 *   Serial.println("Skopiuj tę wartość do kodu jako calibration_factor.");
 * }
 */
