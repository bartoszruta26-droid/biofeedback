# Analiza Parametrów Siły Chwytu i Wytrzymałości Mięśniowej (Test Ściskania Piłki)

## Wstęp
Dokumentacja dotyczy analizy sygnału ciśnienia $P(t)$ (proporcjonalnego do siły $F(t)$) zarejestrowanego podczas testu ściskania piłki rehabilitacyjnej/sportowej wyposażonej w czujnik.
**Protokół testu:** 3 serie po 8 powtórzeń maksymalnych lub submaksymalnych ściśnięć.

### Definicje Zmiennych i Stałych
W poniższych wzorach stosuje się następujące oznaczenia:
*   $F(t)$: Wartość siły (lub ciśnienia) w czasie $t$.
*   $t$: Czas trwania pomiaru.
*   $N$: Liczba powtórzeń w serii (w tym protokole $N=8$).
*   $S$: Liczba serii (w tym protokole $S=3$).
*   $i$: Indeks powtórzenia ($i = 1, \dots, N$).
*   $j$: Indeks serii ($j = 1, \dots, S$).
*   $F_{peak, i, j}$: Maksymalna siła osiągnięta w $i$-tym powtórzeniu $j$-tej serii.
*   $t_{start, i, j}$: Moment rozpoczęcia $i$-tego powtórzenia (przekroczenie progu bazowego).
*   $t_{end, i, j}$: Moment zakończenia $i$-tego powtórzenia (powrót do progu bazowego).
*   $t_{peak, i, j}$: Moment osiągnięcia siły szczytowej w danym powtórzeniu.
*   $\Delta t_{i, j} = t_{end, i, j} - t_{start, i, j}$: Czas trwania pojedynczego powtórzenia (faza aktywna).
*   $\bar{F}_{base}$: Średnia siła w fazie spoczynku (baseline).
*   $MVC$: Maksymalna Ochotnicza Kontraksja (wartość referencyjna, np. najlepsze ściśnięcie w całym teście).

---

## I. Parametry Siły Maksymalnej i Szczytowej

### 1. Maksymalna Siła Szczytowa (Global Peak Force)
*   **Wzór:** $F_{global\_max} = \max_{j} (\max_{i} (F_{peak, i, j}))$
*   **Opis:** Absolutnie najwyższa wartość siły zarejestrowana w całym teście.
*   **Cel:** Ocena maksymalnego potencjału siłowego mięśni zginaczy przedramienia.
*   **Trend poprawy:** 📈 **Wzrost** (większa siła maksymalna).

### 2. Średnia Siła Szczytowa (Mean Peak Force)
*   **Wzór:** $\bar{F}_{peak} = \frac{1}{S \cdot N} \sum_{j=1}^{S} \sum_{i=1}^{N} F_{peak, i, j}$
*   **Opis:** Średnia arytmetyczna wszystkich szczytów siły.
*   **Cel:** Ogólna ocena poziomu siłowego pacjenta z niwelacją przypadkowych błędów pojedynczych ruchów.
*   **Trend poprawy:** 📈 **Wzrost**.

### 3. Siła Szczytowa w Pierwszym Powtórzeniu (First Rep Peak)
*   **Wzór:** $F_{first} = \frac{1}{S} \sum_{j=1}^{S} F_{peak, 1, j}$
*   **Opis:** Średnia siła pierwszych ściśnięć w każdej serii.
*   **Cel:** Ocena siły "świeżej", bez wpływu zmęczenia metabolicznego.
*   **Trend poprawy:** 📈 **Wzrost**.

### 4. Siła Szczytowa w Ostatnim Powtórzeniu (Last Rep Peak)
*   **Wzór:** $F_{last} = \frac{1}{S} \sum_{j=1}^{S} F_{peak, N, j}$
*   **Opis:** Średnia siła ostatnich ściśnięć w każdej serii.
*   **Cel:** Ocena zdolności do generowania siły w stanie zmęczenia.
*   **Trend poprawy:** 📈 **Wzrost**.

