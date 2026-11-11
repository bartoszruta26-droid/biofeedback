# Instrukcja sterowania grami Biofeedback

> Dokument opisuje sterowanie zaimplementowane w gałęzi main. Jedynym urządzeniem wejściowym podczas rozgrywki jest tensometr/HX711. Klawiatura, mysz, ekran dotykowy i gamepad nie sterują grą. Działania specjalne są wywoływane gestami siły albo automatycznie przez zegar gry.

## Zasady wspólne

1. Wybierz grę w module treningowym i uruchom ją przyciskiem **Start**.
2. Jeżeli gra korzysta z tensometru/HX711, podłącz urządzenie i sprawdź, czy aplikacja odbiera aktualne próbki. Wartość siły jest interpretowana zgodnie z mechaniką konkretnej gry — nie każda gra używa jej w ten sam sposób.
3. Wszystkie ruchy, gesty i akcje rozgrywki muszą wynikać z jednego strumienia tensometru. W grach z automatycznym działaniem (np. ogień) świeży sygnał uzbraja automatykę; utrata sygnału ją zatrzymuje.
4. **Pauza**, **wznowienie**, **zatrzymanie** i **reset** są obsługiwane przez wspólny interfejs modułu treningowego/silnika gry, a nie przez uniwersalny skrót klawiszowy.
5. Jeżeli gra pokazuje komunikat o braku połączenia z czujnikiem, sprawdź połączenie urządzenia i strumień pomiarowy. Nie traktuj braku sygnału jako wartości zerowej ani jako polecenia wykonania ruchu.
6. Po zmianie gry lub ponownym uruchomieniu sprawdź, czy gra rozpoczęła nową sesję i czy wynik/życia zostały zresetowane zgodnie z mechaniką gry.
7. Silnik gier ma docelowy krok symulacji 120 Hz i żąda odświeżania w odstępach około 8 ms. Renderowanie używa lekkiej grafiki pikselowej, bez antyaliasingu i zbędnych gradientów pełnoekranowych. Rzeczywista liczba wyświetlonych klatek zależy od odświeżania monitora, kompozytora pulpitu i obciążenia Raspberry Pi 4; 90 Hz wymaga również odpowiedniego trybu wyświetlacza.
8. Automatyczne strzały w Archer i Scorched Earth wymagają świeżych próbek tensometru. Po utracie sygnału odliczanie jest wstrzymywane i rozpoczyna się ponownie po odzyskaniu pomiaru.

## Sterowanie poszczególnymi grami

### 1. Sinus Force Trainer

**Cel:** prowadź kursor położenia siły do ruchomego punktu celu, w miejscu pionowej linii przerywanej.

- **Tensometr/HX711:** jedyny kontroler. Skalibrowana siła 0–100% płynnie ustawia pionowe położenie kursora; brak świeżej próbki przez ponad 350 ms zatrzymuje naliczanie wyniku i zeruje serię.
- **Przebieg celu:** pięć ustawianych przed sesją trybów: sinusoidalny, trójkątny, łagodne plateau, piłokształtny i impulsowy. Obszar tolerancji podąża za wybranym przebiegiem.
- **Punktacja:** wynik rośnie za utrzymanie kursora w paśmie celu; dokładność zwiększa liczbę punktów, seria rośnie wraz z czasem trafienia, a utrzymanie dokładności co najmniej 90% przez 3 i 5 sekund uruchamia premie precyzji.
- **Rekord serii:** HUD pokazuje bieżącą oraz najlepszą serię w sesji.
- **Panel przed sesją:** wybór przebiegu celu, poziom trudności 1–5 oraz automatyczna zmiana trudności. Kalibracja zakresu siły jest odrębną procedurą pomiarową i nie jest częścią tego panelu. Podczas rozgrywki wejściem pozostaje wyłącznie tensometr.

### 2. Tan Force Trainer (Tangens)

**Cel:** dopasuj pionowe położenie kursora do punktu celu na wykresie tangensa, w miejscu pionowej linii przerywanej.

