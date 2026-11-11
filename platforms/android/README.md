# Biofeedback Android

Natywny interfejs Android oparty na Kotlinie i Jetpack Compose.

## Wymagania
- JDK 17
- Android SDK Platform 35
- Android Gradle Plugin 8.7.3
- Gradle 8.9 lub nowszy zgodny z AGP 8.7

## Budowanie
Z katalogu platforms/android uruchom:

    gradle :app:assembleDebug

## Zakres wersji 0.1
Aplikacja zawiera responsywną nawigację dolną i widoki Pulpit, Pomiary,
Pacjenci, Trening oraz Urządzenia. Status jest jawnie oznaczony jako podgląd.
Nie ma jeszcze komunikacji z API, Bluetooth, USB ani portem szeregowym.
Kolejnym etapem będzie wspólny klient API/WebSocket współdzielący kontrakt
z WebUI i korzystający z backendu MeasurementEngine.