### 5. Różnica Siły Szczytowej (Peak Drop Absolute)
*   **Wzór:** $\Delta F_{drop} = F_{first} - F_{last}$
*   **Opis:** Bezwzględny ubytek siły między początkiem a końcem serii.
*   **Cel:** Ilościowa ocena utraty siły na skutek zmęczenia.
*   **Trend poprawy:** 📉 **Spadek** (mniejszy spadek siły oznacza lepszą wytrzymałość).

### 6. Wskaźnik Zmęczenia (Fatigue Index - FI)
*   **Wzór:** $FI = \frac{F_{first} - F_{last}}{F_{first}} \times 100\%$
*   **Opis:** Procentowy spadek siły od pierwszego do ostatniego powtórzenia.
*   **Cel:** Normalizowana miara odporności na zmęczenie.
*   **Trend poprawy:** 📉 **Spadek** (wartość bliższa 0% jest lepsza).

### 7. Wskaźnik Wytrzymałości (Endurance Ratio)
*   **Wzór:** $ER = \frac{\text{Średnia } F_{peak} \text{ w serii 3}}{\text{Średnia } F_{peak} \text{ w serii 1}} \times 100\%$
*   **Opis:** Stosunek utrzymania siły w ostatniej serii do pierwszej.
*   **Cel:** Ocena wytrzymałości siłowej w skali całego treningu (między seriami).
*   **Trend poprawy:** 📈 **Wzrost** (wartość bliższa lub >100%).

### 8. Całkowity Impuls Siły (Total Impulse)
*   **Wzór:** $I_{total} = \sum_{j=1}^{S} \sum_{i=1}^{N} \int_{t_{start, i, j}}^{t_{end, i, j}} (F(t) - F_{base}) \, dt$
*   **Opis:** Pole pod wykresem siły w czasie dla wszystkich powtórzeń (po odjęciu baseline).
*   **Cel:** Miara całkowitej pracy wykonanej przez mięśnie.
*   **Trend poprawy:** 📈 **Wzrost**.

### 9. Średni Impuls na Powtórzenie (Mean Impulse per Rep)
*   **Wzór:** $\bar{I}_{rep} = \frac{I_{total}}{S \cdot N}$
*   **Opis:** Uśredniona praca wykonana w jednym ściśnięciu.
*   **Cel:** Ocena efektywności pojedynczego ruchu.
*   **Trend poprawy:** 📈 **Wzrost**.

### 10. Impuls Siły w Serii (Series Impulse)
*   **Wzór:** $I_{series, j} = \sum_{i=1}^{N} \int_{t_{start, i, j}}^{t_{end, i, j}} (F(t) - F_{base}) \, dt$
*   **Opis:** Suma pól pod krzywą dla danej serii $j$.
*   **Cel:** Analiza spadku wydolności między seriami.
*   **Trend poprawy:** 📈 **Wzrost** (lub mniejszy spadek między seriami).

---

## II. Parametry Dynamiki i Szybkości

### 11. Szybkość Narastania Siły (Rate of Force Development - RFD)
*   **Wzór:** $RFD_{avg} = \frac{F_{peak, i, j} - F_{base}}{t_{peak, i, j} - t_{start, i, j}}$
*   **Opis:** Średnie tempo wzrostu siły w fazie koncentrycznej. Często liczone dla konkretnego okna (np. pierwsze 100ms).
*   **Cel:** Ocena mocy eksplozywnej mięśni. Kluczowe w sporcie i zapobieganiu upadkom.
*   **Trend poprawy:** 📈 **Wzrost**.

### 12. Maksymalne RFD (Peak RFD)
*   **Wzór:** $RFD_{max} = \max \left( \frac{dF(t)}{dt} \right)$ w przedziale $[t_{start}, t_{peak}]$
*   **Opis:** Największe chwilowe nachylenie krzywej siły.
*   **Cel:** Ocena maksymalnej zdolności do szybkiej rekrutacji jednostek motorycznych.
*   **Trend poprawy:** 📈 **Wzrost**.

