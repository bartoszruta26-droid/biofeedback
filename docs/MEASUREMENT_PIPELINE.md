# Wspólny silnik pomiarowy

## Cel i granice odpowiedzialności

`biofeedback_measurement` jest niezależną od Qt warstwą domenową. Przyjmuje próbki z adapterów źródłowych, normalizuje obsługiwane jednostki, przypisuje czas odbioru hosta, waliduje wartości i nadaje jednoznaczny status jakości. Utrzymuje ograniczoną historię i udostępnia bezpieczne wielokrotne subskrypcje.

Przepływ:

```text
SerialCommunication
       |
MeasurementSerialAdapter
       |
MeasurementEngine
       |---- Desktop Qt
       |---- TUI
       |---- API / WebSocket (kolejny etap)
       ---- Android (przez API, kolejny etap)
```

## Model próbki

- `sequence`: monotoniczny numer próbki w obrębie instancji silnika.
- `timestampUnixMs`: czas odebrania próbki przez hosta w UTC, w milisekundach od epoki Unix.
- `deviceTimestampMs`: surowy znacznik czasu urządzenia (32-bitowy, milisekundy); nie zastępuje czasu hosta.
- `rawValue`: surowy odczyt przetwornika.
- `value`, `unit`: wartość po kalibracji, przeliczona do jednostki kanonicznej.
- `source`: identyfikator adaptera/źródła.
- `quality`: stan jakości, nigdy nie jest domyślnie prezentowany jako pomiar prawidłowy.

## Jednostki

Dostępne jawne normalizacje:
- masa: `g`, `mg` -> `kg`; `kg` pozostaje `kg`;
- długość: `mm`, `cm` -> `m`; `m` pozostaje `m`;
- napięcie: `mV`, `uV` -> `V`; `V` pozostaje `V`;
- siła: `N`, `newton(s)` -> `N`;
- odczyt surowy: `raw`, `count(s)` -> `count`.

Silnik nie zgaduje wymiaru fizycznego i nie przelicza między różnymi wymiarami (np. kg na N). Adapter musi przekazać jednostkę wynikającą z konfiguracji urządzenia i kalibracji. Wartość surowa bez kalibracji powinna używać `count`.

## Statusy jakości

- `valid` — próbka przeszła walidację.
- `sensor_rejected` — źródłowy sterownik oznaczył próbkę jako niepoprawną.
- `non_finite_value` — wartość jest NaN lub nieskończonością.
- `missing_timestamp` — urządzenie przekazało zerowy znacznik czasu.
- `invalid_unit` — jednostka nie należy do jawnie obsługiwanego zestawu.
- `invalid_source` — brak identyfikatora źródła.
- `outside_configured_range` — wartość po normalizacji przekracza zakres konfiguracji silnika.

Próbki odrzucone są zachowywane w historii i przekazywane subskrybentom ze statusem jakości. Konsument musi sprawdzić `isValid()`/ `quality` przed wykreśleniem, agregacją lub eksportem.

## Integracja z portem szeregowym

`sensor::MeasurementSerialAdapter` rejestruje wielokrotny callback w istniejącym `SerialCommunication`, mapuje `SensorData` do `MeasurementInput` i przekazuje próbkę do silnika. Adapter odłącza callback przy niszczeniu i ignoruje callbacki będące w locie po zamknięciu adaptera.

Przykład:

```cpp
auto engine = std::make_shared<biofeedback::measurement::MeasurementEngine>();
sensor::SerialCommunication serial;
sensor::MeasurementSerialAdapter adapter(serial, engine, "hx711-main", "count");
```

Dla urządzenia skalibrowanego do siły jednostkę należy ustawić jawnie jako `N`; nie należy automatycznie zakładać, że wskazanie HX711 oznacza kilogramy lub niutony.

## Ograniczenia obecnego etapu

- API HTTP nie udostępnia jeszcze próbek ani danych pacjentów.
- WebSocket nie jest jeszcze serwerowo zaimplementowany.
- WebUI i Android nie pobierają jeszcze próbek z silnika.
- Adapter serialny przekazuje wartość `calibratedValue` istniejącego sterownika; nie zastępuje ani nie powiela jego kalibracji.
- Znacznik czasu urządzenia jest przechowywany jako surowe 32 bity; rozwinięcie przepełnienia licznika firmware wymaga osobnego protokołu synchronizacji.
- Zakres walidacji jest konfigurowany w jednostce kanonicznej; domyślnie jest szeroki i nie stanowi klinicznego zakresu referencyjnego.

## Testy

`measurement_engine_regression` sprawdza normalizację jednostek, błędne dane, niepoprawny znacznik czasu, zakres, sekwencję, ograniczenie historii i subskrypcje. Test nie wymaga fizycznego urządzenia.