- **Tensometr:** reguluje położenie kursora w pionie. Koryguj nacisk, aby turkusowy kursor pokrywał się z punktem celu.
- **Asymptoty:** wykres tangensa zawiera nieciągłości. Przerwy na wykresie są zamierzone — nie oznaczają błędu renderowania. Nie próbuj „przeskakiwać” przez asymptotę gwałtownym ruchem siły.
- **Wskaźniki:** siła, poziom, tolerancja, wynik i oznaczenie trafienia.
- **Ustawienia:** poziom trudności, ostrość przebiegu, kalibrację i automatyczną trudność zmienia się w panelu ustawień. Podczas rozgrywki wejściem pozostaje wyłącznie tensometr.

### 3. Pong Force Trainer

**Cel:** odbijaj piłkę lewą paletką i zdobywaj punkty przeciwko paletce sterowanej przez komputer.

- **Tensometr:** zmienia pionowe położenie lewej paletki. Reguluj nacisk, aby ustawić paletkę na torze piłki.
- **Skalowanie:** wymiary paletek i piłki oraz prędkości są odniesione do logicznego pola gry 640×480. Rozdzielczość HD nie zmniejsza względnej szerokości paletki ani nie zwiększa względnej prędkości piłki.
- **Wskaźniki:** wynik gracza i przeciwnika, siła wejściowa oraz poziom trudności.
- **Sterowanie:** położenie lewej paletki wyznacza wyłącznie tensometr.
- **Punktacja:** punkt gracza jest naliczany po tym, gdy piłka minie stronę przeciwnika; odbicia od paletek i ścian nie są punktami.

### 4. Arkanoid

**Cel:** odbijaj piłkę paletką i rozbijaj wszystkie klocki.

- **Tensometr:** steruje poziomym położeniem paletki. Zmieniaj nacisk, aby przesuwać paletkę pod piłkę.
- **Wskaźniki:** wynik, poziom, liczba żyć i trudność.
- **Skalowanie:** paletka, piłka i klocki zachowują proporcje przy zmianie rozdzielczości.
- **Trudność:** prędkość piłki rośnie łagodnie, a paletka pozostaje szeroka również na ekranach HD.
- **Punktacja:** trafienie w klocek daje punkty; klocki wymagające kilku trafień mogą pozostać aktywne po pierwszym uderzeniu.
- **Sterowanie:** paletka jest sterowana wyłącznie tensometrem.

### 5. Flappy Bird

**Cel:** przeprowadź ptaka przez szczeliny między przeszkodami, unikając zderzeń.

- **Tensometr:** krótki nacisk wywołuje skok ptaka. Skok jest wyzwalany przejściem z braku nacisku do nacisku, a nie samym ciągłym utrzymywaniem sygnału.
- **Praktyka:** naciskaj krótko i rytmicznie. Długie przytrzymanie nie powinno być traktowane jako seria niezależnych skoków.
- **Wskaźniki:** wynik, siła, poziom, prędkość gry i liczba miniętych przeszkód.
- **Sterowanie:** skok jest wywoływany wyłącznie impulsem tensometru. Każdy nowy skok wymaga zwolnienia nacisku i ponownego przekroczenia progu aktywacji. Klawiatura i mysz nie wywołują skoku.

### 6. Mario Run

<!-- Aktualizacja dokumentacji po wyrównaniu bazowej regresji silnika 120 Hz. -->

**Cel:** przebiegaj kolejne poziomy, omijaj przeszkody, pokonuj przeciwników, zbieraj monety i premie. Sceneria przewija się wielowarstwowo, a tempo oraz częstotliwość przeciwników rosną wraz z poziomem.

- **Tensometr/HX711:** jedyny kontroler. Nacisk co najmniej 30% rozpoczyna ładowanie skoku, spadek poniżej 20% wykonuje skok; ponowne przekroczenie progu w powietrzu wykonuje pojedynczy podwójny skok. Histereza chroni przed drganiami progu.
- **Goombasy:** chodzą po podłożu. Lądowanie na nich od góry podczas opadania eliminuje przeciwnika i daje 200 punktów; kontakt z boku odbiera życie.
- **Rośliny piranie:** okresowo wysuwają się z rur. Aktywna roślina odbiera życie przy kontakcie.
- **Życia i ochrona:** trzy życia na start; trafienie odbiera jedno życie i uruchamia 1,6 s ochrony. Po utracie ostatniego życia rozgrywka kończy się.
- **Monety:** złote monety nad trasą dają 50 punktów, a seria zbiorów zwiększa premię do 150 punktów.
- **Gwiazda:** rzadka premia daje 5 s nietykalności i dodatkowe punkty.
- **Otoczenie:** parallax gór, wzgórz, drzew i podłoża; animowane chmury, ptaki, słońce, monety i gwiazda.
- **Wskaźniki:** wynik, życia, monety, poziom, postęp do kolejnego poziomu, pasek ładowania i czas ochrony.
- **Sterowanie:** tylko tensometr/HX711; klawiatura, mysz i ekran dotykowy nie sterują rozgrywką.