### 13. Czas Dojścia do Szczytu (Time to Peak - TTP)
*   **Wzór:** $TTP_{i, j} = t_{peak, i, j} - t_{start, i, j}$
*   **Opis:** Czas potrzebny na osiągnięcie maksymalnej siły od momentu startu.
*   **Cel:** Ocena szybkości reakcji mięśniowej. Krótszy czas przy wysokiej sile = duża moc.
*   **Trend poprawy:** 📉 **Spadek** (szybsze osiąganie szczytu), o ile nie kosztem siły.

### 14. Średni Czas Dojścia do Szczytu (Mean TTP)
*   **Wzór:** $\overline{TTP} = \frac{1}{S \cdot N} \sum_{j=1}^{S} \sum_{i=1}^{N} TTP_{i, j}$
*   **Opis:** Uśredniony czas narastania siły.
*   **Cel:** Ogólna charakterystyka dynamiki ruchu pacjenta.
*   **Trend poprawy:** 📉 **Spadek** (bardziej dynamiczny ruch).

### 15. Szybkość Rozluźnienia (Rate of Relaxation - ROR)
*   **Wzór:** $ROR_{avg} = \frac{F_{peak, i, j} - F_{base}}{t_{end, i, j} - t_{peak, i, j}}$
*   **Opis:** Tempo spadku siły po osiągnięciu szczytu.
*   **Cel:** Ocena zdolności do szybkiego rozkurczu mięśnia (ważne w koordynacji).
*   **Trend poprawy:** 📈 **Wzrost** (szybszy rozkurcz).

### 16. Czas Rozluźnienia (Relaxation Time)
*   **Wzór:** $RT_{i, j} = t_{end, i, j} - t_{peak, i, j}$
*   **Opis:** Czas od szczytu do powrotu do wartości bazowej.
*   **Cel:** Detekcja sztywności mięśniowej lub trudności z kontrolą eccentriczną.
*   **Trend poprawy:** 📉 **Spadek** (szybszy powrót do spoczynku).

### 17. Stosunek Czasu Narastania do Spadku (Contraction/Relaxation Ratio)
*   **Wzór:** $R_{CR} = \frac{TTP_{i, j}}{RT_{i, j}}$
*   **Opis:** Proporcja czasu fazy ściskania do fazy puszczania.
*   **Cel:** Ocena symetrii kontroli ruchu. Zdrowy wzorzec często ma szybszą fazę ekscentryczną.
*   **Trend poprawy:** ➡️ **Optymalizacja** (zbliżenie do wzorca fizjologicznego, często < 1).

### 18. Czas do Progowych Wartości Siły (Time to Thresholds)
*   **Wzór:** $t_{x\%} = \text{czas, gdy } F(t) = x\% \cdot F_{peak}$
*   **Opis:** Czasy osiągnięcia 25%, 50%, 75% siły maksymalnej.
*   **Cel:** Szczegółowa analiza profilu przyspieszenia siły.
*   **Trend poprawy:** 📉 **Spadek** (szybsze osiąganie progów).

### 19. Siła Eksplozywna (Explosive Force at 1s)
*   **Wzór:** $F_{1s} = F(t_{start} + 1000ms)$ (jeśli $TTP > 1s$) lub $F_{peak}$
*   **Opis:** Siła osiągnięta w określonym, krótkim czasie od startu.
*   **Cel:** Ocena mocy w specyficznym oknie czasowym.
*   **Trend poprawy:** 📈 **Wzrost**.

---

## III. Parametry Stabilności i Kontroli Ruchu

### 20. Współczynnik Chwiejności Szczytu (Peak Variability)
*   **Wzór:** $CV_{peak} = \frac{\sigma(F_{peak, i})}{\mu(F_{peak, i})} \times 100\%$ (dla powtórzeń w serii)
*   **Opis:** Współczynnik zmienności (odchylenie standardowe / średnia) sił szczytowych.
*   **Cel:** Ocena powtarzalności i stabilności układu nerwowego.
*   **Trend poprawy:** 📉 **Spadek** (bardziej przewidywalny ruch).

