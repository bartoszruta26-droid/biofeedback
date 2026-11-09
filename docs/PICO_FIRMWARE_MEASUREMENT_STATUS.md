# 🍓 Firmware Raspberry Pi Pico — stan komunikacji pomiarowej

> 📘 Dokument techniczny • Język: polski • Status: częściowa implementacja diagnostyczna  
> ⚠️ Dokument opisuje cel firmware Pico; nie potwierdza gotowości akwizycji czujnika.

## 1. Aktualny stan implementacji

Repozytorium zawiera cel budowania oparty na Raspberry Pi Pico SDK oraz ograniczoną pętlę obsługi komend UART.

| Funkcja | Stan |
|---|---|
| Komunikacja UART0 | Zaimplementowana: GP0 TX / GP1 RX, 115200 bodów, 8N1 |
| Sprzętowy watchdog | Zaimplementowany: limit 2 sekund |
| Ograniczony parser komend | Zaimplementowany |
| Diagnostyka PING / STATUS / SAFE | Zaimplementowana |
| Obsługa nieznanych komend | Odrzucanie |
| Akwizycja czujnika i ramki pomiarowe | Jeszcze niezaimplementowane w tym celu firmware Pico |
| Kalibracja z fizycznymi czujnikami | Niezweryfikowana |

## 2. Protokół diagnostyczny host–Pico

- UART: 115200 bodów, 8 bitów danych, brak parzystości, 1 bit stopu.
- Komendy są ograniczonymi liniami drukowalnego ASCII zakończonymi CR lub LF.
- `PING` zwraca `OK,PONG,FW=0.2.0,PROTO=1`.
- `STATUS` zwraca `OK,STATUS,READY=1,PROTO=1`.
- `SAFE` zwraca `OK,SAFE`.
- Nieznane lub niepoprawne komendy są odrzucane.

Kanał diagnostyczny jest oddzielony od binarnej ramki pomiarowej HX711 (little-endian: znacznik czasu, wartość surowa ze znakiem, CRC-8, zakończenie `0xAA 0x55`). Parser hosta i firmware nie mogą utożsamiać komend diagnostycznych z ramkami pomiarowymi.

## 3. Dalsze prace inżynierskie

1. Potwierdzić dokładne modele czujników i interfejsy elektryczne podłączone do Pico.
2. Zaimplementować ograniczoną akwizycję czujnika z jawnym stanem poprawności próbki i stanami awarii.
3. Zastosować skalibrowane jednostki i semantykę znaczników czasu wspólne z hostowym silnikiem pomiarowym.
4. Dodać testy protokołu po stronie firmware oraz testy integracyjne host–Pico.
5. Wykonać weryfikację na fizycznym sprzęcie z obciążeniami odniesienia i udokumentowanymi wynikami kalibracji.

Pomyślna kompilacja firmware nie potwierdza dokładności fizycznego czujnika ani przydatności klinicznej.
