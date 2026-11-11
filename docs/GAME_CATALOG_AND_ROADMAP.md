# Katalog gier i plan rozwoju — sterowanie wyłącznie tensometrem

**Repozytorium:** `bartoszruta26-droid/biofeedback`  
**Podstawa audytu:** `main` — `d3b9afd8635f6e9fe5e8f2a015763abb19742007`  
**Priorytet rozwoju:** rozrywka i grywalność  
**Status dokumentu:** plan, specyfikacja oraz opis stanu wdrożenia gier już zintegrowanych z `main`; pozycje bez statusu implementacji pozostają planem rozwoju

## 1. Bezwzględna zasada sterowania

Jedynym urządzeniem wejściowym sterującym rozgrywką jest **jeden tensometr odczytywany przez HX711**. Gra otrzymuje jeden strumień skalibrowanej wartości siły. Nie wolno uzależniać mechaniki gry od klawiatury, myszy, ekranu dotykowego, gamepada, drugiego tensometru, akcelerometru, kamery ani innego czujnika.

Dotyczy to wszystkich czynności wykonywanych w trakcie gry: ruchu, celowania, skoku, strzału, przyspieszenia, użycia premii, zatwierdzania wyboru, pauzy, wznowienia i ponowienia rozgrywki. Elementy interfejsu poza rozgrywką mogą służyć do konfiguracji urządzenia i wyboru gry, ale nie mogą być ukrytym drugim kontrolerem gry. Docelowo również menu gry powinno być obsługiwane gestami tensometru.

### 1.1. Wspólny model sygnału

Warstwa wejściowa powinna dostarczać każdej grze ten sam, niezależny od sprzętu model:

- `forceN` — skalibrowana siła w niutonach;
- `normalizedForce` — wartość 0–1 wyznaczona względem indywidualnego zakresu kalibracji;
- `sampleTimestamp` — czas próbki;
- `sampleValid` — informacja o poprawności próbki;
- `signalFresh` — informacja, czy próbki napływają w dopuszczalnym czasie;
- `risingEdge` / `fallingEdge` — wykryte rozpoczęcie i zakończenie nacisku;
- `holdDuration` — czas nieprzerwanego utrzymywania nacisku;
- `releaseDuration` — czas odpoczynku od ostatniego nacisku.

Wartość znormalizowana nie oznacza jednakowej siły dla wszystkich użytkowników. Zakres powinien wynikać z kalibracji i limitów przypisanych do konkretnej sesji. Nie wolno automatycznie zwiększać maksymalnego obciążenia tylko po to, by podnieść poziom trudności.

### 1.2. Słownik gestów jednego tensometru

Gesty są interpretowane z tego samego przebiegu siły; nie są dodatkowymi urządzeniami wejściowymi.

| Gest | Definicja funkcjonalna | Typowe zastosowanie |
|---|---|---|
| Spoczynek | siła w indywidualnym paśmie spoczynkowym | pozycja neutralna, oczekiwanie |
| Płynna zmiana siły | bieżąca wartość steruje parametrem ciągłym | pozycja, wysokość, kąt, prędkość |
| Krótki impuls | przekroczenie progu, a następnie zwolnienie | skok, strzał, zatwierdzenie |
| Przytrzymanie | utrzymanie siły ponad progiem przez określony czas | ładowanie, napinanie, lot |
| Podwójny impuls | dwa krótkie impulsy w oknie czasowym | dopalacz, umiejętność specjalna |
| Długi gest | kontrolowane przytrzymanie z potwierdzeniem zwolnieniem | pauza, wznowienie, ponowienie |
| Gest wyboru trybu | dłuższy gest przełącza aktywny parametr; aktualna siła reguluje ten parametr | gry wymagające kolejno kąta i mocy |

Progi, czasy, histereza i filtry muszą być konfigurowalne oraz testowane. Nie należy zakładać, że identyczny próg siły będzie właściwy dla każdego użytkownika.

### 1.3. Reguły nadrzędne

1. **Brak sygnału nie jest zerem.** Utrata połączenia, timeout, nieaktualna próbka, błąd CRC, NaN lub nieskończoność powodują zatrzymanie sterowania, a nie ruch postaci.
2. **Fail-closed.** Po utracie świeżości sygnału gra przechodzi do bezpiecznego stanu: zatrzymuje ruch zależny od siły, anuluje ładowanie i blokuje akcje wymagające nowej próbki.
3. **Kalibracja przed grą.** Ustalenie spoczynku, kierunku narastania, zakresu roboczego i limitu siły jest osobnym etapem. Kalibracja nie może wymagać przekraczania bezpiecznego limitu.
4. **Jeden strumień danych.** Gry nie otwierają portu szeregowego ani nie uruchamiają własnego odczytu HX711. Otrzymują próbki przez wspólny potok pomiarowy aplikacji.
5. **Jeden stopień swobody.** W każdej chwili bieżąca siła reguluje jeden jawnie wskazany parametr. Gdy gra potrzebuje kilku parametrów, używa sekwencyjnego wyboru trybu lub gestów czasowych.
6. **Brak sterowania awaryjnego klawiaturą.** Testy gry muszą symulować próbki tensometru. Klawiatura może być używana wyłącznie przez programistyczne narzędzia testowe, nie przez użytkownika w gotowej grze.
7. **Brak wymagań klinicznych w grach rozrywkowych.** Wynik gry jest wynikiem gry, a nie diagnozą, oceną sprawności ani dowodem skuteczności terapii.

## 2. Stan obecnego repozytorium

### 2.1. Architektura

W aktualnym `main` istnieje wspólny `GameEngine`:

- `include/games/GameEngine.hpp`
- `src/games/GameEngine.cpp`
- `include/games/GameFactory.hpp`

Silnik udostępnia cykl życia gry, pętlę aktualizacji, renderowanie, wynik oraz `handleSensorInput(double)`. Pętla gry żąda odświeżania co 11 ms (docelowo około 90 Hz) i wykonuje symulację ze stałym krokiem 1/90 s. Widgety są nieprzezroczyste, pomijają systemowe czyszczenie tła, antyaliasing i zbędne gradienty pełnoekranowe. Dane napływające z wątku odczytu są łączone do najnowszej próbki i przekazywane do wątku GUI; obsługiwane są wartości niefinitywne oraz unieważnianie opóźnionych wywołań po zatrzymaniu lub resecie.

Wspólny potok pomiarowy przekazuje skalibrowaną siłę w niutonach. Test `tests/game_sensor_ownership_regression.py` pilnuje, aby gry nie tworzyły własnych strumieni szeregowych i korzystały z centralnego źródła pomiaru.

### 2.2. Istniejące gry

W katalogach `src/games` i `include/games` znajduje się trzynaście gier:

