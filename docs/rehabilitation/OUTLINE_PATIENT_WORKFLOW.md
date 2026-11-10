# Outline — indywidualny proces rehabilitacji pacjenta

## Cel i zakres

Outline ma prowadzić kompletny, audytowalny proces rehabilitacji: od kwalifikacji i pomiaru bazowego, przez planowanie celów i realizację sesji, po ocenę wyniku. Konspekt pozostaje szablonem klinicznym; pacjent otrzymuje odrębny, wersjonowany plan indywidualny. Dane sesji nie mogą modyfikować źródłowego konspektu.

Zakres pierwszej implementacji:
1. Indywidualna karta pacjenta i powiązanie jej z planem rehabilitacji.
2. Cele terapeutyczne z wartością początkową, docelową, terminem i metodą pomiaru.
3. Rejestracja bólu przed i po każdym ćwiczeniu oraz podsumowanie reakcji po sesji.
4. Rejestrowanie dawki: serie, powtórzenia, czas, obciążenie, RPE, jakość wykonania i pomiar biofeedbacku.
5. Reguły oceny gotowości do progresji oraz jawna akceptacja terapeuty.
6. Raport końcowy porównujący wartości bazowe i aktualne.

Poza zakresem pierwszego etapu: samodzielne rozpoznawanie chorób, automatyczna zmiana ograniczeń pooperacyjnych, automatyczne ordynowanie ćwiczeń i interpretowanie pomiarów jako diagnozy.

## Model danych

### PatientReference

Dane osobowe pozostają w istniejącym module pacjentów. Sesja rehabilitacyjna przechowuje wyłącznie wewnętrzny identyfikator pacjenta (UUID), bez PESEL, nazwiska, adresu i innych danych bezpośrednich. Widok może pobrać nazwę z modułu pacjentów po autoryzowanym powiązaniu. Eksport raportu wymaga świadomego wyboru wersji identyfikowanej albo zanonimizowanej.

Pola powiązania:
- patient_id: UUID wymagany;
- created_at, updated_at: UTC ISO-8601;
- record_version: dodatnia liczba całkowita;
- consent_status i consent_recorded_at: stan zgody oraz data, jeśli ma zastosowanie;
- clinician_id: identyfikator operatora/terapeuty, jeśli system obsługuje konta.

### RehabilitationEpisode

Jeden epizod odpowiada jednemu problemowi funkcjonalnemu i może obejmować wiele sesji.

Pola:
- id, patient_id, outline_id, outline_version;
- region, side, working_diagnosis, diagnosis_status;
- onset_date, episode_start_date, planned_review_date, episode_end_date;
- care_path: conservative, post_meniscectomy, post_repair, other;
- precautions: jawna lista ograniczeń, źródło ograniczenia i data weryfikacji;
- red_flag_screen: wynik, data, osoba wykonująca i komentarz;
- baseline_assessment_id, current_status;
- status: draft, active, paused, completed, archived.

Ograniczenia pooperacyjne są polami epizodu, a nie parametrem możliwym do nadpisania przez algorytm progresji. Zmiana wymaga działania uprawnionego terapeuty i zapisania uzasadnienia.

### PatientAssessment

Ocena jest niezmiennym zapisem wykonanym w określonym czasie. Korekta tworzy nową wersję/ocenę, nie nadpisuje historii.

Pola wspólne:
- id, episode_id, assessed_at, assessor_id;
- pain_rest_nrs, pain_activity_nrs (0–10; dopuszczalne null);
- patient_reported_outcome: instrument, version, raw_score, normalized_score;
- range_of_motion: joint, movement, value, unit, method;
- strength: movement, value, unit, device, protocol, calibration_id;
- swelling_or_edema: method, value, unit;
- functional_tests: test_id, protocol, result, unit, side;
- notes, measurement_quality, missing_data_reason.

Każdy pomiar siły musi zawierać jednostkę, stronę, pozycję, metodę, identyfikator urządzenia i status kalibracji. Wartości bez poprawnej kalibracji nie mogą być przedstawiane jako wiarygodna siła w N/kg. Wynik RAW można zachować jako osobny kanał techniczny.

### TherapeuticGoal

Cel ma być mierzalny, terminowy i powiązany z funkcją pacjenta.

