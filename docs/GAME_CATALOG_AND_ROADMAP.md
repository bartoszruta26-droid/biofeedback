# Katalog gier i plan rozwoju — sterowanie wyłącznie tensometrem

**Repozytorium:** `bartoszruta26-droid/biofeedback`  
**Podstawa audytu:** `main` — `d3b9afd8635f6e9fe5e8f2a015763abb19742007`  
**Priorytet rozwoju:** rozrywka i grywalność  
**Status dokumentu:** plan i specyfikacja; nie jest potwierdzeniem implementacji nowych mechanik

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

Silnik udostępnia cykl życia gry, pętlę aktualizacji, renderowanie, wynik oraz `handleSensorInput(double)`. Pętla gry używa timera 8 ms i symulacji o stałym kroku 1/120 s. Dane napływające z wątku odczytu są łączone do najnowszej próbki i przekazywane do wątku GUI; obsługiwane są wartości niefinitywne oraz unieważnianie opóźnionych wywołań po zatrzymaniu lub resecie.

Wspólny potok pomiarowy przekazuje skalibrowaną siłę w niutonach. Test `tests/game_sensor_ownership_regression.py` pilnuje, aby gry nie tworzyły własnych strumieni szeregowych i korzystały z centralnego źródła pomiaru.

### 2.2. Istniejące gry

W katalogach `src/games` i `include/games` znajduje się dziesięć gier:

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

Aktualny opis zachowania znajduje się w `docs/GAME_CONTROLS.md`. Audyt tej instrukcji wykazał rozbieżność z nową zasadą: dokument nadal opisuje skróty klawiaturowe, a w Car Race, Archer, Galaga i Scorched Earth klawiatura jest opisana jako część sterowania. Przed uznaniem całego katalogu za zgodny należy usunąć te ścieżki wejściowe i dodać testy, które wykryją ich ponowne wprowadzenie. Należy również sprawdzić zmiany w otwartych PR-ach dotyczących Mario Run, Archer i Scorched Earth — część opisów tych prac wymienia sterowanie klawiaturą.

### 2.3. Kompilacja i testy

- Obecny plik główny CMake jawnie używa `find_package(Qt5 ...)` oraz celów `Qt5::...`. Aktualny przepływ CI buduje Qt 5; nie potwierdza jeszcze kompilacji Qt 6.
- W repozytorium istnieją testy silnika i fabryki gier: `tests/game_engine_regression.cpp`, `tests/game_factory_regression.cpp`.
- Istnieje kontrola własności strumienia czujnika: `tests/game_sensor_ownership_regression.py`.
- Firmware Nano jest kompilowany w workflow `.github/workflows/arduino-nano.yml`. Test protokołu znajduje się w `tests/hx711_calibration_protocol_regression.py`.
- Firmware Nano używa HX711, transmisji szeregowej 115200 baud, ramek binarnych z CRC i raportowania próbek z częstotliwością do 80 SPS (zależnie od modułu i warunków sprzętowych).
- Istnieje osobny workflow dla Raspberry Pi Pico. Plan gier nie powinien wymagać zmiany protokołu czujnika tylko po to, aby dodać nową grę.
- Ostatnie widoczne przebiegi CI dla wskazanego `main` zakończyły się sukcesem: Linux desktop GUI build, security regression, patient-log privacy regression oraz pakiet Raspberry Pi ARM64. Są to wyniki CI, a nie testy z fizycznym tensometrem.
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
- **Tensometr:** trzy stabilne przedziały siły wybierają lewy, środkowy lub prawy pas. Przełączenie pasa wymaga przekroczenia progu z histerezą. Dopalacz: podwójny krótki impuls albo inny jednoznaczny gest czasowy; gest nie może kolidować ze zwykłą zmianą pasa.
- **Brak innych wejść:** ruch lewo/prawo i dopalacz z klawiatury muszą być usunięte. Gdy sygnał jest nieaktualny, samochód nie zmienia pasa i nie aktywuje dopalacza.