### 21. Liczba Mikrokorekt (Micro-corrections Count)
*   **Wzór:** Liczba lokalnych ekstremów (min/max) w pochodnej $dF/dt$ w fazie plateau.
*   **Opis:** Ilość drobnych korekt siły podczas próby utrzymania maksymalnego napięcia.
*   **Cel:** Wykrywanie drżeń mięśniowych lub niepewności neurologicznej.
*   **Trend poprawy:** 📉 **Spadek** (płynniejszy ruch).

### 22. Gładkość Ruchu (Jerk Index)
*   **Wzór:** $Jerk = \frac{1}{T} \int_{0}^{T} \left| \frac{d^3F(t)}{dt^3} \right| dt$ (lub aproksymacja dyskretna)
*   **Opis:** Miara zmiany przyspieszenia siły (trzecia pochodna).
*   **Cel:** Ocena płynności sterowania mięśniem. Wysoki jerk = ruch szarpany.
*   **Trend poprawy:** 📉 **Spadek**.

### 23. Kontrola Plateau (Plateau Control Error)
*   **Wzór:** $Error_{plateau} = \frac{1}{T_{hold}} \int_{t_1}^{t_2} |F(t) - F_{target}| \, dt$
*   **Opis:** Średni błąd utrzymania zadanej siły (np. 80% MVC) w zadanym czasie.
*   **Cel:** Ocena precyzji modulacji siły.
*   **Trend poprawy:** 📉 **Spadek** (mniejszy błąd).

### 24. Czas Reakcji (Reaction Time)
*   **Wzór:** $RT = t_{start} - t_{stimulus}$
*   **Opis:** Czas od bodźca (wizualnego/dźwiękowego) do przekroczenia progu siły.
*   **Cel:** Ocena szybkości przetwarzania informacji i odpowiedzi motorycznej.
*   **Trend poprawy:** 📉 **Spadek**.

### 25. Regularność Cykli (Cycle Regularity)
*   **Wzór:** $\sigma_{\Delta t} = \sqrt{\frac{1}{N-1}\sum(\Delta t_i - \overline{\Delta t})^2}$
*   **Opis:** Odchylenie standardowe czasów trwania poszczególnych powtórzeń.
*   **Cel:** Ocena rytmiczności wykonywania ćwiczenia.
*   **Trend poprawy:** 📉 **Spadek** (bardziej metronomiczny rytm).

### 26. Współczynnik Zmienności Siły (Coefficient of Variation - CV)
*   **Wzór:** $CV = \frac{\sigma_F}{\mu_F} \times 100\%$ (liczone dla całej serii)
*   **Opis:** Relacja rozrzutu siły do jej średniej wartości.
*   **Cel:** Uniezależniona od wielkości siły miara stabilności.
*   **Trend poprawy:** 📉 **Spadek**.

### 27. Stosunek Pracy Negatywnej do Pozytywnej (Eccentric/Concentric Ratio)
*   **Wzór:** $R_{work} = \frac{\int_{t_{peak}}^{t_{end}} F(t) dt}{\int_{t_{start}}^{t_{peak}} F(t) dt}$
*   **Opis:** Porównanie pola pod krzywą w fazie opuszczania i podnoszenia.
*   **Cel:** Ocena balansu między fazą koncentryczną a ekscentryczną.
*   **Trend poprawy:** ➡️ **Zbalansowanie** (zależne od celu treningowego).

### 28. Minimalna Siła Między Powtórzeniami (Baseline Stability)
*   **Wzór:** $\bar{F}_{rest} = \frac{1}{N-1} \sum \text{średnia}(F(t))$ w przerwach między powtórzeniami.
*   **Opis:** Średnia siła w fazach relaksu.
*   **Cel:** Wykrywanie niepełnego rozluźnienia mięśnia (przykurcze, wysoki tonus).
*   **Trend poprawy:** 📉 **Spadek** (do wartości bazowej/blisko 0).

