# Wzory Matematyczne - Parametry Biomechaniczne i Treningowe

## Wprowadzenie

Dokument zawiera wzory matematyczne do obliczania ponad 50 parametrów biomechanicznych i treningowych na podstawie danych z tensometru podłączonego do Raspberry Pi podczas ćwiczeń z taśmą lateksową (3 serie × 8 powtórzeń).

---

## Definicje Zmiennych i Stałych

### Zmienne Podstawowe

| Symbol | Opis | Jednostka |
|--------|------|-----------|
| $F(t)$ | Siła mierzona w czasie $t$ | N (Newton) |
| $t$ | Czas pomiaru | s (sekunda) |
| $t_0$ | Czas początkowy serii | s |
| $t_{end}$ | Czas końcowy serii | s |
| $n$ | Numer powtórzenia ($n = 1, 2, ..., 8$) | - |
| $s$ | Numer serii ($s = 1, 2, 3$) | - |
| $N_{reps}$ | Liczba powtórzeń w serii (= 8) | - |
| $N_{sets}$ | Liczba serii (= 3) | - |
| $\Delta t$ | Interwał próbkowania | s |
| $f_s$ | Częstotliwość próbkowania | Hz |

### Stałe i Parametry Kalibracyjne

| Symbol | Opis | Wartość typowa |
|--------|------|----------------|
| $F_{max\_sensor}$ | Maksymalna siła czujnika | zależna od sensora |
| $k_{elastic}$ | Stała sprężystości taśmy | N/m |
| $L_0$ | Długość początkowa taśmy | m |
| $m_{patient}$ | Masa pacjenta | kg |
| $g$ | Przyspieszenie ziemskie | 9.81 m/s² |

---

## Kategoria 1: Parametry Siłowe (1-10)

### 1. Maksymalna Siła Chwilowa (Peak Force)

$$F_{peak}^{(s,n)} = \max_{t \in [t_{start}^{(s,n)}, t_{end}^{(s,n)}]} F(t)$$

**Opis:** Maksymalna wartość siły osiągnięta podczas pojedynczego powtórzenia.

**Składowe:**
- $F_{peak}^{(s,n)}$ - szczytowa siła dla serii $s$, powtórzenia $n$
- $t_{start}^{(s,n)}$ - czas rozpoczęcia powtórzenia
- $t_{end}^{(s,n)}$ - czas zakończenia powtórzenia

---

### 2. Średnia Siła z Powtórzenia (Mean Force per Rep)

$$\bar{F}^{(s,n)} = \frac{1}{T^{(s,n)}} \int_{t_{start}^{(s,n)}}^{t_{end}^{(s,n)}} F(t) \, dt$$

**Wersja dyskretna:**

$$\bar{F}^{(s,n)} = \frac{1}{M} \sum_{i=1}^{M} F_i^{(s,n)}$$

**Opis:** Średnia wartość siły w trakcie trwania pojedynczego powtórzenia.

**Składowe:**
- $\bar{F}^{(s,n)}$ - średnia siła dla serii $s$, powtórzenia $n$
- $T^{(s,n)}$ - czas trwania powtórzenia
- $M$ - liczba próbek w powtórzeniu
- $F_i^{(s,n)}$ - $i$-ta próbka siły

---

### 3. Minimalna Siła z Powtórzenia (Minimum Force)

$$F_{min}^{(s,n)} = \min_{t \in [t_{start}^{(s,n)}, t_{end}^{(s,n)}]} F(t)$$

**Opis:** Minimalna wartość siły podczas fazy relaksacji między powtórzeniami.

---

### 4. Zakres Siły (Force Range)

$$R_F^{(s,n)} = F_{peak}^{(s,n)} - F_{min}^{(s,n)}$$

**Opis:** Różnica między maksymalną a minimalną siłą w powtórzeniu.

---

### 5. Średnia Siła z Serii (Mean Force per Set)

$$\bar{F}_{set}^{(s)} = \frac{1}{N_{reps}} \sum_{n=1}^{N_{reps}} \bar{F}^{(s,n)}$$

