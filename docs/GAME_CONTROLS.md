# Instrukcja sterowania grami Biofeedback

> Dokument opisuje sterowanie zaimplementowane w aktualnej gałęzi integracyjnej Biofeedback. Tensometr jest kontrolerem położenia/kąta w grach, które go obsługują; strzały w Archer i Scorched Earth są wyzwalane automatycznie przez zegar gry. Skróty klawiaturowe odnoszą się wyłącznie do gier, w których są jawnie wymienione.

## Zasady wspólne

1. Wybierz grę w module treningowym i uruchom ją przyciskiem **Start**.
2. Jeżeli gra korzysta z tensometru/HX711, podłącz urządzenie i sprawdź, czy aplikacja odbiera aktualne próbki. Wartość siły jest interpretowana zgodnie z mechaniką konkretnej gry — nie każda gra używa jej w ten sam sposób.
3. Sterowanie klawiaturą działa, gdy widok gry ma fokus. Kliknij obszar gry, jeżeli klawisze nie reagują.
4. **Pauza**, **wznowienie**, **zatrzymanie** i **reset** są obsługiwane przez wspólny interfejs modułu treningowego/silnika gry, a nie przez uniwersalny skrót klawiszowy.
5. Jeżeli gra pokazuje komunikat o braku połączenia z czujnikiem, sprawdź połączenie urządzenia i strumień pomiarowy. Nie traktuj braku sygnału jako wartości zerowej ani jako polecenia wykonania ruchu.
6. Po zmianie gry lub ponownym uruchomieniu sprawdź, czy gra rozpoczęła nową sesję i czy wynik/życia zostały zresetowane zgodnie z mechaniką gry.

## Sterowanie poszczególnymi grami

### 1. Sinus Force Trainer

**Cel:** prowadź kursor na wysokości punktu celu na sinusoidzie, w miejscu pionowej linii przerywanej.

- **Tensometr:** zmiana siły zmienia pionowe położenie kursora. Zwiększaj lub zmniejszaj nacisk, aby dopasować kursor do czerwonego punktu celu.
- **Wskaźnik:** turkusowe kółko pokazuje położenie gracza, czerwony punkt położenie celu; zielone oznaczenie sygnalizuje trafienie.
- **Informacje na ekranie:** bieżąca siła, tolerancja w pikselach, poziom trudności i status połączenia.
- **Klawiatura:** w tej wersji nie ma odrębnych skrótów gry do regulacji siły ani fazy. Start/pauza/reset są dostępne w interfejsie treningowym.

### 2. Tan Force Trainer (Tangens)

**Cel:** dopasuj pionowe położenie kursora do punktu celu na wykresie tangensa, w miejscu pionowej linii przerywanej.

- **Tensometr:** reguluje położenie kursora w pionie. Koryguj nacisk, aby turkusowy kursor pokrywał się z punktem celu.
- **Asymptoty:** wykres tangensa zawiera nieciągłości. Przerwy na wykresie są zamierzone — nie oznaczają błędu renderowania. Nie próbuj „przeskakiwać” przez asymptotę gwałtownym ruchem siły.
- **Wskaźniki:** siła, poziom, tolerancja, wynik i oznaczenie trafienia.
- **Klawiatura:** w wersji bazowej nie ma osobnych skrótów do regulacji siły lub przebiegu. Sterowanie sesją odbywa się w interfejsie treningowym.

### 3. Pong Force Trainer

**Cel:** odbijaj piłkę lewą paletką i zdobywaj punkty przeciwko paletce sterowanej przez komputer.

- **Tensometr:** zmienia pionowe położenie lewej paletki. Reguluj nacisk, aby ustawić paletkę na torze piłki.
- **Wskaźniki:** wynik gracza i przeciwnika, siła wejściowa oraz poziom trudności.
- **Klawiatura:** w wersji bazowej brak odrębnego sterowania paletką klawiszami.
- **Punktacja:** punkt gracza jest naliczany po tym, gdy piłka minie stronę przeciwnika; odbicia od paletek i ścian nie są punktami.

### 4. Arkanoid

**Cel:** odbijaj piłkę paletką i rozbijaj wszystkie klocki.

- **Tensometr:** steruje poziomym położeniem paletki. Zmieniaj nacisk, aby przesuwać paletkę pod piłkę.
- **Wskaźniki:** wynik, poziom, liczba żyć i trudność.
- **Punktacja:** trafienie w klocek daje punkty; klocki wymagające kilku trafień mogą pozostać aktywne po pierwszym uderzeniu.
- **Klawiatura:** w wersji bazowej brak odrębnych skrótów sterowania paletką.

### 5. Flappy Bird

**Cel:** przeprowadź ptaka przez szczeliny między przeszkodami, unikając zderzeń.

- **Tensometr:** krótki nacisk wywołuje skok ptaka. Skok jest wyzwalany przejściem z braku nacisku do nacisku, a nie samym ciągłym utrzymywaniem sygnału.
- **Praktyka:** naciskaj krótko i rytmicznie. Długie przytrzymanie nie powinno być traktowane jako seria niezależnych skoków.
- **Wskaźniki:** wynik, siła, poziom, prędkość gry i liczba miniętych przeszkód.
- **Klawiatura:** w wersji bazowej nie ma osobnego skrótu skoku; sesję uruchamia się i resetuje z interfejsu.