### 3.8. Archer Force Trainer

**Koncepcja:** strzelanie z łuku do tarcz, również ruchomych, z punktacją za precyzję.

- **Grafika:** łucznik, łuk z animacją naciągu, czytelna tarcza, tor lotu strzały, wiatr przedstawiony graficznie i efekty trafienia.
- **Funkcje:** tarcze statyczne i ruchome, różne odległości, serie trafień, strefy punktowe, poziomy i podsumowanie celności.
- **Tensometr:** tryb celowania mapuje siłę na kąt. Przytrzymanie rozpoczyna naciąganie cięciwy, a zwolnienie oddaje strzał; siła w chwili zwolnienia określa moc strzału w bezpiecznym zakresie. Jeśli konieczne jest rozdzielenie kąta i mocy, długi gest przełącza aktywny parametr, a nie dodatkowy przycisk.
- **Brak innych wejść:** klawisze korekty kąta i spacji do strzału są niedopuszczone. Tor lotu i wiatr są wyłącznie informacją wizualną, nie wymagają osobnego sterownika.

### 3.9. Galaga

**Koncepcja:** statek kosmiczny niszczy formacje przeciwników i unika ich pocisków.

- **Grafika:** gwiezdne tło, statek gracza, formacje przeciwników, pociski, wybuchy, bossowie i czytelny pasek życia.
- **Funkcje:** fale, przeciwnicy o różnych zachowaniach, premie, osłony, bossowie, serie eliminacji i rekordy.
- **Tensometr:** siła mapuje się na poziomą pozycję statku. Ogień jest automatyczny w czasie aktywnej rozgrywki, dzięki czemu gracz nie potrzebuje drugiego przycisku. EMP lub inna umiejętność specjalna może być aktywowana podwójnym impulsem, z długim czasem odnowienia i wizualnym potwierdzeniem.
- **Brak innych wejść:** klawisze ruchu, ognia i EMP muszą zostać usunięte. Automatyczny ogień jest funkcją gry, a nie osobnym źródłem wejścia.

### 3.10. Scorched Earth

**Koncepcja:** turowa gra artyleryjska, w której gracz dobiera kąt i siłę strzału, uwzględniając teren i wiatr.

- **Grafika:** pagórkowaty teren, działa/czołgi, czytelna trajektoria pocisku, efekty eksplozji, wiatr i pasek stanu pojazdów.
- **Funkcje:** przeciwnik sterowany algorytmem, różne rodzaje terenu i pocisków, zmienna pogoda, kampania i poziomy trudności.
- **Tensometr:** sekwencyjne sterowanie jednym parametrem. Krótki gest przełącza wybór między kątem i mocą; aktualna siła ustawia wybrany parametr. Długi gest zatwierdzony zwolnieniem oddaje strzał. Interfejs stale pokazuje, który parametr jest aktywny.
- **Brak innych wejść:** klawisze W/S, +/- i spacja są niedopuszczone. Przeciwnik wykonuje swoją turę automatycznie.

## 4. Proponowane nowe gry — priorytet rozrywkowy

Wszystkie nowe gry mają korzystać z istniejącego wspólnego potoku siły i silnika gier. Każda pozycja poniżej opisuje możliwy zakres pierwszej wersji.

### 4.1. Precision Crane — dźwig precyzyjny

