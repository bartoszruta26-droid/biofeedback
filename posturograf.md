# Parametry Posturograficzne - Analiza Ruchów Środka Nacisku (CoP)

W posturografii, analizując ruchy środka nacisku (CoP - Center of Pressure) w osi X (poprzecznej) i Y (podłużnej), można wyznaczyć wiele parametrów opisujących stabilność postawy. Poniżej znajduje się zestawienie kilkudziesięciu takich parametrów wraz z ich wzorami, opisem zmiennych oraz znaczeniem klinicznym.

---

## I. Parametry Czasowo-Przestrzenne (Statyczne i Kinematyczne)

### 1. Średnia pozycja CoP (Mean Position)
**Wzór:**
```
x̄ = (1/N) × Σ(xi)  dla i=1 do N
ȳ = (1/N) × Σ(yi)  dla i=1 do N
```

**Zmienne:**
- `xi, yi` – współrzędne CoP w chwili i
- `N` – liczba próbek

**Opis:** Średnie położenie CoP podczas pomiaru.

**Zastosowanie:** Wykrywanie asymetrii obciążenia (np. czy pacjent staje bardziej na prawą czy lewą nogę).

---

### 2. Odchylenie standardowe w osi X (SD X)
**Wzór:**
```
σx = √[(1/(N-1)) × Σ((xi - x̄)²)]  dla i=1 do N
```

**Zmienne:**
- `xi` – współrzędna CoP w osi X w chwili i
- `x̄` – średnia pozycja w osi X
- `N` – liczba próbek

**Opis:** Rozrzut wychyleń w kierunku bocznym (medialno-lateralnym).

**Zastosowanie:** Ocena kontroli równowagi w płaszczyźnie czołowej.

---

### 3. Odchylenie standardowe w osi Y (SD Y)
**Wzór:**
```
σy = √[(1/(N-1)) × Σ((yi - ȳ)²)]  dla i=1 do N
```

**Zmienne:**
- `yi` – współrzędna CoP w osi Y w chwili i
- `ȳ` – średnia pozycja w osi Y
- `N` – liczba próbek

**Opis:** Rozrzut wychyleń w kierunku przód-tył (antero-posterior).

**Zastosowanie:** Ocena kontroli w płaszczyźnie strzałkowej.

---

### 4. Całkowita droga migracji CoP (Total Path Length)
**Wzór:**
```
L = Σ(√((x(i+1) - xi)² + (y(i+1) - yi)²))  dla i=1 do N-1
```

**Zmienne:**
- `xi, yi` – współrzędne CoP w chwili i
- `x(i+1), y(i+1)` – współrzędne CoP w chwili i+1
- `N` – liczba próbek

**Opis:** Sumaryczna długość toru, po jakim poruszał się CoP.

**Zastosowanie:** Miara aktywności systemu posturalnego. Dłuższa droga może wskazywać na trudności z utrzymaniem równowagi.

---

### 5. Średnia prędkość migracji CoP (Mean Velocity)
**Wzór:**
```
Vmean = L / T
```

**Zmienne:**
- `L` – całkowita droga migracji CoP
- `T` – czas trwania pomiaru (w sekundach)

**Opis:** Średnia szybkość ruchu CoP.

**Zastosowanie:** Bardzo czuły marker destabilizacji. Wzrost prędkości często poprzedza upadki.

---

### 6. Maksymalne wychylenie w osi X (Range X)
**Wzór:**
```
Rangex = max(xi) - min(xi)  dla wszystkich i
```

**Zmienne:**
- `max(xi)` – największa wartość w osi X
- `min(xi)` – najmniejsza wartość w osi X

**Opis:** Pełna rozpiętość wychyleń bocznych.

**Zastosowanie:** Określenie granic stabilności w osi X.

---

### 7. Maksymalne wychylenie w osi Y (Range Y)
**Wzór:**
```
Rangey = max(yi) - min(yi)  dla wszystkich i
```

**Zmienne:**
- `max(yi)` – największa wartość w osi Y
- `min(yi)` – najmniejsza wartość w osi Y

**Opis:** Pełna rozpiętość wychyleń przód-tył.

**Zastosowanie:** Określenie granic stabilności w osi Y.

