# Biofeedback Android

> 📘 Weryfikacja kodu: 2026-11-09 • Zakres: `ApiClient.kt`, `MainActivity.kt`.

Natywny interfejs Android oparty na Kotlinie i Jetpack Compose.

## Wymagania

- JDK 17
- Android SDK Platform 35
- Android Gradle Plugin 8.7.3
- Gradle 8.9 lub nowszy zgodny z AGP 8.7

## Budowanie

Z katalogu `platforms/android`:

```bash
gradle :app:assembleDebug
```

## Funkcje potwierdzone w kodzie

- Logowanie Bearer i odczyt bieżącej tożsamości.
- Klient wymaga adresu API z prefiksem `https://`; żądania HTTP bez TLS są odrzucane.
- Ekran Pacjenci obsługuje pobieranie, dodawanie, edycję i usuwanie kart przez wspólne API.
- Ekran Pomiary odczytuje zapisane rekordy kolekcji `measurements`.
- Ekran Trening odczytuje sesje i wyniki treningowe oraz sesje i wyniki gier.
- Token Bearer jest przechowywany w pamięci procesu i usuwany przy wylogowaniu lub odpowiedzi 401.

Klient `ApiClient` udostępnia ogólne metody CRUD dla sześciu kolekcji, ale nie oznacza to, że ekrany Androida udostępniają wszystkie te operacje. W bieżącym interfejsie zapisywane i edytowane są karty pacjentów; pozostałe widoki służą do odczytu archiwów.

## Ograniczenia

- Pomiary i trening pozostają w trybie podglądu; aplikacja nie odbiera strumienia próbek i nie tworzy nowych sesji pomiarowych, treningowych ani wyników gier z danych czujnika.
- Ekran Urządzenia jest informacyjny; bezpośrednia komunikacja Bluetooth, USB i port szeregowy nie jest zaimplementowana.
- Brak WebSocket/SSE w serwerze oznacza brak strumienia czasu rzeczywistego.
- Operacje na kolekcjach serwera wymagają obecnie roli `administrator`; zakresowe RBAC nie jest wdrożone.
- Zdalne połączenie wymaga bramy TLS/reverse proxy przed serwerem API, który domyślnie nasłuchuje na loopback.
- Testy kompilacji Androida i testy instrumentacyjne należy raportować oddzielnie od testów C++/Desktop.

## Powiązana dokumentacja

- [Kontrakt API i rzeczywisty stan endpointów](../../platforms/webui/API_CONTRACT.md)
- [Macierz funkcji interfejsów](../../docs/audit/GUI_COMPONENT_MATRIX.md)