**Opis:** Średnia siła ze wszystkich powtórzeń w danej serii.

---

### 6. Maksymalna Siła z Serii (Maximum Force per Set)

$$F_{peak\_set}^{(s)} = \max_{n \in [1, N_{reps}]} F_{peak}^{(s,n)}$$

**Opis:** Najwyższa siła osiągnięta w którejkolwiek z powtórzeń serii.

---

### 7. Całkowita Praca Mechaniczna Serii (Total Work per Set)

$$W^{(s)} = \sum_{n=1}^{N_{reps}} \int_{t_{start}^{(s,n)}}^{t_{end}^{(s,n)}} F(t) \cdot v(t) \, dt$$

**Gdzie prędkość:**

$$v(t) = \frac{d}{dt}\left(\frac{F(t)}{k_{elastic}}\right) = \frac{1}{k_{elastic}} \cdot \frac{dF(t)}{dt}$$

**Wersja uproszczona:**

$$W^{(s)} \approx \frac{1}{k_{elastic}} \sum_{n=1}^{N_{reps}} \sum_{i=1}^{M-1} F_i \cdot (F_{i+1} - F_i)$$

**Opis:** Całkowita praca wykonana podczas całej serii.

**Składowe:**
- $W^{(s)}$ - praca mechaniczna serii $s$ [J]
- $v(t)$ - prędkość rozciągania taśmy
- $k_{elastic}$ - stała sprężystości taśmy

---

### 8. Średnia Praca na Powtórzenie (Average Work per Rep)

$$\bar{W}_{rep}^{(s)} = \frac{W^{(s)}}{N_{reps}}$$

**Opis:** Średnia praca wykonana w jednym powtórzeniu.

---

### 9. Impuls Siły (Force Impulse)

$$I^{(s,n)} = \int_{t_{start}^{(s,n)}}^{t_{end}^{(s,n)}} F(t) \, dt$$

**Wersja dyskretna:**

$$I^{(s,n)} = \sum_{i=1}^{M} F_i \cdot \Delta t$$

**Opis:** Całka siły po czasie, miara całkowitego "nakładu" siłowego.

**Składowe:**
- $I^{(s,n)}$ - impuls siły [N·s]
- $\Delta t$ - interwał próbkowania

---

### 10. Średni Impuls z Serii (Average Impulse per Set)

$$\bar{I}_{set}^{(s)} = \frac{1}{N_{reps}} \sum_{n=1}^{N_{reps}} I^{(s,n)}$$

**Opis:** Średni impuls siły dla wszystkich powtórzeń w serii.

---

## Kategoria 2: Parametry Czasowe (11-20)

### 11. Czas Trwania Powtórzenia (Repetition Duration)

$$T^{(s,n)} = t_{end}^{(s,n)} - t_{start}^{(s,n)}$$

**Opis:** Całkowity czas wykonania pojedynczego powtórzenia.

---

### 12. Czas Fazy Koncentrycznej (Concentric Phase Time)

$$T_{conc}^{(s,n)} = t_{peak}^{(s,n)} - t_{start}^{(s,n)}$$

**Opis:** Czas od rozpoczęcia ruchu do osiągnięcia szczytowej siły (faza napinania).

**Składowe:**
- $t_{peak}^{(s,n)}$ - czas osiągnięcia $F_{peak}$

---

### 13. Czas Fazy Ekscentrycznej (Eccentric Phase Time)

$$T_{ecc}^{(s,n)} = t_{end}^{(s,n)} - t_{peak}^{(s,n)}$$

**Opis:** Czas od szczytowej siły do zakończenia powtórzenia (faza rozluźniania).

---

### 14. Stosunek Faz (Concentric-Eccentric Ratio)

$$R_{CE}^{(s,n)} = \frac{T_{conc}^{(s,n)}}{T_{ecc}^{(s,n)}}$$

**Opis:** Wskaźnik tempa wykonania ruchu (fazы napinania do rozluźniania).

---

### 15. Czas Przerwy Między Powtórzeniami (Inter-Repetition Rest)