---

### 8. Powierzchnia elipsy zaufania 95% (95% Confidence Ellipse Area)
**Wzór:**
```
Area = π × 1.96² × σx × σy × √(1 - ρ²)
```

**Zmienne:**
- `π` – stała Pi (≈ 3.14159)
- `1.96` – współczynnik dla 95% przedziału ufności (rozklad normalny)
- `σx` – odchylenie standardowe w osi X
- `σy` – odchylenie standardowe w osi Y
- `ρ` – współczynnik korelacji Pearsona między X i Y

**Opis:** Pole elipsy, w której znajduje się 95% punktów CoP.

**Zastosowanie:** Popularny wskaźnik ogólnej stabilności. Większa powierzchnia oznacza większą chwiejność.

---

### 9. Powierzchnia otoczki wypukłej (Convex Hull Area)
**Wzór:** Obliczana geometrycznie (np. algorytmem Grahama lub Shamosa-Hoeya)

**Zmienne:**
- `xi, yi` – wszystkie współrzędne CoP

**Opis:** Pole najmniejszego wypukłego obszaru obejmującego wszystkie punkty CoP.

**Zastosowanie:** Uchwytuje skrajne wychylenia lepiej niż elipsa.

---

### 10. Średnia odległość od środka (Mean Distance from Center)
**Wzór:**
```
MD = (1/N) × Σ(√((xi - x̄)² + (yi - ȳ)²))  dla i=1 do N
```

**Zmienne:**
- `xi, yi` – współrzędne CoP w chwili i
- `x̄, ȳ` – średnia pozycja CoP
- `N` – liczba próbek

**Opis:** Średnia odległość CoP od jego średniej pozycji.

**Zastosowanie:** Alternatywa dla RMS, mniej wrażliwa na skrajne wartości.

---

### 11. Odległość RMS (Root Mean Square Distance)
**Wzór:**
```
RMS = √[(1/N) × Σ(((xi - x̄)² + (yi - ȳ)²))]  dla i=1 do N
```

**Zmienne:**
- `xi, yi` – współrzędne CoP w chwili i
- `x̄, ȳ` – średnia pozycja CoP
- `N` – liczba próbek

**Opis:** Pierwiastek ze średniej kwadratów odległości od średniej pozycji.

**Zastosowanie:** Miara energii wychyleń, bardziej wrażliwa na odchylenia niż średnia.

---

### 12. Stosunek drogi do powierzchni (Path/Area Ratio)
**Wzór:**
```
RPA = L / Area
```

**Zmienne:**
- `L` – całkowita droga migracji CoP
- `Area` – powierzchnia (np. elipsy zaufania lub otoczki wypukłej)

**Opis:** Relacja między aktywnością a efektywnością utrzymania stabilności.

**Zastosowanie:** Wysoki stosunek wskazuje na nieefektywną kontrolę (dużo ruchu, mało efektu).

---

### 13. Skewness (Asymetria rozkładu) w osi X
**Wzór:**
```
Skewx = [N / ((N-1)(N-2))] × Σ(((xi - x̄) / σx)³)  dla i=1 do N
```

**Zmienne:**
- `xi` – współrzędna CoP w osi X w chwili i
- `x̄` – średnia pozycja w osi X
- `σx` – odchylenie standardowe w osi X
- `N` – liczba próbek

**Opis:** Miara asymetrii rozkładu wychyleń w osi X.

**Zastosowanie:** Wykrywanie tendencji do pochylania się w jedną stronę (np. po udarze).

---

### 14. Kurtoza (Kurtosis) w osi X
**Wzór:**
```
Kurtx = [(1/N) × Σ(((xi - x̄) / σx)⁴)] - 3  dla i=1 do N
```

**Zmienne:**
- `xi` – współrzędna CoP w osi X w chwili i
- `x̄` – średnia pozycja w osi X
- `σx` – odchylenie standardowe w osi X
- `N` – liczba próbek

**Opis:** Miara spiczastości rozkładu wychyleń (wartość -3 to kurtoza rozkładu normalnego).

**Zastosowanie:** Wysoka kurtoza oznacza, że pacjent często przebywa blisko średniej, z rzadkimi wychyleniami.

