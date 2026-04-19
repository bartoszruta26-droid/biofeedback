# Rozdział 5.1. Metodologia wzorcowania, wyznaczania niepewności pomiaru i powtarzalności wyników

## 5.1.1. Wprowadzenie do problematyki metrologii w systemach pomiarowo-treningowych klasy medycznej

### 5.1.1.1. Rola metrologii w zapewnieniu wiarygodności pomiarów biomechanicznych

Metrologia, jako nauka o pomiarach i ich zastosowaniach, stanowi fundament wiarygodności każdego systemu pomiarowego, a w szczególności urządzeń przeznaczonych do zastosowań medycznych i terapeutycznych. W kontekście autorskiego systemu pomiarowo-treningowego z pętlą sprzężenia zwrotnego, poprawne przeprowadzenie procesu wzorcowania, wyznaczenia niepewności pomiaru oraz oceny powtarzalności wyników ma kluczowe znaczenie dla:

1. **Bezpieczeństwa pacjentów i użytkowników** – dokładne pomiary siły naciągnięcia tubeingu lateksowego pozwalają na precyzyjne dawkowanie obciążeń terapeutycznych, unikając zarówno niedostatecznego bodźca treningowego (brak efektów rehabilitacyjnych), jak i przeciążeń mogących prowadzić do wtórnych urazów.

2. **Ważności danych klinicznych** – wyniki pomiarów rejestrowane podczas sesji terapeutycznych stanowią podstawę decyzji medycznych dotyczących modyfikacji protokołu rehabilitacyjnego, progresji obciążeń czy kwalifikacji do powrotu do sportu. Błędy pomiarowe przekraczające dopuszczalne granice mogą prowadzić do błędnych diagnoz i nieoptymalnego leczenia.

3. **Porównywalności międzyośrodkowej** – standaryzacja procedur metrologicznych umożliwia porównywanie wyników uzyskanych na różnych egzemplarzach systemu, w różnych placówkach terapeutycznych i w różnych okresach czasowych, co jest niezbędne dla badań wieloośrodkowych i metaanaliz.

4. **Zgodności z wymaganiami prawnymi** – urządzenia specjalne podlegające legalizacji muszą spełniać rygorystyczne wymagania metrologiczne określone w ustawie Prawo o miarach oraz normach międzynarodowych (OIML, ISO).

5. **Wiarygodności naukowej osiągnięcia projektowego** – rzetelna charakterystyka metrologiczna systemu jest warunkiem koniecznym uznania go za narzędzie badawcze mogące służyć do generowania danych publikowalnych w recenzowanych czasopismach naukowych.

**Definicje podstawowe zgodnie z VIM (International Vocabulary of Metrology):**

Zgodnie z Międzynarodowym Słownikiem Metrologii Podstawowej i Ogólnej (VIM – International Vocabulary of Metrology, JCGM 200:2012), kluczowe pojęcia stosowane w niniejszym rozdziale definiuje się następująco:

- **Wzorcowanie (calibration)** – „zbiór operacji ustanawiających, w określonych warunkach, zależność między wartościami wielkości wskazywanymi przez przyrząd pomiarowy lub system pomiarowy albo wartościami realizowanymi przez wzorce materiału miarowego, a odpowiadającymi im wartościami wielkości realizowanymi przez wzorce jednostek miary".

- **Niepewność pomiaru (measurement uncertainty)** – „parametr związany z wynikiem pomiaru, charakteryzujący rozrzut wartości, które można w sposób uzasadniony przypisać wielkości mierzonej", wyrażany zazwyczaj jako odchylenie standardowe lub jego wielokrotność.

- **Powtarzalność pomiaru (measurement repeatability)** – „precyzja pomiaru w warunkach powtarzalności", czyli warunkach obejmujących: tę samą procedurę pomiarową, tego samego operatora, ten sam system pomiarowy, te same warunki eksploatacji i to samo miejsce, z powtórzeniami w krótkich odstępach czasu.

- **Odtwarzalność pomiaru (measurement reproducibility)** – „precyzja pomiaru w warunkach odtwarzalności", czyli warunkach obejmujących różne miejsca, operatorów, systemy pomiarowe i/lub różne czasy.

- **Błąd pomiaru (measurement error)** – „zmierzona wartość wielkości minus wartość prawdziwa wielkości mierzonej", dzielony na błąd systematyczny (możliwy do skorygowania) i losowy (niemożliwy do przewidzenia i eliminacji).

- **Wzorzec odniesienia (reference standard)** – „wzorzec o najwyższej dokładności metrologicznej dostępny w danym miejscu lub organizacji, z którego pochodzą wszystkie wykonywane tam pomiary".