### 29. Indeks Efektywności Energetycznej (Efficiency Index)
*   **Wzór:** $EI = \frac{I_{total}}{\sum \Delta t_{active}}$
*   **Opis:** Stosunek całkowitego impulsu do czasu aktywnej pracy.
*   **Cel:** Ile pracy wykonuje mięsień w jednostce czasu aktywności.
*   **Trend poprawy:** 📈 **Wzrost**.

### 30. Asymetria Czasu (Time Symmetry Index)
*   **Wzór:** $ASI_{time} = \frac{|TTP - RT|}{TTP + RT} \times 100\%$
*   **Opis:** Miara asymetrii czasowej między ściskaniem a puszczaniem.
*   **Cel:** Identyfikacja dominacji jednej fazy ruchu.
*   **Trend poprawy:** 📉 **Spadek** (dążenie do symetrii, chyba że celowo trenujemy jedną fazę).

---

## IV. Parametry Specyficzne dla Serii i Powtórzeń

### 31. Deficyt Regeneracji Międzyseriowej (Inter-set Recovery Deficit)
*   **Wzór:** $Deficit = \frac{\bar{F}_{peak, seria 1} - \bar{F}_{peak, seria 2}}{\bar{F}_{peak, seria 1}} \times 100\%$
*   **Opis:** Procentowy spadek średniej siły między kolejnymi seriami.
*   **Cel:** Ocena zdolności regeneracyjnych w krótkim czasie.
*   **Trend poprawy:** 📉 **Spadek** (lepsza regeneracja).

### 32. Spadek Impulsu w Serii (Intra-set Impulse Drop)
*   **Wzór:** $\Delta I_{half} = \sum_{i=1}^{N/2} I_i - \sum_{i=N/2+1}^{N} I_i$
*   **Opis:** Różnica sumy impulsów między pierwszą a drugą połową serii.
*   **Cel:** Wykrywanie momentu wystąpienia zmęczenia w trakcie serii.
*   **Trend poprawy:** 📉 **Spadek** (mniejsza różnica).

### 33. Trend Zmęczenia Liniowego (Linear Fatigue Slope)
*   **Wzór:** $k = \text{nachylenie prostej regresji } F_{peak, i} = k \cdot i + b$
*   **Opis:** Nachylenie linii trendu siły szczytowej w funkcji numeru powtórzenia.
*   **Cel:** Ilościowa ocena tempa narastania zmęczenia.
*   **Trend poprawy:** 📉 **Spadek** (mniej ujemne nachylenie, bliższe 0).

### 34. Procentowy Udział Fazy Maksymalnej (Peak Duration Ratio)
*   **Wzór:** $R_{peak} = \frac{t(F > 0.9 \cdot F_{peak})}{\Delta t_{rep}} \times 100\%$
*   **Opis:** Jaki procent czasu trwania powtórzenia mięsień pracuje powyżej 90% maksimum.
*   **Cel:** Ocena zdolności do utrzymania wysokiego napięcia.
*   **Trend poprawy:** 📈 **Wzrost**.

### 35. Wskaźnik "Lock-out" (Stability at Peak)
*   **Wzór:** Liczba powtórzeń, gdzie $\sigma_F$ w oknie $\pm 50ms$ wokół szczytu > próg.
*   **Opis:** Detekcja niestabilności w momencie największego wysiłku.
*   **Cel:** Bezpieczeństwo stawów przy maksymalnym obciążeniu.
*   **Trend poprawy:** 📉 **Spadek** liczby niestabilnych powtórzeń.

### 36. Kurtoza Rozkładu Siły (Force Kurtosis)
*   **Wzór:** $K = \frac{1}{N}\sum(\frac{F_i - \mu}{\sigma})^4 - 3$
*   **Opis:** Miara "spiczastości" rozkładu wartości siły w czasie.
*   **Cel:** Wysoka kurtoza = dużo czasu spent w bazie i szczycie, mało w przejściach (ruch schodkowy). Niska = ruch sinusoidalny.
*   **Trend poprawy:** ➡️ **Zależny od celu** (dla mocy często wyższa, dla płynności niższa).