1. Sinus Force Trainer — `SinGame.cpp`
2. Tan Force Trainer — `TanGame.cpp`
3. Pong Force Trainer — `PongGame.cpp`
4. Arkanoid — `ArkanoidGame.cpp`
5. Flappy Bird — `FlappyBirdGame.cpp`
6. Mario Run — `MarioGame.cpp`
7. Car Race — `CarRaceGame.cpp`
8. Archer Force Trainer — `ArcherGame.cpp`
9. Galaga — `GalagaGame.cpp`
10. Scorched Earth — `ScorchedEarthGame.cpp`
11. Precision Crane — `PrecisionCraneGame.cpp`
12. Rhythm Force — `RhythmForceGame.cpp`
13. Force Maze — `ForceMazeGame.cpp`

Aktualny opis zachowania znajduje się w `docs/GAME_CONTROLS.md`. W gałęzi `main` usunięto obsługę zdarzeń klawiatury i myszy z klas gier. Sterowanie kierunkowe i pozycjonowanie odbywa się tensometrem, a działania specjalne są realizowane gestami siły lub automatycznie przez zegar gry. Reguła jest dodatkowo sprawdzana testem statycznym, aby późniejsze zmiany nie wprowadziły ponownie drugiego kontrolera.

### 2.3. Standard wydajności i skalowania

- Cel silnika: 90 Hz, timer 11 ms i stały krok symulacji 1/90 s.
- Każda gra używa wymiarów pola gry jako układu logicznego; rozmiary obiektów, prędkości i progi kolizji muszą zachowywać proporcje przy zmianie rozdzielczości.
- Zmiana rozdzielczości nie może zwiększać trudności przez zmniejszenie względnego rozmiaru paletki, celu, szczeliny ani strefy trafienia.
- Renderowanie jest zoptymalizowane dla Raspberry Pi 4: nieprzezroczyste widgety, brak antyaliasingu, płaskie palety pikselowe i ograniczenie gradientów/poświat pełnoekranowych.
- 90 Hz jest celem pętli gry, a nie gwarancją wyświetlenia 90 klatek. Rzeczywisty wynik zależy od trybu monitora, kompozytora i obciążenia urządzenia.

## 2.4. Kompilacja i testy

- Obecny plik główny CMake jawnie używa `find_package(Qt5 ...)` oraz celów `Qt5::...`. Aktualny przepływ CI buduje Qt 5; nie potwierdza jeszcze kompilacji Qt 6.
- W repozytorium istnieją testy silnika i fabryki gier: `tests/game_engine_regression.cpp`, `tests/game_factory_regression.cpp`.
- Istnieje kontrola własności strumienia czujnika: `tests/game_sensor_ownership_regression.py`.
- Firmware Nano jest kompilowany w workflow `.github/workflows/arduino-nano.yml`. Test protokołu znajduje się w `tests/hx711_calibration_protocol_regression.py`.
- Firmware Nano używa HX711, transmisji szeregowej 115200 baud, ramek binarnych z CRC i raportowania próbek z częstotliwością do 80 SPS (zależnie od modułu i warunków sprzętowych).
- Istnieje osobny workflow dla Raspberry Pi Pico. Plan gier nie powinien wymagać zmiany protokołu czujnika tylko po to, aby dodać nową grę.
- Dla commita `16a5693e8b7e8e197d997c2be7be4b54e313565b` wszystkie cztery kontrole CI zakończyły się sukcesem: Linux desktop GUI build, security regression, patient-log privacy regression oraz pakiet Raspberry Pi ARM64 Debian Trixie. Linux uruchomił 34/34 testy CTest (0 błędów), w tym `game_factory_regression`; ARM64 pomyślnie zbudował cele regresyjne i uruchomił testy. Są to wyniki testów automatycznych, a nie testy z fizycznym tensometrem.
- Nie potwierdzono w tym audycie fizycznych testów wszystkich gier z urządzeniem HX711.

## 3. Katalog istniejących gier — docelowa specyfikacja

Poniższe opisy rozdzielają obecną koncepcję gry od docelowego sterowania zgodnego z zasadą jednego tensometru. Jeżeli obecna wersja używa dodatkowych wejść, są one do usunięcia.

### 3.1. Sinus Force Trainer

**Koncepcja:** prowadzenie kursora po zmiennym przebiegu sinusoidalnym i utrzymywanie go możliwie blisko punktu celu.

- **Grafika:** ciemne tło, czytelna siatka, kontrastowa linia sinusoidy, wyróżniony punkt celu, kursor gracza, pasek siły i informacja o błędzie. Animacja przesuwa cel wzdłuż przebiegu.
- **Funkcje:** poziomy trudności przez zmianę częstotliwości i amplitudy celu; punktacja za czas przebywania w tolerancji; podsumowanie dokładności i stabilności.
- **Tensometr:** siła reguluje pionowe położenie kursora. Im większa siła, tym wyższe położenie według indywidualnie skalibrowanej mapy. Wygładzanie ogranicza drżenie, ale nie może opóźniać sterowania nadmiernie.
- **Brak innych wejść:** brak klawiszy do przesuwania kursora lub zmiany fazy.

### 3.2. Tan Force Trainer

**Koncepcja:** dopasowanie kursora do celu poruszającego się po przebiegu tangensa.

- **Grafika:** układ współrzędnych, odcinki funkcji rozdzielone asymptotami, wyraźne strefy niedozwolone i punkt celu.
- **Funkcje:** warianty przebiegu i zakresu, punktacja za dokładność, ostrzeżenie wizualne przy zbliżaniu się do obszaru nieciągłości.
- **Tensometr:** ciągła siła steruje pionowym położeniem kursora. Gra nie wymaga gwałtownego przechodzenia przez asymptoty.
- **Brak innych wejść:** brak klawiaturowej korekty położenia.

### 3.3. Pong Force Trainer

**Koncepcja:** klasyczny Pong przeciwko komputerowemu przeciwnikowi.

- **Grafika:** minimalistyczne boisko, dwie paletki, piłka z czytelnym śladem, wynik i subtelne efekty odbicia.
- **Funkcje:** serie punktów, różne prędkości piłki, stopniowanie trudności, warianty wielkości paletki i długości meczu.
- **Tensometr:** wartość siły mapowana jest na pionową pozycję paletki gracza. Pozycja jest ograniczona do pola gry; filtr i histereza ograniczają przypadkowe drgania.
- **Brak innych wejść:** gracz nie steruje paletką klawiaturą. Przeciwnik jest sterowany przez algorytm gry.

### 3.4. Arkanoid

**Koncepcja:** odbijanie piłki i niszczenie układów klocków.