$$T_{rest\_rep}^{(s,n)} = t_{start}^{(s,n+1)} - t_{end}^{(s,n)}$$

**Opis:** Czas odpoczynku między kolejnymi powtórzeniami w tej samej serii.

---

### 16. Czas Trwania Serii (Set Duration)

$$T_{set}^{(s)} = t_{end}^{(s,N_{reps})} - t_{start}^{(s,1)}$$

**Opis:** Całkowity czas od pierwszego do ostatniego powtórzenia w serii.

---

### 17. Czas Przerwy Między Seriami (Inter-Set Rest)

$$T_{rest\_set}^{(s)} = t_{start}^{(s+1,1)} - t_{end}^{(s,N_{reps})}$$

**Opis:** Czas odpoczynku między zakończeniem jednej serii a rozpoczęciem następnej.

---

### 18. Średni Czas Powtórzenia w Serii (Average Rep Time per Set)

$$\bar{T}_{rep}^{(s)} = \frac{1}{N_{reps}} \sum_{n=1}^{N_{reps}} T^{(s,n)}$$

**Opis:** Średni czas trwania powtórzenia w obrębie serii.

---

### 19. Tempo Wykonania Ruchu (Movement Velocity Estimate)

$$v_{avg}^{(s,n)} = \frac{R_F^{(s,n)}}{k_{elastic} \cdot T^{(s,n)}}$$

**Opis:** Szacunkowa średnia prędkość rozciągania taśmy.

**Składowe:**
- $v_{avg}^{(s,n)}$ - średnia prędkość [m/s]
- $k_{elastic}$ - stała sprężystości taśmy [N/m]

---

### 20. Częstotliwość Powtórzeń (Repetition Frequency)

$$f_{rep}^{(s)} = \frac{N_{reps}}{T_{set}^{(s)}}$$

**Opis:** Liczba powtórzeń na jednostkę czasu w serii.

**Składowe:**
- $f_{rep}^{(s)}$ - częstotliwość [Hz lub rep/s]

---

## Kategoria 3: Parametry Mocy i Energii (21-30)

### 21. Moc Szczytowa (Peak Power)

$$P_{peak}^{(s,n)} = F_{peak}^{(s,n)} \cdot v_{peak}^{(s,n)}$$

**Gdzie:**

$$v_{peak}^{(s,n)} = \max_{t} \left|\frac{1}{k_{elastic}} \cdot \frac{dF(t)}{dt}\right|$$

**Opis:** Maksymalna moc wygenerowana podczas powtórzenia.

**Składowe:**
- $P_{peak}^{(s,n)}$ - moc szczytowa [W]
- $v_{peak}^{(s,n)}$ - maksymalna prędkość rozciągania

---

### 22. Średnia Moc z Powtórzenia (Average Power per Rep)

$$\bar{P}^{(s,n)} = \frac{W^{(s,n)}}{T^{(s,n)}}$$

**Opis:** Średnia moc wygenerowana w trakcie pojedynczego powtórzenia.

**Składowe:**
- $W^{(s,n)}$ - praca wykonana w powtórzeniu [J]

---

### 23. Średnia Moc z Serii (Average Power per Set)

$$\bar{P}_{set}^{(s)} = \frac{W^{(s)}}{T_{set}^{(s)}}$$

**Opis:** Średnia moc dla całej serii.

---

### 24. Całkowita Energia Zużyta (Total Energy Expenditure)

$$E_{total} = \sum_{s=1}^{N_{sets}} W^{(s)}$$

**Opis:** Suma pracy mechanicznej ze wszystkich serii.

---

### 25. Moc Znormalizowana do Masy Ciała (Normalized Power)

$$P_{norm}^{(s,n)} = \frac{\bar{P}^{(s,n)}}{m_{patient}}$$

**Opis:** Moc względem masy ciała pacjenta.

**Składowe:**
- $P_{norm}^{(s,n)}$ - moc znormalizowana [W/kg]
- $m_{patient}$ - masa ciała pacjenta [kg]

