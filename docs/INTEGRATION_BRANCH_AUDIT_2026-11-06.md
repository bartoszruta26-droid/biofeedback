# 🔍 Audyt techniczny gałęzi integration/all-work — 2026-11-06

> 🕒 Dokument audytowy / planistyczny • Wnioski i statusy odnoszą się do wskazanej daty, gałęzi i commita; nie są automatycznie bieżącym stanem main.

Data: 2026-11-06
Repozytorium: bartoszruta26-droid/biofeedback
Audytowana gałąź: integration/all-work
Commit bazowy: 6b1e84d891ef5cb13ec20edb568f05d349978422

## 1. Zakres i ograniczenia

Gałąź integration/all-work jest na moment audytu identyczna z main (0 commitów różnicy; porównanie GitHub: status identical). Oznacza to, że poniższe ustalenia dotyczą aktualnego kodu bazowego, a nie zintegrowanego zestawu zmian z pozostałych gałęzi.

Wykonano statyczny przegląd drzewa repozytorium (111 plików) oraz kluczowych plików uwierzytelniania, kryptografii, danych pacjentów, komunikacji szeregowej, CMake, testów i instalatora. Nie wykonano kompilacji, testów jednostkowych, analizy dynamicznej, testów penetracyjnych ani testów sprzętowych. Dla badanego SHA GitHub nie zwrócił statusów CI ani powiązanych uruchomień workflow; nie jest to dowód zaliczenia testów.

## 2. Podsumowanie wykonawcze

Wynik: **NIEZGODNE / NIE GOTOWE DO UŻYCIA PRODUKCYJNEGO Z DANYMI PACJENTÓW**.

Najważniejsze problemy:
- jawne hasło konta administratora w config/users.json;
- mechanizm określany jako szyfrowanie jest XOR + Base64, bez poufności kryptograficznej i integralności;
- generator kluczy używa std::rand();
- uwierzytelnianie odszyfrowuje przechowywane hasło i porównuje je zwykłym operatorem string;
- szyfrowanie danych pacjenta jest opcjonalne i domyślnie wyłączone;
- zapis plików nie jest atomowy i nie zapewnia bezpiecznego odzyskania po przerwanym zapisie;
- logowanie diagnostyczne jest domyślnie aktywne w obszarach uwierzytelniania, danych i kryptografii.

## 3. Ustalenia

### BF-SEC-001 — Krytyczne: jawne hasło administratora
Plik: config/users.json
Plik zawiera konto administratora z hasłem zapisanym wprost i encrypted=false. Wartość należy traktować jako ujawnioną. Należy natychmiast zmienić dane dostępowe, sprawdzić historię Git i klony repozytorium oraz wdrożyć bezpieczne pierwsze uruchomienie bez domyślnego hasła. Samo usunięcie wartości z bieżącego pliku nie usuwa jej z historii.

### BF-SEC-002 — Krytyczne: XOR + Base64 nie jest bezpiecznym szyfrowaniem
Pliki: src/data/Encryption.cpp, include/data/Encryption.hpp
Encryption::encrypt wykonuje XOR z kluczem, a następnie Base64. Deszyfrowanie odwraca XOR. Base64 jest kodowaniem, nie szyfrowaniem. Konstrukcja nie zapewnia odpornej poufności ani uwierzytelnienia danych i jest podatna na analizę oraz modyfikację. Nie używać do ochrony danych medycznych ani haseł. Docelowo zastosować wersjonowany format AEAD, np. AES-256-GCM przez OpenSSL EVP, z losowym nonce, tagiem uwierzytelniającym i kontrolą błędów.

### BF-SEC-003 — Wysokie: niekryptograficzny generator kluczy
Plik: src/data/Encryption.cpp
Encryption::generateKey korzysta z std::rand() modulo 93. std::rand() nie jest kryptograficznym generatorem liczb losowych; wynik może być przewidywalny i obciążony modulo. Należy użyć CSPRNG OpenSSL (np. RAND_bytes) i sprawdzać błędy generatora. Nie logować wygenerowanych znaków ani sekretów.

### BF-AUTH-004 — Krytyczne: odwracalne przechowywanie haseł i porównanie zwykłe
Pliki: src/core/Authentication.cpp, include/core/Authentication.hpp
login() odszyfrowuje zapisane hasło i porównuje storedPassword == password. Hasła powinny być przechowywane jako adaptacyjne hashe (Argon2id albo PBKDF2-HMAC-SHA-256 z parametrami wersjonowanymi i solą per konto), a weryfikacja powinna używać dedykowanej funkcji. Należy wymusić niepuste hasło i unieważniać sesję po błędach.

### BF-DATA-005 — Krytyczne: szyfrowanie danych pacjenta domyślnie wyłączone
Pliki: include/data/DataManager.hpp, src/data/DataManager.cpp
savePatientData(..., encrypt=false) ma domyślnie wyłączone szyfrowanie; DataManager startuje z encryptionEnabled=false. Dane obejmują m.in. PESEL, dane kontaktowe, adres i historię medyczną. Jeśli encrypt=true, ale klucz nie został ustawiony, kod zapisuje jawny JSON, ponieważ szyfrowanie odbywa się wyłącznie przy warunku encrypt && encryptionEnabled. Należy zastosować fail-closed: żądanie szyfrowania bez klucza ma kończyć się błędem, a polityka przechowywania danych wrażliwych nie może zależeć od przypadkowego argumentu wywołania.

