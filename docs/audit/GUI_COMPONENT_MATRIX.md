# Macierz modułów interfejsów

Statusy: **D** — potwierdzona implementacja w przeglądanym kodzie; **C** — częściowa/ograniczona; **S** — szkielet lub placeholder; **N** — brak potwierdzonej implementacji. Macierz nie jest deklaracją parytetu funkcji.

| Moduł | Desktop Qt | TUI | Web UI | Android |
|---|---:|---:|---:|---:|
| Pulpit/status | D | D | C | S |
| Pacjenci | D | C | S | S |
| Pomiary | D | C | S | S |
| Trening | D | C | S | S |
| Gry biofeedback | D | N | N | N |
| Urządzenia | D | C | S | S |
| Kalibracja | C | C | N | N |
| Diagnostyka | D | C | C | S |
| Ustawienia | D | C | C | S |
| Użytkownicy/role | C | N | N | N |
| Historia/raporty | C | C | N | N |

## Reguła odbioru

Status D wymaga nie tylko widoku, ale działania na rzeczywistym źródle danych, poprawnej obsługi błędów i testu. Widoki demonstracyjne lub informacyjne pozostają S/C.