### 7. Car Race

**Cel:** prowadź samochód przez trzy pasy, omijaj ruch uliczny, utrzymuj serię bezkolizyjnych minięć i rozsądnie wykorzystuj dopalacz.

- **Tensometr:** wybiera pas na podstawie wartości siły: niska wartość — lewy pas, środkowa — pas środkowy, wysoka — prawy pas. Sygnał jest filtrowany, a progi histerezy ograniczają przypadkowe przełączanie przy wahaniach pomiaru.
- **Dopalacz gestem:** przytrzymaj siłę na poziomie co najmniej 95% przez 0,25–1,2 s, a następnie zwolnij do 60% lub mniej. Samochód zachowuje dotychczasowy pas w trakcie gestu; po jego zakończeniu zwykłe mapowanie siły ponownie wybiera pas. Dopalacz działa przez 1,8 s i ma 5 s odnowienia.
- **Płynność kierowania:** samochód wizualnie dojeżdża do wybranego pasa z krótką animacją. Pozycja kolizji nadal odpowiada wybranemu pasowi.
- **Dopalacz:** przez 1,8 s zwiększa prędkość ruchu planszy. Po użyciu obowiązuje 5 s odnowienia; pasek HUD pokazuje stopień gotowości. Animowany płomień za samochodem sygnalizuje aktywne przyspieszenie.
- **Punktacja:** punkty są przyznawane za pokonany dystans oraz za bezpieczne minięcie pojazdu. Kolejne bezkolizyjne minięcia budują serię i zwiększają premię punktową. Kolizja zeruje serię.
- **Żetony dopalacza:** co kilka pojazdów pojawia się złoty żeton. Zebranie go daje 25 punktów i skraca czas odnowienia dopalacza o 2 sekundy. Żetony trzeba zebrać, przejeżdżając przez ich pas.
- **Kolizje:** zderzenie odbiera jedno życie. Po uderzeniu działa ochrona przez 1,1 s; miganie samochodu i lekkie drżenie obrazu sygnalizują trafienie.
- **Animacja i perspektywa:** tło ma kilka warstw przesuwających się z różną prędkością. Odległe pasma krajobrazu poruszają się wolniej niż drzewa przy drodze; znaczniki pobocza i przerywane linie pasa przesuwają się w rytmie prędkości, a ich rozmiar rośnie ku dolnej krawędzi ekranu. Koła, zawieszenie, światła pojazdów i płomień dopalacza są animowane.
- **Wskaźniki:** wynik, dystans, prędkość, liczba żyć, poziom trudności, liczba zebranych żetonów, seria minięć i pasek gotowości dopalacza. Po utracie wszystkich żyć pojawia się ekran końca gry.
- **Wskazówka:** planuj zmianę pasa z wyprzedzeniem. Nie zmieniaj pasa w ostatniej chwili, gdy pojazd jest już blisko dolnej części ekranu.

### 8. Archer Force Trainer

**Cel:** ćwicz precyzję celowania, przewidywanie toru balistycznego i synchronizację strzału z ruchem celu.

- **Tensometr/HX711:** jest jedynym kontrolerem celowania. Wartość siły 0–100% ustawia kąt łuku; nie ma alternatywnego sterowania klawiaturą ani myszą.
- **Automatyczny strzał:** po uzyskaniu świeżego sygnału pierwszy strzał następuje po 2,4 s, a następne co 2,4 s. Po utracie sygnału odliczanie zostaje wyzerowane; klawisz spacji nie wyzwala strzału.
- **Tor lotu:** kropkowana linia pokazuje przewidywany tor strzały z uwzględnieniem grawitacji i bieżącego kąta. Jest wskazówką, a poruszający się cel może zmienić położenie przed trafieniem.
- **Strefy celu:** zewnętrzna część tarczy daje 10 punktów, środkowa 20, a środek 30. Trafienie zwiększa prędkość celu, maksymalnie do 150 jednostek na sekundę.
- **Statystyki:** wynik, kąt celowania, łączna liczba oddanych strzałów, trafień, pudeł i strzał w locie.
- **Ograniczenia:** jednocześnie może istnieć maksymalnie 8 aktywnych strzał. Strzała, która opuści planszę, jest liczona jako pudło.
- **Praktyka:** obserwuj kropkowany tor lotu i ruch celu. Utrzymuj tensometr na wartości odpowiadającej potrzebnemu kątowi; strzał zostanie oddany automatycznie po upływie czasu.

