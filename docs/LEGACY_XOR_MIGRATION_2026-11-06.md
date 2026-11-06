# Kontrolowana migracja danych legacy XOR → AES-256-GCM

## Cel i granice

To narzędzie jest jednorazowym, ręcznie uruchamianym migratorem poza ścieżką wykonania aplikacji. Nie dodaje obsługi XOR do klasy `Encryption::decrypt`, `DataManager` ani logowania użytkownika. Format wejściowy odpowiada historycznej implementacji: `Base64(data[i] XOR key[i % key.size()])`.

## Interfejs

Program `biofeedback_legacy_migrate <źródło> <wynik_BFENC> <kopia_legacy>` wymaga:
- `BIOFEEDBACK_LEGACY_XOR_KEY` — dokładny stary klucz w postaci bajtów tekstu;
- `BIOFEEDBACK_ENCRYPTION_KEY` — nowy tajny materiał kluczowy, minimum 32 bajty.

Klucze są pobierane ze środowiska, nie z argumentów procesu. Nie należy wpisywać ich do powłoki w sposób pozostawiający historię poleceń. W systemie docelowym należy użyć bezpiecznego menedżera sekretów lub kontrolowanego mechanizmu uruchomieniowego.

## Zasady bezpieczeństwa

1. Źródło jest otwierane tylko do odczytu i musi być zwykłym plikiem; dowiązania symboliczne są odrzucane.
2. Limit wejścia wynosi 100 MiB.
3. Base64 jest walidowane rygorystycznie; niepoprawne dane przerywają migrację.
4. Plik źródłowy nie jest nadpisywany ani usuwany.
5. Kopia legacy i plik wynikowy muszą jeszcze nie istnieć. Nie ma trybu nadpisywania.
6. Kopia oryginalnego szyfrogramu jest tworzona z prawami 0600, synchronizowana i porównywana bajt w bajt.
7. Dane są odszyfrowywane wyłącznie wewnątrz migratora, szyfrowane przez istniejący AES-256-GCM, a wynik jest weryfikowany przez ponowne odszyfrowanie.
8. Klucz i bufor tekstu jawnego nie są wypisywane do logów; bufor tekstu jawnego jest czyszczony po użyciu.
9. Migracja nie usuwa ani nie zmienia źródła. Niepoprawne wejście, brak klucza, istniejące pliki docelowe lub błąd zapisu powodują odmowę.

## Procedura operacyjna

1. Zatrzymać aplikację i wykonać niezależną kopię całego katalogu danych.
2. Pracować na dodatkowej kopii roboczej, nigdy na jedynym egzemplarzu danych.
3. Ustalić właściwy historyczny klucz XOR na podstawie dokumentacji i konfiguracji konkretnej instalacji. Migrator nie odzyskuje ani nie zgaduje klucza.
4. Ustawić klucze przez bezpieczny mechanizm sekretów.
5. Uruchomić migrator dla pojedynczego pliku, wskazując nową, nieistniejącą ścieżkę wyniku i kopii.
6. Zweryfikować raport, zachowanie źródła, kopię, tag GCM i zgodność danych po odszyfrowaniu.
7. Przeprowadzić walidację semantyczną JSON/rekordów przez aplikację lub osobny walidator domenowy przed przełączeniem danych.
8. Zachować źródło i kopię legacy do czasu zatwierdzenia migracji i testu odtworzenia.

## Ograniczenia

Migrator nie rozpoznaje automatycznie schematów JSON ani nie potwierdza poprawności domenowej dokumentacji pacjenta. Nie ustala, czy wybrany stary klucz jest właściwy na podstawie tagu uwierzytelniającego — XOR/Base64 nie zapewnia uwierzytelnienia. Weryfikacja kryptograficzna AES-GCM potwierdza nowy plik względem nowego klucza, ale poprawność starego tekstu jawnego wymaga walidacji semantycznej i porównania z wiarygodnym źródłem.

Nie uruchamiać na rzeczywistych danych pacjentów przed niezależnym przeglądem kodu, próbą odtworzenia kopii i walidacją na kopii rzeczywistych danych.