---

### 15. Prędkość w osi X (Velocity X)
**Wzór:**
```
Vx = (1/T) × Σ(|x(i+1) - xi|)  dla i=1 do N-1
```

**Zmienne:**
- `xi` – współrzędna CoP w osi X w chwili i
- `T` – czas trwania pomiaru
- `N` – liczba próbek

**Opis:** Składowa prędkości w ruchu bocznym.

**Zastosowanie:** Ocena specyficznej kontroli w osi X.

---

### 16. Prędkość w osi Y (Velocity Y)
**Wzór:**
```
Vy = (1/T) × Σ(|y(i+1) - yi|)  dla i=1 do N-1
```

**Zmienne:**
- `yi` – współrzędna CoP w osi Y w chwili i
- `T` – czas trwania pomiaru
- `N` – liczba próbek

**Opis:** Składowa prędkości w ruchu przód-tył.

**Zastosowanie:** Ocena specyficznej kontroli w osi Y.

---

### 17. Wskaźnik anizotropii (Anisotropy Index)
**Wzór:**
```
AI = (σmajor - σminor) / (σmajor + σminor)
```

**Zmienne:**
- `σmajor` – odchylenie standardowe wzdłuż głównej osi elipsy
- `σminor` – odchylenie standardowe wzdłuż pobocznej osi elipsy

**Opis:** Stopień kierunkowości chwiejności.

**Zastosowanie:** Zdrowi ludzie mają wyższą anizotropię (więcej ruchu AP niż ML).

---

### 18. Częstotliwość migracji (Sway Frequency)
**Wzór:**
```
Fmean = Vmean / (2 × π × RMS)
```

**Zmienne:**
- `Vmean` – średnia prędkość migracji CoP
- `π` – stała Pi (≈ 3.14159)
- `RMS` – odległość root mean square

**Opis:** Przybliżenie średniej częstotliwości oscylacji.

**Zastosowanie:** Pomaga odróżnić strategie kontroli (niska = kołysanie, wysoka = drżenia).

---

### 19. Liczba pików (Peak Count)
**Wzór:** Zliczenie lokalnych maksimów w sygnale przekraczających próg (np. 1 SD)

**Zmienne:**
- `xi, yi` – współrzędne CoP
- Próg: np. `σx` lub `σy`

**Opis:** Liczba wyraźnych prób korekcji postawy.

**Zastosowanie:** Ocena mikro-strategii kontroli.

---

### 20. Liczba zerowych przejść (Zero Crossings Rate)
**Wzór:**
```
ZCR = (liczba zmian znaku (xi - x̄)) / T
```

**Zmienne:**
- `xi` – współrzędna CoP w osi X w chwili i
- `x̄` – średnia pozycja w osi X
- `T` – czas trwania pomiaru

**Opis:** Częstotliwość przechodzenia przez średnią pozycję.

**Zastosowanie:** Prosta miara dynamiki oscylacji.

---

## II. Parametry Częstotliwościowe (Analiza Spektralna)

### 21. Dominująca częstotliwość (Peak Frequency)
**Wzór:**
```
fdom = arg max(P(f))  dla wszystkich f
```

**Zmienne:**
- `P(f)` – moc widmowa w funkcji częstotliwości (z FFT)
- `arg max` – argument (częstotliwość), dla którego funkcja osiąga maksimum

**Opis:** Częstotliwość z największą energią.

**Zastosowanie:** Identyfikacja głównego mechanizmu kontroli (np. 0.5-1 Hz – odruchy, >5 Hz – tremor).

---

### 22. Moc całkowita (Total Power)
**Wzór:**
```
Ptotal = Σ(P(fk) × Δf)  dla wszystkich k
```

**Zmienne:**
- `P(fk)` – moc widmowa w częstotliwości fk
- `Δf` – rozdzielczość częstotliwości (szerokość pasma)

**Opis:** Całkowita energia sygnału w domenie częstotliwości.

**Zastosowanie:** Ogólna miara intensywności chwiejności (proporcjonalna do wariancji w czasie).

---

### 23. Moc w paśmie niskim (Low Frequency Power - LF, np. 0-0.5 Hz)
**Wzór:**
```
PLF = ∫ P(f) df  od 0 do 0.5 Hz
```

