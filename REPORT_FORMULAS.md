# Raport Matematyczny: Analiza Danych z Tensometru i Taśmy Lateksowej

**Autor:** Asystent AI  
**Data:** 2023-10-27  
**Kontekst:** Rehabilitacja i trening siłowy (3 serie × 8 powtórzeń)  
**Sprzęt:** Tensometr (HX711), Raspberry Pi, Taśma lateksowa  

---

## 📋 Wstęp

Niniejszy dokument stanowi kompletne opracowanie matematyczne parametrów biomechanicznych wyliczanych na podstawie surowych danych z tensometru. Dane obejmują rejestrację siły w funkcji czasu podczas ćwiczeń z taśmą oporową.

Wzory zostały zapisane w notacji **LaTeX**, co zapewnia ich precyzyjne odwzorowanie graficzne w środowiskach obsługujących rendering matematyczny (np. GitHub, Overleaf, edytory Markdown).

---

## 📊 Legenda Zmiennych

Zanim przejdziemy do wzorów, zdefiniujmy zmienne używane w całym raporcie:

| Symbol | Opis | Jednostka |
| :--- | :--- | :--- |
| $F(t)$ | Siła chwilowa w czasie $t$ | $[N]$ (Niuton) |
| $t$ | Czas | $[s]$ (sekunda) |
| $t_{start}, t_{end}$ | Czas rozpoczęcia i zakończenia powtórzenia/serii | $[s]$ |
| $n$ | Numer powtórzenia ($1 \dots 8$) | - |
| $s$ | Numer serii ($1 \dots 3$) | $N_{reps} = 8, N_{sets} = 3$ | - |
| $F_{max}$ | Maksymalna siła zarejestrowana w danym przedziale | $[N]$ |
| $F_{min}$ | Minimalna siła (siła spoczynkowa/napięcia wstępnego) | $[N]$ |
| $\Delta t$ | Przedział czasowy | $[s]$ |
| $k$ | Stała sprężystości taśmy (sztywność) | $[N/m]$ |
| $x(t)$ | Wydłużenie taśmy w czasie $t$ | $[m]$ |
| $\bar{F}$ | Średnia arytmetyczna siły | $[N]$ |
| $\sigma_F$ | Odchylenie standardowe siły | $[N]$ |

---

## 1. Parametry Siłowe (Force Metrics)

### 1.1. Szczytowa Siła (Peak Force)
Maksymalna wartość siły osiągnięta w trakcie pojedynczego powtórzenia lub serii.

$$ PF = \max_{t \in [t_{start}, t_{end}]} \{ F(t) \} $$

*   **Zastosowanie:** Ocena maksymalnego potencjału siłowego mięśnia.

### 1.2. Średnia Siła (Mean Force)
Średnia wartość siły w trakcie fazy koncentrycznej (naciągania).

$$ \bar{F} = \frac{1}{t_{end} - t_{start}} \int_{t_{start}}^{t_{end}} F(t) \, dt $$

*   **Zastosowanie:** Określenie ogólnego obciążenia mięśnia.

### 1.3. Impuls Siły (Force Impulse)
Całkowity "nakład" siły w czasie. Kluczowy dla hipertrofii.

$$ J = \int_{t_{start}}^{t_{end}} F(t) \, dt \approx \sum_{i=1}^{k} F_i \cdot \Delta t_i $$

*   **Zmienne:** $F_i$ – próbka siły, $\Delta t_i$ – czas między próbkami.

### 1.4. Zakres Siły (Force Range)
Różnica między maksymalnym napięciem a napięciem początkowym.

$$ \Delta F = F_{max} - F_{min} $$

### 1.5. Współczynnik Obciążenia Względnego (Relative Load Index)
Stosunek siły maksymalnej do siły przy pełnym rozciągnięciu taśmy (teoretycznej).

$$ RLI = \frac{PF}{k \cdot x_{max}} \times 100\% $$