- **Grafika:** plansze z kolorowymi klockami, czytelna paletka, animowana piłka, efekty rozbijania i dyskretne cząsteczki.
- **Funkcje:** poziomy, klocki o różnej wytrzymałości, premie, dodatkowe życia, zmiana tempa i wzory plansz.
- **Tensometr:** siła steruje poziomym położeniem paletki. Pozycja jest mapowana na całą szerokość pola gry; prędkość paletki może zależeć od szybkości zmiany siły, a nie od dodatkowego wejścia.
- **Brak innych wejść:** wszystkie premie aktywowane są przez zetknięcie piłki z obiektem lub przez gest tensometru zdefiniowany dla danej premii.

### 3.5. Flappy Bird

**Koncepcja:** prowadzenie ptaka przez szczeliny przeszkód.

- **Grafika:** lekka, kolorowa sceneria z warstwowym przewijaniem, wyraźny ptak, przeszkody, licznik punktów i animacja skrzydeł.
- **Funkcje:** różne zestawy przeszkód, zmienna prędkość, rekordy i krótkie serie poziomów.
- **Tensometr:** krótki impuls siły wywołuje pojedynczy skok. Wysokość impulsu może regulować siłę podbicia w ograniczonym zakresie; długie przytrzymanie nie może generować niekontrolowanej serii skoków.
- **Brak innych wejść:** rozpoczęcie i ponowienie gry odbywa się gestem tensometru lub z poziomu nadrzędnego menu aplikacji.

### 3.6. Mario Run

**Koncepcja:** bieg przez planszę, omijanie przeszkód, zbieranie monet i docieranie do kolejnych etapów.

- **Grafika:** przewijana platformowa plansza, animowany bohater, wyraźne przeszkody, monety, efekty skoku, lądowania i kolizji.
- **Funkcje:** trzy życia, przeszkody o różnej wysokości, premie, poziomy, rekordy i stopniowanie prędkości. Podwójny skok musi mieć czytelny sygnał graficzny i ograniczoną dostępność.
- **Tensometr:** narastająca siła ładuje skok; zwolnienie nacisku wyzwala skok. Czas i poziom siły mogą wspólnie określać wysokość skoku, ale muszą mieć ograniczenia, aby nie wymagać maksymalnego napięcia.
- **Brak innych wejść:** drugi skok, restart i inne akcje muszą zostać przypisane do gestów siły/czasu lub automatyki gry. Skróty klawiaturowe nie są dopuszczone w wersji docelowej.

### 3.7. Car Race

**Koncepcja:** wyścig z ruchem ulicznym, omijaniem pojazdów, zbieraniem żetonów i budowaniem serii bezkolizyjnych minięć.

- **Grafika:** perspektywiczna droga z trzema pasami, kilka warstw tła, animowane pojazdy i koła, znaczniki pobocza, efekty dopalacza i kolizji.
- **Funkcje:** trasy, poziomy prędkości, różne pojazdy, żetony, serie minięć, mistrzostwa i rekordy.
- **Tensometr:** trzy stabilne przedziały siły wybierają lewy, środkowy lub prawy pas. Przełączenie pasa wymaga przekroczenia progu z histerezą.
- **Dopalacz:** przytrzymanie co najmniej 95% siły przez 0,25–1,2 s i zwolnienie do środkowej strefy aktywuje dopalacz. Samochód zachowuje pas podczas gestu.
- **Brak innych wejść:** ruch lewo/prawo i dopalacz z klawiatury muszą być usunięte. Gdy sygnał jest nieaktualny, samochód nie zmienia pasa i nie aktywuje dopalacza.

### 3.8. Archer Force Trainer

**Koncepcja:** strzelanie z łuku do tarcz, również ruchomych, z punktacją za precyzję.

- **Grafika:** łucznik, łuk z animacją naciągu, czytelna tarcza, tor lotu strzały, wiatr przedstawiony graficznie i efekty trafienia.
- **Funkcje:** tarcze statyczne i ruchome, różne odległości, serie trafień, strefy punktowe, poziomy i podsumowanie celności.
- **Tensometr:** steruje kątem łuku w całym zakresie roboczym.
- **Automatyczny strzał:** po uzyskaniu świeżej próbki gra odlicza 2,4 s i oddaje strzał; kolejne strzały następują co 2,4 s. Utrata świeżości sygnału zeruje odliczanie.
- **Brak innych wejść:** klawiatura i mysz nie sterują łukiem ani strzałem.

### 3.9. Galaga

**Koncepcja:** statek kosmiczny niszczy formacje przeciwników i unika ich pocisków.

- **Grafika:** gwiezdne tło, statek gracza, formacje przeciwników, pociski, wybuchy, bossowie i czytelny pasek życia.
- **Funkcje:** fale, przeciwnicy o różnych zachowaniach, premie, osłony, bossowie, serie eliminacji i rekordy.
- **Tensometr:** siła mapuje się na poziomą pozycję statku.
- **Automatyczny ogień:** przy świeżym sygnale statek oddaje strzał co 0,30 s; po utracie sygnału ogień ustaje.
- **Impuls EMP:** przytrzymanie co najmniej 95% siły przez 0,35–1,2 s i zwolnienie do 30% lub mniej aktywuje EMP.
- **Brak innych wejść:** ruch, ogień i EMP nie korzystają z klawiatury ani myszy.

### 3.10. Scorched Earth

**Koncepcja:** turowa gra artyleryjska, w której gracz dobiera kąt i siłę strzału, uwzględniając teren i wiatr.

- **Grafika:** pagórkowaty teren, działa/czołgi, czytelna trajektoria pocisku, efekty eksplozji, wiatr i pasek stanu pojazdów.
- **Funkcje:** przeciwnik sterowany algorytmem, różne rodzaje terenu i pocisków, zmienna pogoda, kampania i poziomy trudności.
- **Tensometr:** ustawia kąt działa w zakresie 15–80 stopni; moc pocisku pozostaje stała i zbalansowana.
- **Automatyczny strzał:** po uzyskaniu świeżego sygnału w turze gracza następuje odliczanie 2 s i wypuszczenie pocisku. Utrata sygnału zeruje odliczanie.
- **Brak innych wejść:** klawiatura i mysz nie sterują kątem, mocą ani strzałem. Przeciwnik wykonuje swoją turę automatycznie.

## 4. Proponowane nowe gry — priorytet rozrywkowy

Wszystkie nowe gry mają korzystać z istniejącego wspólnego potoku siły i silnika gier. Każda pozycja poniżej opisuje możliwy zakres pierwszej wersji.

### 4.1. Precision Crane — dźwig precyzyjny

