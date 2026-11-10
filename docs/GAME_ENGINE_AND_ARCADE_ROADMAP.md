# 🎮 Plan rozwoju silnika gier i modułu arcade

> 🕒 Dokument audytowy / planistyczny • Wnioski i statusy odnoszą się do wskazanej daty, gałęzi i commita; nie są automatycznie bieżącym stanem main.

## Zakres wymagany

### Gry do utrzymania i poprawy
- Sinus Force Trainer
- Tan Force Trainer
- Pong Force Trainer
- Flappy Bird
- Mario Run
- Arkanoid

### Gry do zaimplementowania
- Galaga
- Archer
- Scorched Earth (w pierwotnym zgłoszeniu zapisano „Scored Earth”)
- Car Race

## Zasady architektury

1. Wszystkie gry korzystają ze wspólnego `GameEngine`: jeden zegar monotoniczny, stały krok symulacji, ograniczona praca nadrabiania czasu, wspólny cykl życia i statystyki.
2. Dane czujnika są wejściem sterującym, a nie bezpośrednim dostępem do stanu gry z wątku portu szeregowego. Zmiany stanu widgetów wykonuje wyłącznie wątek GUI.
3. Strumienie czujnika o dużej częstotliwości należy scalać do najnowszej próbki, aby kolejka zdarzeń Qt nie rosła szybciej niż interfejs potrafi ją obsłużyć.
4. Każda gra musi obsługiwać start, pauzę, wznowienie, zatrzymanie, reset, zmianę rozmiaru okna, brak sensora i nieprawidłowe dane wejściowe.
5. Ruch i kolizje powinny być niezależne od częstotliwości odświeżania GUI. Długie zatrzymanie aplikacji nie może powodować nieograniczonej symulacji nadrabiającej.
6. Wynik, poziom, życia, błędy i czas muszą być spójne z mechaniką konkretnej gry oraz wspólnymi statystykami silnika.

## Wymagania funkcjonalne dla nowych gier

- **Galaga:** statek gracza, fale przeciwników, pociski, kolizje, życia, punkty, narastająca trudność i sterowanie ruchem z czujnika.
- **Archer:** celowanie sterowane czujnikiem, łuk i strzały, ruchome cele, wykrywanie trafień, punkty i poziomy trudności.
- **Scorched Earth:** artyleria, regulacja kąta/siły strzału, trajektoria balistyczna, wpływ wiatru, teren i cele; fizyka musi używać stałego kroku.
- **Car Race:** sterowanie pasem/pozycją pojazdu, ruch ulicy i przeszkód, kolizje, punkty za dystans oraz stopniowanie trudności.

## Plan walidacji

- Testy jednostkowe cyklu życia silnika, zegara, pauzy, resetu, ograniczenia kroku i wejścia czujnika.
- Testy fabryki potwierdzające utworzenie każdej gry po wszystkich obsługiwanych aliasach identyfikatorów.
- Testy scenariuszowe: kolizja, zdobycie punktu, utrata życia, zmiana poziomu, zakończenie i reset.
- Testy bez sensora, z wartościami NaN/Inf, bardzo dużymi wartościami i gwałtownymi zmianami wejścia.
- Pełna kompilacja Qt5 oraz uruchomienie testów w trybie `offscreen`.
- Testy sprzętowe z rzeczywistym Arduino/HX711 jako osobny etap; nie są zastępowane przez CI.

## Stan prac