### 37. Mediana Siły Szczytowej (Median Peak Force)
*   **Wzór:** $Med(F_{peak})$
*   **Opis:** Wartość środkowa uporządkowanego zbioru sił szczytowych.
*   **Cel:** Odporna na wartości odstające (outliers) miara centralna.
*   **Trend poprawy:** 📈 **Wzrost**.

### 38. Odchylenie od Celu (Target Deviation)
*   **Wzór:** $Err_{target} = \frac{1}{N} \sum |F_{peak, i} - F_{cel}|$
*   **Opis:** Średni błąd absolutny względem zadanej siły docelowej (np. w treningu propriocepcji).
*   **Cel:** Ocena czucia głębokiego i kontroli dawki wysiłku.
*   **Trend poprawy:** 📉 **Spadek**.

### 39. Czas do Pierwszego 50% Maksimum (Time to 50%)
*   **Wzór:** $t_{50\%} = t(F=0.5 \cdot F_{peak}) - t_{start}$
*   **Opis:** Szybkość wejścia w połowę zakresu siły.
*   **Cel:** Wczesna faza rekrutacji włókien.
*   **Trend poprawy:** 📉 **Spadek**.

### 40. Czas Trwania Fazy Plateau (Plateau Duration)
*   **Wzór:** $T_{plat} = \text{czas, przez który } F(t) > 0.95 \cdot F_{peak}$
*   **Opis:** Długość utrzymywania siły bliskiej maksimum.
*   **Cel:** Wytrzymałość statyczna w szczytowym zakresie.
*   **Trend poprawy:** 📈 **Wzrost**.

---

## V. Parametry Zaawansowane i Statystyczne

### 41. Powierzchnia Pętli Histerezy (Hysteresis Loop Area)
*   **Wzór:** $A_{hyst} = \oint F \, d(\text{deformacja})$ (aproksymowane jako pole między krzywą narastania i opadania po normalizacji czasu).
*   **Opis:** Energia tracona wewnątrz materiału (piłki) i układu mięśniowego podczas cyklu.
*   **Cel:** Ocena właściwości lepkosprężystych i efektywności mechanicznej.
*   **Trend poprawy:** 📉 **Spadek** (mniejsze straty energii, większa sprężystość) lub stabilizacja.

### 42. Entropia Shannona Siły (Shannon Entropy)
*   **Wzór:** $H = -\sum p(x) \log p(x)$, gdzie $p(x)$ to histogram wartości siły.
*   **Opis:** Miara nieuporządkowania sygnału siły.
*   **Cel:** Bardzo niska entropia = sztywność; bardzo wysoka = chaos/trzor. Optymalna = zdrowa adaptacyjność.
*   **Trend poprawy:** ➡️ **Optymalizacja** (unikanie skrajności).

### 43. Długość Krzywej Siły (Path Length)
*   **Wzór:** $L = \sum \sqrt{(\Delta t)^2 + (\Delta F)^2}$
*   **Opis:** Geometryczna długość trajektorii wykresu siły w czasie.
*   **Cel:** Miara złożoności i "nerwowości" ruchu. Dłuższa krzywa = więcej mikroruchów.
*   **Trend poprawy:** 📉 **Spadek** (bardziej bezpośrednia droga do celu).

### 44. Wariancja Siły w Serii (Within-Set Variance)
*   **Wzór:** $\sigma^2_{series, j} = \frac{1}{N-1}\sum_{i=1}^N (F_{peak, i, j} - \bar{F}_{peak, j})^2$
*   **Opis:** Rozrzut siły w obrębie jednej serii.
*   **Cel:** Spójność neuronalna w warunkach narastającego zmęczenia.
*   **Trend poprawy:** 📉 **Spadek**.