---

## 2. Parametry Czasowe (Temporal Metrics)

### 2.1. Czas Pod Napięciem (Time Under Tension - TUT)
Całkowity czas, w którym mięsień pracuje przeciwko oporowi w danej serii.

$$ TUT_{set} = \sum_{n=1}^{8} (t_{end\_rep_n} - t_{start\_rep_n}) $$

### 2.2. Tempo Fazy Koncentrycznej (Concentric Duration)
Czas trwania naciągania taśmy (faza pozytywna).

$$ T_{conc} = t_{peak\_force} - t_{start\_rep} $$

### 2.3. Tempo Fazy Ekscentrycznej (Eccentric Duration)
Czas trwania powrotu taśmy do pozycji początkowej.

$$ T_{ecc} = t_{start\_next\_rep} - t_{peak\_force} $$

### 2.4. Stosunek Faz (Con/Ecc Ratio)
Wskaźnik kontroli ruchu.

$$ R_{tempo} = \frac{T_{conc}}{T_{ecc}} $$

*   **Wartość optymalna:** Często dąży się do $1:2$ lub $1:3$ w rehabilitacji.

### 2.5. Czas Przerwy Międzypowtórzeniowej (Inter-Repetitive Rest)
Czas relaksu między kolejnymi powtórzeniami w serii.

$$ T_{rest\_rep} = t_{start\_rep_{n+1}} - t_{end\_rep_n} $$

---

## 3. Parametry Mocy i Energii (Power & Energy)

### 3.1. Moc Chwilowa (Instantaneous Power)
Szybkość wykonywania pracy. Wymaga estymacji prędkości wydłużania taśmy.

$$ P(t) = F(t) \cdot v(t) = F(t) \cdot \frac{dx(t)}{dt} $$

*   **Uwaga:** $x(t) \approx \frac{F(t)}{k}$ (dla taśmy idealnie sprężystej).

### 3.2. Szczytowa Moc (Peak Power)
Maksymalna wartość mocy w trakcie powtórzenia.

$$ PP = \max_{t} \{ P(t) \} $$

### 3.3. Średnia Moc (Mean Power)
Średnia moc wygenerowana w fazie koncentrycznej.

$$ \bar{P} = \frac{1}{T_{conc}} \int_{t_{start}}^{t_{peak}} P(t) \, dt $$

### 3.4. Praca Mechaniczna (Mechanical Work)
Energia zużyta na rozciągnięcie taśmy.

$$ W = \int_{x_{start}}^{x_{end}} F(x) \, dx = \frac{1}{2} k (x_{end}^2 - x_{start}^2) $$

### 3.5. Gęstość Mocy (Power Density per Rep)
Stosunek wykonanej pracy do czasu trwania powtórzenia.

$$ PD = \frac{W_{rep}}{T_{rep}} $$

---

## 4. Dynamika i Płynność Ruchu (Dynamics & Smoothness)

### 4.1. Szybkość Rozwoju Siły (Rate of Force Development - RFD)
Kluczowy parametr eksplozywności.

$$ RFD = \frac{\Delta F}{\Delta t} = \frac{F(t_2) - F(t_1)}{t_2 - t_1} $$

*   Gdzie $\Delta t$ to często pierwszy 200ms od rozpoczęcia ruchu.

### 4.2. Średnie RFD (Mean RFD)
Średnia szybkość narastania siły do momentu osiągnięcia szczytu.

$$ RFD_{mean} = \frac{F_{max} - F_{onset}}{t_{peak} - t_{onset}} $$

### 4.3. Indeks Płynności (Smoothness Index / Jerk)
Pochodna przyspieszenia (trzecia pochodna położenia), wyrażona przez zmianę siły. Mniejsza wartość oznacza płynniejszy ruch.

$$ Jerk(t) = \frac{d}{dt} \left( \frac{dF(t)}{dt} \right) \cdot \frac{1}{k} $$