**Status:** zaimplementowana i rozwijana gra na `main`. Obejmuje sterowanie jednym tensometrem, model wahadła tłumionego, ruchome kontenery i kolizje, trzy typy ładunków, pięć misji po trzy dostawy, premię precyzji oraz trwały zapis statystyk JSON z atomowym zapisem przez `QSaveFile`. Testy regresyjne obejmują rotację ładunków, ruch przeszkód i kolizję w niezerowej fazie ruchu, pełną kampanię 15 dostaw, zakończenie kampanii oraz odczyt rekordów po utworzeniu nowego obiektu gry. Commit `16a5693e8b7e8e197d997c2be7be4b54e313565b` przeszedł wszystkie cztery kontrole CI; Linux zaliczył 34/34 testy CTest, w tym `game_factory_regression`, a ARM64 zaliczył kompilację i testy regresyjne. Fizyczny test Nano/HX711/tensometru pozostaje odrębnym etapem odbioru.

- **Grafika:** port w widoku bocznym, suwnica z prowadnicą, wózek jezdny, lina, hak, ładunek, strefa odbioru, woda i tło portowe. Kołysanie podwieszonego ładunku sygnalizuje bezwładność.
- **Funkcje:** podnoszenie i opuszczanie ładunku, transport poziomy, precyzyjne dostawy, serie udanych dostaw, premie punktowe, trzy pomyłki na sesję i pięć poziomów trudności ze zwężającą się strefą celu.
- **Punktacja precyzji:** każda poprawna dostawa otrzymuje premię 0–100 pkt zależną od odległości środka ładunku od środka strefy celu; premia jest pokazywana w HUD i sumowana z punktami bazowymi, serią oraz poziomem trudności.
- **Model kołysania i bezwładności (pierwsza implementacja):** zastąpiono sinusoidę wymuszaną czasem modelem wahadła tłumionego. Przyspieszenie wózka wzbudza wychylenie ładunku, długość liny wyznacza częstość własną, a tłumienie wygasza ruch po zatrzymaniu lub odczepieniu. Kołysanie jest stanem fizyki, nie dodatkowym wejściem sterującym.
- **Przeszkody portowe:** dwa ruchome kontenery przemieszczają się w korytarzu powietrznym; przecięcie ich obrysu przez podwieszony ładunek powoduje kolizję, utratę próby i przerwanie serii.
- **Rodzaje ładunków:** lekki (0,65 kg), standardowy (1,0 kg) i ciężki (1,65 kg) mają odmienne wymiary, długości zawieszenia i parametry odpowiedzi wahadła. Typ zmienia się rotacyjnie po dostawach.
- **Kampania:** pięć misji po trzy dostawy; trudność celu rośnie w toku rozgrywki, a po piętnastej dostawie kampania zostaje ukończona.
- **Rekordy i historia:** najlepszy wynik oraz podsumowanie dostaw, średniej precyzji i kolizji są zapisywane atomowo w pliku JSON w katalogu danych aplikacji.
- **Tensometr:** skalibrowana siła 0–100% jest jedynym źródłem sterowania. W zakresie roboczym 18–70% mapuje się płynnie na pozycję wózka albo wysokość haka; zakres 0–18% jest strefą zwolnienia, która utrzymuje bieżącą pozycję bez skoku do skrajnego położenia. Długie przytrzymanie wysokiej siły (co najmniej 82%) i zwolnienie przełącza oś poziomą/pionową. Krótkie przytrzymanie (0,15–0,85 s) i zwolnienie wywołuje chwyt albo zwolnienie ładunku. W danej chwili regulowany jest tylko jeden parametr.
- **Bezpieczeństwo wejścia:** nieprawidłowe próbki są ignorowane; po 350 ms bez świeżej próbki ruch i gesty są wstrzymywane. Utrata sygnału nie jest interpretowana jako polecenie.
- **Status zakresu:** podstawowy cykl przenoszenia, przeszkody, kampania, punktacja i trwałe rekordy są zaimplementowane. Automatyczne testy regresyjne obejmują kolizje, pełną kampanię oraz odtworzenie zapisanych statystyk po restarcie obiektu gry.
- **Wyróżnik:** zręcznościowa gra o precyzji i płynności ruchu, sterowana wyłącznie jednym tensometrem.

### 4.2. Rhythm Force — Rytm siły

**Status:** zaimplementowana pierwsza pełna wersja gry na `main`. Gra jest zarejestrowana w `GameFactory`, dostępna w selektorze gier zakładki treningowej i budowana jako część aplikacji desktopowej oraz celów regresyjnych. Testy deterministyczne obejmują ocenę idealnego i dobrego trafienia, dopasowanie szczytowej siły, odrzucenie impulsu o nieprawidłowym czasie trwania, utratę sygnału, zamrożenie osi czasu, ponowne uzbrojenie po neutralnym sygnale i ograniczenia poziomu trudności.