---

### 26. Wskaźnik Szybkości Rozwoju Siły (Rate of Force Development - RFD)

$$RFD^{(s,n)} = \frac{F_{peak}^{(s,n)} - F_{start}^{(s,n)}}{T_{conc}^{(s,n)}}$$

**Opis:** Szybkość narastania siły w fazie koncentrycznej.

**Składowe:**
- $RFD^{(s,n)}$ - wskaźnik rozwoju siły [N/s]
- $F_{start}^{(s,n)}$ - siła początkowa powtórzenia

---

### 27. Średni RFD z Serii (Average RFD per Set)

$$\overline{RFD}_{set}^{(s)} = \frac{1}{N_{reps}} \sum_{n=1}^{N_{reps}} RFD^{(s,n)}$$

**Opis:** Średnia szybkość rozwoju siły w serii.

---

### 28. Maksymalny RFD (Maximum RFD)

$$RFD_{max}^{(s)} = \max_{n \in [1, N_{reps}]} RFD^{(s,n)}$$

**Opis:** Najwyższy wskaźnik rozwoju siły w serii.

---

### 29. Energia Sprężysta Zmagazynowana (Elastic Potential Energy)

$$E_{elastic}^{(s,n)} = \frac{1}{2} k_{elastic} \cdot (\Delta L^{(s,n)})^2$$

**Gdzie wydłużenie:**

$$\Delta L^{(s,n)} = \frac{F_{peak}^{(s,n)}}{k_{elastic}}$$

**Uproszczony wzór:**

$$E_{elastic}^{(s,n)} = \frac{(F_{peak}^{(s,n)})^2}{2 \cdot k_{elastic}}$$

**Opis:** Energia potencjalna zmagazynowana w rozciągniętej taśmie.

**Składowe:**
- $E_{elastic}^{(s,n)}$ - energia sprężysta [J]
- $\Delta L^{(s,n)}$ - maksymalne wydłużenie taśmy [m]

---

### 30. Sprawność Mechaniczna (Mechanical Efficiency Estimate)

$$\eta^{(s)} = \frac{W^{(s)}}{E_{metabolic\_est}}$$

**Gdzie szacunek energii metabolicznej:**

$$E_{metabolic\_est} = \alpha \cdot E_{total} + \beta \cdot T_{total}$$

**Opis:** Szacunkowa sprawność wykonania ćwiczenia.

**Składowe:**
- $\eta^{(s)}$ - sprawność mechaniczna [%]
- $\alpha, \beta$ - współczynniki empiryczne
- $T_{total}$ - całkowity czas treningu

---

## Kategoria 4: Parametry Zmienności i Stabilności (31-40)

### 31. Odchylenie Standardowe Siły w Serii (Force Standard Deviation)

$$\sigma_F^{(s)} = \sqrt{\frac{1}{N_{reps}-1} \sum_{n=1}^{N_{reps}} (\bar{F}^{(s,n)} - \bar{F}_{set}^{(s)})^2}$$

**Opis:** Miara zmienności siły między powtórzeniami w serii.

---

### 32. Współczynnik Zmienności Siły (Coefficient of Variation - CV)

$$CV_F^{(s)} = \frac{\sigma_F^{(s)}}{\bar{F}_{set}^{(s)}} \times 100\%$$

**Opis:** Znormalizowana miara zmienności siły (w procentach).

---

### 33. Indeks Zmęczenia Serii (Fatigue Index within Set)

$$FI_{set}^{(s)} = \frac{F_{peak}^{(s,1)} - F_{peak}^{(s,N_{reps})}}{F_{peak}^{(s,1)}} \times 100\%$$

**Opis:** Procentowy spadek siły od pierwszego do ostatniego powtórzenia w serii.

---

### 34. Indeks Zmęczenia Międzyseryjnego (Inter-Set Fatigue Index)

$$FI_{inter} = \frac{F_{peak\_set}^{(1)} - F_{peak\_set}^{(3)}}{F_{peak\_set}^{(1)}} \times 100\%$$