**Zmienne:**
- `P(f)` – moc widmowa w funkcji częstotliwości

**Opis:** Energia powolnych oscylacji.

**Zastosowanie:** Związana z integracją sensoryczną (wzrok, błędnik).

---

### 24. Moc w paśmie wysokim (High Frequency Power - HF, np. 0.5-10 Hz)
**Wzór:**
```
PHF = ∫ P(f) df  od 0.5 Hz do fmax
```

**Zmienne:**
- `P(f)` – moc widmowa w funkcji częstotliwości
- `fmax` – maksymalna częstotliwość analizy (zwykle połowa częstotliwości próbkowania)

**Opis:** Energia szybkich oscylacji.

**Zastosowanie:** Związana z refleksami rdzeniowymi lub sztywnością mięśniową.

---

### 25. Stosunek mocy HF/LF
**Wzór:**
```
RHF/LF = PHF / PLF
```

**Zmienne:**
- `PHF` – moc w paśmie wysokim
- `PLF` – moc w paśmie niskim

**Opis:** Relacja między szybkimi a wolnymi mechanizmami.

**Zastosowanie:** Wzrost może sugerować strategię sztywną (co-contraction) w odpowiedzi na niestabilność.

---

### 26. Częstotliwość środkowa (Mean Frequency)
**Wzór:**
```
fmean = (∫ f × P(f) df) / (∫ P(f) df)
```

**Zmienne:**
- `f` – częstotliwość
- `P(f)` – moc widmowa w funkcji częstotliwości

**Opis:** Średnia ważona częstotliwości widma.

**Zastosowanie:** Syntetyczny wskaźnik przesunięcia widma.

---

### 27. Szerokość pasma mocy (Power Bandwidth)
**Wzór:** Zakres częstotliwości, w którym skupione jest np. 95% mocy całkowitej

**Zmienne:**
- `P(f)` – moc widmowa
- `Ptotal` – moc całkowita

**Opis:** Miara rozproszenia energii w zakresie częstotliwości.

**Zastosowanie:** Wąskie pasmo = rytmiczne kołysanie; szerokie = chaotyczne.

---

## III. Parametry Nieliniowe

### 28. Entropia aproksymacyjna (Approximate Entropy - ApEn)
**Wzór:** Złożony algorytm obliczeniowy: `ApEn(m, r, N)`

**Zmienne:**
- `m` – długość wzorca (zwykle 2 lub 3)
- `r` – tolerancja (zwykle 0.1-0.25 × SD danych)
- `N` – liczba próbek

**Opis:** Miara regularności i przewidywalności sygnału.

**Zastosowanie:** Niższa ApEn = mniej adaptacyjny, bardziej sztywny styl kontroli (np. starość, Parkinson).

---

### 29. Entropia próbkowa (Sample Entropy - SampEn)
**Wzór:** Ulepszona wersja ApEn, algorytm obliczeniowy: `SampEn(m, r, N)`

**Zmienne:**
- `m` – długość wzorca
- `r` – tolerancja
- `N` – liczba próbek

**Opis:** Miara nieregularności, mniej zależna od długości danych.

**Zastosowanie:** Zdrowy system ma umiarkowaną SampEn. Zbyt niska = sztywność, zbyt wysoka = chaos.

---

### 30. Wymiar fraktalny (Fractal Dimension)
**Wzór (metoda Katza):**
```
D = log(L) / log(d)
```

**Zmienne:**
- `L` – całkowita długość krzywej trajektorii CoP
- `d` – średnica zbioru (maksymalna odległość między dowolnymi dwoma punktami)
- `log` – logarytm naturalny lub dziesiętny

**Opis:** Miara złożoności i "poszarpania" trajektorii CoP.

**Zastosowanie:** Wyższy wymiar wskazuje na bardziej złożony ruch.

---

### 31. Współczynnik Hursta (Hurst Exponent)
**Wzór:** Analiza R/S (Rescaled Range Analysis)

**Zmienne:**
- `xi` – dane czasowe (współrzędne CoP)
- `N` – liczba próbek

**Opis:** Mierzy długoterminowe zależności (pamięć) w sygnale.

