# 🔐 Ustalenia audytu bezpieczeństwa interfejsów

> 🕒 Dokument audytowy / planistyczny • Wnioski dotyczą wskazanego zakresu i momentu przeglądu; przed użyciem jako kryterium odbioru zweryfikuj bieżący kod.

## BF-SEC-001 — dane osobowe w logach

Priorytet: P0.  
Miejsce rozpoznania: `src/core/Application.cpp`, obsługa sygnału `PatientTab::patientAdded`.  
Problem: log zawiera imię, nazwisko i PESEL pacjenta.  
Ryzyko: niepotrzebne ujawnienie identyfikatorów w plikach logów, konsoli, artefaktach diagnostycznych i kopiach.  
Naprawa: zastąpić danymi technicznymi (np. nieodwracalny identyfikator rekordu lub pseudonimowy identyfikator zdarzenia); sprawdzić wszystkie logi i komunikaty debug.  
Test: test logowania syntetycznego pacjenta potwierdza brak PESEL, imienia i nazwiska w logach.

## BF-SEC-002 — API statusu nie jest API aplikacyjnym

Priorytet: P0 przed ekspozycją poza loopback.  
Miejsce: `src/api/HttpApiServer.cpp`, `src/api/main.cpp`.  
Stan: tylko health/status; operacje urządzeń i repozytorium są wyłączone. Serwer wiąże się z loopback.  
Ryzyko: nie wolno rozszerzać lub wystawiać API do LAN bez kontroli dostępu, TLS i walidacji.  
Naprawa: uwierzytelnianie, autoryzacja RBAC, zarządzanie sesją/tokenami, ochrona przed CSRF jeśli używane cookies, limity i audyt.

## BF-SEC-003 — bezpieczeństwo sterowania urządzeniem

Priorytet: P0 przed wdrożeniem komend zdalnych.  
Wymagania: jawna lista dozwolonych komend, zakresy parametrów, kontrola stanu urządzenia, uprawnienia, blokada współbieżnych komend, timeout, bezpieczne zatrzymanie, audyt i testy awarii.

## BF-SEC-004 — dane pacjentów i eksport

Priorytet: P1.  
Wymagania: minimalizacja danych, kontrola dostępu do eksportu, bezpieczne pliki tymczasowe, atomowy zapis, testy odtworzenia i brak danych w logach błędów.

## Zasada

Ten dokument jest przeglądem kodu, a nie certyfikacją ani opinią prawną. Wymagania regulacyjne i ochrony danych należy ocenić osobno dla docelowego przeznaczenia systemu.
