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

## Zakres bieżącej wersji
Aplikacja zawiera responsywną nawigację dolną i widoki Pulpit, Pomiary,
Pacjenci, Trening oraz Urządzenia. Klient API obsługuje logowanie Bearer
oraz CRUD sześciu kolekcji serwera: pacjentów, pomiarów, sesji i wyników
treningowych oraz sesji i wyników gier. Ekran Pacjenci umożliwia pobranie listy,
dodawanie, edycję i usuwanie kart przez wspólne API.

Ekrany Pomiary, Trening i Urządzenia pozostają podglądowe: aplikacja nie
odbiera jeszcze strumienia pomiarowego i nie tworzy sesji treningowych
ani wyników gier z rzeczywistych danych. Komunikacja bezpośrednia Bluetooth,
USB i port szeregowy nie jest zaimplementowana. Serwer wymaga HTTPS dla
połączeń zdalnych; aplikacja Android nie łączy się z jego lokalnym HTTP
bezpośrednio. Użyj bramy TLS/reverse proxy.