### 9. Galaga

**Cel:** steruj statkiem, niszcz kolejne formacje przeciwników, zbieraj ulepszenia i unikaj wrogiego ostrzału.

- **Tensometr:** ustawia poziome położenie statku w obrębie planszy. Wartość 0–100% odpowiada położeniu od lewej do prawej krawędzi.
- **Automatyczny ogień:** przy świeżym sygnale tensometru statek oddaje strzał co 0,30 s. Po utracie sygnału ogień zostaje zatrzymany.
- **Impuls EMP gestem:** przytrzymaj co najmniej 95% siły przez 0,35–1,2 s, a następnie zwolnij do 30% lub mniej. Gest zużywa jeden ładunek EMP. Na początku sesji dostępne są 2 ładunki; po ukończeniu fali przywracany jest 1, maksymalnie do 4.
- **Efekt EMP:** usuwa aktywne pociski przeciwników i zadaje jedno uszkodzenie każdemu aktywnemu przeciwnikowi. Ładunki są ograniczone; ich stan jest widoczny w HUD.
- **Ulepszenia:** co piąty pokonany przeciwnik może pozostawić premię. Niebieska premia daje potrójny strzał, zielona uruchamia osłonę, a żółta zwiększa szybkostrzelność. Premie zbiera się przez zetknięcie statku z opadającym symbolem.
- **Punktacja i seria:** kolejne eliminacje w krótkim odstępie zwiększają premię punktową. Seria wygasa po przerwie.
- **Trudność:** następne fale zwiększają liczebność i wytrzymałość części przeciwników oraz tempo ostrzału. Ukończenie fali daje premię punktową i dodatkowy ładunek EMP.
- **Ochrona:** po trafieniu statek otrzymuje krótką ochronę przed kolejnym uszkodzeniem. Zebrana osłona pochłania następne trafienie.
- **Wskaźniki:** wynik, numer fali, życia, ładunki EMP, seria eliminacji i aktywne ulepszenie.

### 10. Scorched Earth

**Cel:** w turowej rozgrywce artyleryjskiej traf przeciwnika, ustawiając kąt działa tensometrem i uwzględniając wiatr.

- **Tensometr:** jest jedynym kontrolerem kąta działa. Siła strzału jest stała i zbalansowana; żadna klawiatura, mysz ani ekran dotykowy nie sterują rozgrywką.
- **Automatyczny strzał:** świeży sygnał tensometru uzbraja odliczanie 2 s w turze gracza; po utracie sygnału odliczanie jest zerowane. Siła i kąt pocisku zależą od ustawionej mechaniki gry i aktualnej próbki siły.
- **Wskaźniki:** zdrowie obu czołgów, runda, wynik i aktualny wiatr.
- **Wskazówka:** wiatr wpływa na tor pocisku. Po strzale poczekaj na zakończenie lotu i ruch przeciwnika; następna tura gracza rozpoczyna nowe dwusekundowe odliczanie.

## Skrócona tabela sterowania

| Gra | Tensometr | Automatyka / dodatkowe sterowanie |
|---|---|---|
| Sinus | pionowe położenie kursora względem celu | brak skrótów gry |
| Tangens | pionowe położenie kursora względem celu | brak skrótów gry |
| Pong | pionowe położenie lewej paletki | brak skrótów gry |
| Arkanoid | poziome położenie paletki | brak skrótów gry |
| Flappy Bird | impuls skoku po przekroczeniu progu | brak dodatkowego wejścia |
| Mario Run | przytrzymanie ładuje skok, zwolnienie wykonuje skok | brak skrótów skoku |
| Car Race | wybór jednego z trzech pasów | gest 95% i zwolnienie — dopalacz |
| Archer | kąt celowania 0–100% | automatyczny strzał co 2,4 s |
| Galaga | pozioma pozycja statku | automatyczny ogień, gest 95% i zwolnienie — EMP |
| Scorched Earth | kąt działa | automatyczny strzał po 2 s |
| Precision Crane | aktualna oś: pozycja wózka lub wysokość haka | gesty chwytu/zwolnienia i zmiany osi |
| Reaction Force | impuls siły w paśmie celu | czas reakcji, automatyczny harmonogram bodźców |