- [x] Scalanie PR #208 do `integration/all-work`.
- [x] Ograniczenie liczby oczekujących zdarzeń wejścia czujnika przez scalanie próbek do najnowszej wartości.
- [x] `GameEngine::reset()` odtwarza również stan gry pochodnej; `start()` korzysta z tej samej ścieżki, bez podwójnego wywołania inicjalizacji. Test regresyjny sprawdza dokładną liczbę inicjalizacji po starcie, pauzie, resecie i ponownym uruchomieniu.
- [x] Mario: inicjalizacja chmurek zabezpieczona dla widoku o zerowym rozmiarze; test resetu przed ustawieniem rozmiaru widgetu oraz podczas sesji.
- [x] Mario: zabezpieczono wszystkie losowania położenia chmurek dodatnią granicą również podczas `initialize()` i aktualizacji po zmianie rozmiaru; poprawka statyczna, oczekuje na CI.
- [x] Regresja dla serii 1000 próbek wysłanych z wątku roboczego.
- [ ] Audyt i poprawki mechaniki istniejących gier (w pierwszej kolejności Arkanoid, Pong, Flappy Bird i Mario).
- [x] Zabezpieczenia wejściowego deltaTime w Arkanoidzie, Flappy Bird, Mario, Sinus Force Trainer i Tan Force Trainer; normalizacja wektora odbicia piłki w Pongu.
- [x] Sinus/Tan Force Trainer: odrzucanie NaN/Inf w próbkach siły, kontrola dodatniego i skończonego kroku czasu oraz ograniczenie pojedynczej aktualizacji do 100 ms.
- [x] Testy regresyjne wejścia i deltaTime dla Sinus/Tan: bardzo duża delta, NaN, Inf, wartości ujemne oraz uszkodzone próbki czujnika.
- [x] Pierwsza grywalna implementacja Galaga: statek, sterowanie czujnikiem/klawiaturą, pociski obu stron, kolizje, życia, fale i punktacja. Pozostaje rozszerzyć testy scenariuszowe i zbalansować rozgrywkę.
- [x] Pierwsza grywalna implementacja Archer: regulacja kąta z czujnika/klawiatury, strzały balistyczne, ruchomy cel i punktacja. Pozostaje rozszerzyć testy scenariuszowe i dopracować rozgrywkę.
- [x] Pierwsza grywalna implementacja Scorched Earth: artyleria turowa, kąt i siła strzału, grawitacja, wiatr, deformacja terenu, zdrowie czołgów i przeciwnik AI. Pozostaje rozszerzyć testy scenariuszowe, dopracować AI i zbalansować fizykę.
- [x] Pierwsza grywalna implementacja Car Race: trzy pasy, sterowanie czujnikiem/klawiaturą, przeszkody, kolizje, życia, dystans, wynik i rosnąca prędkość. Pozostaje rozszerzyć testy scenariuszowe i dopracować rozgrywkę.
- [x] Rejestracja nowych gier w GameFactory oraz udostępnienie ich w selektorach TrainingTab i OutlineTab.
- [x] Archer, Car Race i Scorched Earth: obsługa zmiany rozmiaru aktywnego widoku; zachowanie/ponowne dopasowanie celu, przeszkód, terenu, czołgów i pocisków.
- [x] Galaga: skalowanie statku, przeciwników i pocisków podczas zmiany rozmiaru; ograniczenie pozycji aktorów do nowego widoku.
- [x] Galaga: czasowa ochrona po trafieniu, czyszczenie wrogich pocisków bez unieważniania iteracji oraz pojedyncza utrata życia przy zejściu formacji; fala jest odtwarzana dopiero po rzeczywistej utracie życia.
- [x] Galaga: kolizje pocisków sprawdzają cały przebyty prostokąt (swept collision), aby szybkie pociski nie przenikały przez przeciwników ani statek między klatkami.
- [x] Testy zmiany rozmiaru Galaga i Car Race z aktywnymi aktorami/przeszkodami.
- [x] Car Race: test scenariuszowy potwierdza utratę życia po kolizji z pierwszą przeszkodą na deterministycznie wybranym pasie oraz odtworzenie trzech żyć i pustej listy przeszkód przez reset; klasa udostępnia wyłącznie do odczytu liczbę pozostałych żyć i przeszkód na potrzeby diagnostyki/testów.
- [x] Car Race: scenariusz trzech kolejnych kolizji na pasach środkowym, lewym i prawym potwierdza utratę trzech żyć i zatrzymanie gry po trzeciej kolizji; CI dla `98595bd8a3dd55ad0a4f2a267bac49f403cf044a` zakończyło się powodzeniem.
- [x] Galaga: testuje początkową liczbę żyć, pierwszą falę (24 przeciwników), zachowanie gry po aktywnej rozgrywce i odtworzenie stanu początkowego przez reset.
- [x] Archer: test potwierdza trafienie i wzrost prędkości celu (narastanie trudności), a osobny scenariusz pudła sprawdza usunięcie nieaktywnej strzały bez naliczenia punktów.
- [x] Scorched Earth: test stanu początkowego, resetu i rzeczywistego trafienia czołgu; CI dla `98595bd8a3dd55ad0a4f2a267bac49f403cf044a` zakończyło się powodzeniem. Poprzedni przebieg wykazał, że tor 38,5° mijał rozszerzony obszar kolizji, a zmiana rozmiaru w trakcie lotu zmieniała warunki fizyki. Test skorygowano do 40° i przeniesiono zmianę rozmiaru za rozstrzygnięcie strzału. Bezpośrednie testy przejścia rundy po zniszczeniu czołgu i regeneracji pozostają do dopisania.
- [x] Testy regresyjne zmiany rozmiaru podczas aktywnej rozgrywki Archer i Scorched Earth; Car Race ma już test małego widoku podczas gry.
- [x] Galaga: dodano scenariusz trafienia pociskiem wroga z pozycji prawego skraju sensora; test oczekuje utraty dokładnie jednego życia i dalszego działania gry.
- [x] Galaga: test odczytuje pozostały czas ochrony po trafieniu, potwierdza jego dodatnią wartość i zmniejszanie się w czasie bez utraty kolejnego życia. Dodano wyłącznie diagnostyczny getter `hitProtectionRemaining()`.
- [ ] Galaga: osobny test dwóch pocisków trafiających w jednym oknie ochronnym, aby bezpośrednio potwierdzić blokadę wielokrotnej utraty życia.
- [ ] Scorched Earth: bezpośrednie testy przejścia rundy po zniszczeniu czołgu, przywrócenia zdrowia przeciwnika i leczenia gracza.
- [x] Flappy Bird: dodano obsługę zmiany rozmiaru aktywnej gry; pozycja i prędkość pionowa ptaka, szerokość przeszkód, geometria luk oraz położenia istniejących przeszkód są przeskalowywane, a pozycja ptaka ograniczana do nowego widoku.
- [x] Flappy Bird: dodano scenariusz regresyjny zmiany rozmiaru 640×480 → 180×120 → 640×480 podczas aktywnej sesji; oczekuje na kompilację i testy.
- [x] Mario: dodano obsługę zmiany rozmiaru aktywnej gry, skalując postać, prędkość pionową, przeszkody i chmurki oraz ograniczając położenia dekoracji do nowego widoku.
- [x] Mario: dodano scenariusz regresyjny zmiany rozmiaru 640×480 → 180×120 → 640×480 podczas aktywnej sesji; oczekuje na kompilację i testy.
- [ ] Dalszy audyt mechaniki istniejących gier (Arkanoid, Pong, Flappy Bird i Mario), w tym kolizji i zachowania przy zmianie rozmiaru.
- [x] CI dla commitu `98595bd8a3dd55ad0a4f2a267bac49f403cf044a`: 6/6 workflow zakończonych powodzeniem (Linux desktop GUI build, security regression i cztery polityki serial).
- [ ] Scenariusz Galaga dodany w `1749b9d`, diagnostyczny getter w `9be675a`, a asercje czasu ochrony w `4367ea0`; wszystkie te zmiany oczekują na CI i nie były kompilowane/testowane.
- [ ] Poprawka granic losowania chmurek Mario w `d156a81`; nie uruchamiano kompilacji ani testów zgodnie z bieżącym zakresem prac.
- [ ] Obsługa zmiany rozmiaru Flappy Bird i test regresyjny w commitach `6434ac0`, `780467c`, `8c3cb57`; zmiany nie były kompilowane ani testowane.
- [ ] Obsługa zmiany rozmiaru Mario i test regresyjny w commitach `715b736`, `813d82d`, `0343bac`; zmiany nie były kompilowane ani testowane.
- [x] Pong: kolizja z paletkami uwzględnia cały obrys piłki oraz kierunek jej ruchu, dzięki czemu kontakt przy krawędzi paletki nie zależy wyłącznie od położenia środka piłki. Commit: `fb32a1a7b1fc97ddc9ceac5949457389ac3587f8`; oczekuje na kompilację i testy.
- [x] Flappy Bird: po zmianie rozmiaru ograniczono promień ptaka i geometrię luk istniejących przeszkód do nowego widoku, także dla małych i wąskich okien. Commit: `1a6da0998e24e6886971e241a25b2e4d2cca0370`; oczekuje na kompilację i testy.
- [ ] Statyczny przegląd poprawek Pong i Flappy Bird zakończony; zachowanie kolizji i resize wymaga potwierdzenia testami po wznowieniu walidacji.
- [x] Arkanoid: prostokąt kolizji piłki jest wyznaczany po ewentualnym odbiciu od paletki, a więc dalsza detekcja klocków korzysta z aktualnej pozycji piłki, nie z nieaktualnego obrysu sprzed korekty. Commit: `2aa83f96acffccf6f48028f951a7e96021edd536`; oczekuje na kompilację i testy.
- [x] Galaga: generowanie formacji przeciwników uwzględnia szerokość i wysokość widoku; w małych oknach liczba kolumn/wierszy i rozmiar aktorów są ograniczane, aby nowa fala nie nakładała się ani nie wychodziła poza ekran. Commit: `218a66fd1419988583b138a8ac82ae7ca0da51cb`; oczekuje na kompilację i testy.
- [ ] Galaga: bezpośrednia regresja dwóch trafień w jednym oknie ochronnym nadal wymaga scenariusza testowego bez dodawania sztucznego publicznego interfejsu do zadawania obrażeń.
- [ ] Scorched Earth: przejście rundy po zniszczeniu czołgu i leczenie gracza wymagają dalszego testu scenariuszowego; nie dodawać produkcyjnych metod wyłącznie na potrzeby testów.
- [ ] Testy scenariuszowe i pełna walidacja CI dla wszystkich gier po kolejnych zmianach.