- **Koncepcja rozgrywki:** gracz wykonuje krótkie impulsy nacisku zsynchronizowane z rytmem. Każdy takt ma znacznik czasu i docelową siłę. Wynik zależy od dokładności chwili rozpoczęcia impulsu oraz od tego, jak blisko wartości docelowej znajduje się szczyt nacisku.
- **Grafika i perspektywa:** scena w stylistyce futurystycznego tunelu rytmicznego, z gradientowym tłem, poświatą horyzontu, perspektywiczną siatką podłoża i trapezowym torem. Znaczniki taktów zmieniają położenie, skalę i czytelność zależnie od głębi; aktywny takt ma świetlną obwódkę i poświatę. Panel HUD pokazuje tempo, poziom, numer taktu, wynik, serię, perfekcyjne trafienia i celność.
- **Przebieg rundy:** jedna runda obejmuje 32 takty po krótkim wstępie. Po rozliczeniu ostatniego taktu gra kończy rundę i zatrzymuje pętlę rozgrywki. Wszystkie czynności rozgrywki są obsługiwane tensometrem.
- **Tempo i trudność:** pięć poziomów ustawia tempo odpowiednio na 60, 68, 76, 84 i 92 BPM. Wraz z poziomem trudności okno dopuszczalnego błędu czasu zwęża się z 280 do 180 ms, a okno oceny perfekcyjnej z 90 do 50 ms. Poziom jest ograniczany do zakresu 1–5.
- **Sekwencja siły:** docelowy szczyt nacisku zmienia się między taktami w deterministycznym zakresie 40–84%. Wartość docelowa jest pokazywana przy znaczniku taktu, dzięki czemu użytkownik może przygotować zarówno czas, jak i amplitudę impulsu.
- **Gest tensometru:** strefa spoczynku wynosi 0–18%. Przekroczenie 18% rozpoczyna impuls, a powrót do 18% lub niżej kończy go i uruchamia ocenę. Dopuszczalny czas impulsu wynosi 60–750 ms. Wartość maksymalna zarejestrowana w trakcie impulsu jest porównywana z siłą docelową.
- **Ocena rytmu:** wynik czasowy ma trzy poziomy: perfekcyjny, dobry i trafienie. Impuls poza oknem trafienia albo o nieprawidłowym czasie trwania jest pomyłką. Impuls przedwczesny nie zużywa jeszcze oczekującego taktu; spóźniony takt zostaje rozliczony jako utracony.
- **Punktacja:** część czasowa daje do 100 pkt, dopasowanie szczytowej siły do 50 pkt, a seria kolejnych trafień dodaje premię rosnącą do 50 pkt. Gra udostępnia wynik rundy, liczbę trafień, pomyłek, trafień perfekcyjnych, bieżącą i najlepszą serię oraz procent trafień.
- **Animacja i informacja zwrotna:** niezależny zegar wizualny steruje pulsowaniem aktywnego impulsu i wygaszaniem efektu trafienia; zegar animacji działa także wtedy, gdy wykres rytmu jest zamrożony z powodu braku świeżych próbek. Pasek siły pokazuje bieżący nacisk, zakres tolerancji celu oraz dokładny znacznik docelowej wartości.
- **Rytm i brak sygnału:** oś czasu postępuje wyłącznie przy świeżym sygnale. Po 350 ms bez poprawnej próbki gra anuluje trwający impuls i zamraża rytm, zamiast zaliczać nieobecność użytkownika jako pudło. Po ponownym połączeniu wymagane jest zwolnienie tensometru do strefy neutralnej, zanim wejście zostanie ponownie uzbrojone.
- **Walidacja wejścia:** wartości niefinitywne są odrzucane, a siła wejściowa ograniczana do 0–100%. Jedynym wejściem rozgrywki pozostaje centralny strumień jednego tensometru; gra nie otwiera portu szeregowego i nie tworzy własnego czytnika HX711.
- **Integracja aplikacji:** identyfikator `rhythm_force` jest dostępny w fabryce gier i na liście gier używanych w planie treningowym. Gra korzysta ze wspólnego cyklu życia `GameEngine`, jego pętli aktualizacji i standardowego przekazywania próbek czujnika.
- **Osiągnięcia:** trwały zapis lokalny przez `QSettings`; odblokowania za pierwszy takt, pierwsze perfekcyjne trafienie, serię 8, bezbłędną rundę, co najmniej 16 perfekcyjnych trafień oraz celność co najmniej 90%. Nowe osiągnięcie jest sygnalizowane krótkim komunikatem w scenie gry. Ekran końcowy prezentuje rangę rundy, wynik, celność, liczbę perfekcyjnych trafień i odblokowane wyróżnienia. Osobno zapisywane są rekordy punktowe, celności i najlepszej serii, zachowywane pomiędzy uruchomieniami gry.
- **Granice zastosowania:** wynik jest wynikiem zręcznościowej gry rytmicznej. Sam w sobie nie stanowi diagnozy, pomiaru sprawności ani dowodu skuteczności terapii.

### 4.3. Force Maze — labirynt siły

**Status:** pierwsza grywalna implementacja dodana do `main`, zarejestrowana w fabryce i selektorze zakładki treningowej. Gra wykorzystuje wspólny `GameEngine`, a wejście otrzymuje wyłącznie przez istniejący strumień próbek tensometru.

- **Koncepcja rozgrywki:** gracz prowadzi świetlną kulkę przez przewijający się labirynt. Trasa składa się z 18 odcinków na poziom i pięciu poziomów kampanii. Korytarz zmienia położenie stopniowo, a tempo przesuwu rośnie po ukończeniu kolejnych poziomów.
- **Grafika i perspektywa:** pseudo-trójwymiarowy tunel z punktem zbiegu, perspektywicznym skalowaniem szerokości kafli i odległości, zróżnicowanym oświetleniem planów, poświatą kulki, obracającym się refleksiem, cząsteczkowym śladem ruchu, pulsującymi kryształami, bramkami, pułapkami i znacznikami punktów kontrolnych. Kolizja uruchamia krótkie drżenie kamery i czerwony błysk. Animacja ma niezależny zegar wizualny.
- **Sterowanie jednym tensometrem:** jeden kanał siły steruje położeniem poziomym. Szeroka strefa neutralna 10–30% nie wywołuje ruchu i utrzymuje ostatnią pozycję docelową; poza nią wartości 0–10% prowadzą płynnie w jedną stronę, a 30–100% w przeciwną. W HUD strefa neutralna jest oznaczona pasem na mierniku. Przycisk „Ręka: prawa/lewa” w górnej części ekranu odwraca kierunek mapowania i zapisuje wybór w ustawieniach. Po zmianie ręki wymagane jest ponowne ustawienie nacisku w strefie neutralnej, co zapobiega skokowi kulki. Zmiana ręki jest ustawieniem interfejsu, a nie dodatkowym wejściem sterującym w rozgrywce. Ruch jest wygładzany filtrem wykładniczym, a oś postępu pozostaje automatyczna.
- **Przebieg i trudność:** pięć poziomów po 18 odcinków, trzy życia na kampanię, stopniowo rosnąca prędkość przewijania i deterministycznie generowana trasa. Punkty kontrolne rozmieszczone są co pięć odcinków; kolizja odbiera życie i cofa gracza do ostatniego punktu kontrolnego.
- **Kryształy i punktacja:** kryształy są rozmieszczane deterministycznie w bezpiecznym korytarzu. Zebranie kryształu daje 100 pkt plus premię poziomu, osiągnięcie punktu kontrolnego 150 pkt, ukończenie poziomu 250 pkt razy numer poziomu, a ukończenie kampanii dodatkowe 1000 pkt. Kolizja odejmuje 25 pkt.
- **Informacja zwrotna:** HUD pokazuje poziom, wynik, liczbę zebranych i dostępnych kryształów, pozostałe życia i punkty kontrolne. Kolizja daje czerwony błysk, punkt kontrolny złotą obwódkę, a kryształ pulsującą animację. Pasek statusu informuje o gotowości wejścia, sterowaniu i stanie bezpieczeństwa.
- **Osiągnięcia:** pierwszy kryształ (`first_crystal`), kolekcjoner 10 kryształów (`crystal_collector`), pierwszy punkt kontrolny (`checkpoint`), ukończenie poziomu bez kolizji (`clean_level`), trzy czyste poziomy (`three_clean_levels`), dotarcie do poziomu 3 (`level_three`), dotarcie do poziomu 5 (`level_five`), ukończenie kampanii (`maze_master`), kampania bez kolizji (`untouched_campaign`) oraz zebranie 25 kryształów (`crystal_master`). Odblokowania są zapisywane lokalnie w `QSettings`.
- **Rekordy:** najlepszy wynik, największa liczba kryształów i najwyższy osiągnięty poziom są trwale przechowywane w `QSettings` pod `games/force_maze/records`.
- **Bezpieczeństwo sygnału:** wartości niefinitywne są odrzucane, wartości siły ograniczane do 0–100%, a po 350 ms bez świeżej próbki ruch świata i sterowanie zostają zatrzymane. Po uruchomieniu lub utracie sygnału wymagane jest ponowne uzbrojenie przez próbkę neutralną. Brak sygnału nie jest interpretowany jako zerowa siła ani ruch w stronę krawędzi.
- **Testy regresyjne:** utworzenie przez fabrykę, mapowanie jednego kanału, brak ruchu w całym paśmie 10–30%, skrajne wartości siły, odwrócenie kierunku po zmianie ręki, trwałość ustawienia, ponowne uzbrojenie w strefie neutralnej, odrzucenie NaN, zamrożenie postępu po przekroczeniu limitu świeżości sygnału oraz odblokowanie kamieni milowych kampanii. Testy symulują próbki; nie zastępują odbioru fizycznego Nano/HX711/tensometru.
- **Granice zastosowania:** wynik jest wynikiem gry zręcznościowej, a nie diagnozą ani oceną sprawności klinicznej.