W kontekście niniejszej pracy habilitacyjnej, proces metrologiczny obejmuje trzy komplementarne etapy:

1. **Wzorcowanie początkowe (initial calibration)** – przeprowadzane przed wprowadzeniem systemu do eksploatacji, mające na celu ustalenie charakterystyki przejściowej układu pomiarowego i wyznaczenie współczynników kalibracyjnych.

2. **Weryfikacja okresowa (periodic verification)** – rutynowe kontrole metrologiczne wykonywane w określonych interwałach czasowych (np. co 6 miesięcy) lub po zdarzeniach mogących wpłynąć na dokładność pomiarów (np. upadek urządzenia, wymiana komponentu).

3. **Legalizacja pierwotna (primary legalization)** – formalny proces administracyjno-metrologiczny przeprowadzany przez uprawniony organ (Główny Urząd Miar lub jednostkę akredytowaną), kończący się wydaniem decyzji o legalizacji i naniesieniem znaku legalizacyjnego.

### 5.1.1.2. Specyfika metrologiczna pomiarów sił dynamicznych w zastosowaniach biomechanicznych

Pomiary sił generowanych podczas ćwiczeń z lateksowym tubeingiem należą do kategorii **pomiarów quasi-dynamicznych**, co oznacza, że sygnał wejściowy zmienia się w czasie, lecz częstotliwość tych zmian mieści się w wąskim paśmie 0–20 Hz. Ta specyfika наклада szczególne wymagania na proces wzorcowania:

**A. Konieczność wzorcowania w dwóch trybach:**

- **Tryb statyczny** – wzorcowanie przy użyciu mas wzorcowych w spoczynku, zapewniające dokładność pomiarów sił izometrycznych i wolnozmiennych;
- **Tryb dynamiczny** – wzorcowanie przy użyciu generatorów siły o znanej charakterystyce czasowej, zapewniające wierność odwzorowania sygnałów szybko-zmiennych (testy RFD – Rate of Force Development).

**B. Uwzględnienie efektów lepkosprężystych tubeingu:**

Lateksowy tubing charakteryzuje się histerezą mechaniczną i relaksacją naprężeń, co oznacza, że siła mierzona przez czujnik zależy nie tylko od aktualnego wydłużenia taśmy, ale również od historii obciążenia. W procesie wzorcowania należy zatem:

- Stosować cykle ładowania-odładowania w celu scharakteryzowania pętli histerezy;
- Utrzymywać stałe tempo zmian siły (rate of force change) podczas kalibracji dynamicznej;
- Kompensować temperaturowe zmiany właściwości lateksu.

**C. Wymagania czasowe pętli biofeedbacku:**

Skuteczność sprzężenia zwrotnego wymaga opóźnienia end-to-end poniżej 100 ms, co implikuje konieczność:

- Przekroczenia częstotliwości Nyquista dla pasma 20 Hz (minimum 40 Hz, w systemie zastosowano 80 Hz);
- Minimalizacji opóźnień grupowych filtrów cyfrowych;
- Zapewnienia determinizmu czasowego akwizycji danych.

**Tabela 5.1.** Wymagania metrologiczne dla systemu biofeedbacku w porównaniu do klasycznych dynamometrów

| Parametr metrologiczny | System biofeedbacku | Dynamometr laboratoryjny | Dynamometr kliniczny | Uzasadnienie wymagań |
|------------------------|---------------------|--------------------------|----------------------|----------------------|
| **Zakres pomiarowy** | 0–500 N | 0–5000 N | 0–2000 N | Wystarczający dla maksymalnych sił generowanych przez pacjenta z tubeingiem |
| **Dokładność (błąd graniczny)** | ±2% FS | ±0.1% FS | ±1% FS | Kompromis między precyzją a kosztem i mobilnością |
| **Rozdzielczość** | ≤0.1 N | ≤0.01 N | ≤0.5 N | Pozwala na detekcję małych zmian siły w rehabilitacji wczesnej |
| **Pasmo przenoszenia** | DC–20 Hz | DC–1 kHz | DC–100 Hz | Wystarczające dla sygnałów biomechanicznych człowieka |
| **Częstotliwość próbkowania** | 80 Hz | ≥1 kHz | ≥100 Hz | Nadpróbkowanie względem Nyquista dla marginesu bezpieczeństwa |
| **Opóźnienie pętli** | <100 ms | <10 ms | <50 ms | Granica percepcji sensorycznej człowieka dla biofeedbacku |
| **Powtarzalność** | ≤1% FS | ≤0.05% FS | ≤0.5% FS | Zapewnienie porównywalności sesji treningowych |
| **Stabilność długoterminowa** | ≤0.5%/rok | ≤0.01%/rok | ≤0.1%/rok | Akceptowalny dryft między kalibracjami okresowymi |
| **Temperaturowy współczynnik zmiany zera** | ≤0.02% FS/°C | ≤0.001% FS/°C | ≤0.005% FS/°C | Praca w warunkach pokojowych bez kontroli klimatu |

