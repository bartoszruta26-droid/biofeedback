# Ocena stanu funkcjonalnego pacjenta według ICF

## Przeznaczenie

Karta pacjenta udostępnia formularz „Ocena funkcjonalna ICF”. Jest to ustrukturyzowany wywiad przesiewowo-opisowy oparty na wybranych domenach Międzynarodowej Klasyfikacji Funkcjonowania, Niepełnosprawności i Zdrowia (ICF). Nie jest to pełny, standaryzowany kwestionariusz ani kompletny ICF Core Set.

Ocena odnosi się domyślnie do typowego funkcjonowania w ostatnich 30 dniach. Należy uwzględniać perspektywę pacjenta, jego środowisko, zasoby i cele.

## Zakres domen

### Funkcje organizmu — kody b

- b130 — energia i napęd
- b134 — funkcje snu
- b152 — funkcje emocjonalne
- b280 — odczuwanie bólu
- b710 — ruchomość stawów
- b730 — siła mięśni
- b455 — tolerancja wysiłku i wydolność

### Aktywność i uczestniczenie — kody d

Dla każdej domeny rejestruje się osobno wykonanie w rzeczywistym środowisku oraz zdolność w warunkach standaryzowanych.

- d410 — zmiana podstawowej pozycji ciała
- d415 — utrzymywanie pozycji ciała
- d430 — podnoszenie i przenoszenie przedmiotów
- d450 — chodzenie
- d455 — przemieszczanie się
- d510 — mycie się
- d520 — pielęgnacja części ciała
- d540 — ubieranie się
- d550 — jedzenie
- d560 — picie
- d640 — wykonywanie prac domowych
- d850 — praca zarobkowa
- d920 — rekreacja i wypoczynek

### Czynniki środowiskowe — kody e

- e115 — produkty i technologie do użytku osobistego
- e120 — produkty i technologie ułatwiające poruszanie się
- e150 — projektowanie i dostępność budynków użyteczności publicznej
- e310 — najbliższa rodzina
- e355 — pracownicy ochrony zdrowia
- e580 — usługi, systemy i polityka ochrony zdrowia

## Kwalifikatory

Dla funkcji organizmu oraz aktywności i uczestniczenia:

- 0 — brak problemu / trudności
- 1 — problem / trudność niewielka
- 2 — umiarkowana
- 3 — znaczna
- 4 — całkowita
- 8 — nieokreślona
- 9 — nie dotyczy

Dla czynników środowiskowych:

- 0 — czynnik neutralny
- -1 do -4 — bariera o narastającym nasileniu
- +1 do +4 — ułatwiacz o narastającym nasileniu
- 8 — nieokreślony
- 9 — nie dotyczy

W bazie zapisywane są wartości kwalifikatorów, a nie indeksy pozycji listy rozwijanej. Brak oceny nie jest automatycznie zamieniany na brak problemu.

## Trwałość i prywatność

Ocena jest zapisywana jako pole `icf_assessment` w kanonicznym rekordzie pacjenta w kolekcji `patients`. Zapis korzysta z wersjonowania i kontroli optymistycznej repozytorium. Zwykła edycja danych demograficznych zachowuje istniejącą ocenę. Ocena jest ponownie ładowana przy otwieraniu kartoteki.

## Ograniczenia kliniczne

Formularz stanowi pomoc w zebraniu informacji i planowaniu opieki. Nie generuje automatycznie rozpoznania, stopnia niepełnosprawności, kwalifikacji do świadczeń ani decyzji terapeutycznych. Wyniki wymagają interpretacji klinicznej w kontekście badania, wywiadu i dokumentacji. Zakres domen należy dobrać do problemu zdrowotnego; w razie potrzeby należy użyć właściwego, zwalidowanego narzędzia lub odpowiedniego ICF Core Set.