$$ SI = \frac{1}{T} \int_{0}^{T} \left| \frac{d^2 F(t)}{dt^2} \right|^2 dt $$

### 4.4. Liczba Korekt Ruchu (Movement Corrections Count)
Liczba lokalnych minimów/maximów pochodnej siły wskazująca na drżenie mięśni.

$$ N_{corr} = \text{count} \left( \frac{dF(t)}{dt} = 0 \right) \quad \text{dla } t \in (t_{start}, t_{peak}) $$

### 4.5. Symetria Narastania Siły (Force Rise Symmetry)
Porównanie czasu dojścia do 50% i 100% siły maksymalnej.

$$ Sym = \frac{t_{50\% F_{max}} - t_{start}}{t_{100\% F_{max}} - t_{start}} $$

---

## 5. Zmienność i Stabilność (Variability & Stability)

### 5.1. Współczynnik Zmienności Siły (Coefficient of Variation - CV)
Miara stabilności generowania siły w obrębie serii.

$$ CV_F = \frac{\sigma_F}{\bar{F}} \times 100\% $$

*   **Interpretacja:** Niższy CV oznacza lepszą kontrolę nerwowo-mięśniową.

### 5.2. Odchylenie Standardowe Powtórzeń (Rep-to-Rep SD)
Zmienność szczytowej siły między kolejnymi powtórzeniami.

$$ SD_{reps} = \sqrt{ \frac{1}{N-1} \sum_{n=1}^{N} (PF_n - \overline{PF})^2 } $$

### 5.3. Entropia Sygnału (Sample Entropy)
Miara nieregularności sygnału siły. Wysoka entropia może sugerować zmęczenie lub brak koordynacji.

$$ SampEn = -\ln \left( \frac{A}{B} \right) $$

*   Gdzie $A$ i $B$ to liczby par wzorców o podobnej strukturze w sygnale.

### 5.4. Spektralna Gęstość Mocy (Power Spectral Density - Median Frequency)
Analiza częstotliwościowa sygnału. Przesunięcie mediany w dół wskazuje na zmęczenie mięśni.

$$ MDF = \text{median}(f) \quad \text{gdzie} \quad \int_{0}^{MDF} PSD(f) df = 0.5 \int_{0}^{f_{max}} PSD(f) df $$

### 5.5. Stabilność Izometryczna (jeśli występuje faza hold)
Odchylenie siły w fazie przytrzymania.

$$ Stability = \frac{1}{T_{hold}} \int_{t_{hold\_start}}^{t_{hold\_end}} |F(t) - \bar{F}_{hold}| \, dt $$

---

## 6. Parametry Postępu i Wydajności (Performance & Progression)

### 6.1. Indeks Zmęczenia (Fatigue Index - FI)
Spadek mocy lub siły między pierwszą a ostatnią serią/powtórzeniem.

$$ FI = \left( 1 - \frac{PF_{last\_rep}}{PF_{first\_rep}} \right) \times 100\% $$

### 6.2. Dekrement Mocy (Power Decrement)
Procentowy spadek średniej mocy między serią 1 a serią 3.

$$ \Delta P_{sets} = \frac{\bar{P}_{set1} - \bar{P}_{set3}}{\bar{P}_{set1}} \times 100\% $$

### 6.3. Efektywność Mechaniczna (Mechanical Efficiency)
Stosunek pracy użytecznej do całkowitego impulsu siły.

$$ ME = \frac{W_{total}}{J_{total}} $$

### 6.4. Wskaźnik Objętości Treningowej (Volume Load)
Całkowita praca wykonana w sesji (suma prac wszystkich powtórzeń).

$$ VL = \sum_{s=1}^{3} \sum_{n=1}^{8} W_{s,n} $$

### 6.5. Quality of Movement Score (QMS)
Złożony wskaźnik łączący płynność, symetrię i stabilność (wartość znormalizowana 0-100).

