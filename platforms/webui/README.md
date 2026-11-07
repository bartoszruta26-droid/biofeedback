# Biofeedback WebUI

Responsywny interfejs webowy dla systemu Biofeedback. Wersja 0.1 zawiera
nawigację po modułach, pulpit statusowy, tryb jasny/ciemny i widoki
oczekujące na wspólne usługi aplikacyjne.

## Uruchomienie

Z katalogu głównego repozytorium:

    python3 -m http.server 8080 --directory platforms/webui

Następnie otwórz http://localhost:8080.

## Ważne ograniczenie wersji 0.1

To warstwa prezentacji. Nie łączy się jeszcze z API, portem szeregowym ani
MeasurementEngine. Nie generuje fikcyjnych odczytów, kart pacjentów ani
wyników terapii. Statusy jawnie wskazują brak połączenia.

Kontrakt integracyjny opisano w API_CONTRACT.md.