Pola:
- id, episode_id, title, functional_context;
- metric_id, baseline_value, target_value, current_value, unit;
- target_date, priority, status;
- measurement_method, reassessment_interval_days;
- patient_agreed: boolean, agreed_at;
- achieved_at, clinician_comment.

Cele mogą dotyczyć bólu, zakresu ruchu, siły, tolerancji obciążenia, testu funkcjonalnego albo konkretnej aktywności życia codziennego/pracy/sportu. Nie należy porównywać wartości pochodzących z różnych metod lub jednostek bez jawnej normalizacji.

### ExercisePrescription

Indywidualna recepta ćwiczenia jest kopią parametrów z szablonu z możliwością klinicznej indywidualizacji.

Pola:
- id, episode_id, source_exercise_id, exercise_name;
- order_index, phase, permitted_rom, permitted_load;
- sets_target, reps_target, hold_seconds, rest_seconds;
- resistance_value, resistance_unit, tempo, rpe_target;
- pain_policy_id, progression_rule_id;
- assigned_at, assigned_by, active, stop_reason.

### ExerciseObservation

Jeden rekord dotyczy jednego wykonania/serii ćwiczenia.

Pola:
- id, session_id, prescription_id, exercise_id, sequence_index;
- pain_before_nrs, pain_after_nrs, pain_peak_nrs;
- completed_sets, completed_repetitions, duration_seconds;
- resistance_value, resistance_unit, rpe, perceived_difficulty;
- movement_quality: acceptable, compensatory, unsafe, not_assessed;
- biofeedback: metric, value, unit, sample_count, quality, calibration_id;
- symptom_response_24h: pending, returned_to_baseline, worse, improved, unknown;
- skipped, skip_reason, therapist_note, recorded_at.

Skale bólu są jawnie opisane: NRS 0 oznacza brak bólu, 10 — najgorszy wyobrażalny ból. Brak odpowiedzi jest null, a nie zero. Ból przed i po ćwiczeniu to osobne pomiary i nie wolno ich scalać w jedną średnią.

### RehabilitationSession

Pola:
- id, episode_id, patient_id, started_at, ended_at;
- therapist_id, session_status, session_notes;
- pre_session_pain_nrs, post_session_pain_nrs;
- adverse_event, adverse_event_description;
- exercise_observation_ids;
- progression_review_status, report_snapshot_id.

## Kryteria progresji

Reguły są wersjonowanymi, konfigurowalnymi propozycjami klinicznymi. Każda reguła określa warunki, okno obserwacji, minimalną liczbę poprawnych sesji, kryteria wykluczające, proponowaną zmianę i wymagany poziom zatwierdzenia.

Przykładowy zestaw warunków do oceny gotowości:
- pacjent wykonał zaplanowaną dawkę w wymaganej jakości ruchu;
- brak niepożądanej kompensacji i zdarzenia niepożądanego;
- reakcja bólowo-objawowa mieści się w indywidualnym limicie określonym przez terapeutę;
- objawy wracają do ustalonego poziomu w uzgodnionym oknie (np. do 24 godzin), jeśli takie kryterium jest właściwe dla danego przypadku;
- nie występuje wzrost wysięku/obrzęku ani pogorszenie funkcji;
- pomiar biofeedbacku jest poprawny, skalibrowany i ma wystarczającą jakość, jeżeli jest używany jako kryterium.

Wynik silnika reguł:
- NOT_EVALUABLE — brakuje danych, kalibracji lub obserwacji kontrolnej;
- HOLD — utrzymaj dawkę i ponownie oceń;
- REVIEW — wymagana ocena terapeuty;
- SUGGEST_PROGRESS — warunki spełnione; przedstaw propozycję;
- REGRESSION_REVIEW — wystąpiło pogorszenie; zaproponuj przegląd obciążenia.

Silnik nie zmienia samodzielnie ćwiczeń, zakresu ruchu, obciążania, etapu pooperacyjnego ani ograniczeń operatora. Propozycja progresji wymaga akceptacji terapeuty. Każda decyzja zapisuje: wersję reguły, dane wejściowe, wynik, autora zatwierdzenia, datę i uzasadnienie.

