# 🧩 Wspólna warstwa usług urządzeń i pomiarów

> 📘 Weryfikacja kodu: 2026-11-09 • Zakres: `ApplicationServices`, `DeviceService`, `MeasurementService` i punkty uruchomieniowe klientów.  
> Status: biblioteka usług istnieje; nie wszystkie procesy i interfejsy korzystają z jednej instancji.

## 1. Usługi zaimplementowane

- `DeviceService` rejestruje adaptery transportowe, udostępnia listę i stan urządzeń, obsługuje cykl połączenia oraz powiadomienia o zmianach. Nie udostępnia API sterowania efektorami.
- `MeasurementService` zarządza stanem uruchomienia potoku, przyjmowaniem próbek, subskrypcjami i dostępem do ograniczonej historii silnika.
- `ApplicationServices` jest kompozycją `DeviceService` i `MeasurementService` w obrębie pojedynczego procesu.

## 2. Rzeczywista integracja w bieżącym kodzie

- Desktop i TUI korzystają z lokalnych adapterów szeregowych i własnych instancji `MeasurementEngine` w swoich ścieżkach pomiarowych.
- Samodzielny `biofeedback_api` tworzy `MeasurementService`, `AcquisitionService` i `DeviceService`, a następnie przekazuje je do `HttpApiServer`.
- Serwer API może zarejestrować adapter Nano HX711 po ustawieniu `BIOFEEDBACK_SERIAL_PORT`.
- WebUI i Android są klientami HTTP API. Nie mają bezpośredniego dostępu do portu szeregowego.
- Nie ma obecnie jednej wspólnej instancji `ApplicationServices` współdzielonej między Desktop, TUI i osobnym procesem API. Procesy mają odrębne przestrzenie pamięci i odrębne cykle życia.
- WebSocket/SSE nie jest zaimplementowany w serwerze.

## 3. Architektura docelowa

Docelowo lokalne i zdalne interfejsy powinny korzystać z jednego autorytatywnego serwisu sesji i akwizycji:
- jeden właściciel każdego fizycznego portu i jednego strumienia;
- jawne wiązanie urządzenia, pacjenta i sesji;
- trwałe zatwierdzanie próbek niezależnie od odbiorców;
- ograniczone bufory i kontrola backpressure;
- uwierzytelnianie, autoryzacja per pacjent i audyt operacji;
- kontrolowane zatrzymanie oraz odzyskiwanie po restarcie.

Nie wolno utożsamiać istnienia klas usług z ukończeniem migracji klientów do centralnej akwizycji.

## 4. Cykl życia docelowej kompozycji

1. Proces tworzy jedną instancję `ApplicationServices`.
2. Rejestruje adaptery urządzeń przez `DeviceService`.
3. Rozpoczyna sesję przez serwis akwizycji po sprawdzeniu pacjenta i urządzenia.
4. Odbiorcy subskrybują próbki i zdarzenia bez otwierania kolejnego połączenia z urządzeniem.
5. Przed publikacją próbka przechodzi przez trwałą granicę zapisu.
6. Przy zamykaniu proces zatrzymuje sesje i rozłącza adaptery w kontrolowanej kolejności.

## 5. Zakres testów

W repozytorium znajdują się testy `device_service_regression`, `measurement_service_regression`, `acquisition_service_regression` i `acquisition_serial_adapter_regression`. Testy jednostkowe usług nie dowodzą jeszcze, że wszystkie aplikacje korzystają z jednej instancji ani że istnieje strumień sieciowy.