**Opis:** Spadek maksymalnej siły między pierwszą a trzecią serią.

---

### 35. Współczynnik Jednorodności Ruchu (Movement Consistency Index)

$$MCI^{(s)} = 1 - \frac{\sigma_T^{(s)}}{\bar{T}_{rep}^{(s)}}$$

**Gdzie:**

$$\sigma_T^{(s)} = \sqrt{\frac{1}{N_{reps}-1} \sum_{n=1}^{N_{reps}} (T^{(s,n)} - \bar{T}_{rep}^{(s)})^2}$$

**Opis:** Miara regularności tempa wykonywania powtórzeń (1 = idealna regularność).

---

### 36. Indeks Symetrii Faz (Phase Symmetry Index)

$$PSI^{(s,n)} = \left|1 - \frac{T_{conc}^{(s,n)}}{T_{ecc}^{(s,n)}}\right|$$

**Opis:** Miara asymetrii między fazą koncentryczną a ekscentryczną (0 = idealna symetria).

---

### 37. Wariancja Czasu Trwania Powtórzeń (Rep Duration Variance)

$$Var_T^{(s)} = \frac{1}{N_{reps}-1} \sum_{n=1}^{N_{reps}} (T^{(s,n)} - \bar{T}_{rep}^{(s)})^2$$

**Opis:** Wariancja czasów trwania poszczególnych powtórzeń.

---

### 38. Indeks Płynności Ruchu (Smoothness Index)

$$SI^{(s,n)} = \frac{1}{T^{(s,n)}} \int_{t_{start}}^{t_{end}} \left|\frac{d^2F(t)}{dt^2}\right| dt$$

**Wersja dyskretna:**

$$SI^{(s,n)} = \frac{1}{M-2} \sum_{i=2}^{M-1} \left|\frac{F_{i+1} - 2F_i + F_{i-1}}{(\Delta t)^2}\right|$$

**Opis:** Miara płynności ruchu (niższa wartość = bardziej płynny ruch).

**Składowe:**
- $SI^{(s,n)}$ - indeks płynności [N/s²]
- $\frac{d^2F(t)}{dt^2}$ - druga pochodna siły (przyspieszenie zmiany siły)

---

### 39. Entropia Sygnału Siły (Force Signal Entropy)

$$H^{(s)} = -\sum_{k=1}^{K} p_k \cdot \log_2(p_k)$$

**Gdzie:**
- $p_k$ - prawdopodobieństwo wystąpienia siły w $k$-tym przedziale histogramu
- $K$ - liczba przedziałów histogramu

**Opis:** Miara złożoności/losowości sygnału siły.

---

### 40. Indeks Regularności Oddychania (Breathing Pattern Regularity - pośredni)

$$BPR^{(s)} = \frac{1}{1 + \frac{\sigma_{T_{rest\_rep}}^{(s)}}{\bar{T}_{rest\_rep}^{(s)}}}$$

**Opis:** Szacunkowa regularność rytmu ćwiczenia (może korelować z oddechem).

**Składowe:**
- $\sigma_{T_{rest\_rep}}^{(s)}$ - odchylenie standardowe przerw między powtórzeniami
- $\bar{T}_{rest\_rep}^{(s)}$ - średni czas przerwy między powtórzeniami

---

## Kategoria 5: Parametry Postępu i Wydajności (41-50)

### 41. Średnia Siła Całkowita z Treningu (Grand Mean Force)

$$\bar{F}_{grand} = \frac{1}{N_{sets}} \sum_{s=1}^{N_{sets}} \bar{F}_{set}^{(s)}$$

**Opis:** Średnia siła ze wszystkich serii treningu.

---

### 42. Całkowity Impuls Treningu (Total Training Impulse)

$$I_{total} = \sum_{s=1}^{N_{sets}} \sum_{n=1}^{N_{reps}} I^{(s,n)}$$

**Opis:** Suma impulsów siły ze wszystkich powtórzeń.

---

### 43. Objętość Treningowa (Training Volume - Force Based)