### 4.4. Balance Tower — wieża równowagi

**Status:** pierwsza grywalna implementacja dodana do `main`, zarejestrowana w `GameFactory`, selektorze zakładki treningowej i celach kompilacji desktopowej oraz regresyjnej.

- **Koncepcja:** budowanie trójwymiarowo prezentowanej wieży z kolejnych elementów. Każde ułożenie zmienia rozkład masy i położenie środka ciężkości. Gracz zdobywa punkty za precyzję, utrzymuje serię udanych ułożeń i przechodzi trzy rundy o rosnącym poziomie trudności.
- **Grafika:** kolorowe, przestrzenne bloki, platforma bazowa, przesunięcie perspektywiczne osi głębokości, podświetlenie aktualnie kontrolowanego elementu, miernik stabilności, wizualizacja przechyłu, animowane tło i ekran zakończenia rundy.
- **Model układania:** każdy element ma położenie w dwóch osiach, szerokość, głębokość i masę. Umieszczenie wymaga odpowiedniego nakładania się elementu na poprzedni. Zbyt małe podparcie oznacza upadek elementu, utratę życia, przerwanie serii i obniżenie wyniku. Zaliczone elementy tworzą stos, którego środek ciężkości i stabilność są przeliczane po każdym ułożeniu.
- **Sterowanie jednym tensometrem:** jedna próbka siły reguluje położenie aktualnego elementu w aktywnej osi. Strefa neutralna 10–30% utrzymuje pozycję. Zakres poniżej 10% i powyżej 30% mapuje się na przeciwne kierunki osi. Krótki gest wysokiej siły (82% lub więcej, 0,12–0,85 s) zakończony powrotem do strefy neutralnej upuszcza element. Długi gest (powyżej 0,85 s) i zwolnienie przełącza oś poziomą/głębokości. W każdej chwili jest używany jeden kanał wejściowy; nie ma sterowania klawiaturą ani myszą.
- **Rundy i trudność:** trzy rundy kampanii, po 12 elementów do ułożenia w każdej. Po zaliczeniu rundy konstrukcja jest resetowana do stabilnej podstawy, a trudność rośnie przez stopniowe zmniejszanie rozmiaru elementów. Trzy życia pozwalają popełnić ograniczoną liczbę błędów.
- **Punktacja i serie:** wynik uwzględnia poprawne ułożenie, precyzję względem środka poprzedniego elementu i serię kolejnych udanych ruchów. Upadek przerywa serię i odejmuje punkty.
- **Osiągnięcia:** pierwszy element (`first_block`), wieża z co najmniej sześciu elementów (`tower_six`), seria pięciu (`steady_hand`), ukończona runda (`round_complete`), mistrz trzech rund (`tower_master`) oraz ukończenie kampanii bez upadku (`perfect_foundation`). Osiągnięcia i rekordy są zapisywane w `QSettings`.
- **Rekordy:** najlepszy wynik i najwyższa ukończona runda są zachowywane lokalnie pod `games/balance_tower/records`.
- **Bezpieczeństwo sygnału:** wartości niefinitywne są odrzucane. Po 350 ms bez świeżej próbki ruch i gesty są zatrzymywane, a rozpoczęty gest anulowany. Po uruchomieniu lub utracie sygnału wymagane jest ponowne uzbrojenie przez próbkę z neutralnej strefy. Brak sygnału nie jest traktowany jako zerowa siła.
- **Testy regresyjne:** rejestracja fabryki, cykl życia, neutralne uzbrojenie, krótki gest upuszczania, długi gest zmiany osi, stabilność stosu, timeout i ponowne uzbrojenie. Testy symulują wejście; nie zastępują testu fizycznego Nano/HX711/tensometru.
- **Granice zastosowania:** wynik ma charakter rozrywkowy i nie jest diagnozą, oceną równowagi klinicznej ani dowodem skuteczności terapii.

### 4.5. Reaction Force — refleks siły

- **Grafika:** cele pojawiające się w różnych miejscach ekranu, odliczanie, pasek siły i wyraźny sygnał trafienia.
- **Funkcje:** zadania na reakcję, trafienie w zakres siły, serie celnych odpowiedzi i stopniowanie tempa.
- **Tensometr:** pojedynczy impuls musi rozpocząć się w określonym oknie czasowym i osiągnąć wyświetlony zakres siły. Nieprawidłowy lub spóźniony impuls kończy próbę.
- **Wyróżnik:** krótkie, dynamiczne rundy i porównywanie własnych rekordów.

### 4.6. Space Docking — dokowanie statku

- **Grafika:** statek, stacja kosmiczna, port dokujący, znaczniki prędkości i odległości.
- **Funkcje:** precyzyjne podejście, hamowanie, ustawianie statku i seria coraz trudniejszych dokowań.
- **Tensometr:** siła steruje prędkością podejścia lub położeniem w osi aktywnej. Tryb sterowania przełącza się gestem długiego przytrzymania. Zwolnienie do strefy spoczynku oznacza hamowanie.
- **Wyróżnik:** kontrola bezwładności i dokładności w atrakcyjnej oprawie kosmicznej.

### 4.7. Submarine Pilot — łódź podwodna

- **Grafika:** przekrój oceanu, łódź, skały, miny, ławice ryb i strefy głębokości.
- **Funkcje:** omijanie przeszkód, zbieranie skarbów, przejścia przez jaskinie i poziomy.
- **Tensometr:** siła reguluje głębokość. Tryb sekwencyjny może przełączać sterowanie głębokością i prędkością; spoczynek ogranicza prędkość, a krótkie impulsy aktywują krótkie manewry.
- **Wyróżnik:** płynne sterowanie i bogate otoczenie graficzne.

### 4.8. Memory Force — pamięć sekwencji