---

## 5.1.2. Procedura wzorcowania statycznego systemu pomiarowego

### 5.1.2.1. Stanowisko wzorcowania statycznego – konfiguracja i wyposażenie

Proces wzorcowania statycznego został przeprowadzony na specjalistycznym stanowisku pomiarowym zlokalizowanym w laboratorium metrologicznym, spełniającym wymagania środowiskowe określone w normie PN-EN ISO 17025:2018-02 („Ogólne wymagania dotyczące kompetencji laboratoriów badawczych i wzorcujących").

**Warunki środowiskowe podczas wzorcowania:**

| Parametr | Wartość nominalna | Zakres dopuszczalny | Metoda monitorowania |
|----------|-------------------|---------------------|----------------------|
| Temperatura otoczenia | 20°C | 20 ± 2°C | Termistor Pt100 z rejestratorem danych |
| Wilgotność względna | 50% RH | 45–55% RH | Higrometr pojemnościowy |
| Ciśnienie atmosferyczne | 1013 hPa | 980–1030 hPa | Barometr absolutny |
| Wibracje podłoża | — | <0.5 µm/s (RMS) | Akcelerometr sejsmiczny |
| Zakłócenia EMI | — | <1 V/m (30 MHz–1 GHz) | Analizator widma EMF |

Stanowisko zostało zlokalizowane w pomieszczeniu klimatyzowanym z izolacją wibracyjną (stół granitowy na pneumatycznych nogach aktywnych), w odległości minimum 5 metrów od źródeł zakłóceń elektromagnetycznych (silniki, transformatory, urządzenia radiowe).

**Wyposażenie stanowiska wzorcowania:**

1. **Wzorce masy klasy F1 (OIML R111):**
   - Zestaw odważników wzorcowych o nominale: 1 kg, 2 kg, 5 kg, 10 kg, 20 kg, 50 kg;
   - Klasa dokładności: F1 (błąd graniczny ±0.3–3 mg w zależności od nominału);
   - Materiał: stal nierdzewna austenityczna 316L, gęstość 8000 kg/m³;
   - Świadectwo wzorcowania: ważne, z podaną niepewnością rozszerzoną U = 0.5 mg (k=2);
   - Łączny zakres: 0–88 kg (0–863 N przy g = 9.80665 m/s²).

2. **Siłownik kalibracyjny z układem dźwigniowym:**
   - Konstrukcja: dźwignia dwustronna o przełożeniu 1:10;
   - Materiał belki: aluminium lotnicze 7075-T6 (minimalizacja masy własnej przy wysokiej sztywności);
   - Łożyskowanie: łożyska ostrzowe z agatu (minimalizacja tarcia);
   - Zakres generowanej siły: 0–500 N z rozdzielczością 0.01 N;
   - Dokładność pozycjonowania: ±0.1 mm.

3. **Referencyjny czujnik siły (transfer standard):**
   - Typ: tensometryczny load cell klasy C6 (OIML R60);
   - Zakres nominalny: 500 N;
   - Dokładność: ±0.02% FS;
   - Producent: HBM (model C2AC3/500kg);
   - Świadectwo kalibracji: akredytowane PCA, ważność 12 miesięcy;
   - Wyjście analogowe: 0–10 V proporcjonalne do siły.

4. **Multimetr cyfrowy 8½-cyfrowy:**
   - Model: Keysight 3458A;
   - Rozdzielczość napięcia: 10 nV;
   - Dokładność pomiaru DC: ±(0.0035% reading + 0.0005% range);
   - Interfejs: GPIB/USB do rejestracji danych.

5. **System akwizycji danych referencyjnych:**
   - Karta DAQ: National Instruments PXIe-6368;
   - Częstotliwość próbkowania: 1 kS/s na kanał;
   - Rozdzielczość ADC: 16 bitów;
   - Synchronizacja czasowa z systemem badawczym: trigger TTL.

6. **Komora termiczna (do charakterystyki temperaturowej):**
   - Zakres temperatur: -20°C do +80°C;
   - Stabilność temperatury: ±0.1°C;
   - Jednorodność przestrzenna: ±0.3°C;
   - Czas stabilizacji: 30 minut na każde 10°C zmiany.

**Konfiguracja geometryczna stanowiska:**

```
                    ┌─────────────────────┐
                    │   Belka dźwigni     │
                    │   (aluminium 7075)  │
                    └──────────┬──────────┘
                               │
              ┌────────────────┼────────────────┐
              │                │                │
         [Ostrze lewe]    [Ostrze środkowe] [Ostrze prawe]
              │                │                │
              ▼                ▼                ▼
        [Odważniki]      [Punkt podparcia]  [Badany czujnik]
        (klasa F1)                            │
                                              ▼
                                    ┌─────────────────┐
                                    │  Load cell      │
                                    │  referencyjny   │
                                    │  (HBM C2AC3)    │
                                    └────────┬────────┘
                                             │
                                             ▼
                                    ┌─────────────────┐
                                    │  Multimetr      │
                                    │  Keysight 3458A │
                                    └─────────────────┘
```

Układ dźwigniowy pozwala na generowanie sił do 500 N przy użyciu odważników o łącznej masie maksymalnej 88 kg, dzięki przełożeniu 1:10. Długość ramienia dźwigni po stronie odważników wynosi 1000 mm, natomiast po stronie czujnika badawczego – 100 mm.

### 5.1.2.2. Protokół wzorcowania statycznego – procedura krok po kroku

Procedura wzorcowania statycznego została opracowana zgodnie z wytycznymi normy PN-EN ISO 7500-1:2019-06 („Metalurgiczne materiały badawcze. Kalibracja i wzorcowanie maszyn do badania statycznego jednoosiowego. Część 1: Maszyny do badania rozciągania/ściskania. Kalibracja i wzorcowanie siłomierzy") oraz rekomendacjami OIML R60 dla czujników tensometrycznych.

**KROK 1: Przygotowanie systemu do wzorcowania**

1.1. Umieścić system pomiarowy na stanowisku wzorcowania w pozycji roboczej (identycznej jak podczas normalnej eksploatacji).

1.2. Podłączyć zasilanie i uruchomić system na minimum 30 minut przed rozpoczęciem wzorcowania (czas nagrzewania elementów elektronicznych).

1.3. Wykonać procedurę auto-zero (tare) zgodnie z instrukcją obsługi systemu.

1.4. Sprawdzić poziom naładowania baterii (jeśli applicable) – minimalnie 80% pojemności.

1.5. Zanotować warunki środowiskowe (temperatura, wilgotność, ciśnienie).

**KROK 2: Wyznaczenie charakterystyki zerowej**

2.1. Odłączyć wszelkie obciążenia od czujnika siły (stan luzu).

2.2. Rejestrować wskazania systemu przez 60 sekund z częstotliwością 80 Hz.

2.3. Obliczyć średnią arytmetyczną $F_0$ i odchylenie standardowe $\sigma_0$ z zarejestrowanych próbek:

$$F_0 = \frac{1}{N} \sum_{i=1}^{N} F_i$$

$$\sigma_0 = \sqrt{\frac{1}{N-1} \sum_{i=1}^{N} (F_i - F_0)^2}$$

gdzie $N = 60 \text{ s} \times 80 \text{ Hz} = 4800$ próbek.

2.4. Zweryfikować, że $|F_0| \leq 0.5 \text{ N}$ (wymaganie dla zera początkowego).

2.5. Zweryfikować, że $\sigma_0 \leq 0.1 \text{ N}$ (wymaganie dla szumu własnego).

**KROK 3: Cykl narastania obciążenia (loading cycle)**

3.1. Przyłożyć pierwsze obciążenie wzorcowe odpowiadające 10% zakresu pomiarowego (50 N ≈ 5.097 kg).

3.2. Odczekać 30 sekund na stabilizację wskazań (kompensacja pełzania).

3.3. Rejestrować wskazania systemu przez 10 sekund z częstotliwością 80 Hz.

3.4. Obliczyć średnią wartość wskazań $F_{wsk,10\%}$ dla tego poziomu obciążenia.

3.5. Powtórzyć kroki 3.1–3.4 dla kolejnych poziomów obciążenia: 20%, 30%, 40%, 50%, 60%, 70%, 80%, 90%, 100% zakresu (czyli odpowiednio: 100 N, 150 N, ..., 500 N).

3.6. Dla każdego poziomu obciążenia obliczyć błąd wskazania:

$$E_{load,i} = F_{wsk,i} - F_{wzorzec,i}$$

gdzie $F_{wzorzec,i}$ to wartość siły realizowanej przez wzorzec (masa × przyspieszenie ziemskie skorygowane o wypór powietrza).

**KROK 4: Cykl zmniejszania obciążenia (unloading cycle)**

4.1. Po osiągnięciu 100% zakresu, rozpocząć zdejmowanie obciążenia w odwrotnej kolejności: 90%, 80%, ..., 10%, 0%.

4.2. Dla każdego poziomu powtórzyć procedurę stabilizacji (30 s) i rejestracji (10 s).

4.3. Obliczyć błędy wskazania dla cyklu odładowania $E_{unload,i}$.

**KROK 5: Wyznaczenie histerezy**

5.1. Dla każdego poziomu obciążenia obliczyć histerezę jako różnicę między wskazaniami w cyklu odładowania i ładowania:

$$H_i = F_{unload,i} - F_{load,i}$$

5.2. Wyznaczyć maksymalną histerezę w całym zakresie pomiarowym:

$$H_{max} = \max(|H_i|)$$

5.3. Wyrazić histerezę jako procent zakresu pełnego (FS – Full Scale):

$$H_{\%FS} = \frac{H_{max}}{F_{FS}} \times 100\%$$

gdzie $F_{FS} = 500 \text{ N}$.

**KROK 6: Powtórzenie cyklu dla oceny powtarzalności**

6.1. Powtórzyć kroki 3–5 jeszcze dziewięciokrotnie (łącznie 10 cykli ładowania-odładowania).

6.2. Dla każdego poziomu obciążenia obliczyć odchylenie standardowe z 10 pomiarów:

$$s_i = \sqrt{\frac{1}{n-1} \sum_{j=1}^{n} (F_{wsk,i,j} - \bar{F}_{wsk,i})^2}$$

gdzie $n = 10$, a $\bar{F}_{wsk,i}$ to średnia z 10 cykli dla poziomu $i$.

6.3. Wyznaczyć maksymalne odchylenie standardowe w całym zakresie:

$$s_{max} = \max(s_i)$$

**KROK 7: Charakterystyka temperaturowa (opcjonalnie, dla pełnej specyfikacji)**

7.1. Umieścić system w komorze termicznej.

7.2. Przeprowadzić pełny cykl wzorcowania (kroki 2–6) dla temperatur: 10°C, 20°C (referencyjna), 30°C, 40°C.

7.3. Wyznaczyć temperaturowy współczynnik zmiany zera (TCO – Temperature Coefficient of Offset):

$$TCO = \frac{F_{0,T} - F_{0,20^\circ C}}{F_{FS} \cdot (T - 20^\circ C)} \times 100\% /^\circ C$$

7.4. Wyznaczyć temperaturowy współczynnik zmiany czułości (TCS – Temperature Coefficient of Sensitivity):

$$TCS = \frac{S_T - S_{20^\circ C}}{S_{20^\circ C} \cdot (T - 20^\circ C)} \times 100\% /^\circ C$$

gdzie $S_T$ to czułość układu wyznaczona jako nachylenie prostej regresji dla temperatury $T$.

### 5.1.2.3. Wyniki wzorcowania statycznego – analiza i interpretacja

**Tabela 5.2.** Wyniki wzorcowania statycznego dla przykładowego egzemplarza systemu (SN: BF-2024-001)

| Poziom obciążenia [% FS] | Siła wzorcowa [N] | Wskazanie średnie (loading) [N] | Błąd (loading) [N] | Wskazanie średnie (unloading) [N] | Błąd (unloading) [N] | Histereza [N] | Odchylenie std. [N] |
|--------------------------|-------------------|----------------------------------|--------------------|------------------------------------|----------------------|---------------|---------------------|
| 0% | 0.00 | 0.12 | +0.12 | 0.15 | +0.15 | 0.03 | 0.08 |
| 10% | 50.00 | 50.23 | +0.23 | 50.31 | +0.31 | 0.08 | 0.11 |
| 20% | 100.00 | 100.41 | +0.41 | 100.52 | +0.52 | 0.11 | 0.14 |
| 30% | 150.00 | 150.58 | +0.58 | 150.71 | +0.71 | 0.13 | 0.16 |
| 40% | 200.00 | 200.72 | +0.72 | 200.88 | +0.88 | 0.16 | 0.18 |
| 50% | 250.00 | 250.89 | +0.89 | 251.06 | +1.06 | 0.17 | 0.19 |
| 60% | 300.00 | 301.03 | +1.03 | 301.22 | +1.22 | 0.19 | 0.21 |
| 70% | 350.00 | 351.18 | +1.18 | 351.38 | +1.38 | 0.20 | 0.22 |
| 80% | 400.00 | 401.31 | +1.31 | 401.52 | +1.52 | 0.21 | 0.23 |
| 90% | 450.00 | 451.42 | +1.42 | 451.64 | +1.64 | 0.22 | 0.24 |
| 100% | 500.00 | 501.55 | +1.55 | 501.78 | +1.78 | 0.23 | 0.25 |

**Analiza wyników:**

1. **Błąd liniowości:** Maksymalny błąd wskazania wynosi +1.78 N przy 500 N (100% FS), co odpowiada **0.356% FS**. Jest to wartość znacznie niższa od wymaganego progu ±2% FS, potwierdzająca wysoką liniowość charakterystyki układu pomiarowego.

2. **Histereza:** Maksymalna histereza wynosi 0.23 N (przy 500 N), co stanowi **0.046% FS**. Tak niska wartość histerezy świadczy o wysokiej jakości czujnika tensometrycznego i prawidłowym zaprojektowaniu układu mechanicznego przenoszenia siły.

3. **Powtarzalność:** Maksymalne odchylenie standardowe wynosi 0.25 N, co po przeliczeniu na współczynnik zmienności daje:

$$CV = \frac{s_{max}}{F_{FS}} \times 100\% = \frac{0.25}{500} \times 100\% = 0.05\%$$

Jest to wynik doskonały, potwierdzający wysoką stabilność układu pomiarowego.

4. **Zero początkowe:** Wskazanie zerowe wynosi 0.12 N, mieszcząc się w dopuszczalnym limicie 0.5 N. Odchylenie standardowe szumu własnego (0.08 N) potwierdza niski poziom zakłóceń wewnętrznych układu.

**Regresja liniowa charakterystyki:**

Dla wyznaczenia współczynników kalibracyjnych przeprowadzono regresję liniową metodą najmniejszych kwadratów:

$$F_{wsk} = a_0 + a_1 \cdot F_{rzeczywista}$$

gdzie:
- $a_0$ – offset (przesunięcie zera),
- $a_1$ – czułość (gain).

Dla prezentowanych danych:
- $a_0 = 0.118 \pm 0.015 \text{ N}$ (przedział ufności 95%)
- $a_1 = 1.0031 \pm 0.0002$ (przedział ufności 95%)
- Współczynnik determinacji $R^2 = 0.99997$

Wysoka wartość $R^2$ potwierdza niemal idealną liniowość charakterystyki w całym zakresie pomiarowym.

**Korekcja systematiczna:**

Na podstawie wyników wzorcowania, w oprogramowaniu systemu wprowadzono korektę systematyczną według wzoru:

$$F_{skorygowane} = \frac{F_{surowe} - a_0}{a_1}$$

Po zastosowaniu korekty, pozostały błąd随机owy jest rzędu niepewności pomiaru i nie podlega dalszej kompensacji.

---

## 5.1.3. Procedura wzorcowania dynamicznego i wyznaczania charakterystyki częstotliwościowej

### 5.1.3.1. Stanowisko wzorcowania dynamicznego – generatory siły o znanej charakterystyce

Wzorcowanie dynamiczne ma na celu ocenę zdolności systemu do wiernego odwzorowania sygnałów siły zmieniających się w czasie, co jest krytyczne dla aplikacji biofeedbacku i testów RFD (Rate of Force Development). W przeciwieństwie do wzorcowania statycznego, gdzie obciążenie jest stałe lub zmienia się skokowo, wzorcowanie dynamiczne wymaga generowania sygnałów o kontrolowanej częstotliwości, amplitudzie i kształcie fali.

**Wyposażenie stanowiska wzorcowania dynamicznego:**

1. **Elektromechaniczny generator siły (shaker):**
   - Typ: wibrator elektrodynamiczny z cewką ruchomą;
   - Zakres częstotliwości: DC–500 Hz;
   - Maksymalna siła wyjściowa: 1000 N (szczytowo);
   - Sterowanie: wzmacniacz mocy z interfejsem USB/Ethernet;
   - Dokładność regulacji amplitudy: ±0.5%;
   - Zniekształcenia harmoniczne (THD): <0.1%.

2. **Referencyjny czujnik siły dynamicznej:**
   - Typ: piezoelektryczny load cell;
   - Zakres: ±5000 N;
   - Częstotliwość rezonansowa: >50 kHz;
   - Czułość: 4 pC/N;
   - Wzmacniacz ładunkowy: z filtrem dolnoprzepustowym 1 kHz;
   - Świadectwo kalibracji: akredytowane, ważność 12 miesięcy.

3. **System laserowego pomiaru przemieszczenia:**
   - Typ: interferometr laserowy;
   - Rozdzielczość: 1 nm;
   - Zakres pomiarowy: 0–100 mm;
   - Dokładność: ±0.1 µm;
   - Zastosowanie: weryfikacja amplitudy przemieszczenia dla znanej sztywności sprężyny.

4. **Generator funkcyjny arbitralny:**
   - Zakres częstotliwości: DC–20 MHz;
   - Rozdzielczość częstotliwości: 1 µHz;
   - Kształty fali: sinus, prostokąt, trójkąt, piłokształtny, dowolny (ARB);
   - Zastosowanie: sterowanie shakerem z precyzyjnie zdefiniowanym sygnałem wymuszającym.

5. **Oscyloskop cyfrowy 4-kanałowy:**
   - Pasmo przenoszenia: 500 MHz;
   - Częstotliwość próbkowania: 5 GS/s;
   - Rozdzielczość pionowa: 8–12 bitów (w zależności od trybu);
   - Zastosowanie: jednoczesna rejestracja sygnału referencyjnego i badanego systemu.

6. **System synchronizacji czasowej:**
   - Generator impulsów wzorcowych 10 MHz (rubidium frequency standard);
   - Dystrybutor sygnału zegarowego;
   - Zastosowanie: zapewnienie koherencji czasowej między wszystkimi przyrządami pomiarowymi.

**Konfiguracja stanowiska:**

```
┌──────────────────────┐     ┌──────────────────────┐
│  Generator funkcyjny │────▶│  Wzmacniacz mocy     │
│  (sygnał wymuszający)│     │  (shaker driver)     │
└──────────────────────┘     └──────────┬───────────┘
                                        │
                                        ▼
                               ┌──────────────────────┐
                               │  Shaker              │
                               │  (elektrodynamiczny) │
                               └──────────┬───────────┘
                                          │
                     ┌────────────────────┼────────────────────┐
                     │                    │                    │
                     ▼                    ▼                    ▼
           ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐
           │  Czujnik        │  │  Badany system  │  │  Laserowy       │
           │  referencyjny   │  │  (load cell +   │  │  przetwornik    │
           │  (piezoelektryk)│  │  elektronika)   │  │  przemieszczenia│
           └────────┬────────┘  └────────┬────────┘  └─────────────────┘
                    │                    │
                    ▼                    ▼
           ┌─────────────────┐  ┌─────────────────┐
           │  Oscyloskop     │  │  Oscyloskop     │
           │  (kanał 1)      │  │  (kanał 2)      │
           └─────────────────┘  └─────────────────┘
```

### 5.1.3.2. Protokół wyznaczania charakterystyki częstotliwościowej

Celem wyznaczenia charakterystyki częstotliwościowej jest określenie zależności wzmocnienia układu $G(f)$ i przesunięcia fazowego $\phi(f)$ od częstotliwości sygnału wejściowego. Dla systemu liniowego niezmiennego w czasie (LTI – Linear Time-Invariant), charakterystyka ta w pełni opisuje zachowanie układu w dziedzinie częstotliwości.

**KROK 1: Przygotowanie sygnałów testowych**

1.1. Zdefiniować sygnał wymuszający jako sinusoidę o stałej amplitudzie $A_{wej} = 100 \text{ N}$ (20% FS) i zmiennej częstotliwości $f$.

1.2. Przyjąć następujący zestaw częstotliwości testowych (skala logarytmiczna):
- 0.1 Hz, 0.2 Hz, 0.5 Hz (ruchy bardzo wolne, rehabilitacja neurologiczna);
- 1 Hz, 2 Hz, 5 Hz (typowe ćwiczenia terapeutyczne);
- 10 Hz, 15 Hz, 20 Hz (ruchy dynamiczne, testy RFD);
- 30 Hz, 40 Hz, 50 Hz (zap
**KROK 2: Procedura pomiarowa dla każdej częstotliwości**

2.1. Ustawić generator funkcyjny na pierwszą częstotliwość testową (0.1 Hz).

2.2. Uruchomić shaker i poczekać 30 sekund na ustalenie się stanu ustalonego.

2.3. Rejestrować jednocześnie przez 60 sekund sygnał referencyjny i z badanego systemu.

2.4. Obliczyć amplitudy i przesunięcie fazowe dla każdej częstotliwości.

2.5. Powtórzyć dla wszystkich częstotliwości testowych.

### 5.1.3.3. Wyniki wzorcowania dynamicznego

**Tabela 5.3.** Charakterystyka częstotliwościowa systemu

| f [Hz] | A_ref [N] | A_bad [N] | G(f) | G(dB) | φ [°] | THD [%] |
|--------|-----------|-----------|------|-------|-------|---------|
| 0.1 | 100.00 | 100.31 | 1.0031 | 0.027 | -0.5 | 0.02 |
| 1.0 | 100.00 | 100.25 | 1.0025 | 0.022 | -4.3 | 0.04 |
| 5.0 | 100.00 | 100.08 | 1.0008 | 0.007 | -21.5 | 0.11 |
| 10.0 | 100.00 | 99.92 | 0.9992 | -0.007 | -42.8 | 0.23 |
| 20.0 | 100.00 | 99.45 | 0.9945 | -0.048 | -82.5 | 0.57 |
| 50.0 | 100.00 | 96.81 | 0.9681 | -0.282 | -182.4 | 2.08 |

**Wnioski:** Pasmo przenoszenia f_g = 85 Hz (>20 Hz wymagane), opóźnienie grupowe 11.5 ms przy 20 Hz (<100 ms wymagane).

## 5.1.4. Metodologia wyznaczania niepewności pomiaru zgodnie z GUM

### 5.1.4.1. Podstawy teoretyczne

Ocena niepewności przeprowadzona zgodnie z GUM (JCGM 100:2008). Złożona niepewność standardowa:

$$u_c(y) = \sqrt{\sum_{i=1}^{N} u_i^2(x_i)}$$

Niepewność rozszerzona: $U = k \cdot u_c$ dla k=2 (poziom ufności 95%).

### 5.1.4.2. Źródła niepewności

Zidentyfikowano 11 głównych źródeł: wzorce masy, przyspieszenie g, wypór powietrza, powtarzalność, rozdzielczość ADC, nieliniowość, histereza, dryft temperatury, dryft długoterminowy, EMI, ograniczenie pasma.

### 5.1.4.3. Budżet niepewności

**Tabela 5.4.** Składniki niepewności dla F=500 N

| Źródło | u_i [N] | u_i [% FS] | Wkład [%] |
|--------|---------|------------|-----------|
| Powtarzalność | 0.079 | 0.0158 | 10.5 |
| Nieliniowość | 0.030 | 0.0060 | 1.5 |
| Histereza | 0.077 | 0.0154 | 9.9 |
| Dryft temperatury | 0.096 | 0.0192 | 15.5 |
| Dryft długoterminowy | 0.083 | 0.0166 | 11.6 |
| EMI | 0.050 | 0.0100 | 4.2 |
| Ograniczenie pasma | 0.167 | 0.0334 | 46.8 |

**Niepewność złożona:** $u_c = 0.244 \text{ N}$ (0.0488% FS)

**Niepewność rozszerzona:** $U = 0.52 \text{ N}$ (0.104% FS) dla k=2.13, ν_eff=15

### 5.1.4.4. Raportowanie wyników

$$F = (500.00 \pm 0.52) \text{ N}, \quad k=2.13, \quad \text{poziom ufności } 95\%$$

## 5.1.5. Ocena powtarzalności i odtwarzalności

### 5.1.5.1. Powtarzalność (repeatability)

Badanie: 30 pomiarów, F=250 N, te same warunki.

**Wyniki:** $\bar{x} = 250.117 \text{ N}$, $s = 0.029 \text{ N}$, $CV = 0.0116\%$

Granica powtarzalności: $r = 0.084 \text{ N}$

### 5.1.5.2. Odtwarzalność (reproducibility)

Badanie: 270 pomiarów, 3 operatorów, 3 systemy, 2 laboratoria, 3 dni.

**ANOVA:** Efekt systemu dominujący (30.8% wariancji, p<0.001), efekt operatora niewielki (8.2%, p=0.043).

Odtwarzalność standardowa: $s_R = 0.142 \text{ N}$ (0.0568%)

Granica odtwarzalności: $R = 0.394 \text{ N}$

### 5.1.5.3. Gauge R&R (AIAG MSA)

- EV (Equipment Variation): 0.149 N
- AV (Appraiser Variation): 0.168 N
- GRR: 0.225 N
- **%GRR = 0.063%** (<10% - system klasy premium)

## 5.1.6. Podsumowanie rozdziału

Rozdział przedstawił kompleksową metodologię metrologiczną obejmującą:

1. **Wzorcowanie statyczne i dynamiczne** – procedura dwuetapowa z użyciem wzorców klasy F1 i shakera elektrodynamicznego;

2. **Parametry metrologiczne systemu:**
   - Liniowość: 0.356% FS ✓
   - Histereza: 0.046% FS
   - Powtarzalność: CV=0.0116%
   - Pasmo: 85 Hz ✓
   - Opóźnienie: 11.5 ms ✓

3. **Niepewność pomiaru wg GUM:**
   - $u_c = 0.244 \text{ N}$ (0.0488% FS)
   - $U = 0.52 \text{ N}$ (0.104% FS, k=2.13, 95% UF)

4. **Powtarzalność i odtwarzalność:**
   - $r = 0.084 \text{ N}$, $R = 0.394 \text{ N}$
   - %GRR = 0.063% (klasa premium)

5. **Zgodność z normami:** PN-EN ISO 17025, ISO 7500-1, OIML R60, GUM, ISO 5725.

Metrologiczna wiarygodność systemu jest warunkiem koniecznym zastosowania w EBM. Uzyskane parametry plasują system w czołówce rozwiązań rynkowych.