$$V_{training} = \sum_{s=1}^{N_{sets}} \sum_{n=1}^{N_{reps}} \bar{F}^{(s,n)} \cdot R_F^{(s,n)}$$

**Opis:** Miara objętości treningowej uwzględniająca siłę i zakres ruchu.

---

### 44. Intensywność Treningu (Training Intensity Index)

$$INT = \frac{\bar{F}_{grand}}{F_{max\_reference}} \times 100\%$$

**Opis:** Intensywność względem siły referencyjnej (np. maksymalnej historycznej).

**Składowe:**
- $F_{max\_reference}$ - siła referencyjna (np. 1RM lub rekord osobisty)

---

### 45. Gęstość Treningu (Training Density)

$$D_{training} = \frac{N_{sets} \cdot N_{reps}}{T_{total}}$$

**Gdzie:**

$$T_{total} = T_{set}^{(1)} + T_{set}^{(2)} + T_{set}^{(3)} + T_{rest\_set}^{(1)} + T_{rest\_set}^{(2)}$$

**Opis:** Liczba powtórzeń na jednostkę czasu całego treningu.

---

### 46. Indeks Efektywności Serii (Set Efficiency Index)

$$SEI^{(s)} = \frac{W^{(s)}}{T_{set}^{(s)} \cdot \bar{F}_{set}^{(s)}}$$

**Opis:** Stosunek wykonanej pracy do iloczynu czasu i średniej siły.

---

### 47. Współczynnik Utrzymania Siły (Force Maintenance Coefficient)

$$FMC = \frac{\bar{F}_{set}^{(3)}}{\bar{F}_{set}^{(1)}} \times 100\%$$

**Opis:** Zdolność do utrzymania siły w trzeciej serii względem pierwszej.

---

### 48. Indeks Poprawy Międzyseryjnej (Inter-Set Improvement Index)

$$ISI = \frac{F_{peak\_set}^{(2)} - F_{peak\_set}^{(1)}}{F_{peak\_set}^{(1)}} \times 100\%$$

**Opis:** Zmiana maksymalnej siły między pierwszą a drugą serią (może być ujemna).

---

### 49. Średni Czas Pod Napięciem (Average Time Under Tension)

$$TUT_{avg} = \frac{1}{N_{sets} \cdot N_{reps}} \sum_{s=1}^{N_{sets}} \sum_{n=1}^{N_{reps}} T_{tension}^{(s,n)}$$

**Gdzie:**

$$T_{tension}^{(s,n)} = \text{czas, gdy } F(t) > F_{threshold}$$

**Opis:** Średni czas, przez który mięsień jest pod znacznym napięciem.

**Składowe:**
- $F_{threshold}$ - progowa wartość siły (np. 50% $F_{peak}$)

---

### 50. Całkowity Czas Pod Napięciem (Total Time Under Tension)

$$TUT_{total} = \sum_{s=1}^{N_{sets}} \sum_{n=1}^{N_{reps}} T_{tension}^{(s,n)}$$

**Opis:** Łączny czas pracy mięśnia pod obciążeniem.

---

## Kategoria 6: Parametry Zaawansowane i Wskaźniki Złożone (51-60)

### 51. Wskaźnik Jakości Ruchu (Movement Quality Score)

$$MQS^{(s,n)} = \frac{F_{peak}^{(s,n)}}{SI^{(s,n)} \cdot PSI^{(s,n)} + \epsilon}$$

**Opis:** Złożony wskaźnik jakości wykonania powtórzenia.

**Składowe:**
- $MQS^{(s,n)}$ - wynik jakości ruchu
- $SI^{(s,n)}$ - indeks płynności
- $PSI^{(s,n)}$ - indeks symetrii faz
- $\epsilon$ - mała stała zapobiegająca dzieleniu przez zero (np. 0.001)

---

### 52. Indeks Obciążenia Neuromięśniowego (Neuromuscular Load Index)

$$NML^{(s)} = RFD_{max}^{(s)} \cdot \sigma_F^{(s)} \cdot \frac{T_{set}^{(s)}}{N_{reps}}$$