- **Grafika:** kolorowe pola lub symbole prezentujące sekwencję, pasek postępu i wyraźne potwierdzenia.
- **Funkcje:** zapamiętywanie i odtwarzanie sekwencji o różnej długości, tempie i sile.
- **Tensometr:** każdy element sekwencji jest kodowany poziomem siły i długością impulsu. Gracz odtwarza kolejność, siłę i rytm wyłącznie przez nacisk.
- **Wyróżnik:** gra pamięciowa, w której wejście ma kilka cech czasowo-siłowych, ale nadal pochodzi z jednego czujnika.

### 4.9. Magic Garden — magiczny ogród

- **Grafika:** ogród z roślinami, glebą, kroplami wody, motylami i zmieniającymi się porami dnia.
- **Funkcje:** pielęgnacja roślin, zadania dzienne, odblokowywanie gatunków i rozwój ogrodu.
- **Tensometr:** siła i czas przytrzymania regulują ilość wody. Krótkie impulsy wybierają kolejną roślinę lub zatwierdzają czynność, a wartość siły reguluje intensywność podlewania.
- **Wyróżnik:** spokojna gra z kolekcjonowaniem i długoterminowym postępem.

### 4.10. Balloon Flight — lot balonem

- **Grafika:** balon na tle krajobrazów, chmur, gór i przeszkód, wskaźnik wysokości i prędkości.
- **Funkcje:** przeloty przez obręcze, omijanie burz, zbieranie punktów i odblokowywanie tras.
- **Tensometr:** siła steruje intensywnością ogrzewania i wznoszenia; zmniejszenie nacisku powoduje opadanie. Wybór kierunku może odbywać się w osobnym trybie gestowym, a nie przez drugi kontroler.
- **Wyróżnik:** łagodna mechanika oparta na płynnej modulacji siły.

### 4.11. Fishing Force — wędkarstwo

- **Grafika:** łowisko z wodą, wędką, spławikiem, rybami, wskaźnikiem napięcia żyłki i kolekcją połowów.
- **Funkcje:** zarzucanie, oczekiwanie na branie, holowanie i dobór siły do wielkości ryby.
- **Tensometr:** przytrzymanie ładuje zarzut, zwolnienie wypuszcza zestaw. Podczas holowania siła reguluje napięcie żyłki; zbyt duża siła powoduje zerwanie, a zbyt mała pozwala rybie uciec.
- **Wyróżnik:** gra o kontroli siły i wyczuciu odpowiedniego momentu.

### 4.12. Bridge Builder — budowniczy mostów

- **Grafika:** przekrój doliny, rzeki lub kanionu, elementy konstrukcyjne, podpory i pojazd testowy.
- **Funkcje:** rozmieszczanie elementów, dobór długości i stabilności, testowanie mostu przejazdem.
- **Tensometr:** aktywny element przesuwa się wzdłuż osi siłą; gest przełącza rodzaj parametru, a krótki impuls umieszcza element. Stabilność mostu jest obliczana przez symulację.
- **Wyróżnik:** łamigłówka konstrukcyjna z czytelnym sprzężeniem zwrotnym.

## 5. Wspólny moduł sterowania — najważniejszy element techniczny

Przed dodawaniem wielu nowych gier należy wprowadzić wspólną warstwę wejściową, aby każda gra nie implementowała własnych, sprzecznych progów i filtrów.

### 5.1. Proponowane komponenty

- `ForceInputFrame` — niezmienny opis próbki i jej czasu.
- `ForceCalibrationProfile` — indywidualny zakres, spoczynek, limity i kierunek.
- `ForceSignalProcessor` — walidacja, filtr, histereza, normalizacja i wykrywanie nieaktualnych danych.
- `ForceGestureRecognizer` — krótkie impulsy, podwójne impulsy, przytrzymanie, zwolnienie i długie gesty.
- `GameControlMapping` — jawna konfiguracja mapowania gestów na działania konkretnej gry.
- `GameInputRecorder` — zapis anonimowych przebiegów testowych do deterministycznego odtwarzania w testach.

Nazwy są propozycją projektową, nie istniejącymi obecnie klasami.

### 5.2. Zasady techniczne

1. Pozostawić jeden właścicielski strumień odczytu szeregowego i istniejący wspólny potok pomiarowy.
2. Nie zmieniać protokołu Nano/HX711 dla potrzeb logiki gier, jeśli nie ma technicznej konieczności.
3. Przetwarzać wejście w sposób deterministyczny: ta sama sekwencja próbek i znaczników czasu daje ten sam wynik gestów.
4. Oddzielić częstotliwość próbkowania czujnika od częstotliwości renderowania.
5. Nie generować akcji z brakującej próbki, timeoutu lub błędnej ramki.
6. Zapewnić możliwość odtworzenia nagranego strumienia bez fizycznego urządzenia.
7. W każdej grze prezentować aktualny parametr sterowania, poziom siły i stan połączenia.
8. Zapewnić regulację czułości i strefy neutralnej bez zmiany firmware.

## 6. Plan realizacji — priorytet rozrywka

### Etap 0 — zgodność sterowania i porządek w katalogu

- **Wdrożono:** usunięto obsługę zdarzeń klawiatury i myszy z klas gier na `main`.
- **Wdrożono:** Car Race otrzymał gest dopalacza, Galaga automatyczny ogień i gest EMP, Flappy Bird skok wyłącznie tensometrem.
- **Wdrożono:** Archer i Scorched Earth używają automatycznego strzału zależnego od świeżego sygnału.
- **Wdrożono:** zaktualizowano `docs/GAME_CONTROLS.md` oraz testy regresyjne.
- **Do utrzymania:** kontrola CI ma blokować ponowne dodanie zdarzeń klawiatury/myszy do katalogu gier.

**Warunek zakończenia:** wszystkie 13 istniejących gier używają jednego strumienia tensometru do sterowania rozgrywką.

### Etap 1 — wspólne sterowanie i zgodność platform

- Wprowadzić wspólny model próbki, świeżości sygnału i gestów.
- Dodać testy kalibracji, normalizacji, filtracji, histerezy i rozpoznawania gestów.
- Zachować Qt 5 jako działający wariant bazowy i dodać rzeczywisty wariant budowania z Qt 6.
- Zastąpić rozproszone odwołania do `Qt5::...` kontrolowanym wyborem jednej wersji Qt na całe drzewo budowania. Nie wolno mieszać Qt 5 i Qt 6 w jednym celu.
- Rozszerzyć CI o osobne kompilacje i testy Qt 5 oraz Qt 6.
- Zachować C++17, istniejący silnik gry i zgodność z Arduino Nano/HX711.

**Warunek zakończenia:** oba warianty Qt budują aplikację i testy, a te same nagrane próbki dają identyczne zdarzenia sterujące.

### Etap 2 — dostosowanie trzynastu istniejących gier