### 45. Czas do 90% Maksimum w 1. powtórzeniu (Early Phase Dynamics)
*   **Wzór:** $t_{90\%, 1} = t(F=0.9 \cdot F_{peak, 1}) - t_{start, 1}$
*   **Opis:** Dynamika w pierwszym, najświeższym powtórzeniu.
*   **Cel:** Bazowa ocena mocy eksplozywnej.
*   **Trend poprawy:** 📉 **Spadek**.

### 46. Skuteczność Utrzymania (Hold Efficiency)
*   **Wzór:** $HE = \frac{T(F > 0.8 \cdot F_{peak})}{\Delta t_{rep}}$
*   **Opis:** Frakcja czasu, w którym siła jest efektywnie wysoka.
*   **Cel:** Jak dużo czasu marnowanego jest na fazach przejściowych.
*   **Trend poprawy:** 📈 **Wzrost**.

### 47. Wskaźnik Oscylacji (Oscillation Index)
*   **Wzór:** $OI = \frac{\text{Liczba przecięć średniej}}{\Delta t_{rep}}$
*   **Opis:** Częstotliwość oscylacji siły wokół średniej w trakcie trwania.
*   **Cel:** Wykrywanie tremoru fizjologicznego lub patologicznego.
*   **Trend poprawy:** 📉 **Spadek**.

### 48. Pochodna Siły w 50% Maksimum (Mid-range Slope)
*   **Wzór:** $Slope_{50\%} = \frac{dF}{dt} \bigg|_{F=0.5 F_{peak}}$
*   **Opis:** Nachylenie krzywej w połowie zakresu siły.
*   **Cel:** Charakterystyka środkowej fazy rekrutacji.
*   **Trend poprawy:** 📈 **Wzrost** (steeper slope).

### 49. Czas Trwania Pełnego Cyklu (Total Cycle Time)
*   **Wzór:** $T_{cycle} = t_{start, i+1} - t_{start, i}$
*   **Opis:** Czas od startu jednego powtórzenia do startu następnego (wliczając przerwę).
*   **Cel:** Ocena tempa pracy i długości przerw regeneracyjnych.
*   **Trend poprawy:** ➡️ **Utrzymanie założonego rytmu**.

### 50. Wariancja Czasu Cyklu (Cycle Time Variance)
*   **Wzór:** $\sigma^2_{T_{cycle}}$
*   **Opis:** Zmienność tempa wykonywania kolejnych powtórzeń.
*   **Cel:** Dyscyplina ruchowa i kontrola tempa.
*   **Trend poprawy:** 📉 **Spadek**.

### 51. Skumulowana Siła (Cumulative Force Load)
*   **Wzór:** $F_{cum} = \sum_{k=1}^{M} F_k \cdot \Delta t$ (suma wszystkich próbek siły).
*   **Opis:** Całkowite obciążenie układu mięśniowego w teście.
*   **Cel:** Miara objętości treningowej.
*   **Trend poprawy:** 📈 **Wzrost** (zdolność do zniesienia większego ładunku).

### 52. Dominująca Częstotliwość Fluktuacji (Dominant Frequency)
*   **Wzór:** $f_{dom} = \arg \max_f (FFT(F(t)))$ w paśmie 0-10Hz.
*   **Opis:** Częstotliwość, z którą najczęściej drży mięsień podczas utrzymania siły.
*   **Cel:** Diagnoza typu drżeń (fizjologiczne ~8-12Hz, patologiczne inne).
*   **Trend poprawy:** ➡️ **Przesunięcie w stronę normy fizjologicznej**.

### 53. Wskaźnik Symetrii Serii (Series Symmetry Index)
*   **Wzór:** $SI_{series} = \frac{|\bar{F}_{seria1} - \bar{F}_{seria3}|}{\bar{F}_{seria1} + \bar{F}_{seria3}}$
*   **Opis:** Porównanie początku i końca całego testu (seria 1 vs 3).
*   **Cel:** Globalna odporność na zmęczenie.
*   **Trend poprawy:** 📉 **Spadek**.