### 6. Mario Run

**Cel:** pokonuj przeszkody, ładując skok i zwalniając nacisk w odpowiednim momencie.

- **Tensometr:** przytrzymanie powyżej progu rozpoczyna ładowanie skoku, a zwolnienie nacisku wykonuje skok. Dłuższe ładowanie zwiększa poziom naładowania i siłę skoku w granicach mechaniki gry.
- **Wskaźnik:** pasek ładowania nad postacią pokazuje aktualną gotowość skoku.
- **Postęp:** ekran pokazuje wynik, poziom i liczbę przeszkód pozostałych do kolejnego poziomu.
- **Klawiatura:** w wersji bazowej skok jest sterowany tensometrem; nie ma osobnego skrótu klawiaturowego do ładowania i zwalniania skoku.

### 7. Car Race

**Cel:** prowadź samochód przez trzy pasy, omijaj ruch uliczny, utrzymuj serię bezkolizyjnych minięć i rozsądnie wykorzystuj dopalacz.

- **Tensometr:** wybiera pas na podstawie wartości siły: niska wartość — lewy pas, środkowa — pas środkowy, wysoka — prawy pas. Sygnał jest filtrowany, a progi histerezy ograniczają przypadkowe przełączanie przy wahaniach pomiaru.
- **Klawiatura:** `←` lub `A` — przesuń samochód o jeden pas w lewo; `→` lub `D` — przesuń o jeden pas w prawo; `SPACJA` lub `B` — aktywuj dopalacz. Ruch klawiaturą działa, gdy widok gry ma fokus.
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
- **Automatyczny strzał:** po uruchomieniu gry pierwszy strzał następuje po 2,4 s, a następne co 2,4 s. Klawisz spacji nie wyzwala strzału.
- **Tor lotu:** kropkowana linia pokazuje przewidywany tor strzały z uwzględnieniem grawitacji i bieżącego kąta. Jest wskazówką, a poruszający się cel może zmienić położenie przed trafieniem.
- **Strefy celu:** zewnętrzna część tarczy daje 10 punktów, środkowa 20, a środek 30. Trafienie zwiększa prędkość celu, maksymalnie do 150 jednostek na sekundę.
- **Statystyki:** wynik, kąt celowania, łączna liczba oddanych strzałów, trafień, pudeł i strzał w locie.
- **Ograniczenia:** jednocześnie może istnieć maksymalnie 8 aktywnych strzał. Strzała, która opuści planszę, jest liczona jako pudło.
- **Praktyka:** obserwuj kropkowany tor lotu i ruch celu. Utrzymuj tensometr na wartości odpowiadającej potrzebnemu kątowi; strzał zostanie oddany automatycznie po upływie czasu.

### 9. Galaga

**Cel:** steruj statkiem, niszcz kolejne formacje przeciwników, zbieraj ulepszenia i unikaj wrogiego ostrzału.

- **Tensometr:** ustawia poziome położenie statku w obrębie planszy. Wartość 0–100% odpowiada położeniu od lewej do prawej krawędzi.
- **Klawiatura:** `←` lub `A` — ruch w lewo; `→` lub `D` — ruch w prawo; przytrzymanie `SPACJI` — ciągły ogień; `X` — jednorazowy impuls EMP.
- **Impuls EMP:** usuwa wszystkie aktywne pociski przeciwników i zadaje jedno uszkodzenie każdemu aktywnemu przeciwnikowi. Na początku sesji dostępne są 2 ładunki; po ukończeniu fali przywracany jest 1 ładunek, maksymalnie do 4. Naciśnij `X` ponownie dopiero wtedy, gdy chcesz zużyć kolejny ładunek.
- **Ulepszenia:** co piąty pokonany przeciwnik może pozostawić premię. Niebieska premia daje potrójny strzał, zielona uruchamia osłonę, a żółta zwiększa szybkostrzelność. Premie zbiera się przez zetknięcie statku z opadającym symbolem.
- **Punktacja i seria:** kolejne eliminacje w krótkim odstępie zwiększają premię punktową. Seria wygasa po przerwie.
- **Trudność:** następne fale zwiększają liczebność i wytrzymałość części przeciwników oraz tempo ostrzału. Ukończenie fali daje premię punktową i dodatkowy ładunek EMP.
- **Ochrona:** po trafieniu statek otrzymuje krótką ochronę przed kolejnym uszkodzeniem. Zebrana osłona pochłania następne trafienie.
- **Wskaźniki:** wynik, numer fali, życia, ładunki EMP, seria eliminacji i aktywne ulepszenie.

### 10. Scorched Earth

**Cel:** w turowej rozgrywce artyleryjskiej traf przeciwnika, dobierając kąt i siłę strzału oraz uwzględniając wiatr.