**Opis:** Szacunkowe obciążenie układu nerwowo-mięśniowego.

---

### 53. Wskaźnik Stabilności Postawy (Postural Stability Estimate)

$$PSE^{(s,n)} = \frac{1}{1 + \frac{1}{T^{(s,n)}} \int_{t_{start}}^{t_{end}} \left(\frac{dF(t)}{dt} - \overline{\frac{dF}{dt}}\right)^2 dt}$$

**Opis:** Miara stabilności generowania siły (wyższa = bardziej stabilnie).

---

### 54. Indeks Gotowości Treningowej (Training Readiness Index)

$$TRI = \frac{F_{peak\_set}^{(1)}}{F_{peak\_historical}} \times \left(1 - \frac{CV_F^{(1)}}{100}\right)$$

**Opis:** Wskaźnik gotowości do treningu na podstawie aktualnej formy.

**Składowe:**
- $F_{peak\_historical}$ - historyczne maksimum pacjenta

---

### 55. Wskaźnik Ryzyka Kontuzji (Injury Risk Indicator)

$$IRI^{(s,n)} = \frac{RFD^{(s,n)}}{F_{peak}^{(s,n)}} \cdot \frac{T_{conc}^{(s,n)}}{T_{ecc}^{(s,n)}}$$

**Opis:** Wskaźnik sugerujący ryzyko przeciążenia (wysoki RFD przy krótkim czasie).

---

### 56. Indeks Adaptacji Treningowej (Training Adaptation Index)

$$TAI = \frac{\bar{P}_{set}^{(3)} - \bar{P}_{set}^{(1)}}{\bar{P}_{set}^{(1)}} \times \frac{1}{FI_{inter}}$$

**Opis:** Miara adaptacji do bodźca treningowego.

---

### 57. Wskaźnik Ekonomii Ruchu (Movement Economy Index)

$$MEI^{(s)} = \frac{W^{(s)}}{I_{total}^{(s)} \cdot \bar{F}_{set}^{(s)}}$$

**Opis:** Efektywność wykorzystania impulsu siły do wykonania pracy.

---

### 58. Indeks Zrównoważenia Obciążenia (Load Balance Index)

$$LBI = 1 - \frac{|W^{(1)} - W^{(2)}| + |W^{(2)} - W^{(3)}| + |W^{(1)} - W^{(3)}|}{3 \cdot \bar{W}_{all}}$$

**Gdzie:**

$$\bar{W}_{all} = \frac{W^{(1)} + W^{(2)} + W^{(3)}}{3}$$

**Opis:** Miara równomierności rozłożenia pracy między serie (1 = idealne zrównoważenie).

---

### 59. Wskaźnik Rezerwy Funkcjonalnej (Functional Reserve Index)

$$FRI = \frac{F_{max\_sensor} - F_{peak\_set}^{(1)}}{F_{max\_sensor}} \times 100\%$$

**Opis:** Procent rezerwy względem maksymalnych możliwości sensora/zakresu.

---

### 60. Kompleksowy Wynik Treningowy (Composite Training Score)

$$CTS = w_1 \cdot \frac{\bar{F}_{grand}}{F_{ref}} + w_2 \cdot (1 - CV_F) + w_3 \cdot MCI + w_4 \cdot (1 - FI_{inter})$$

**Gdzie:**
- $w_1, w_2, w_3, w_4$ - wagi sumujące się do 1 (np. 0.25 każda)
- Wszystkie składniki znormalizowane do przedziału [0, 1]

**Opis:** Zagregowany wskaźnik jakości całego treningu.

---

## Kategoria 7: Parametry Dodatkowe (61-65)

### 61. Gradient Zmęczenia (Fatigue Gradient)

$$FG = \frac{FI_{set}^{(1)} + FI_{set}^{(2)} + FI_{set}^{(3)}}{3}$$

**Opis:** Średni wskaźnik zmęczenia we wszystkich seriach.

---

### 62. Indeks Eksplozywnej Siły (Explosive Strength Index)