$$ QMS = 100 \cdot \left( w_1 \cdot \frac{1}{SI} + w_2 \cdot (1 - CV_F) + w_3 \cdot Sym \right) $$

*   $w_1, w_2, w_3$ – wagi kalibracyjne.

---

## 7. Parametry Zaawansowane i Predykcyjne (Advanced Metrics)

### 7.1. Szacowany Moment Siły (Estimated Torque)
Jeśli znana jest geometria ćwiczenia (ramię siły $r$).

$$ \tau(t) = F(t) \cdot r \cdot \sin(\theta(t)) $$

### 7.2. Sztywność Mięśniowo-Ścięgnista (Muscle-Tendon Stiffness Estimation)
Estymacja na podstawie stosunku zmiany siły do zmiany długości w fazie ekscentrycznej.

$$ S_{mt} = \frac{\Delta F_{ecc}}{\Delta x_{ecc}} $$

### 7.3. Czas Reakcji Na Opór (Reaction Time to Load)
Czas od momentu rozpoczęcia ruchu (detekcja progu siły) do momentu znaczącego wzrostu siły.

$$ RT = t_{threshold} - t_{motion\_start} $$

### 7.4. Wskaźnik Ryzyka Kontuzji (Injury Risk Indicator)
Opiera się na nagłych skokach siły (high jerk) przy dużym wydłużeniu.

$$ IRI = \max \left( \frac{dF}{dt} \cdot F(t) \right) \quad \text{dla } x(t) > 0.8 \cdot x_{max} $$

### 7.5. Prognozowane 1RM (One Rep Max Prediction)
Ekstrapolacja na podstawie krzywej siła-prędkość z 8 powtórzeń.

$$ 1RM_{pred} = \frac{W_{load}}{1 - \frac{Reps}{Reps_{max}}} $$
*(Gdzie $W_{load}$ to opór taśmy przy maksymalnym wydłużeniu, a wzór jest uproszczonym modelem linearnym).*

---

## 🛠 Implementacja w Python (Raspberry Pi)

Poniżej przedstawiono fragment kodu w języku Python, ilustrujący sposób obliczania wybranych parametrów z surowych danych.

```python
import numpy as np
from scipy.integrate import simps
from scipy.signal import find_peaks

def calculate_metrics(time, force):
    """
    time: tablica czasów [s]
    force: tablica siły [N]
    """
    # 1. Peak Force
    peak_force = np.max(force)
    
    # 2. Mean Force
    mean_force = np.mean(force)
    
    # 3. Impulse (całka z siły po czasie)
    impulse = simps(force, time)
    
    # 4. RFD (maksymalne nachylenie)
    dt = np.diff(time)
    df = np.diff(force)
    rfd_raw = df / dt
    max_rfd = np.max(rfd_raw)
    
    # 5. Coefficient of Variation
    cv = (np.std(force) / mean_force) * 100 if mean_force != 0 else 0
    
    return {
        "Peak_Force_N": peak_force,
        "Mean_Force_N": mean_force,
        "Impulse_Ns": impulse,
        "Max_RFD_Ns": max_rfd,
        "CV_percent": cv
    }

# Przykładowe użycie
# metrics = calculate_metrics(data_time, data_force)
```

---

## 📝 Wnioski

Przedstawione wzory pozwalają na przekształcenie surowego sygnału z tensometru w zaawansowane wskaźniki treningowe. Dzięki implementacji na Raspberry Pi możliwe jest:
1. **Monitorowanie postępu** w czasie rzeczywistym.
2. **Wykrywanie zmęczenia** i sugerowanie przerw.
3. **Analiza techniki** poprzez wskaźniki płynności i symetrii.
4. **Personalizacja rehabiltacji** dzięki precyzyjnym danym liczbowym.

Dokument ten stanowi podstawę do implementacji algorytmów w oprogramowaniu urządzenia.
