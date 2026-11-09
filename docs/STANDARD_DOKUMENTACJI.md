# 📐 Standard dokumentacji projektu Biofeedback

> 📘 Zasady redakcyjne • Obowiązują dla plików Markdown w katalogu `docs/`.

## 1. Język

- Podstawowym językiem dokumentacji jest poprawna polszczyzna.
- Nazwy bibliotek, klas, funkcji, zmiennych, komend, endpointów, pól JSON, komunikatów protokołu i identyfikatorów pozostają w oryginalnej postaci.
- Przy pierwszym użyciu skrótu podaj polskie rozwinięcie, jeśli jest potrzebne do zrozumienia tekstu.
- Nie tłumacz nazw symboli kodu ani nie zmieniaj ich wielkości liter.
- Angielski pozostaje dopuszczalny w cytowanych komunikatach, kodzie, nazwach standardów i dokumentach źródłowych.

## 2. Układ dokumentu

Zalecany układ:
1. Tytuł z krótką ikoną tematyczną.
2. Jednolinijkowa informacja o rodzaju dokumentu i języku.
3. Cel i zakres.
4. Wymagania, decyzje lub procedura.
5. Stan implementacji i ograniczenia.
6. Testy i kryteria odbioru.
7. Odnośniki do dokumentów powiązanych.

Nie dodawaj pustych sekcji tylko po to, aby wypełnić szablon. Instrukcja instalacji może zaczynać się od wymagań, a audyt od zakresu i ograniczeń.

## 3. Nagłówki i formatowanie

- Nagłówek główny: jeden `#` na plik.
- Sekcje: `##`; podsekcje: `###`; niższych poziomów używaj oszczędnie.
- Nagłówki pisz po polsku i formułuj rzeczownikowo, np. „Wymagania sprzętowe”, „Stan implementacji”, „Procedura kalibracji”.
- Używaj tabel dla porównań i macierzy, list dla sekwencji i wymagań, bloków kodu dla poleceń.
- Nie używaj ozdobników w każdej linii. Ikona w tytule i kilka oznaczeń w sekcjach wystarczą.
- Każdy blok kodu musi mieć poprawny identyfikator języka, np. `bash`, `json`, `text`, `cpp`.

## 4. Oznaczanie statusu i pewności

Stosuj jednoznaczne sformułowania:
- **Wdrożone** — potwierdzone w kodzie wskazanej gałęzi.
- **Przetestowane** — podaj nazwę testu lub workflow, wynik i SHA.
- **Częściowo wdrożone** — opisz dokładnie działający zakres i brakujące elementy.
- **Planowane** — rozwiązanie docelowe, którego nie należy przedstawiać jako gotowego.
- **Niezweryfikowane sprzętowo** — brak testu na rzeczywistym urządzeniu.
- **Historyczne** — wynik dotyczy konkretnej daty, gałęzi i commita.

Nie deklaruj „działa”, „bezpieczne”, „gotowe” ani „zaliczone” bez wskazania podstawy w kodzie, teście lub procedurze odbiorczej. Kompilacja nie jest równoznaczna z testem sprzętowym ani walidacją kliniczną.

## 5. Audyty historyczne

Każdy audyt powinien zawierać:
- datę;
- gałąź;
- bazowy SHA;
- zakres i ograniczenia;
- wykorzystane testy i wyniki;
- ustalenia z priorytetami;
- kryteria zamknięcia.

Po zmianie kodu nie nadpisuj wniosków historycznych tak, jakby dotyczyły nowej wersji. Utwórz nowy audyt albo dodaj wyraźnie oznaczoną aktualizację z nowym SHA.

## 6. Linki i odnośniki

- Używaj względnych linków do plików repozytorium.
- Sprawdzaj, czy wskazany plik istnieje i czy nazwa odpowiada rzeczywistej ścieżce.
- Linki do zewnętrznych standardów i bibliotek prowadź do dokumentacji pierwotnej, jeśli jest dostępna.
- Po zmianie nazwy lub przeniesieniu pliku popraw wszystkie odnośniki, w tym indeks `docs/README.md` i główny `README.md`.

## 7. Kod, polecenia i bezpieczeństwo

- Polecenia muszą być możliwe do odtworzenia i nie mogą zawierać wymyślonych nazw celów lub plików.
- Sekrety, hasła, tokeny, klucze szyfrujące i dane osobowe zastępuj bezpiecznymi symbolami zastępczymi.
- Nie umieszczaj rzeczywistych danych pacjentów w przykładach, logach, zrzutach ani testach dokumentacyjnych.
- W instrukcjach operacyjnych wyjaśniaj skutki poleceń destrukcyjnych i wymagane kopie zapasowe.

## 8. Kontrola przed scaleniem

- [ ] Dokument jest napisany po polsku, z zachowaniem oryginalnych nazw technicznych.
- [ ] Tytuł i nagłówki mają spójną hierarchię.
- [ ] Linki względne prowadzą do istniejących plików.
- [ ] Polecenia, ścieżki i identyfikatory są zgodne z repozytorium.
- [ ] Stan wdrożenia jest oddzielony od planu i audytu historycznego.
- [ ] Wyniki testów mają wskazany zakres i SHA.
- [ ] Nie ma sekretów, danych osobowych ani niepotwierdzonych deklaracji.