Kolejność:
1. Sinus i Tangens — dopracowanie mapowania ciągłego i wariantów przebiegów.
2. Pong i Arkanoid — płynność, fizyka odbić, plansze i premie.
3. Flappy Bird i Mario Run — niezawodne impulsy i ładowanie skoku.
4. Car Race — wybór pasa oraz gest dopalacza.
5. Archer — rozdzielenie celowania, naciągu i strzału przez jeden strumień.
6. Galaga — pozycja statku, automatyczny ogień i gest specjalny.
7. Scorched Earth — sekwencyjny wybór kąta i mocy.

**Warunek zakończenia:** żadna gra nie potrzebuje klawiatury ani myszy podczas rozgrywki; wszystkie akcje są osiągalne gestami tensometru i mają testy.

### Etap 3 — nowe gry, pierwsza fala

Priorytet:
1. Precision Crane — zaimplementowana, z testami kampanii i kolizji
2. Rhythm Force — zaimplementowana pierwsza pełna wersja, zintegrowana z planem treningowym
3. Force Maze
4. Balance Tower
5. Reaction Force

Powód: różnorodne mechaniki, duży potencjał rozrywkowy i możliwość wykorzystania tego samego modelu sygnału bez rozbudowy sprzętu.

**Warunek zakończenia:** każda gra ma kompletną pętlę rozgrywki, instrukcję gestów, ekran wyniku, reset, testy deterministyczne i działanie przy braku sygnału.

### Etap 4 — nowe gry, druga fala

1. Space Docking
2. Submarine Pilot
3. Memory Force
4. Magic Garden
5. Balloon Flight
6. Fishing Force
7. Bridge Builder

**Warunek zakończenia:** nowa gra nie wprowadza odrębnego odczytu czujnika, nie wymaga drugiego wejścia i nie duplikuje logiki filtracji.

### Etap 5 — dopracowanie rozrywki

- kampanie, odblokowywane plansze i osiągnięcia;
- rekordy lokalne i profile trudności;
- animacje, dźwięki i efekty wizualne jako wyłącznie wyjściowe elementy gry;
- tryb krótkich rund i tryb długiej kampanii;
- adaptacja trudności na podstawie wyników gry, bez automatycznego podnoszenia limitu siły;
- wspólny ekran podsumowania i statystyk.

## 7. Testy i kryteria odbioru

### 7.1. Testy jednostkowe wspólnego wejścia

- wartości spoczynkowe i dryft;
- wartości minimalne i maksymalne;
- nagłe skoki i zakłócenia;
- próbki nieaktualne i brak ramek;
- błędne CRC, NaN, Inf i timeout;
- pojedynczy impuls, podwójny impuls, długie przytrzymanie i zwolnienie;
- histereza przy wartościach bliskich progowi;
- deterministyczne odtwarzanie identycznego przebiegu.

### 7.2. Testy każdej gry

- wszystkie parametry sterowania reagują wyłącznie na próbki tensometru;
- każda akcja specjalna ma przypisany gest, próg i czas;
- brak próbki zatrzymuje akcje zależne od wejścia;
- restart nie wykorzystuje starych próbek;
- pauza i wznowienie nie powodują niezamierzonej akcji;
- rozgrywka jest możliwa przy minimalnej i maksymalnej dopuszczonej kalibracji;
- testy nie wymagają fizycznej klawiatury, myszy ani gamepada.

### 7.3. Testy integracyjne i CI

- regresje silnika gier i fabryki;
- test centralnego właściciela strumienia czujnika;
- testy protokołu Nano/HX711;
- kompilacja firmware Nano;
- kompilacja Qt 5 i Qt 6;
- testy Linux desktop GUI;
- testy bez sprzętu z odtwarzaniem zapisanych próbek;
- test sprzętowy Nano + HX711 jako osobny etap odbioru.

### 7.4. Kryteria jakości rozgrywki

- sterowanie ma przewidywalną reakcję i nie zmienia pasa/pozycji samoistnie;
- opóźnienie i filtracja nie utrudniają trafiania w cele;
- gesty nie nakładają się na siebie i są wyjaśnione na ekranie;
- użytkownik widzi aktywny parametr i stan czujnika;
- trudność rośnie przez mechanikę gry, a nie przez wymaganie coraz większej siły;
- każdą sesję można przerwać bez konieczności wykonywania maksymalnego nacisku.

## 8. Decyzje projektowe, których nie wolno naruszyć

- **Jeden tensometr, jeden kanał sterowania.** Żadna gra nie może wymagać dodatkowego urządzenia wejściowego.
- **Nie dodawać sterowania klawiaturą jako „awaryjnego”.** To narusza założenie produktu.
- **Nie używać utraty sygnału jako komendy.** Brak danych oznacza brak sterowania.
- **Nie mnożyć niezależnych czytników HX711.** Odczyt pozostaje centralny.
- **Nie zmieniać firmware bez potrzeby.** Gry konsumują już skalibrowaną siłę, a nie surowy protokół.
- **Nie mieszać wersji Qt.** Każda kompilacja wybiera jedną główną wersję Qt.
- **Nie przedstawiać testu CI jako testu urządzenia.** Odbiór fizyczny wymaga rzeczywistego Nano, HX711 i tensometru.
- **Nie przedstawiać wyników rozgrywki jako rozpoznania medycznego.**

## 9. Pliki powiązane

- `docs/GAME_CONTROLS.md` — bieżąca instrukcja sterowania; wymaga aktualizacji po wdrożeniu zasady wyłącznego sterowania tensometrem.
- `include/games/GameEngine.hpp` i `src/games/GameEngine.cpp` — wspólny silnik gier.
- `include/games/GameFactory.hpp` — rejestr gier.
- `tests/game_engine_regression.cpp` — regresje cyklu życia, czasu i wejścia silnika.
- `tests/game_factory_regression.cpp` — regresje fabryki.
- `tests/game_sensor_ownership_regression.py` — centralna własność strumienia czujnika.
- `tests/hx711_calibration_protocol_regression.py` — kontrola protokołu i kalibracji Nano/HX711.
- `hx711_advanced.ino` — firmware Arduino Nano.
- `firmware/pico/` — firmware Raspberry Pi Pico.
- `.github/workflows/desktop-gui-build.yml`, `.github/workflows/arduino-nano.yml`, `.github/workflows/pico-firmware.yml` — obecne przepływy budowania.

## 10. Źródło zaleceń dotyczących Qt

Dokumentacja Qt opisuje możliwość wspólnego kodu CMake dla Qt 5 i Qt 6 przez wybór `QT_VERSION_MAJOR` i linkowanie celów `Qt${QT_VERSION_MAJOR}::...`. Należy wybrać jedną wersję dla danego drzewa budowania i nie mieszać bibliotek obu głównych wersji w jednym celu.

- [Qt — zgodność Qt 5 i Qt 6 w CMake](https://doc.qt.io/qt-6/cmake-qt5-and-qt6-compatibility.html)
