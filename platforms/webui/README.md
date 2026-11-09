# Biofeedback WebUI

> 📘 Weryfikacja kodu: 2026-11-09 • Zakres: `platforms/webui/` i `src/api/HttpApiServer.cpp`.

Responsywny interfejs przeglądarkowy HTML/CSS/JavaScript. Zawiera nawigację modułową, tryb jasny/ciemny, logowanie oraz klienta wspólnego API HTTP.

## Uruchomienie interfejsu

Z katalogu głównego repozytorium:

```bash
python3 -m http.server 8080 --directory platforms/webui
```

Następnie otwórz `http://localhost:8080`. Jest to wyłącznie serwer plików statycznych WebUI; nie uruchamia backendu Biofeedback API. Samodzielny serwer API należy zbudować i uruchomić oddzielnie zgodnie z `docs/INSTALL_LINUX.md` oraz `docs/ARCHITECTURE_SHARED_DATA_SERVER.md`.

## Funkcje potwierdzone w kodzie

- Logowanie i wylogowanie przez token Bearer.
- Odczyt statusu API.
- CRUD kart pacjentów: lista, dodawanie, edycja i usuwanie.
- Odczyt archiwów pomiarów, sesji treningowych, wyników treningowych, sesji gier i wyników gier.
- Ręczne tworzenie wpisu sesji treningowej i wyniku treningu (`source_mode: manual_entry`).
- Ręczne tworzenie wpisu sesji gry i wyniku gry (`source_mode: manual_entry`).
- Kontrola wersji rekordów przy aktualizacji i usuwaniu.
- Jawne prezentowanie błędów HTTP, sieci i braku uprawnień; brak fikcyjnych danych zastępczych.

Ręczny wpis treningu lub wyniku gry jest archiwizacją danych wprowadzonych przez operatora. Nie uruchamia rzeczywistego treningu z czujnikiem ani silnika gry biofeedback.

## Ograniczenia integracji

- WebUI nie otwiera portu szeregowego i nie prowadzi akwizycji czujnika.
- Serwer nie udostępnia obecnie endpointów połączenia/rozłączenia urządzenia, mimo że klient JavaScript zawiera metody `connectDevice` i `disconnectDevice`.
- Klient zawiera metody subskrypcji `/ws/measurements` i `/ws/events`, ale serwer nie implementuje WebSocket ani SSE. Nie należy traktować tych metod jako działającej funkcji.
- Odczyt próbek z serwerowej akwizycji jest obecnie dostępny wyłącznie przez ograniczony endpoint HTTP `GET /api/v1/acquisition/samples`, wymagający administratora; WebUI nie ma gotowego widoku strumienia.
- Endpointy danych wymagają obecnie roli `administrator`.
- Serwer API domyślnie nasłuchuje na loopback. Wdrożenie zdalne wymaga bramy TLS/reverse proxy i odpowiedniej polityki CORS.

## Testy

Test klienta JavaScript:

```bash
node --test platforms/webui/api-client.test.js
```

Testy te weryfikują zachowanie klienta z kontrolowanymi odpowiedziami HTTP; nie zastępują testów integracyjnych z uruchomionym backendem.