### BF-DATA-006 — Wysokie: zapis plików nie jest atomowy
Plik: src/data/DataManager.cpp, DataManager::writeFile
Plik docelowy jest otwierany bezpośrednio przez std::ofstream, co zwykle obcina istniejącą zawartość. Ponowienie próby po błędzie zapisu ponownie otwiera ten sam plik i może dalej niszczyć poprzednią poprawną kopię. Po file.close() nie jest sprawdzany końcowy stan strumienia. Brakuje zapisu do pliku tymczasowego w tym samym systemie plików, flush/fsync, atomowego rename, kontroli uprawnień oraz strategii odzyskiwania. Wymagany jest test przerwania zapisu i zachowania poprzedniej wersji.

### BF-DATA-007 — Wysokie: eksport/import bez wymuszenia ochrony
Plik: src/data/DataManager.cpp
exportToJson() zapisuje przekazany tekst bez szyfrowania, a importFromJson() odczytuje plik bez weryfikacji formatu, integralności czy polityki dostępu. Trzeba prześledzić wszystkie wywołania eksportu/importu, dodać jawne granice zaufania, walidację schematu i kontrolę uprawnień. Eksport danych osobowych powinien być operacją świadomą, audytowaną i chronioną.

### BF-LOG-008 — Wysokie: diagnostyka włączona domyślnie
Pliki: src/core/Authentication.cpp, src/data/Encryption.cpp, src/data/DataManager.cpp, src/sensor/SerialCommunication.cpp
Wiele flag DEBUG jest ustawionych na 1/true. Logi obejmują ścieżki plików, długości sekretów, metadane operacji i identyfikatory. Należy wyłączyć szczegółowe logowanie w kompilacji produkcyjnej, wprowadzić centralną politykę redakcji, unikać logowania danych identyfikujących pacjentów i sekretów oraz przetestować logi pod kątem wycieków.

### BF-SERIAL-009 — Średnie/wysokie: walidacja parametrów portu nie jest ścisła
Plik: src/sensor/SerialCommunication.cpp
Nieobsługiwane wartości baud rate są po cichu zastępowane wartością domyślną; na Linuxie przełącznik mapuje tylko wybrane prędkości, a pozostałe kończą z B115200. Wynik fcntl(F_GETFL/F_SETFL) nie jest sprawdzany. Część timeoutów i ścieżek błędów wymaga ujednolicenia. Należy odrzucać nieobsługiwane prędkości, sprawdzać każdy wynik wywołań systemowych i dodać testy symulowanej awarii portu.

### BF-BUILD-010 — Średnie: niespójność deklaracji budowania/testów
Pliki: CMakeLists.txt, tests/CMakeLists.txt
Cele auth_regression i data_manager_regression są deklarowane w głównym CMakeLists, a tests/CMakeLists jest jedynie placeholderem. Wymaga to uporządkowania modelu testów, dodania add_test()/CTest i jednoznacznego warunkowania zależności GUI. W aktualnym CMake core nadal kompiluje Authentication.cpp i Encryption.cpp, więc naprawy kryptografii muszą być spójne w bibliotece core i aplikacji GUI.

### BF-INSTALL-011 — Wysokie: instalator wymusza pobieranie main i używa eval
Plik: biofeedback_installer.sh
Opcja aktualizacji wykonuje git pull origin main, co omija gałąź integracyjną i utrudnia kontrolowane wdrażanie. Skrypt używa eval dla poleceń menedżera pakietów. Należy jawnie wskazywać kanał/gałąź wydania, unikać eval i używać tablic argumentów, sprawdzać status każdego polecenia oraz zabezpieczyć operacje czyszczenia katalogów.

## 4. Zalecana kolejność napraw

1. Natychmiastowa rotacja ujawnionego hasła administratora i usunięcie domyślnego konta/hasła z procesu provisioningu; analiza historii Git.
2. Zablokowanie użycia XOR jako ochrony danych wrażliwych; zaprojektowanie formatu AES-256-GCM i wersjonowanej migracji.
3. Zastąpienie odwracalnego przechowywania haseł PBKDF2/Argon2id; migracja przy pierwszym poprawnym logowaniu lub kontrolowany reset haseł.
4. Wymuszenie fail-closed dla danych pacjenta i audyt wszystkich ścieżek zapisu/eksportu/backupów.
5. Atomowy zapis plików z zachowaniem poprzedniej wersji, kontrolą uprawnień i testami awarii.
6. Redakcja logów i wyłączenie debugowania w kompilacji produkcyjnej.
7. Testy regresyjne uwierzytelniania, szyfrowania, integralności, migracji, przerwanego zapisu, importu/eksportu i komunikacji szeregowej.
8. Uporządkowanie CI, CMake, instalatora i instrukcji wydania.

## 5. Kryteria zamknięcia audytu

- Brak sekretów i domyślnych haseł w repozytorium oraz historii, z przeprowadzoną rotacją.
- Brak XOR jako mechanizmu ochrony danych.
- Szyfrowanie AEAD z kontrolą tagu, nonce i wersji formatu.
- Hasła przechowywane wyłącznie jako adaptacyjne hashe.
- Brak jawnego zapisu danych pacjenta w ścieżkach produkcyjnych.
- Zapis odporny na przerwanie procesu i błędy I/O.
- Brak danych wrażliwych w logach.
- Zielone testy CTest/CI na wspieranych konfiguracjach; testy sprzętowe raportowane osobno.
- Przegląd zmian i jawna akceptacja migracji danych.

## 6. Status

Audyt statyczny: wykonano dla wskazanych plików.
Kompilacja: nie wykonano.
Testy jednostkowe/regresyjne: nie wykonano.
Testy dynamiczne/penetracyjne: nie wykonano.
Testy sprzętowe: nie wykonano.
Gotowość produkcyjna: **NIE**.

Ten dokument nie jest certyfikacją bezpieczeństwa ani potwierdzeniem zgodności wyrobu medycznego.