$$ESI^{(s,n)} = \frac{F_{peak}^{(s,n)}}{T_{conc}^{(s,n)}}$$

**Opis:** Miła eksplozywnej komponenty siły.

**Składowe:**
- $ESI^{(s,n)}$ - indeks siły eksplozywnej [N/s]

---

### 63. Współczynnik Powrotu do Bazy (Baseline Recovery Coefficient)

$$BRC^{(s,n)} = \frac{F_{min}^{(s,n)}}{F_{baseline}}$$

**Opis:** Stopień powrotu do siły bazowej między powtórzeniami.

**Składowe:**
- $F_{baseline}$ - siła spoczynkowa przed ćwiczeniem

---

### 64. Indeks Kontrolowanego Opuszczania (Controlled Eccentric Index)

$$CEI^{(s,n)} = \frac{1}{T_{ecc}^{(s,n)}} \int_{t_{peak}}^{t_{end}} \frac{F(t)}{F_{peak}} dt$$

**Opis:** Miara kontroli fazy ekscentrycznej (rozluźniania).

---

### 65. Wskaźnik Postępu Longitudinalnego (Longitudinal Progress Tracker)

$$LPT_{sesja\_k} = \frac{CTS_{k} - CTS_{k-1}}{CTS_{k-1}} \times 100\%$$

**Opis:** Procentowa zmiana kompleksowego wyniku między sesjami treningowymi.

**Składowe:**
- $CTS_{k}$ - wynik bieżącej sesji
- $CTS_{k-1}$ - wynik poprzedniej sesji

---

## Podsumowanie

Powyższe wzory pozwalają na obliczenie **65 parametrów** podzielonych na następujące kategorie:

| Kategoria | Liczba parametrów | Numery |
|-----------|------------------|--------|
| Parametry Siłowe | 10 | 1-10 |
| Parametry Czasowe | 10 | 11-20 |
| Parametry Mocy i Energii | 10 | 21-30 |
| Parametry Zmienności i Stabilności | 10 | 31-40 |
| Parametry Postępu i Wydajności | 10 | 41-50 |
| Parametry Zaawansowane | 10 | 51-60 |
| Parametry Dodatkowe | 5 | 61-65 |
| **RAZEM** | **65** | |

---

## Implementacja w Raspberry Pi

Wszystkie powyższe wzory mogą być zaimplementowane w języku Python lub C++ na platformie Raspberry Pi. Dane z tensometru (HX711) powinny być próbkowane z częstotliwością minimum 50-100 Hz dla dokładnych obliczeń pochodnych i całek.

### Zalecana częstotliwość próbkowania:
- **Minimalna:** 50 Hz
- **Zalecana:** 100-200 Hz
- **Optymalna dla RFD:** 500+ Hz

### Przykładowy pseudokod:

```python
# Przykład obliczania F_peak i mean_force
def calculate_metrics(force_samples, time_samples):
    F_peak = max(force_samples)
    F_mean = sum(force_samples) / len(force_samples)
    F_min = min(force_samples)
    Force_range = F_peak - F_min
    
    # RFD (przybliżenie różnicowe)
    dF_dt = [(force_samples[i+1] - force_samples[i]) / 
             (time_samples[i+1] - time_samples[i]) 
             for i in range(len(force_samples)-1)]
    RFD = max(dF_dt)
    
    return {
        'F_peak': F_peak,
        'F_mean': F_mean,
        'F_min': F_min,
        'Force_range': Force_range,
        'RFD': RFD
    }
```

---

## Bibliografia i Źródła

1. Zatsiorsky, V.M., & Kraemer, W.J. (2006). *Science and Practice of Strength Training*
2. Enoka, R.M. (2017). *Neuromechanics of Human Movement*
3. McGill, S.M. (2010). *Biomechanics of the Spine and Core*
4. Bompa, T.O., & Haff, G.G. (2009). *Periodization: Theory and Methodology of Training*

---

**Autor:** Biofeedback Research Team  
**Data:** 2024  
**Licencja:** Open Source (MIT)