## Uwaga o wersjach i rozszerzeniach

Wszystkie parametry konfiguracyjne (np. trudność, kalibracja, tryb fali i preferowana ręka) należy zmieniać w interfejsie ustawień poza samą rozgrywką. Żadna gra nie może wymagać klawiatury ani myszy do wykonania ruchu lub akcji.

Przy zmianie sterowania w kodzie należy zaktualizować ten plik oraz teksty instruktażowe wyświetlane bezpośrednio w grze. Każdy skrót powinien mieć test regresyjny, a opis powinien odpowiadać faktycznej implementacji.


### 11. Precision Crane — Precyzyjny dźwig

**Cel:** przenieś ładunek z miejsca odbioru do oznaczonej strefy i odłóż go precyzyjnie.

- **Jedyny kontroler:** jeden tensometr/HX711, skalibrowany do zakresu siły 0–100%.
- **Ruch poziomy i pionowy:** roboczy zakres 0–70% siły steruje płynnie aktualnie wybraną osią. Oś pozioma przesuwa wózek suwnicy, a pionowa reguluje wysokość haka.
- **Zmiana osi:** przytrzymaj siłę co najmniej 82% przez minimum 0,85 s i zwolnij do 18% lub mniej. Zmienia się aktywna oś.
- **Chwyt i zwolnienie:** impuls siły co najmniej 82% trwający od 0,15 do 0,85 s, zakończony zwolnieniem do 18% lub mniej, przełącza chwyt/zwolnienie. Hak musi znajdować się w pobliżu ładunku, aby go podnieść.
- **Punktacja:** poprawna dostawa daje punkty i zwiększa serię. Zwolnienie poza strefą celu zużywa jedną z trzech prób. Co cztery dostawy wzrasta trudność.
- **Brak sygnału:** po 350 ms bez świeżej próbki sterowanie jest wstrzymywane. Klawiatura, mysz, dotyk i gamepad nie sterują rozgrywką.

### 12. Reaction Force — Refleks siły

**Cel:** reaguj na sygnał wizualny, rozpoczynając krótki impuls i osiągając wyświetlony zakres siły.

- **Jedyny kontroler:** jeden tensometr/HX711, przekazujący skalibrowaną siłę 0–100% przez wspólny potok pomiarowy. Gra nie otwiera portu szeregowego.
- **Uzbrojenie:** po rozpoczęciu gry zwolnij nacisk do 18% lub mniej. Po pierwszej próbce neutralnej pojawia się faza przygotowania.
- **Reakcja:** rozpocznij impuls po wyświetleniu komunikatu „TERAZ!”. Przekroczenie 22% rozpoczyna impuls; powrót do 18% lub mniej kończy go.
- **Siła docelowa:** przed próbą widoczna jest wartość celu i dopuszczalne pasmo. Szczyt impulsu musi znaleźć się w tym zakresie.
- **Czas impulsu:** prawidłowy impuls trwa od 70 do 600 ms. Zbyt krótki lub zbyt długi nacisk jest pudłem.
- **Czas reakcji:** do 250 ms — „Perfekt”; do 450 ms — „Dobrze”; późniejsza reakcja w oknie próby — „Trafienie”. Po upływie okna próba jest zaliczana jako pudło.
- **Falstart:** impuls przed sygnałem „TERAZ!” przerywa serię i wymaga ponownego zwolnienia tensometru do strefy neutralnej.
- **Runda:** 20 prób. Pięć poziomów trudności zawęża pasmo siły i skraca okno reakcji.
- **Wskaźniki:** wynik, numer próby, bieżąca seria, trafienia, pudła, aktualna siła, siła docelowa i czas reakcji.
- **Brak sygnału:** po 350 ms bez świeżej próbki aktywny gest jest anulowany, a runda wstrzymana. Po odzyskaniu sygnału wymagane jest ponowne uzbrojenie neutralną próbką.
- **Ograniczenie interpretacji:** wynik gry nie jest klinicznym pomiarem czasu reakcji ani diagnozą.