**Zastosowanie:** 
- H < 0.5 = antypersystencja (korekty)
- H > 0.5 = persystencja (dryf)
- Zdrowi mają antypersystencję w krótkim okresie

---

### 32. Detrended Fluctuation Analysis (DFA - Alpha)
**Wzór:** Nachylenie prostej w analizie fluktuacji po usunięciu trendów (regresja log-log)

**Zmienne:**
- `xi` – dane czasowe (współrzędne CoP)
- Skale czasowe do analizy

**Opis:** Miara korelacji długozasięgowych w szeregu niestacjonarnym.

**Zastosowanie:** 
- α ≈ 1.0 – optymalna skala biologiczna (szum różowy)
- α odchylone wskazuje na zaburzenia

---

### 33. Stabilność wieloskalowa (Multiscale Entropy - MSE)
**Wzór:** Obliczenie SampEn na danych zagrubionych (coarsened) na różnych skalach czasowych

**Zmienne:**
- `SampEn` – entropia próbkowa
- Skala czasowa (τ)

**Opis:** Entropia obliczana dla wielu skal czasowych.

**Zastosowanie:** Zdrowe systemy utrzymują wysoką entropię na wielu skalach. Spadek entropii na wyższych skalach może wskazywać na starzenie lub chorobę.

---

### 34. Czas stabilizacji (Time to Stabilization - TTS)
**Wzór:** Czas od momentu zaburzenia do osiągnięcia ustalonego poziomu chwiejności

**Zmienne:**
- Moment zaburzenia (t0)
- Próg stabilności (np. 5% powierzchni stopy lub ustalona wartość SD)
- Czas osiągnięcia progu (tstab)
- TTS = tstab - t0

**Opis:** Dynamika powrotu do równowagi po zaburzeniu.

**Zastosowanie:** Kluczowy parametr w testach dynamicznych, np. po kontuzji.

---

### 35. Średnia częstotliwość zmian kierunku (Mean Direction Change Rate)
**Wzór:**
```
MDCR = (liczba zmian znaku prędkości) / T
```

**Zmienne:**
- Zmiany znaku: gdy (x(i+1) - xi) zmienia znak lub (y(i+1) - yi) zmienia znak
- `T` – czas trwania pomiaru

**Opis:** Jak często zmienia się kierunek ruchu CoP.

**Zastosowanie:** Wskaźnik „nerwowości" lub niestabilności mikro-strategii.

---

### 36. Stosunek wychyleń AP/ML (Romberg Quotient variant)
**Wzór:**
```
Q = σy / σx
```
lub
```
Q = Rangey / Rangex
```

**Zmienne:**
- `σy` – odchylenie standardowe w osi Y (AP)
- `σx` – odchylenie standardowe w osi X (ML)

**Opis:** Proporcja chwiejności przód-tył do bok-bok.

**Zastosowanie:** Zmiana stosunku może wskazywać na specyficzne deficyty sensoryczne lub motoryczne.

---

### 37. Indeks nieregularności (Irregularity Index)
**Wzór:**
```
II = SD(Velocity) / Mean(Velocity)
```

**Zmienne:**
- `SD(Velocity)` – odchylenie standardowe prędkości
- `Mean(Velocity)` – średnia prędkość

**Opis:** Miara zmienności tempa ruchu CoP.

**Zastosowanie:** Ocena płynności ruchów korekcyjnych.

---

### 38. Maksymalna odległość od środka (Max Distance from Center)
**Wzór:**
```
MaxD = max(√((xi - x̄)² + (yi - ȳ)²))  dla wszystkich i
```

**Zmienne:**
- `xi, yi` – współrzędne CoP w chwili i
- `x̄, ȳ` – średnia pozycja CoP

**Opis:** Najdalszy punkt trajektorii od średniej pozycji.

**Zastosowanie:** Identyfikacja maksymalnych granic stabilności.

---

### 39. Współczynnik korelacji X-Y (X-Y Correlation)
**Wzór:**
```
ρxy = [Σ((xi - x̄)(yi - ȳ))] / [(N-1) × σx × σy]  dla i=1 do N
```