**Status:** pierwsza grywalna wersja zaimplementowana i scalona do `main` (PR #270). Uzupełniono zabezpieczenie fail-closed po utracie świeżości sygnału: po 350 ms ruch zostaje zatrzymany, gest anulowany, a wskazanie siły zerowane do prezentacji neutralnej; próbki NaN/Inf nie odświeżają watchdogu. Dodano regresje utraty/przywrócenia sygnału i rozróżniania krótkiego impulsu od gestu poniżej minimalnego czasu.

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
- **Status zakresu:** dostępna jest pierwsza grywalna wersja z podstawowym cyklem przenoszenia i punktacją. Zaawansowane przeszkody, kampania i zapisywanie rekordów pozostają dalszymi etapami.
- **Wyróżnik:** zręcznościowa gra o precyzji i płynności ruchu, sterowana wyłącznie jednym tensometrem.

### 4.2. Rhythm Force — rytm siły

- **Grafika:** scena muzyczna, pulsujące znaczniki rytmu, tor nadchodzących impulsów i wizualny miernik siły.
- **Funkcje:** poziomy rytmiczne, sekwencje o różnych tempach, mnożniki serii i ocena trafienia w okno czasowe.
- **Tensometr:** krótkie impulsy muszą trafić w rytm. Siła impulsu odpowiada wysokości znacznika; czas rozpoczęcia i zwolnienia określa precyzję rytmiczną.
- **Wyróżnik:** rozgrywka oparta na rytmie, czasie i stopniowaniu nacisku; bez mikrofonu i bez odtwarzania dźwięku jako wejścia.

### 4.3. Force Maze — labirynt siły

- **Grafika:** labirynt 2D z kulką, ścianami, bramkami, pułapkami i punktami kontrolnymi.
- **Funkcje:** przejście trasy bez dotykania ścian, zbieranie kryształów, poziomy o rosnącej złożoności.
- **Tensometr:** siła steruje położeniem kulki wzdłuż osi aktywnej. Gest wyboru trybu przełącza oś poziomą/pionową. Alternatywnie plansza może być projektowana jako tor jednokierunkowy, aby cały czas używać tylko jednej osi.
- **Wyróżnik:** zręcznościowe wyzwania przestrzenne bez dodatkowego kontrolera.

### 4.4. Balance Tower — wieża równowagi

- **Grafika:** kolorowe elementy układane na platformie, widoczny środek ciężkości, animacja przechyłu i stabilności.
- **Funkcje:** układanie elementów, utrzymanie równowagi, serie i wyzwania na czas.
- **Tensometr:** siła przesuwa aktualnie wybrany element w lewo/prawo lub reguluje jego położenie w osi aktywnej. Krótki impuls upuszcza element; długi gest zmienia oś lub zatwierdza ruch.
- **Wyróżnik:** prosta do zrozumienia mechanika z dużym potencjałem kolejnych plansz.

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

### Etap 0 — uzgodnienie zasady sterowania i porządek w katalogu

- Przyjąć ten dokument jako nadrzędną specyfikację.
- Przejrzeć otwarte PR-y dotyczące gier przed scaleniem; usunąć z nich sterowanie klawiaturą.
- Zaktualizować `docs/GAME_CONTROLS.md` dopiero po rzeczywistym wdrożeniu sterowania wyłącznie tensometrem.
- Dodać kontrolę CI zakazującą klawiaturowych i myszowych ścieżek sterowania w docelowych klasach gier.
- Przygotować macierz: gra → parametr ciągły → gest akcji → stan awaryjny.

**Warunek zakończenia:** każda z dziesięciu gier ma jednoznaczny projekt sterowania jednym sygnałem siły.

### Etap 1 — wspólne sterowanie i zgodność platform

- Wprowadzić wspólny model próbki, świeżości sygnału i gestów.
- Dodać testy kalibracji, normalizacji, filtracji, histerezy i rozpoznawania gestów.
- Zachować Qt 5 jako działający wariant bazowy i dodać rzeczywisty wariant budowania z Qt 6.
- Zastąpić rozproszone odwołania do `Qt5::...` kontrolowanym wyborem jednej wersji Qt na całe drzewo budowania. Nie wolno mieszać Qt 5 i Qt 6 w jednym celu.
- Rozszerzyć CI o osobne kompilacje i testy Qt 5 oraz Qt 6.
- Zachować C++17, istniejący silnik gry i zgodność z Arduino Nano/HX711.

**Warunek zakończenia:** oba warianty Qt budują aplikację i testy, a te same nagrane próbki dają identyczne zdarzenia sterujące.

### Etap 2 — dostosowanie dziesięciu istniejących gier

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
1. Precision Crane
2. Rhythm Force
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
