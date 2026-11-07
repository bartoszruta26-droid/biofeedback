# Konfigurowalne szyfrowanie plików

## Tryb deweloperski

W `config/config.json`:

```json
"encryption": {
  "enabled": false,
  "algorithm": "AES-256-GCM",
  "key_derivation": "SHA-256 key normalization (32+ byte external secret)",
  "iterations": 100000
}
```

Przy `enabled: false` pliki kont administratora i użytkowników oraz dane pacjentów, wyniki ćwiczeń, plany treningowe i eksport JSON są zapisywane jako zwykły JSON/tekst. Jest to tryb deweloperski i nie powinien być używany z rzeczywistymi danymi pacjentów.

Hasła kont pozostają przechowywane jako skróty PBKDF2-HMAC-SHA-256 z solą. Wyłączenie szyfrowania plików nie wyłącza haszowania haseł i nie zapisuje haseł w postaci jawnej.

## Tryb szyfrowania

Ustaw `encryption.enabled` na `true` i dostarcz sekret przez zmienną środowiskową `BIOFEEDBACK_ENCRYPTION_KEY` (co najmniej 32 bajty losowego materiału). Sekretu nie wolno umieszczać w repozytorium ani w pliku konfiguracyjnym.

Dane plikowe są szyfrowane przez istniejący format wersjonowany `BFENC:v1:` oparty na AES-256-GCM, losowym 96-bitowym nonce i 128-bitowym tagu uwierzytelniającym. Błędny klucz, uszkodzony tag lub nieobsługiwany format powoduje błąd odczytu.

## Zmiana ustawienia

- Zmiana z `false` na `true`: nowe zapisy są szyfrowane. Istniejące pliki jawne pozostają jawne do chwili ich ponownego zapisania; przed użyciem produkcyjnym należy wykonać kontrolowaną migrację całego katalogu danych.
- Zmiana z `true` na `false`: nowe zapisy są jawne. Pliki z prefiksem `BFENC:v1:` nadal są rozpoznawane i odszyfrowywane przy odczycie, o ile dostarczono ten sam klucz. Nie usuwać klucza, dopóki wszystkie pliki zaszyfrowane nie zostaną świadomie zmigrowane.
- Brak klucza przy `enabled: true` zatrzymuje inicjalizację aplikacji.

## Ograniczenie bezpieczeństwa

Tryb `enabled: false` oznacza brak poufności danych zapisanych na dysku. Uprawnienia plików i kontrola dostępu systemu operacyjnego nadal mają znaczenie, ale nie zastępują szyfrowania. Nie jest to tryb akceptowalny dla środowiska produkcyjnego zawierającego dane osobowe lub dane dotyczące zdrowia.