- **Tensometr:** jest jedynym kontrolerem kąta działa. Siła strzału pozostaje ustawiona na stałym, zbalansowanym poziomie.
- **Automatyczny strzał:** po rozpoczęciu tury gracza gra odlicza 2 s, a następnie automatycznie wypuszcza pocisk. Klawiatura, mysz i spacja nie zmieniają kąta, mocy ani chwili strzału.
- **Wskaźniki:** zdrowie obu czołgów, runda, wynik i siła/wiatr zgodnie z informacją na ekranie.
- **Wskazówka:** wiatr wpływa na tor pocisku. Po strzale poczekaj na zakończenie lotu i ruch przeciwnika; następna tura gracza rozpoczyna nowe dwusekundowe odliczanie.

## Skrócona tabela sterowania

| Gra | Tensometr | Klawiatura |
|---|---|---|
| Sinus | pionowe położenie kursora względem celu | brak skrótów gry |
| Tangens | pionowe położenie kursora względem celu | brak skrótów gry |
| Pong | pionowe położenie lewej paletki | brak skrótów gry |
| Arkanoid | poziome położenie paletki | brak skrótów gry |
| Flappy Bird | impuls skoku po przekroczeniu progu | brak skrótu skoku |
| Mario Run | przytrzymanie ładuje skok, zwolnienie wykonuje skok | brak skrótów skoku |
| Car Race | wybór jednego z trzech pasów | `←`/`A`, `→`/`D` |
| Archer | kąt celowania 0–100% | automatyczny strzał co 2,4 s |
| Galaga | pozioma pozycja statku | `←`/`A`, `→`/`D`, `SPACJA` — ogień, `X` — EMP |
| Scorched Earth | kąt działa | automatyczny strzał po 2 s |

## Uwaga o wersjach i rozszerzeniach

Niektóre usprawnienia sterowania są rozwijane w osobnych gałęziach i PR-ach. Dopóki nie zostaną scalone z gałęzią integracyjną, nie należy traktować ich jako dostępnych w tej wersji aplikacji. Dotyczy to między innymi dodatkowych skrótów kalibracji, adaptacyjnej trudności, alternatywnych przebiegów ćwiczeń oraz sterowania myszą w wybranych grach.

Przy zmianie sterowania w kodzie należy zaktualizować ten plik oraz teksty instruktażowe wyświetlane bezpośrednio w grze. Każdy skrót powinien mieć test regresyjny, a opis powinien odpowiadać faktycznej implementacji.


### 11. Precision Crane — Precyzyjny dźwig

**Cel:** przenieś ładunek z miejsca odbioru do oznaczonej strefy i odłóż go precyzyjnie.

- **Jedyny kontroler:** jeden tensometr/HX711, skalibrowany do zakresu siły 0–100%.
- **Ruch poziomy i pionowy:** roboczy zakres 0–70% siły steruje płynnie aktualnie wybraną osią. Oś pozioma przesuwa wózek suwnicy, a pionowa reguluje wysokość haka.
- **Zmiana osi:** przytrzymaj siłę co najmniej 82% przez minimum 0,85 s i zwolnij do 18% lub mniej. Zmienia się aktywna oś.
- **Chwyt i zwolnienie:** impuls siły co najmniej 82% trwający od 0,15 do 0,85 s, zakończony zwolnieniem do 18% lub mniej, przełącza chwyt/zwolnienie. Hak musi znajdować się w pobliżu ładunku, aby go podnieść.
- **Punktacja:** poprawna dostawa daje punkty i zwiększa serię. Zwolnienie poza strefą celu zużywa jedną z trzech prób. Co cztery dostawy wzrasta trudność.
- **Brak sygnału:** po 350 ms bez świeżej próbki sterowanie jest wstrzymywane. Klawiatura, mysz, dotyk i gamepad nie sterują rozgrywką.


### 11. Precision Crane — Precyzyjny dźwig

**Cel:** przenieś ładunek z miejsca odbioru do oznaczonej strefy i odłóż go precyzyjnie.

- **Jedyny kontroler:** jeden tensometr/HX711, skalibrowany do zakresu siły 0–100%.
- **Ruch poziomy i pionowy:** roboczy zakres 0–70% siły steruje płynnie aktualnie wybraną osią. Oś pozioma przesuwa wózek suwnicy, a pionowa reguluje wysokość haka.
- **Zmiana osi:** przytrzymaj siłę co najmniej 82% przez minimum 0,85 s i zwolnij do 18% lub mniej. Zmienia się aktywna oś.
- **Chwyt i zwolnienie:** impuls siły co najmniej 82% trwający od 0,15 do 0,85 s, zakończony zwolnieniem do 18% lub mniej, przełącza chwyt/zwolnienie. Hak musi znajdować się w pobliżu ładunku, aby go podnieść.
- **Punktacja:** poprawna dostawa daje punkty i zwiększa serię. Zwolnienie poza strefą celu zużywa jedną z trzech prób. Co cztery dostawy wzrasta trudność.
- **Brak sygnału:** po 350 ms bez świeżej próbki sterowanie jest wstrzymywane. Klawiatura, mysz, dotyk i gamepad nie sterują rozgrywką.