### 54. Czas do Drugiego Szczytu (Double Peak Detection)
*   **Wzór:** $\Delta t_{peak2} = t_{peak2} - t_{peak1}$ (jeśli występuje drugi lokalny maks).
*   **Opis:** Detekcja prób korekty siły w trakcie jednego ściśnięcia.
*   **Cel:** Wykrywanie niepewności ruchowej lub bólu hamującego ruch.
*   **Trend poprawy:** 📉 **Brak drugiego szczytu** (pojedynczy, gładki pik).

### 55. Miara "Rozmycia" Szczytu (Peak Spread)
*   **Wzór:** Szerokość połówkowa szczytu (FWHM) na wykresie siły.
*   **Opis:** Jak ostry jest szczyt siły.
*   **Cel:** Ostry szczyt = ruch dynamiczny; szeroki = ruch wolny/zahamowany.
*   **Trend poprawy:** ➡️ **Zależny od celu** (dla mocy: wąski; dla kontroli: szerszy).

### 56. Skala Fraktalna Sygnału (Fractal Dimension)
*   **Wzór:** Metoda Higuchi lub Katz na sygnale $F(t)$.
*   **Opis:** Miara złożoności geometrycznej krzywej siły.
*   **Cel:** Zdrowe systemy biologiczne mają określoną złożoność fraktalną. Choroba/starzenie często ją redukuje.
*   **Trend poprawy:** ➡️ **Powrót do normy dla wieku** (często wzrost złożoności).

### 57. Wskaźnik Adaptacji w Serii (Intra-set Adaptation)
*   **Wzór:** Korelacja między numerem powtórzenia a czasem trwania fazy ekscentrycznej.
*   **Opis:** Czy pacjent zwalnia z czasem, by kompensować utratę siły?
*   **Cel:** Strategia radzenia sobie ze zmęczeniem.
*   **Trend poprawy:** ➡️ **Mniejsza konieczność kompensacji**.

### 58. Średnia Siła Podstawowa (Baseline Mean)
*   **Wzór:** $\mu_{base} = \text{mean}(F(t))$ dla $t$ poza fazami aktywnymi.
*   **Opis:** Poziom napięcia spoczynkowego.
*   **Cel:** Wykrywanie stresu, bólu lub niewłaściwego ustawienia ręki.
*   **Trend poprawy:** 📉 **Spadek** (do zera/normy).

### 59. Współczynnik Korelacji Siły w Seriach (Inter-set Correlation)
*   **Wzór:** $r = \text{corr}(Seria_1, Seria_2)$ (wektory sił w czasie).
*   **Opis:** Podobieństwo kształtu krzywych siły między seriami.
*   **Cel:** Powtarzalność wzorca ruchowego mimo zmęczenia.
*   **Trend poprawy:** 📈 **Wzrost** (bliższe 1.0).

### 60. Wskaźnik Złożoności Ruchu (Movement Complexity Index)
*   **Wzór:** Kombinacja Entropii Próbkowej (SampEn) i DFA (Detrended Fluctuation Analysis).
*   **Opis:** Syntetyczna miara zdrowia układu neuromięśniowego.
*   **Cel:** Układy chore są zbyt regularne (sztywne) lub zbyt losowe.
*   **Trend poprawy:** ➡️ **Optymalizacja** (złożoność umiarkowana).

---

## Podsumowanie Interpretacji Trendów

*   **📈 Wzrost:** Pożądany dla parametrów siły, mocy, wytrzymałości, efektywności i powtarzalności wzorca.
*   **📉 Spadek:** Pożądany dla parametrów czasu (reakcji, dojścia do szczytu), zmienności (CV, wariancja), zmęczenia (spadki siły) i drżeń.
*   **➡️ Optymalizacja:** Dla parametrów, które mają wartość złotego środka (entropia, symetria, kształt krzywej), gdzie zarówno zbyt niskie, jak i zbyt wysokie wartości mogą świadczyć o patologii.