Wszystkie prace prowadzić na gałęziach opartych o `integration/all-work`. Nie wprowadzać zmian bezpośrednio do `main`.


### Dalszy audyt i poprawki przed walidacją CI
- [x] Pong: pozycja kontaktu z paletką jest ograniczana do przedziału 0–1, dzięki czemu trafienie obrysem piłki przy górnej/dolnej krawędzi nie generuje kąta odbicia spoza zakresu paletki. Punkt jest przyznawany dopiero po całkowitym opuszczeniu pola gry przez piłkę. Commit: `18e77b16913aa400764626b09f2c4d52028b2a8a`.
- [x] Flappy Bird: parametry bazowej trudności i wysokości luk nie są już trwale zmniejszane podczas zmiany rozmiaru okna. Istniejące przeszkody nadal są dopasowywane do nowego widoku, a nowe luki są ograniczane do dostępnej wysokości. Commit: `4e2995d88603db25333a359234d019d950010f7e`.
- [ ] Pełna kompilacja Qt5 i testy regresyjne po powyższych poprawkach; wynik CI należy przypisać wyłącznie do aktualnego SHA, nie do wcześniejszych przebiegów.
- [x] CI wykryło błąd kompilacji w deklaracji `QResizeEvent`: forward declaration w `FlappyBirdGame.hpp` znajdowała się wewnątrz przestrzeni nazw `games`, przez co nagłówki pozostałych gier rozpoznawały błędny typ `games::QResizeEvent`. Deklarację przeniesiono do przestrzeni globalnej. Commit: `dd13f6b61aa34f798f6f13cfdc15d79128ca4ac3`. Poprzednie przebiegi CI dla SHA `8cd5cd5` zakończyły się błędem kompilacji; ponowna walidacja wymaga potwierdzenia dla nowego SHA.
- [x] Drugi przebieg kompilacji wykazał niejednorodne typy argumentów Qt `qBound/qMax` (float/double) w nowym ograniczeniu kąta Pong oraz istniejącym przeliczaniu luki Flappy Bird. Ujednolicono obliczenia do `double` przed konwersją wyniku do stanu gry. Commity: `57b28c12f35891bba176299064b820b2a24f21c5`, `f5b64697f2d87c863ddcff420683fb465998f214`. Wymagana ponowna kompilacja aktualnego HEAD.
- [x] Test `game_factory_regression` wykrył przypadek zmiany rozmiaru ukrytego widgetu: Qt może przekazać nieprawidłowy `oldSize`, przez co pozycja ptaka była jedynie przycinana do nowego widoku (w małym oknie tuż przy ziemi). Flappy Bird przechowuje teraz ostatni znany rozmiar logiczny, ustawia go podczas inicjalizacji, a przy braku poprawnego rozmiaru odtwarza proporcje pozycji początkowej. Usunięto też skalowanie bazowych parametrów luk, aby trudność nie zależała od historii zmian okna. Commity: `c4d4031e6649f3e667139fd4f41d2508e81a91e3`, `9b0cd8cd0ac1368f7773b27c06e64fc7d974dc93`. Wymagane ponowne uruchomienie kompilacji i testów.
- [x] Kolejny przebieg potwierdził, że test nadal nie przechodzi, ponieważ ukryty widget może w ogóle nie otrzymać `resizeEvent()` przed następną klatką. Przeniesiono dopasowanie geometrii do wspólnej metody `adaptToSize()`, wywoływanej zarówno z `resizeEvent()`, jak i defensywnie z `update()` przy wykryciu zmiany rzeczywistych wymiarów. Dzięki temu zmiana rozmiaru jest obsługiwana również dla niewidocznych widgetów. Commity: `fd30a1479c076130080ec07c086f36216da5f2e4`, `4df128b2ea3478bfb3c1a315950a1a2b63bc9eae`. Oczekuje na ponowną kompilację i testy.

