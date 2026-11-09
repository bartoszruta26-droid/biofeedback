# 🧮 Macierz funkcji interfejsów — stan kodu main

> 🕒 Audyt zgodności z kodem • Data: 2026-11-09 • Bazowy SHA: `28272ea153864747d947038a3c0e853f9c1864ec`  
> Macierz opisuje zakres widoków i operacji znalezionych w kodzie. Nie jest deklaracją równoważności funkcjonalnej ani walidacji sprzętowej.

**Statusy:** **D** — funkcja rzeczywiście zaimplementowana w danym kliencie; **C** — zakres ograniczony, częściowy lub zależny od API; **S** — ekran informacyjny/szkielet; **N** — brak funkcji w kliencie.

| Moduł | Desktop Qt | TUI | WebUI | Android |
|---|---:|---:|---:|---:|
| Pulpit / status | D | D | C | S |
| Karty pacjentów: odczyt | D | D | D | D |
| Karty pacjentów: dodawanie / edycja / usuwanie | D | N | D | D |
| Pomiary: lokalny odczyt czujnika | D | D | N | N |
| Pomiary: archiwum z API | D | D | D | C |
| Pomiary: transmisja na żywo z API | N | N | N | N |
| Trening: sesja z czujnikiem | D | D | N | N |
| Trening: ręczny wpis sesji/wyniku | N | N | D | N |
| Trening: odczyt historii | D | C | D | C |
| Gry: rzeczywista rozgrywka biofeedback | D | N | N | N |
| Gry: archiwum wyników | D | N | D | C |
| Gry: ręczny wpis wyniku | N | N | D | N |
| Urządzenia: lokalny port szeregowy | D | D | N | N |
| Urządzenia: lista z serwerowego API | C | N | C | S |
| Urządzenia: zdalne łączenie/rozłączanie przez API | N | N | N | N |
| Kalibracja czujnika | C | N | N | N |
| Diagnostyka | D | C | C | S |
| Ustawienia | D | C | C | S |
| Użytkownicy / zarządzanie rolami | C | N | N | N |
| Raporty / historia | C | C | C | C |

## Uwagi interpretacyjne

1. **Desktop Qt:** posiada lokalny tor szeregowy, pomiary, trening i gry. Aplikacja zapisuje karty, sesje pomiarowe, sesje treningowe, wyniki ćwiczeń i wyniki gier we wspólnym repozytorium. Archiwizacja sesji treningowej i pomiarowej wymaga wybranego pacjenta w logice `Application.cpp`.
2. **TUI:** odczytuje listę pacjentów i pozwala wybrać kartę; wybór jest opcjonalny. Otwiera port szeregowy lokalnie, buforuje próbki i zapisuje pomiar przy rozłączeniu/wyjściu. Zapisuje sesje i wyniki treningowe. Nie ma CRUD pacjentów ani silnika gier.
3. **WebUI:** wykonuje CRUD pacjentów, odczytuje archiwum pomiarów, treningów i gier, a także umożliwia ręczne utworzenie wpisu treningu oraz sesji/wyniku gry. Ręczny wpis nie jest rozgrywką ani pomiarem z czujnika. Nie ma działającego strumienia WebSocket.
4. **Android:** loguje się przez HTTPS, wykonuje CRUD kart pacjentów i odczytuje kolekcje pomiarów, treningów i gier. Pozostałe widoki są w trybie podglądu; nie tworzy sesji pomiarowych/treningowych/gier i nie łączy się bezpośrednio z urządzeniem.
5. **API:** operacje na kolekcjach i sterowanie akwizycją wymagają obecnie roli `administrator`. Serwerowy rejestr urządzeń jest tylko do odczytu. Endpoint próbek jest odpytywany przez HTTP i zwraca ograniczoną historię w pamięci; nie jest to strumień czasu rzeczywistego.

## Podstawa weryfikacji

Przejrzano m.in. `src/core/Application.cpp`, `src/tab/TrainingTab.cpp`, `src/tab/MeasurementTab.cpp`, `src/tui/main.cpp`, `src/api/HttpApiServer.cpp`, `platforms/webui/app.js`, `platforms/webui/api-client.js` oraz `platforms/android/app/src/main/java/pl/biofeedback/app/MainActivity.kt`.

Status D wymaga rzeczywistej operacji, obsługi błędów i testu odpowiedniej warstwy. Sam ekran, metoda klienta lub wpis w planie nie wystarcza.