**Zmienne:**
- `xi, yi` – współrzędne CoP w chwili i
- `x̄, ȳ` – średnie pozycje
- `σx, σy` – odchylenia standardowe
- `N` – liczba próbek

**Opis:** Mierzy liniową zależność między ruchami w osi X i Y.

**Zastosowanie:** Niska korelacja sugeruje niezależne kontrolowanie ruchów w obu osiach.

---

### 40. Wariancja całkowita (Total Variance)
**Wzór:**
```
Vartotal = σx² + σy²
```

**Zmienne:**
- `σx` – odchylenie standardowe w osi X
- `σy` – odchylenie standardowe w osi Y

**Opis:** Suma wariancji w obu osiach.

**Zastosowanie:** Ogólna miara rozproszenia CoP, alternatywa dla RMS.

---

### 41. Średnia prędkość kątowa (Mean Angular Velocity)
**Wzór:**
```
AV = (1/T) × Σ(|θ(i+1) - θi|)  dla i=1 do N-1
```
gdzie:
```
θi = arctan2(yi - ȳ, xi - x̄)
```

**Zmienne:**
- `θi` – kąt względem średniej pozycji w chwili i
- `arctan2` – funkcja arcus tangens dwóch zmiennych
- `x̄, ȳ` – średnia pozycja CoP
- `T` – czas trwania pomiaru

**Opis:** Tempo zmiany kierunku ruchu CoP wokół średniego położenia.

**Zastosowanie:** Ocena rotacyjnej niestabilności.

---

### 42. Energia kinetyczna średnia (Mean Kinetic Energy)
**Wzór:**
```
KE = (1/(N-1)) × Σ(((x(i+1) - xi)² + (y(i+1) - yi)²))  dla i=1 do N-1
```

**Zmienne:**
- `xi, yi` – współrzędne CoP w chwili i
- `N` – liczba próbek

**Opis:** Miara średniej energii zużytej na mikro-korekty (proporcjonalna).

**Zastosowanie:** Alternatywna miara aktywności systemu posturalnego.

---

### 43. Kurtoza 2D (2D Kurtosis)
**Wzór:** Uogólnienie kurtozy na dwie osie, bazujące na macierzy kowariancji

**Zmienne:**
- `xi, yi` – współrzędne CoP
- Macierz kowariancji [Cov(x,x), Cov(x,y); Cov(y,x), Cov(y,y)]

**Opis:** Miara "spiczastości" rozkładu punktów CoP w przestrzeni 2D.

**Zastosowanie:** Informacja o koncentracji punktów wokół środka.

---

### 44. Entropia Shannona (Shannon Entropy)
**Wzór:**
```
H = -Σ(pi × log(pi))  dla wszystkich i
```

**Zmienne:**
- `pi` – prawdopodobieństwo znalezienia CoP w i-tej komórce siatki (liczba punktów w komórce / N)
- `log` – logarytm (zwykle naturalny lub dwójkowy)
- `N` – całkowita liczba próbek

**Opis:** Miara niepewności w rozmieszczeniu punktów CoP.

**Zastosowanie:** Im bardziej równomiernie rozłożony jest CoP, tym wyższa entropia.

---

### 45. Czas przebywania w ćwiartkach (Quadrant Time)
**Wzór:** Procentowy czas spędzony w każdej z 4 ćwiartek względem średniej pozycji (x̄, ȳ)

**Ćwiartki:**
- Q1: x > x̄, y > ȳ (prawy przód)
- Q2: x < x̄, y > ȳ (lewy przód)
- Q3: x < x̄, y < ȳ (lewy tył)
- Q4: x > x̄, y < ȳ (prawy tył)

**Zmienne:**
- `x̄, ȳ` – średnia pozycja CoP
- Liczba próbek w każdej ćwiartce / N × 100%

**Opis:** Dystrybucja czasu w przestrzeni.

**Zastosowanie:** Identyfikacja preferencji posturalnych (np. chęć przeniesienia ciężaru na przód).

---

### 46. Minimalna powierzchnia otoczki (Minimum Enclosing Circle Area)
**Wzór:** Obliczana geometrycznie (algorytm Welzla lub podobny)

**Zmienne:**
- `xi, yi` – wszystkie współrzędne CoP
- Promień最小的 okręgu obejmującego wszystkie punkty