### Arkanoid Force Trainer — audyt funkcjonalności i poprawki (2026-11-10)

- Dodano kontrolę aktualności wejścia: po 500 ms bez próbki tensometru symulacja zostaje zamrożona, a gra wyświetla stan oczekiwania. Brak danych nie powoduje ukrytej utraty życia.
- Wejście siły jest walidowane, ograniczane do 0–100% i wygładzane wykładniczo; pozycja paletki jest interpolowana niezależnie od częstotliwości próbkowania.
- Piłka pozostaje przy paletce podczas przygotowania serwisu. Serwis następuje po krótkim odliczaniu; możliwe jest także ręczne uruchomienie spacją, Enterem lub kliknięciem.
- Po utracie życia kolejny serwis wymaga świeżego sygnału i krótkiego czasu przygotowania. Koniec gry następuje po utracie trzeciego życia.
- Zmieniono układ klocków na responsywny względem szerokości i wysokości pola gry; zachowywane są proporcje położenia piłki, paletki i klocków przy zmianie rozmiaru okna.
- HUD pokazuje wynik, poziom, życia, siłę i stan czujnika bez zasłaniania pola gry.
- Rozszerzono `game_factory_regression` o oczekiwanie na sygnał, ograniczanie i wygładzanie siły, sterowanie paletką, trafienie klocka, zamrożenie po utracie sygnału oraz zmianę rozmiaru.
- Poprawki wymagają kompilacji Qt5 i pełnego CI dla aktualnego HEAD. Testy symulacyjne nie zastępują sprawdzenia z Arduino Nano/HX711.


- [x] Dodatkowo naprawiono błąd kompilacji testów: test wywoływał chronioną metodę update() bezpośrednio; zastąpiono to obserwacją rzeczywistej pętli Qt.
- [x] Dodano sterowanie klawiaturą lewo/prawo (A/D) jako tryb awaryjny, działający także po utracie świeżości sygnału tensometru.
- [x] Dodano regresję potwierdzającą ruch paletki klawiaturą po przekroczeniu limitu czasu próbek sensora.