Nie stosuj jednego uniwersalnego progu bólu dla wszystkich rozpoznań. Progi i tolerancja obciążenia są definiowane dla danego epizodu oraz etapu leczenia. Alarmy bezpieczeństwa mają pierwszeństwo przed oceną progresji.

## Przepływ sesji

1. Terapeuta wybiera pacjenta i aktywny epizod.
2. Aplikacja pokazuje ostrzeżenia, ograniczenia, cele i datę ostatniej oceny.
3. Przed sesją zapisuje się ból spoczynkowy i/lub ból przy aktywności.
4. Dla każdego ćwiczenia rejestruje się ból przed wykonaniem.
5. Podczas ćwiczenia zapisywane są dawka, jakość ruchu, RPE i poprawne pomiary biofeedbacku.
6. Po ćwiczeniu zapisywany jest ból i komentarz pacjenta.
7. Po sesji zapisywany jest ból końcowy, tolerancja sesji i zdarzenia niepożądane.
8. Po wymaganym czasie uzupełnia się odpowiedź 24-godzinną.
9. Silnik reguł ocenia kompletne dane i przedstawia rekomendację.
10. Terapeuta akceptuje, odrzuca lub modyfikuje rekomendację z uzasadnieniem.

Sesję można przerwać i wznowić. Stan niekompletny nie może zostać oznaczony jako ukończony. Brak danych nie jest interpretowany jako wynik prawidłowy.

## Raport końcowy

Raport powinien zawierać:
- identyfikację epizodu i okres rehabilitacji;
- rozpoznanie robocze, stronę, ścieżkę leczenia i ograniczenia;
- cele uzgodnione z pacjentem oraz status każdego celu;
- tabelę pomiarów bazowych, pośrednich i końcowych;
- różnicę bezwzględną, procentową tylko gdy matematycznie i klinicznie ma sens, jednostkę i metodę;
- wyniki kwestionariuszy z nazwą i wersją narzędzia;
- wykres trendu bólu przed/po ćwiczeniach;
- wykres siły/zakresu ruchu wyłącznie dla porównywalnych pomiarów;
- frekwencję, wykonane dawki, tolerancję i przerwane ćwiczenia;
- historię propozycji progresji oraz decyzji terapeuty;
- zdarzenia niepożądane, ograniczenia i brakujące dane;
- wnioski, dalszy plan i podpis/identyfikator osoby zatwierdzającej.

Porównanie musi zestawiać tę samą stronę, pozycję, metodę, urządzenie/protokół i jednostkę. Raport ma oznaczać zmianę jako opisową, jeżeli nie ma podstaw do interpretacji klinicznej. Nie wolno automatycznie orzekać o skuteczności leczenia ani powrocie do sportu wyłącznie na podstawie jednego pomiaru.

## Interfejs Outline

### Panel lewy — praca z pacjentem
- multimedia i instrukcja aktualnego ćwiczenia;
- wykres biofeedbacku w czasie rzeczywistym;
- dziennik bólu przed/po ćwiczeniu;
- postęp sesji i jakość wykonania;
- podgląd protokołu klinicznego;
- podsumowanie reakcji po sesji.

### Panel prawy — sterowanie i dane
- wybór pacjenta i epizodu;
- karta kliniczna i ograniczenia;
- cele terapeutyczne;
- indywidualna dawka ćwiczeń;
- ustawienia sesji i reguł progresji;
- akceptacja/odrzucenie rekomendacji;
- generowanie raportu.

### Widoki pomocnicze
- Karta pacjenta: dane identyfikacyjne tylko z modułu PatientTab, historia epizodów i zgody.
- Ocena: pomiary bazowe/aktualne i kwestionariusze.
- Cele: lista celów z trendami i terminami.
- Historia: sesje, obserwacje, decyzje i raporty.
- Raport: podgląd, eksport PDF/CSV i wybór anonimizacji.

## Prywatność, bezpieczeństwo i audyt

