# Instrukcja sterowania grami Biofeedback

> Dokument opisuje sterowanie zaimplementowane w aktualnej gałęzi integracyjnej Biofeedback. Skróty klawiaturowe odnoszą się do aktywnego widoku gry. Funkcje, które są dopiero w osobnych gałęziach lub Pull Requestach, nie są przedstawiane jako dostępne w tej wersji.

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

**Cel:** prowadź samochód przez trzy pasy ruchu, omijaj nadjeżdżające pojazdy, buduj wynik i używaj dopalacza w odpowiednim momencie.

- **Tensometr:** steruje wyborem pasa. Niska wartość kieruje na lewy pas, środkowa utrzymuje pas środkowy, a wysoka wybiera prawy pas. Filtrowanie i histereza ograniczają przeskakiwanie między pasami przy szumie pomiarowym.
- **Klawiatura:** `←` lub `A` — zmiana o jeden pas w lewo; `→` lub `D` — zmiana o jeden pas w prawo; `SPACJA` lub `B` — uruchomienie dopalacza.
- **Dopalacz:** zwiększa prędkość na 1,8 s; po użyciu obowiązuje 5 s odnowienia. Wskaźnik na ekranie pokazuje stan dopalacza i czas odnowienia.
- **Punktacja:** wynik rośnie za przejechany dystans i bezpieczne minięcie przeszkody. Tempo wzrasta wraz z dystansem.
- **Kolizje:** zderzenie odbiera jedno życie. Ochrona przez 1,1 s zapobiega wielokrotnemu odejmowaniu żyć przy nakładających się kontaktach.
- **Wskaźniki:** wynik, dystans, prędkość, liczba żyć, wybrany pas i stan dopalacza.

### 8. Archer Force Trainer

**Cel:** ćwicz precyzję celowania, przewidywanie toru balistycznego i synchronizację strzału z ruchem celu.

- **Tensometr/HX711:** ustawia kąt łuku w zakresie gry. Wartość siły 0–100% jest mapowana na kąt od około +31° (niższe celowanie) do około −52° (wyższe celowanie). Zmiana siły natychmiast aktualizuje kąt.
- **Klawiatura:** przytrzymaj `W` lub `↑`, aby płynnie podnosić kąt; przytrzymaj `S` lub `↓`, aby go obniżać. Po zwolnieniu klawisza regulacja zatrzymuje się. Podczas przytrzymania klawisza sterowanie klawiaturą ma pierwszeństwo przed próbkami tensometru; po zwolnieniu wraca sterowanie czujnikiem. `SPACJA` oddaje pojedynczy strzał; kolejne strzały są ograniczone czasem odnowienia około 320 ms.
- **Tor lotu:** kropkowana linia pokazuje przewidywany tor strzały z uwzględnieniem grawitacji i bieżącego kąta. Jest wskazówką, a poruszający się cel może zmienić położenie przed trafieniem.
- **Strefy celu:** zewnętrzna część tarczy daje 10 punktów, środkowa 20, a środek 30. Trafienie zwiększa prędkość celu, maksymalnie do 150 jednostek na sekundę.
- **Statystyki:** wynik, kąt celowania, łączna liczba oddanych strzałów, trafień, pudeł i strzał w locie.
- **Ograniczenia:** jednocześnie może istnieć maksymalnie 8 aktywnych strzał. Strzała, która opuści planszę, jest liczona jako pudło.
- **Praktyka:** najpierw ustaw kąt tensometrem, następnie koryguj go krótkimi przytrzymaniami klawiszy. Obserwuj kropkowany tor oraz kierunek ruchu celu i oddaj strzał z wyprzedzeniem.

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

- **Tensometr:** ustawia kąt działa w zakresie gry.
- **Klawiatura:** `W` lub `↑` — zwiększ kąt; `S` lub `↓` — zmniejsz kąt; `+` lub `=` — zwiększ siłę; `-` — zmniejsz siłę; `SPACJA` — oddaj strzał, gdy trwa tura gracza.
- **Wskaźniki:** zdrowie obu czołgów, runda, wynik i siła/wiatr zgodnie z informacją na ekranie.
- **Wskazówka:** wiatr wpływa na tor pocisku. Po oddaniu strzału poczekaj na zakończenie lotu i ruch przeciwnika; nie można oddać kolejnego strzału w trakcie aktywnego pocisku.

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
| Archer | kąt celowania 0–100% | przytrzymaj `W`/`↑` lub `S`/`↓` do płynnej korekty, `SPACJA` — strzał |
| Galaga | pozioma pozycja statku | `←`/`A`, `→`/`D`, `SPACJA` — ogień, `X` — EMP |
| Scorched Earth | kąt działa | `W`/`↑`, `S`/`↓`, `+`/`-`, `SPACJA` |

## Uwaga o wersjach i rozszerzeniach

Niektóre usprawnienia sterowania są rozwijane w osobnych gałęziach i PR-ach. Dopóki nie zostaną scalone z gałęzią integracyjną, nie należy traktować ich jako dostępnych w tej wersji aplikacji. Dotyczy to między innymi dodatkowych skrótów kalibracji, adaptacyjnej trudności, alternatywnych przebiegów ćwiczeń oraz sterowania myszą w wybranych grach.

Przy zmianie sterowania w kodzie należy zaktualizować ten plik oraz teksty instruktażowe wyświetlane bezpośrednio w grze. Każdy skrót powinien mieć test regresyjny, a opis powinien odpowiadać faktycznej implementacji.
