# 🧩 Wspólna warstwa usług urządzeń i pomiarów

> 📘 Dokumentacja projektu • Język: polski • Nazwy techniczne, identyfikatory API i symbole kodu pozostają oryginalne.

ApplicationServices jest kompozycją pojedynczego potoku pomiarowego i rejestru urządzeń w obrębie procesu. Interfejsy lokalne powinny otrzymywać referencje do usług devices() i measurements(), zamiast tworzyć własne silniki lub historie.

## Usługi

- DeviceService rejestruje adaptery transportowe, udostępnia listę i stan urządzeń, obsługuje cykl połączenia oraz powiadomienia o zmianach.
- MeasurementService zarządza uruchomieniem potoku, przyjmowaniem próbek, subskrypcjami i ograniczoną historią.
- ApplicationServices zapewnia wspólną własność komponentów w danym procesie.

## Integracja interfejsów

Desktop GUI, TUI i serwer API powinny korzystać z tej samej instancji ApplicationServices w procesie aplikacji. WebUI i Android są klientami zdalnymi i powinny korzystać z kontraktu API delegującego do tych usług. Klienci zdalni nie łączą się bezpośrednio z portem szeregowym ani nie tworzą własnego silnika pomiarowego.

Adapter urządzenia implementuje wyłącznie transport i stan połączenia. DeviceService celowo nie udostępnia poleceń sterowania elektrodami, anteną, PWM ani innymi efektorami.

## Cykl życia

1. Aplikacja tworzy jeden obiekt ApplicationServices.
2. Rejestruje adaptery urządzeń przez DeviceService.
3. Uruchamia pomiary przez MeasurementService po przygotowaniu źródła danych.
4. Interfejsy subskrybują zdarzenia i próbki, a przy zamykaniu usuwają subskrypcje.
5. Przy zamykaniu aplikacja zatrzymuje pomiary i rozłącza adaptery.

## Zakres tej zmiany

Dodano wspólną bibliotekę usług, kompozycję ApplicationServices i regresję automatyczną. Nie oznacza to jeszcze przepięcia wszystkich istniejących ekranów GUI/TUI i endpointów API na tę instancję. Mosty HTTP/WebSocket oraz integracja WebUI i Androida wymagają wstrzyknięcia ApplicationServices w punktach startowych odpowiednich procesów.