- Dane medyczne i identyfikacyjne są danymi szczególnej kategorii. Dostęp wymaga kontroli uprawnień i celu przetwarzania.
- Nie umieszczaj PESEL, nazwiska, daty urodzenia ani notatek klinicznych w logach technicznych, nazwach plików, identyfikatorach zdarzeń ani telemetrii.
- Zastosuj szyfrowanie danych w spoczynku i w kopiach zapasowych, kontrolę dostępu, blokadę sesji, kopie zapasowe i test odtwarzania.
- Każdy odczyt/zmiana/eksport danych medycznych powinien mieć ślad audytowy adekwatny do architektury i wymagań prawnych.
- Zastosuj wersjonowanie rekordów i ochronę przed nadpisaniem równoległych zmian.
- Raporty eksportowane domyślnie powinny być minimalizowane; użytkownik wybiera, czy potrzebne są dane identyfikujące.
- Nie używaj rzeczywistych danych pacjentów w testach automatycznych. Stosuj syntetyczne dane.
- Ustal retencję, procedurę usuwania/anonimizacji i kontrolę eksportu przed wdrożeniem produkcyjnym.

## Plan wdrożenia

### Etap 1 — model domenowy i trwałość
- dodać wersjonowane rekordy epizodów, ocen, celów, recept, sesji, obserwacji i decyzji progresji;
- wykorzystać istniejący JsonResourceRepository tylko po rozszerzeniu walidacji schematów i polityki prywatności;
- dodać testy walidacji, konfliktów wersji, szyfrowania, uprawnień i izolacji pacjentów.

### Etap 2 — karta i ocena bazowa
- powiązać Outline z istniejącym PatientTab przez UUID;
- utworzyć epizod i ocenę bazową;
- dodać edytor celów i ograniczeń.

### Etap 3 — rejestracja sesji
- formularze bólu przed/po;
- rejestracja dawki i jakości;
- wiązanie pomiarów z sesją i ćwiczeniem;
- obsługa przerwania/wznowienia i odpowiedzi 24-godzinnej.

### Etap 4 — reguły progresji
- deterministyczny silnik reguł niezależny od GUI;
- wynik wraz z uzasadnieniem i brakującymi danymi;
- zatwierdzanie przez terapeutę i dziennik decyzji.

### Etap 5 — raport i walidacja
- raport porównawczy z wykresami;
- eksport z anonimizacją;
- testy regresyjne i testy scenariuszy klinicznych na danych syntetycznych;
- przegląd prywatności, bezpieczeństwa i użyteczności przed wdrożeniem.

## Kryteria akceptacji

- Sesja jest zawsze powiązana z pacjentem, epizodem i konkretną wersją konspektu.
- Ból przed i po ćwiczeniu zapisuje się osobno; null nie jest zamieniany na zero.
- Każdy pomiar ma jednostkę, metodę, stronę i status jakości/kalibracji.
- Cele zawierają wartość bazową, docelową, termin i metodę oceny.
- Silnik progresji zwraca wynik deterministyczny i uzasadnienie; nie modyfikuje planu bez zatwierdzenia.
- Ograniczenia pooperacyjne blokują niezgodne propozycje.
- Raport pokazuje porównywalne pomiary, ich różnicę, metodę i brakujące dane.
- Testy potwierdzają izolację danych pacjentów, ochronę przed konfliktami wersji i brak danych osobowych w logach.
- Testy nie używają rzeczywistych danych pacjentów.

## Status implementacji w repozytorium

Pierwszy element wykonawczy został dodany:
- `include/rehabilitation/ProgressionRules.hpp` — niezależny od GUI, deterministyczny silnik oceny kryteriów;
- `tests/rehabilitation_progression_rules_regression.cpp` — testy braku danych, zakresów NRS, nieprawidłowych wartości liczbowych, jakości ruchu, dawki, zdarzeń niepożądanych, czerwonych flag, konfliktu ograniczeń pooperacyjnych i zatwierdzania przez terapeutę;
- test zarejestrowany w CMake jako `rehabilitation_progression_rules_regression`.

Silnik zwraca wyłącznie rekomendację. Nie zapisuje danych, nie zmienia recepty ćwiczenia i nie wykonuje progresji. Zatwierdzenie wymaga uwierzytelnionego terapeuty oraz jawnej akceptacji; warstwa aplikacyjna musi dopisać do tego trwały ślad audytowy. Interfejs, trwały model epizodu, rejestracja bólu i raport porównawczy pozostają kolejnymi etapami.