**Opis:** Pole najmniejszego koła obejmującego wszystkie punkty CoP.

**Zastosowanie:** Alternatywa dla Convex Hull, bardziej wrażliwa na symetrię.

---

### 47. Długość fraktalna trajektorii (Fractal Path Length)
**Wzór:** Estymacja długości trajektorii przy różnych rozdzielczościach (skalach)

**Zmienne:**
- `xi, yi` – współrzędne CoP
- Różne skale pomiaru (ε)

**Opis:** Miara złożoności samej ścieżki CoP.

**Zastosowanie:** Związana z efektywnością mikro-korekt.

---

### 48. Stosunek długości trajektorii do promienia (Path/Radius Ratio)
**Wzór:**
```
RPR = L / Rmean
```
gdzie:
```
Rmean = (1/N) × Σ(√((xi - x̄)² + (yi - ȳ)²))  dla i=1 do N
```

**Zmienne:**
- `L` – całkowita droga migracji CoP
- `Rmean` – średnia odległość od środka
- `N` – liczba próbek

**Opis:** Relacja między aktywnością ruchową a średnim wychyleniem.

**Zastosowanie:** Miara efektywności ruchu.

---

### 49. Wariancja prędkości (Velocity Variance)
**Wzór:**
```
VV = Var(√((Δxi)² + (Δyi)²) / Δt)
```

**Zmienne:**
- `Δxi = x(i+1) - xi`
- `Δyi = y(i+1) - yi`
- `Δt` – czas między próbkami (1/częstotliwość próbkowania)
- `Var` – wariancja

**Opis:** Miara zmienności prędkości ruchu CoP.

**Zastosowanie:** Wskaźnik niestabilności tempa korekt.

---

### 50. Indeks efektywności (Efficiency Index)
**Wzór:**
```
EI = Aconvex_hull / L
```

**Zmienne:**
- `Aconvex_hull` – powierzchnia otoczki wypukłej
- `L` – całkowita droga migracji CoP

**Opis:** Proporcja obszaru eksplorowanego do całkowitej drogi.

**Zastosowanie:** Miara efektywności przemieszczania się środka ciężkości.

---

## Podsumowanie

Parametry te pozwalają na kompleksową analizę stabilności postawy z różnych perspektyw:

### Statyczna (rozmiar i forma wychyleń):
- Powierzchnia elipsy zaufania
- RMS (Root Mean Square)
- SD (odchylenie standardowe)
- Range (zakres wychyleń)
- Convex Hull Area

### Kinematyczna (ruch i tempo):
- Całkowita droga migracji (Path Length)
- Średnia prędkość (Mean Velocity)
- Częstotliwość migracji (Sway Frequency)
- Liczba pików i zerowych przejść

### Spektralna (mechanizmy kontrolne):
- Moc w pasmach LF/HF
- Dominująca częstotliwość (Peak Frequency)
- Stosunek HF/LF
- Częstotliwość środkowa (Mean Frequency)

### Nieliniowa (złożoność i adaptacyjność):
- Entropie (ApEn, SampEn, Shannon, MSE)
- Wymiar fraktalny (Fractal Dimension)
- Współczynnik Hursta
- DFA (Alpha)

---

## Ważne uwagi

1. **Nie istnieje jeden uniwersalny parametr** – interpretacja wyników wymaga analizy wielu wskaźników w kontekście klinicznym i celu badania.

2. **Częstotliwość próbkowania** – większość parametrów zależy od częstotliwości próbkowania (zalecane minimum 50-100 Hz).

3. **Czas trwania pomiaru** – parametry statyczne wymagają minimum 30-60 sekund, parametry częstotliwościowe i nieliniowe mogą wymagać dłuższych rejestracji.

4. **Warunki badania** – wyniki zależą od:
   - Pozycji stóp (razem, osobno, tandem)
   - Otwarte/zamknięte oczy
   - Podłoże (twarde, miękkie, niestabilne)
   - Wiek i stan zdrowia pacjenta

5. **Wartości referencyjne** – należy korzystać z norm dostosowanych do wieku, płci i warunków badania.

---

*Opracowano na podstawie literatury z zakresu posturografii i biomechaniki.*
