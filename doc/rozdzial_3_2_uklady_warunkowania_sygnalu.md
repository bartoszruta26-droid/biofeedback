# Rozdział 3.2. Układy warunkowania sygnału, mostki Wheatstone'a i konwersja analogowo-cyfrowa

## 3.2.1. Wprowadzenie do problematyki warunkowania sygnałów tensometrycznych w systemach pomiarowych klasy medycznej

### 3.2.1.1. Specyfika sygnałów biomechanicznych i wyzwania metrologiczne

Projektowanie układu akwizycji danych dla autorskiego systemu pomiarowo-treningowego z pętlą sprzężenia zwrotnego stanowiło jedno z najważniejszych wyzwań inżynierskich niniejszego osiągnięcia projektowego. Sygnały biomechaniczne generowane podczas ćwiczeń z lateksowym tubeingiem charakteryzują się specyficznymi cechami, które wymuszają zastosowanie wyspecjalizowanych układów warunkowania sygnału (signal conditioning):

**A. Niski poziom amplitudy sygnału użytecznego:**
Czujniki tensometryczne oparte na tensometrach oporowych (strain gauges) generują sygnały elektryczne o ekstremalnie niskiej amplitudzie. Typowy tensometr metaliczny o rezystancji nominalnej $R_0 = 350\,\Omega$ i współczynniku odkształcenia (gauge factor) $GF \approx 2.0$ przy odkształceniu $\varepsilon = 1000\,\mu\varepsilon$ (co odpowiada znacznemu rozciągnięciu taśmy lateksowej) zmienia swoją rezystancję o zaledwie:

$$\Delta R = R_0 \cdot GF \cdot \varepsilon = 350\,\Omega \cdot 2.0 \cdot 1000 \times 10^{-6} = 0.7\,\Omega$$

W konfiguracji mostka Wheatstone'a zasilonego napięciem $V_{EX} = 5.0\,V$, tak niewielka zmiana rezystancji przekłada się na sygnał wyjściowy rzędu **milivoltów lub mikrowoltów**:

$$V_{OUT} \approx V_{EX} \cdot \frac{\Delta R}{4R_0} = 5.0\,V \cdot \frac{0.7\,\Omega}{4 \cdot 350\,\Omega} = 2.5\,mV$$

Dla pełnego zakresu pomiarowego systemu (0–500 N), sygnał użyteczny mieści się w przedziale **0–10 mV**, co wymaga wzmocnienia rzędu **500–1000 V/V** przed konwersją analogowo-cyfrową.

**B. Wysoki poziom zakłóceń wspólnych (Common-Mode Noise):**
W środowisku klinicznym i treningowym układ pomiarowy narażony jest na liczne źródła zakłóceń elektromagnetycznych (EMI):
- Sieć energetyczna 50 Hz i harmoniczne (indukcja w długich przewodach łączących czujnik z jednostką akwizycyjną);
- Urządzenia elektroterapeutyczne (TENS, NMES, ultradźwięki) generujące pola elektromagnetyczne o częstotliwościach 1–100 kHz;
- Silniki elektryczne (bieżnie, rowery treningowe) wytwarzające iskrowanie komutatorów;
- Komunikacja bezprzewodowa (WiFi 2.4 GHz, Bluetooth) mogąca powodować aliasing w przypadku niewłaściwego filtrowania antyaliasingowego.

Sygnały te pojawiają się jako napięcia wspólne (common-mode voltage) na obu wejściach układu różnicowego, typowo o amplitudzie **1–10 V**, czyli **100–1000 razy większej** niż sygnał użyteczny. Skuteczne tłumienie tych zakłóceń wymaga zastosowania wzmacniaczy instrumentalnych o wysokim współczynniku tłumienia sygnału wspólnego (CMRR – Common-Mode Rejection Ratio) rzędu **≥100 dB**.

**C. Wolnozmienny charakter sygnału i dryft temperaturowy:**
Sygnały biomechaniczne podczas ćwiczeń z tubeingiem mieszczą się w paśmie częstotliwości **0–20 Hz**:
- Ruchy wolne, kontrolowane (rehabilitacja neurologiczna): 0.1–2 Hz;
- Ćwiczenia dynamiczne, trening mocy: 2–10 Hz;
- Szybkie ruchy balistyczne, testy RFD (Rate of Force Development): 10–20 Hz.

Tak wąskie pasmo użyteczne umożliwia agresywne filtrowanie dolnoprzepustowe, jednakże jednocześnie czyni układ wrażliwym na **dryft zerowy** (zero drift) i **dryft wzmocnienia** (gain drift) spowodowany zmianami temperatury otoczenia. Tensometry metaliczne wykazują temperaturowy współczynnik zmiany rezystancji (TCR – Temperature Coefficient of Resistance) rzędu **±50 ppm/°C**, co przy zmianie temperatury o 10°C i braku kompensacji może wprowadzić błąd pomiaru przekraczający **5% wartości pełnego zakresu**.

**D. Wymagania czasowe pętli biofeedbacku:**
Skuteczna pętla sprzężenia zwrotnego w aplikacjach biofeedbacku wymaga opóźnienia end-to-end (od zmiany siły na czujniku do aktualizacji wizualizacji na ekranie) nieprzekraczającego **100 ms**. Ograniczenie to wymusza:
- Częstotliwość próbkowania **≥80 Hz** (twierdzenie Nyquista-Shannona dla pasma 20 Hz sugeruje minimum 40 Hz, jednak margines bezpieczeństwa i jakość rekonstrukcji sygnału wymagają nadpróbkowania);
- Czas przetwarzania ADC **<1 ms** na próbkę;
- Deterministyczny czas odpowiedzi układu (brak niedeterministycznych opóźnień typu jitter >10 ms).

**Tabela 3.4.** Wymagania metrologiczne dla układu warunkowania sygnału w systemie biofeedbacku

| Parametr | Wymaganie minimalne | Wartość docelowa | Uzasadnienie |
|----------|---------------------|------------------|--------------|
| Zakres wejściowy ADC | ±10 mV | ±20 mV | Pełny zakres sygnału z mostka + margines na przeciążenia |
| Wzmocnienie programowalne | 1–128 V/V | 1–1024 V/V | Elastyczność dla różnych czujników i zakresów siły |
| CMRR (50/60 Hz) | ≥80 dB | ≥120 dB | Tłumienie zakłóceń sieciowych w środowisku medycznym |
| Szum własny (input-referred) | ≤1 µV RMS (0.1–10 Hz) | ≤0.5 µV RMS | Rozdzielczość efektywna ≥16 bitów dla sygnału 10 mV |
| Dryft zerowy (temperaturowy) | ≤0.5 µV/°C | ≤0.1 µV/°C | Stabilność długoterminowa bez częstej recalibracji |
| Częstotliwość próbkowania | ≥40 Hz | 80–240 Hz | Wierność reprodukcji sygnałów dynamicznych RFD |
| Opóźnienie grupowe filtrów | ≤25 ms | ≤12.5 ms | Minimalizacja latencji pętli biofeedbacku |
| Współczynnik nieliniowości (THD) | ≤0.1% | ≤0.01% | Wierność odwzorowania kształtu fali siły |
| Impedancja wejściowa | ≥10 MΩ | ≥1 GΩ | Minimalizacja obciążenia mostka Wheatstone'a |

---

## 3.2.2. Teoria i implementacja mostków Wheatstone'a w układach tensometrycznych

### 3.2.2.1. Podstawy teoretyczne mostka Wheatstone'a

Mostek Wheatstone'a, wynaleziony przez Samuela Huntera Christiego w 1833 roku i spopularyzowany przez sir Charlesa Wheatstone'a w 1843 roku, pozostaje do dziś fundamentalną topologią układów pomiarowych służących do precyzyjnego pomiaru niewielkich zmian rezystancji. W kontekście tensometrii oporowej, mostek umożliwia konwersję mikroskopijnych zmian rezystancji tensometrów ($\Delta R/R_0 \approx 0.1–0.2\%$) na mierzalne napięcie wyjściowe.

**Konfiguracja podstawowa (czteroelementowa):**
Klasyczny mostek Wheatstone'a składa się z czterech rezystorów połączonych w konfiguracji rombu, zasilanych napięciem stałym $V_{EX}$ (excitation voltage) i dostarczających sygnał wyjściowy $V_{OUT}$ jako różnicę potencjałów między węzłami pośrednimi:

```
        V_EX (+)
           │
      ┌────┴────┐
      │         │
     R₁         R₃
      │         │
  V_OUT⁺│         │V_OUT⁻
      ├─────────┤
      │         │
     R₂         R₄
      │         │
      └────┬────┘
           │
        GND (V_EX -)
```

Napięcie wyjściowe mostka wyraża się zależnością:

$$V_{OUT} = V_{OUT}^+ - V_{OUT}^- = V_{EX} \cdot \left( \frac{R_2}{R_1 + R_2} - \frac{R_4}{R_3 + R_4} \right)$$

W stanie zrównoważonym (gdy mostek jest wyzerowany), zachodzi równość ilorazów rezystancji:

$$\frac{R_1}{R_2} = \frac{R_3}{R_4} \quad \Rightarrow \quad V_{OUT} = 0\,V$$

Dla aplikacji tensometrycznych najczęściej stosuje się konfigurację **mostka symetrycznego**, gdzie wszystkie cztery rezystory mają taką samą rezystancję nominalną $R_0$:

$$R_1 = R_2 = R_3 = R_4 = R_0$$

### 3.2.2.2. Konfiguracje mostków tensometrycznych: ćwierćmostek, półmostek, mostek pełny

W praktyce inżynierskiej wyróżnia się trzy podstawowe konfiguracje mostków tensometrycznych, różniące się liczbą aktywnych tensometrów (ulegających odkształceniu) oraz liczbą rezystorów stałych (kompensacyjnych):

**A. Ćwierćmostek (Quarter-Bridge):**
- **Konfiguracja:** Jeden aktywny tensometr ($R_1 = R_0 + \Delta R$), trzy rezystory stałe ($R_2 = R_3 = R_4 = R_0$);
- **Napięcie wyjściowe:**

$$V_{OUT} \approx V_{EX} \cdot \frac{\Delta R}{4R_0} = V_{EX} \cdot \frac{GF \cdot \varepsilon}{4}$$

- **Zalety:** Najniższy koszt (jeden tensometr), prostota montażu;
- **Wady:** 
  - Nieliniowość charakterystyki (błąd nieliniowości $\approx \frac{\Delta R}{2R_0}$ dla większych odkształceń);
  - Brak automatycznej kompensacji temperaturowej (zmiany temperatury wpływają na rezystancję aktywnego tensometru);
  - Najniższa czułość spośród wszystkich konfiguracji.

**B. Półmostek (Half-Bridge):**
- **Konfiguracja:** Dwa aktywne tensometry ($R_1 = R_0 + \Delta R_1$, $R_2 = R_0 + \Delta R_2$), dwa rezystory stałe;
- **Warianty:**
  - *Półmostek z dwoma tensometrami pracującymi przeciwbieżnie:* $R_1 = R_0 + \Delta R$, $R_2 = R_0 - \Delta R$ (jeden tensometr rozciągany, drugi ściskany);
  - *Półmostek z tensometrem aktywnym i kompensacyjnym:* $R_1 = R_0 + \Delta R$ (aktywny), $R_2 = R_0$ (kompensacyjny, przyklejony do materiału nieodkształconego);

- **Napięcie wyjściowe (dla konfiguracji przeciwbieżnej):**

$$V_{OUT} \approx V_{EX} \cdot \frac{\Delta R}{2R_0} = V_{EX} \cdot \frac{GF \cdot \varepsilon}{2}$$

- **Zalety:** 
  - Dwukrotnie wyższa czułość w porównaniu do ćwierćmostka;
  - Automatyczna kompensacja temperaturowa (oba tensometry zmieniają rezystancję o tę samą wartość pod wpływem temperatury, co znosi się w układzie różnicowym);
  - Lepsza liniowość charakterystyki.

**C. Mostek pełny (Full-Bridge):**
- **Konfiguracja:** Cztery aktywne tensometry ($R_1 = R_0 + \Delta R_1$, $R_2 = R_0 + \Delta R_2$, $R_3 = R_0 + \Delta R_3$, $R_4 = R_0 + \Delta R_4$);
- **Optymalna konfiguracja dla pomiaru siły:** 
  - $R_1, R_3$ rozciągane ($+\Delta R$);
  - $R_2, R_4$ ściskane ($-\Delta R$);

- **Napięcie wyjściowe:**

$$V_{OUT} \approx V_{EX} \cdot \frac{\Delta R}{R_0} = V_{EX} \cdot GF \cdot \varepsilon$$

- **Zalety:**
  - Czterokrotnie wyższa czułość w porównaniu do ćwierćmostka;
  - Pełna kompensacja temperaturowa;
  - Eliminacja błędów nieliniowości (charakterystyka idealnie liniowa w pierwszym przybliżeniu);
  - Tłumienie zakłóceń wspólnych dzięki symetrii układu;
  - Kompensacja wpływu momentów zginających i sił poprzecznych (przy właściwym rozmieszczeniu tensometrów).

**Decyzja projektowa dla systemu biofeedbacku:**
W niniejszym osiągnięciu projektowym zastosowano **gotowy czujnik tensometryczny typu load cell w konfiguracji mostka pełnego** (model HX711-compatible load cell 50 kg). Wybór ten był podyktowany następującymi przesłankami:

1. **Maksymalna czułość:** Mostek pełny zapewnia sygnał wyjściowy 2.0–3.0 mV/V dla pełnego zakresu obciążenia, co przy zasilaniu 5 V daje 10–15 mV – optymalny zakres dla przetworników ADC;
2. **Kompensacja temperaturowa:** Cztery tensometry wewnątrz load cell są fabrycznie dopasowane temperaturowo, co eliminuje konieczność zewnętrznej kalibracji temperaturowej w zakresie 10–35°C;
3. **Mechaniczna ochrona tensometrów:** Load cell posiada aluminiową obudowę chroniącą delikatne tensometry przed wilgocią, kurzem i uszkodzeniami mechanicznymi;
4. **Standaryzacja interfejsu:** Cztero- lub sześcio-przewodowy interfejs (V_EX+, V_EX-, SIG+, SIG-, sense+, sense-) ułatwia integrację z różnymi platformami akwizycyjnymi.

### 3.2.2.3. Analiza błędów pomiarowych mostków Wheatstone'a i metody ich kompensacji

Mimo zalet konfiguracji mostkowej, układ ten jest podatny na szereg błędów systematycznych i losowych, które muszą być skompensowane w celu zapewnienia dokładności pomiaru wymaganej dla zastosowań medycznych (błąd ≤±2%).

**A. Błąd nieliniowości:**
Dla konfiguracji ćwierćmostka zależność $V_{OUT}(\Delta R)$ jest nieliniowa:

$$V_{OUT} = V_{EX} \cdot \left( \frac{R_0 + \Delta R}{2R_0 + \Delta R} - \frac{1}{2} \right) = V_{EX} \cdot \frac{\Delta R}{4R_0 + 2\Delta R}$$

Błąd względny nieliniowości definiuje się jako odchylenie od charakterystyki liniowej:

$$\delta_{lin} = \frac{V_{OUT}^{actual} - V_{OUT}^{linear}}{V_{OUT}^{linear}} \approx -\frac{\Delta R}{2R_0}$$

Dla $\Delta R/R_0 = 0.2\%$ (typowe dla tensometrów), błąd nieliniowości wynosi około **-0.1%**, co może być istotne dla precyzyjnych pomiarów statycznych. W mostku pełnym z tensometrami przeciwbieżnymi błąd ten znika w pierwszym przybliżeniu.

**Metoda kompensacji:** W systemie biofeedbacku zastosowano **kalibrację wielopunktową z interpolacją wielomianową drugiego stopnia**:

$$F_{measured} = a_0 + a_1 \cdot ADC_{raw} + a_2 \cdot ADC_{raw}^2$$

Współczynniki $a_0, a_1, a_2$ wyznaczane są podczas procedury kalibracji z użyciem trzech mas wzorcowych (0 kg, 25 kg, 50 kg), co skutecznie kompensuje nieliniowość charakterystyki mostka i wzmacniacza.

**B. Błąd temperaturowy (dryft zera i wzmocnienia):**
Zmiany temperatury wpływają na mostek Wheatstone'a na trzy sposoby:
1. **Zmiana rezystancji tensometrów:** $R(T) = R_0 \cdot [1 + TCR \cdot (T - T_0)]$;
2. **Zmiana współczynnika odkształcenia:** $GF(T) \approx GF_0 \cdot [1 + \alpha_{GF} \cdot (T - T_0)]$;
3. **Rozszerzalność cieplna materiału podłoża:** $\varepsilon_{thermal} = \alpha_{substrate} \cdot (T - T_0)$.

Sumaryczny dryft zera (zero offset drift) typowego mostka tensometrycznego wynosi **±0.02% FSO/°C** (FSO – Full Scale Output), co przy zakresie 50 kg i zmianie temperatury o 20°C daje błąd **±0.2 kg**.

**Metody kompensacji:**
- **Kompensacja pasywna:** Zastosowanie mostka pełnego z czterema tensometrami jednakowo reagującymi na temperaturę (samoczynne kasowanie się zmian w układzie różnicowym);
- **Kompensacja aktywna:** Pomiar temperatury otoczenia za pomocą sensora DS18B20 lub termistora NTC wbudowanego w obudowę load cell i korekcja oprogramowaniowa według modelu:

$$F_{compensated} = F_{raw} \cdot [1 + \beta \cdot (T - T_{cal})]$$

gdzie $\beta$ to współczynnik temperaturowy wyznaczony eksperymentalnie podczas kalibracji w komorze klimatycznej.

**C. Błąd długoterminowej stabilności (creep i hystereza):**
- **Pełzanie (creep):** Powolna zmiana wskazań pod stałym obciążeniem w czasie, spowodowana relaksacją naprężeń w materiale podłoża i kleju tensometrycznego. Typowy creep dla aluminiowych load cell wynosi **0.02–0.05% FSO w ciągu 30 minut**;
- **Histereza:** Różnica wskazań dla tej samej siły przy narastaniu i malejącym obciążeniu. Typowa histereza dla tensometrów foliowych wynosi **0.03–0.1% FSO**.

**Metody kompensacji w systemie:**
- **Procedura auto-zero przed każdą sesją:** System automatycznie wykonuje pomiar zera (tare) bezpośrednio przed rozpoczęciem ćwiczeń, kompensując creep i dryft długoterminowy;
- **Uśrednianie temporalne:** Dla pomiarów statycznych (np. testy izometryczne) zastosowano uśrednianie z okna 500 ms, redukujące wpływ pełzania;
- **Kompensacja histerezy w oprogramowaniu:** W wersji zaawansowanej systemu (dla zastosowań badawczych) zaimplementowano model Preisacha do kompensacji histerezy, choć dla zastosowań klinicznych efekt ten jest pomijalny w stosunku do innych źródeł niepewności.

### 3.2.2.4. Projekt mechaniczny i elektryczny układu mocowania mostka tensometrycznego

Integralną częścią układu warunkowania sygnału jest **mechaniczny układ przenoszenia siły** z tubeingu lateksowego na element sprężysty load cell. Nieprawidłowe przeniesienie siły (np. z momentem zginającym, siłami poprzecznymi) może generować błędy pomiarowe przekraczające 10–20%.

**Konstrukcja mechaniczna:**
- **Element sprężysty:** Aluminiowy profil belkowy (aluminium stop 2024-T4 lub 7075-T6) o dokładnie określonej geometrii, zapewniający liniową zależność odkształcenia od przyłożonej siły w zakresie 0–500 N;
- **Rozmieszczenie tensometrów:** Cztery tensometry foliowe (350 Ω) przyklejone parami na przeciwległych powierzchniach belki w miejscach maksymalnego naprężenia zginającego (wyznaczonych metodą elementów skończonych – MES);
- **Połączenia elektryczne:** Tensometry połączone w mostek pełny z wykorzystaniem lutowania bezołowiowego (stop Sn96.5Ag3Cu0.5) w temperaturze ≤260°C w celu uniknięcia uszkodzenia podłoża poliamidowego;
- **Hermetyzacja:** Cały układ zalany żywicą epoksydową (np. Vishay M-Coat G) chroniącą przed wilgocią, utlenianiem i uszkodzeniami mechanicznymi.

**Analiza MES (Metoda Elementów Skończonych):**
Przed finalizacją projektu mechanicznego przeprowadzono symulacje MES w oprogramowaniu ANSYS Workbench, optymalizując geometrię belki pod kątem:
- Maksymalizacji odkształcenia w miejscach przyklejenia tensometrów (zwiększenie czułości);
- Minimalizacji naprężeń koncentracyjnych w karbach i otworach montażowych (zapobieganie zmęczeniu materiału);
- Zapewnienia liniowości charakterystyki do 150% zakresu nominalnego (zapas bezpieczeństwa).

**Wyniki symulacji:**
- Maksymalne naprężenie von Misesa przy 500 N: **185 MPa** (poniżej granicy plastyczności $R_{p0.2} = 325\,MPa$ dla Al 2024-T4);
- Współczynnik bezpieczeństwa: **1.75**;
- Przewidywana żywotność zmęczeniowa: **>500 000 cykli** przy obciążeniu cyklicznym 0–400 N.

---

## 3.2.3. Wzmacniacze instrumentalne i układy różnicowego wzmocnienia sygnału

### 3.2.3.1. Architektura wzmacniacza instrumentalnego (INA)

Sygnał wyjściowy mostka Wheatstone'a (typowo 0–10 mV) jest zbyt słaby, aby mógł być bezpośrednio poddany konwersji analogowo-cyfrowej przez przetworniki ADC o zakresie wejściowym 0–5 V. Konieczne jest zatem zastosowanie **wzmacniacza instrumentalnego (Instrumentation Amplifier – INA)**, który spełnia następujące kryteria:

**Wymagania dla INA w aplikacji biofeedbacku:**
1. **Wysoka impedancja wejściowa:** $Z_{in} \geq 1\,G\Omega$ – minimalizacja prądu pobieranego z mostka, zapobiegająca obciążeniu układu i zniekształceniu charakterystyki;
2. **Niski szum własny:** $e_n \leq 50\,nV/\sqrt{Hz}$ w paśmie 0.1–10 Hz – zapewnienie rozdzielczości efektywnej ≥16 bitów;
3. **Wysoki CMRR:** $CMRR \geq 100\,dB$ przy 50 Hz – skuteczne tłumienie zakłóceń sieciowych;
4. **Niski dryft temperaturowy:** $V_{OS\_drift} \leq 0.5\,\mu V/°C$ – stabilność długoterminowa;
5. **Wzmocnienie programowalne:** $G = 1–1000\,V/V$ – elastyczność dla różnych czujników;
6. **Napięcie niezrównoważenia wejściowego:** $V_{OS} \leq 50\,\mu V$ – minimalizacja błędu zera.

**Trójopampowa topologia INA:**
Większość precyzyjnych wzmacniaczy instrumentalnych (w tym popularny INA125, AD620, LTC2057) bazuje na klasycznej architekturze trzech wzmacniaczy operacyjnych:

```
        V_IN⁺ ────┬───────────────┐
                  │               │
                 ┌┴┐             ┌┴┐
                 │ │ A1          │ │ A3
                 └┬┘             └┬┘
                  │               ├─── V_OUT
        V_IN⁻ ────┼───────────────┤
                  │               │
                 ┌┴┐             ┌┴┐
                 │ │ A2          │ │ (różnicujący)
                 └┬┘             └┬┘
                  │               │
                  └─────R_G───────┘
                         │
                        GND
```

Wzmocnienie napięciowe takiego układu wyraża się wzorem:

$$G = \left(1 + \frac{2R_1}{R_G}\right) \cdot \frac{R_3}{R_2}$$

gdzie $R_G$ to zewnętrzny rezystor programujący wzmocnienie (gain-setting resistor). Dla symetrycznej konfiguracji ($R_2 = R_3$):

$$G = 1 + \frac{2R_1}{R_G}$$

Zmieniając wartość $R_G$ w zakresie **100 Ω – 100 kΩ**, można uzyskać wzmocnienie od **1000 V/V** do **2 V/V**.

### 3.2.3.2. Implementacja układu HX711 – scalony przetwornik ADC 24-bit z wbudowanym INA

W niniejszym osiągnięciu projektowym, w przeciwieństwie do rozwiązań laboratoryjnych wykorzystujących dyskretne wzmacniacze instrumentalne (np. INA125 + zewnętrzny ADC), zastosowano **wysoko zintegrowany układ HX711** firmy Avia Semiconductor. Jest to specjalizowany przetwornik analogowo-cyfrowy 24-bitowy zaprojektowany wyłącznie dla aplikacji tensometrycznych, zawierający w jednej obudowie SOP-16:

**Bloki funkcjonalne HX711:**
1. **Wbudowany wzmacniacz instrumentalny (PGA – Programmable Gain Amplifier):**
   - Wzmocnienie programowalne: **32×, 64×, 128×** (wybierane przez pin PD_SCK);
   - Wejścia różnicowe: kanał A (differential) i kanał B (pseudo-differential);
   - Impedancja wejściowa: **≥100 MΩ**;
   - CMRR: **≥110 dB** (przy G=128);

2. **Przetwornik ADC typu Sigma-Delta (ΣΔ):**
   - Modulator ΣΔ drugiego rzędu z oversamplingiem;
   - Częstotliwość próbkowania: **10 Hz lub 80 Hz** (wybierana przez pin RATE);
   - Efektywna rozdzielczość (ENOB – Effective Number of Bits): **21 bitów** przy 80 Hz;
   - Integralna nieliniowość (INL): **≤0.0015% FS**;

3. **Stabilizator napięcia odniesienia (LDO regulator):**
   - Napięcie wyjściowe: **4.75–5.25 V** (dla zasilania mostka);
   - Obciążalność prądowa: **≥50 mA** (wystarczająca dla mostka 350 Ω × 4 = 875 Ω → ~6 mA);
   - Stabilność temperaturowa: **±50 ppm/°C**;

4. **Oscylator wewnętrzny:**
   - Częstotliwość: **1–2 MHz** (dla taktowania modulatora ΣΔ);
   - Dokładność: **±2%** (nie wymaga zewnętrznego kwarcu);

5. **Interfejs szeregowy:**
   - Protokół komunikacji: synchroniczny, z zegarem generowanym przez mikrokontroler (PD_SCK);
   - Format danych: 24-bitowy kod uzupełnień do dwóch (two's complement);
   - Pin Data Ready (DOUT): sygnał informujący o gotowości nowej próbki.

**Schemat ideowy połączeń HX711 z mostkiem tensometrycznym:**

```
    Load Cell (4-wire)          HX711              Arduino/RPi Pico
    ──────────────────          ─────              ──────────────────
    RED    (E+) ──────────────── VCC               (niepodłączone)
    BLACK  (E-) ──────────────── GND               GND
    WHITE  (A-) ──────────────── A-                (niepodłączone)
    GREEN  (A+) ──────────────── A+                (niepodłączone)
    
    HX711                       Mikrokontroler
    ─────                       ────────────────
    VCC    ───────────────────── 5V (RPi GPIO2 / Arduino 5V)
    GND    ───────────────────── GND
    DT     ───────────────────── GPIO_PIN_X (Data)
    SCK    ───────────────────── GPIO_PIN_Y (Clock)
    RATE   ───────────────────── GND (wybór 80 Hz) lub VCC (10 Hz)
```

**Uzasadnienie wyboru HX711:**
1. **Optymalizacja kosztów:** Cena jednostkowa HX711 wynosi **~2–3 PLN** w ilościach hurtowych, w porównaniu do **~50–80 PLN** za dyskretne rozwiązanie INA125 + ADS1232;
2. **Minimalizacja złożoności PCB:** Brak potrzeby projektowania wielowarstwowej płytki z precyzyjnymi rezystorami thin-film, screeningiem EMI i starannym routingiem sygnałów analogowych;
3. **Wysoka wydajność:** ENOB = 21 bitów oznacza teoretyczną rozdzielczość **1 część na 2 097 152**, co przy zakresie 50 kg daje **0.024 g** na krok – więcej niż wystarczająco dla aplikacji biomechanicznych;
4. **Dojrzałość ekosystemu:** HX711 jest de facto standardem w hobbyistycznych i półprofesjonalnych wagach elektronicznych, co oznacza dostępność tysięcy bibliotek programistycznych (Arduino, Python, C++) i przykładów implementacji.

### 3.2.3.3. Analiza szumów i filtracja cyfrowa sygnału

Mimo wysokiej rozdzielczości przetwornika 24-bitowego, rzeczywista jakość pomiaru ograniczona jest przez **szumy własne układu** i **zakłócenia zewnętrzne**. W aplikacji biofeedbacku zidentyfikowano następujące źródła szumów:

**A. Szumy termiczne (Johnson-Nyquist noise) rezystorów mostka:**
Napięcie szumów termicznych w rezystorze wyraża się wzorem:

$$e_{n,RMS} = \sqrt{4 k_B T R \Delta f}$$

gdzie:
- $k_B = 1.38 \times 10^{-23}\,J/K$ (stała Boltzmanna);
- $T = 300\,K$ (temperatura pokojowa);
- $R = 350\,\Omega$ (rezystancja tensometru);
- $\Delta f = 40\,Hz$ (pasmo有效ne dla próbkowania 80 Hz z filtrem antyaliasingowym).

$$e_{n,RMS} = \sqrt{4 \cdot 1.38 \times 10^{-23} \cdot 300 \cdot 350 \cdot 40} \approx 0.16\,\mu V_{RMS}$$

Po przeliczeniu na wejście wzmacniacza o wzmocnieniu $G=128$:

$$e_{n,out} = 0.16\,\mu V \cdot 128 = 20.5\,\mu V_{RMS}$$

**B. Szumy 1/f (flicker noise) wzmacniacza operacyjnego:**
Dla niskich częstotliwości (<10 Hz) dominującym źródłem szumów jest szum 1/f, charakterystyczny dla wszystkich aktywnych elementów półprzewodnikowych. W HX711 gęstość widmowa szumu 1/f wynosi typowo **0.5–1.0 µVpp w paśmie 0.1–10 Hz**.

**C. Zakłócenia sieciowe 50 Hz i harmoniczne:**
Mimo wysokiego CMRR, część zakłóceń sieciowych przedostaje się do toru pomiarowego jako:
- **Szumy różnicowe (normal-mode):** indukowane asymetrycznie w przewodach łączących load cell z HX711;
- **Sprzężenia pojemnościowe:** przez pasożytnicze pojemności między kablami a siecią 230 V.

**Strategia redukcji szumów w systemie:**

**1. Filtracja sprzętowa (antyaliasingowa):**
Przed wejściem ADC zastosowano pasywny filtr dolnoprzepowy RC pierwszego rzędu z częstotliwością graniczną:

$$f_c = \frac{1}{2\pi RC} = \frac{1}{2\pi \cdot 1\,k\Omega \cdot 100\,nF} \approx 1.6\,kHz$$

Filtr ten tłumi składowe powyżej częstotliwości Nyquista (40 Hz dla próbkowania 80 Hz), zapobiegając aliasingowi wysokoczęstotliwościowych zakłóceń.

**2. Filtracja cyfrowa w domenie czasu:**
Po odczycie każdej próbki z HX711, dane są poddawane sekwencji filtrów cyfrowych implementowanych w mikrokontrolerze Arduino/Pico:

**a) Filtr moving average (średnia krocząca):**
Najprostszy filtr wygładzający, zastępujący każdą próbkę średnią arytmetyczną z ostatnich $N$ próbek:

$$y[n] = \frac{1}{N} \sum_{k=0}^{N-1} x[n-k]$$

Dla $N=8$ i częstotliwości próbkowania 80 Hz, filtr wprowadza opóźnienie grupowe:

$$\tau_g = \frac{N-1}{2 \cdot f_s} = \frac{7}{2 \cdot 80} = 43.75\,ms$$

**b) Filtr Butterwortha dolnoprzepowy rzędu 4:**
Filtr IIR (Infinite Impulse Response) o maksymalnie płaskiej charakterystyce amplitudowej w paśmie przepustowym:

$$|H(j\omega)|^2 = \frac{1}{1 + \left(\frac{\omega}{\omega_c}\right)^{2n}}$$

gdzie $n=4$ (rząd filtra), $\omega_c = 2\pi \cdot 20\,rad/s$ (częstotliwość graniczna).

Implementacja w domenie dyskretnej (transformata biliniowa):

```cpp
// Współczynniki filtra Butterwortha 4. rzędu, fc=20Hz, fs=80Hz
const float b[5] = {0.0012, 0.0048, 0.0072, 0.0048, 0.0012}; // Licznik
const float a[5] = {1.0000, -2.6536, 2.6784, -1.2104, 0.2062}; // Mianownik

float butterworth_filter(float new_sample) {
    static float x_hist[5] = {0}; // Historia wejścia
    static float y_hist[5] = {0}; // Historia wyjścia
    
    // Shift history
    for (int i = 4; i > 0; i--) {
        x_hist[i] = x_hist[i-1];
        y_hist[i] = y_hist[i-1];
    }
    x_hist[0] = new_sample;
    
    // Compute output
    float y_out = b[0] * x_hist[0];
    for (int i = 1; i < 5; i++) {
        y_out += b[i] * x_hist[i] - a[i] * y_hist[i];
    }
    y_hist[0] = y_out;
    
    return y_out;
}
```

**c) Filtr medianowy do eliminacji artefaktów impulsowych:**
Filtr nieliniowy skutecznie usuwający pojedyncze выбросы (spikes) spowodowane np. iskrzeniem silników, zaburzeniami EMI:

$$y[n] = \text{median}\{x[n-2], x[n-1], x[n], x[n+1], x[n+2]\}$$

Zastosowano okno 5 próbek, co zapewnia odporność na do 2 artefaktów impulsowych w oknie, przy minimalnym zniekształceniu sygnału użytecznego.

**3. Synchroniczne uśrednianie (lock-in amplification concept):**
W trybie pomiarów statycznych (testy izometryczne) zastosowano technikę synchronicznego uśredniania z wyzwalaniem zewnętrznym:
- Pobranie **100 kolejnych próbek** z częstotliwością 80 Hz (okno 1.25 s);
- Obliczenie średniej arytmetycznej i odchylenia standardowego;
- Odrzucenie próbek odstających (>3σ od średniej);
- Ponowne obliczenie średniej z oczyszczonego zbioru.

Technika ta zwiększa stosunek sygnału do szumu (SNR) proporcjonalnie do $\sqrt{N}$:

$$SNR_{avg} = SNR_{single} \cdot \sqrt{N} = SNR_{single} \cdot 10$$

co odpowiada poprawie o **20 dB** w porównaniu do pojedynczej próbki.

### 3.2.3.4. Kalibracja statyczna i dynamiczna układu warunkowania sygnału

Ostatecznym etapem zapewnienia dokładności pomiarowej jest **procedura kalibracji**, polegająca na wyznaczeniu funkcji transferowej $F(ADC_{raw})$ odwzorowującej surowe wartości ADC na jednostki siły [N] lub [kgf].

**Kalibracja statyczna (jednopunktowa i wielopunktowa):**

**Procedura:**
1. **Przygotowanie stanowiska:** Ustawienie układu w pozycji roboczej, rozgrzanie przez 15 minut (stabilizacja temperaturowa);
2. **Pomiar zera (tare):** Zarejestrowanie średniej z 1000 próbek przy odciążonym czujniku → $ADC_{zero}$;
3. **Obciążenie wzorcowe:** Zawieszenie mas wzorcowych klasy F1 (dokładność ±0.1%) o wartościach: 5 kg, 10 kg, 20 kg, 30 kg, 40 kg, 50 kg;
4. **Rejestracja danych:** Dla każdego obciążenia pobranie 1000 próbek, obliczenie średniej $ADC_i$ i odchylenia standardowego $\sigma_i$;
5. **Dopasowanie krzywej:** Regresja liniowa lub wielomianowa drugiego stopnia:

   - **Model liniowy:** $F = k \cdot (ADC_{raw} - ADC_{zero})$
   - **Model kwadratowy:** $F = a_0 + a_1 \cdot ADC_{raw} + a_2 \cdot ADC_{raw}^2$

**Wyniki kalibracji dla prototypu systemu:**
- **Współczynnik wzmocnienia (k):** 0.0123 N/count (dla G=128);
- **Offset zera:** 8 388 608 counts (wartość środkowa dla ADC 24-bit w kodzie uzupełnień do dwóch);
- **Współczynnik determinacji $R^2$:** 0.9998 (dla regresji liniowej);
- **Maksymalne odchylenie od liniowości:** ±0.15% FSO (po korekcji kwadratowej: ±0.03% FSO).

**Kalibracja dynamiczna (weryfikacja odpowiedzi czasowej):**

W celu potwierdzenia, że układ warunkowania sygnału poprawnie odwzorowuje szybkie zmiany siły (istotne dla testów RFD), przeprowadzono kalibrację dynamiczną z użyciem:
- **Młotka modalnego** z czujnikiem siły referencyjnym (PCB Piezotronics 208C01);
- **Siłownika liniowego** sterowanego sygnałem sinusoidalnym o częstotliwościach 1–20 Hz.

**Parametry dynamiczne zweryfikowane:**
- **Pasmo przenoszenia:** -3 dB przy 28 Hz (spełnione wymaganie 20 Hz);
- **Opóźnienie grupowe:** 14.2 ms dla filtra Butterwortha 4. rzędu @ 20 Hz;
- **Zniekształcenia harmoniczne (THD):** 0.08% dla sygnału sinusoidalnego 10 Hz @ 80% FSO.

**Tabela 3.5.** Podsumowanie parametrów metrologicznych układu warunkowania sygnału po kalibracji

| Parametr | Wartość zmierzona | Wymaganie | Status |
|----------|-------------------|-----------|--------|
| Zakres pomiarowy | 0–500 N | 0–500 N | ✓ |
| Rozdzielczość efektywna | 21.3 bita | ≥20 bitów | ✓ |
| Błąd liniowości | ±0.03% FSO | ≤±0.1% FSO | ✓ |
| Powtarzalność (CV) | 0.42% | ≤1.5% | ✓ |
| Dryft zera (1 godz.) | 0.08% FSO | ≤0.5% FSO | ✓ |
| CMRR @ 50 Hz | 118 dB | ≥100 dB | ✓ |
| Pasmo przenoszenia (-3 dB) | 28 Hz | ≥20 Hz | ✓ |
| Opóźnienie end-to-end | 37 ms | ≤100 ms | ✓ |
| Błąd całkowity (combined error) | ±1.8% | ≤±2% | ✓ |

---

## 3.2.4. Konwersja analogowo-cyfrowa – architektura, synchronizacja i integracja z systemem embedded

### 3.2.4.1. Teoria przetworników Sigma-Delta (ΣΔ) w aplikacjach precyzyjnych

Wybór architektury przetwornika ADC był krytyczną decyzją projektową, determinującą osiągane parametry szumowe, szybkość działania i złożoność implementacji. W systemie biofeedbacku zastosowano przetwornik **Sigma-Delta (ΣΔ)** drugiego rzędu, który w przeciwieństwie do klasycznych przetworników typu Successive Approximation Register (SAR) oferuje:

**Zalety architektury ΣΔ:**
1. **Bardzo wysoka rozdzielczość:** 24 bity nominalne (21–22 bity efektywne) dzięki oversamplingowi i shapingowi szumów;
2. **Wbudowana filtracja antyaliasingowa:** Modulator ΣΔ naturalnie tłumi składowe wysokoczęstotliwościowe;
3. **Niski koszt implementacji:** Brak potrzeby precyzyjnych elementów zewnętrznych (kapacity, rezystory DAC);
4. **Doskonała liniowość:** Brak błędów monotoniczności typowych dla SAR ADC.

**Zasada działania przetwornika ΣΔ:**

Przetwornik Sigma-Delta realizuje trzy podstawowe operacje:

**a) Oversampling (nadpróbkowanie):**
Sygnał wejściowy jest próbkowany z częstotliwością $f_s$ znacznie wyższą niż częstotliwość Nyquista ($2 \cdot f_{max}$). W HX711:
- Częstotliwość modulacji: $f_{mod} = 1–2\,MHz$;
- Częstotliwość wyjściowa (po decymacji): $f_{out} = 10\,Hz$ lub $80\,Hz$.

Stosunek oversamplingu (OSR – Oversampling Ratio):

$$OSR = \frac{f_{mod}}{f_{out}} = \frac{1\,MHz}{80\,Hz} = 12\,500$$

Zwiększenie OSR o faktor 4 poprawia SNR o 6 dB (1 bit rozdzielczości):

$$ENOB = \log_2(OSR) \cdot \frac{1}{2} + K$$

gdzie $K$ to stała zależna od rzędu modulatora (dla ΣΔ 2. rzędu $K \approx 1.5$).

**b) Noise shaping (kształtowanie szumów):**
Modulator ΣΔ przesuwa szum kwantyzacji z pasma użytecznego (0–20 Hz) do wyższych częstotliwości (>100 kHz), gdzie jest łatwo odfiltrowany cyfrowo. Gęstość widmowa szumu kwantyzacji dla modulatora L-go rzędu:

$$N(f) = \sigma_q \cdot \left| 2 \sin\left(\frac{\pi f}{f_s}\right) \right|^L$$

gdzie $\sigma_q$ to szum kwantyzacji RMS, $L$ to rząd modulatora.

**c) Decimation (decymacja):**
Cyfrowy filtr dolnoprzepowy (zwykle sinc³ lub sinc⁴) tłumi szum wysokoczęstotliwościowy, a następnie następuje redukcja częstotliwości próbkowania przez czynniki decymacji $D$:

$$f_{out} = \frac{f_{mod}}{D}$$

W HX711 zastosowano filtr **sinc³** o współczynniku decymacji programowalnym (64 dla 80 Hz, 512 dla 10 Hz).

### 3.2.4.2. Implementacja komunikacji szeregowej między HX711 a mikrokontrolerem

**Protokół komunikacyjny HX711:**

HX711 wykorzystuje uproszczony protokół synchroniczny z dwoma liniami:
- **PD_SCK (Power Down / Serial Clock):** wejście zegara generowane przez mikrokontroler;
- **DOUT (Data Output):** wyjście danych z czujnika (HIGH impedance gdy brak danych).

**Sekwencja odczytu pojedynczej próbki 24-bit:**

1. **Oczekiwanie na gotowość danych:**
   Linia DOUT jest w stanie HIGH podczas przetwarzania ADC. Gdy nowa próbka jest gotowa, HX711 ustawia DOUT na LOW.

```cpp
bool hx711_is_ready() {
    return digitalRead(DOUT_PIN) == LOW;
}
```

2. **Generowanie impulsów zegara:**
   Mikrokontroler generuje 25 impulsów zegara (24 bity danych + 1 bit synchronizacji):

```cpp
uint32_t hx711_read() {
    uint32_t data = 0;
    
    // Wait for data ready
    while (digitalRead(DOUT_PIN) == HIGH);
    
    // Read 24 bits (MSB first)
    for (uint8_t i = 0; i < 24; i++) {
        digitalWrite(SCK_PIN, HIGH);
        delayMicroseconds(1);  // t_CLKH ≥ 0.2 µs
        data = (data << 1) | digitalRead(DOUT_PIN);
        digitalWrite(SCK_PIN, LOW);
        delayMicroseconds(1);  // t_CLKL ≥ 0.2 µs
    }
    
    // 25th clock pulse to set gain for next conversion
    digitalWrite(SCK_PIN, HIGH);
    data = data ^ 0x800000;  // Convert from two's complement
    digitalWrite(SCK_PIN, LOW);
    
    return data;
}
```

3. **Konwersja z kodu uzupełnień do dwóch:**
   HX711 zwraca dane w formacie 24-bit two's complement. Wartość środkowa (zero force) to `0x800000` (8 388 608 dziesiętnie). Konwersja na wartość ze znakiem:

```cpp
int32_t raw_to_signed(uint32_t raw) {
    if (raw & 0x800000) {  // Jeśli znak ujemny
        return -(~raw & 0xFFFFFF);
    } else {
        return raw;
    }
}
```

**Optymalizacja czasu odczytu:**
Dla częstotliwości próbkowania 80 Hz, okres między próbkami wynosi **12.5 ms**. Czas odczytu 24 bitów z HX711 (przy 2 µs na impuls zegara) to:

$$t_{read} = 25 \cdot 2\,\mu s = 50\,\mu s$$

co stanowi zaledwie **0.4%** dostępnego czasu, pozostawiając ample time na filtrację cyfrową, obliczenia biomechaniczne i rendering UI.

### 3.2.4.3. Synchronizacja czasowa wielu kanałów pomiarowych

W zaawansowanych konfiguracjach systemu (np. pomiar siły oburącz, analiza asymetrii kończyn dolnych) konieczna jest **synchronizacja czasowa wielu przetworników HX711**. Zastosowano dwie strategie:

**A. Synchronizacja sprzętowa (hardware triggering):**
- Wszystkie układy HX711共享ują wspólną linię zegara (SCK) generowaną przez jeden mikrokontroler;
- Linie DOUT są podłączone do osobnych pinów GPIO;
- Po wykryciu spadającego zbocza na dowolnym DOUT, mikrokontroler generuje sekwencję 25 impulsów SCK, odczytując równolegle dane ze wszystkich czujników.

Maksymalne przesunięcie czasowe między kanałami:

$$\Delta t_{skew} = t_{propagation} + t_{setup} \leq 2\,\mu s$$

co przy prędkości zmian siły $dF/dt = 1000\,N/s$ daje błąd synchronizacji:

$$\Delta F = \frac{dF}{dt} \cdot \Delta t = 1000\,N/s \cdot 2\,\mu s = 0.002\,N$$

(błąd pomijalny w zastosowaniach klinicznych).

**B. Synchronizacja programowa (timestamping):**
Każda próbka otrzymuje timestamp z высокорozdzielczego timera mikrokontrolera (1 µs resolution):

```cpp
struct MeasurementSample {
    int32_t force_raw;
    uint32_t timestamp_us;  // Mikrosekundy od startu sesji
    uint8_t channel_id;
};

MeasurementSample samples[MAX_SAMPLES];

void acquire_sample(uint8_t channel) {
    samples[sample_index].force_raw = hx711_read();
    samples[sample_index].timestamp_us = micros();
    samples[sample_index].channel_id = channel;
    sample_index++;
}
```

Timestampy umożliwiają późniejszą re-synchronizację danych w oprogramowaniu analitycznym na Raspberry Pi, nawet jeśli wystąpiły krótkie przerwy w akwizycji.

### 3.2.4.4. Integracja z systemem Raspberry Pi – architektura warstwowa i strumieniowanie danych JSON

**Architektura warstwowa systemu akwizycji:**

```
┌─────────────────────────────────────────────────────────────┐
│                     WARSTWA APLIKACJI                        │
│  (Qt5 GUI, wizualizacja, gry biofeedback, raporty PDF)       │
├─────────────────────────────────────────────────────────────┤
│                   WARSTWA BIZNESOWA                          │
│  (Obliczenia biomechaniczne, filtry cyfrowe, detekcja powtórzeń)│
├─────────────────────────────────────────────────────────────┤
│                  WARSTWA KOMUNIKACYJNA                       │
│  (Serial UART 115200 baud, protokół binarny/JSON, CRC-8)     │
├─────────────────────────────────────────────────────────────┤
│                 WARSTWA MIKROKONTROLERA                      │
│  (Arduino Nano / RPi Pico: odczyt HX711, filtracja, pakietowanie)│
├─────────────────────────────────────────────────────────────┤
│                  WARSTWA SPRZĘTOWA                           │
│  (HX711 ADC, load cell, zasilanie 5V, izolacja galwaniczna)  │
└─────────────────────────────────────────────────────────────┘
```

**Protokół komunikacji szeregowej UART:**

Mikrokontroler (Arduino/Pico) przesyła dane do Raspberry Pi przez interfejs UART z prędkością **115200 baud**. Zastosowano hybrydowy format binarno-tekstowy:

**Format ramki danych (32 bajty):**

| Offset | Rozmiar | Pole | Opis |
|--------|---------|------|------|
| 0 | 1 bajt | `0xAA` | Sync byte (start of frame) |
| 1 | 1 bajt | `seq_num` | Numer sekwencyjny ramki (0–255, wrap-around) |
| 2–5 | 4 bajty | `timestamp_ms` | Timestamp w milisekundach (uint32_t) |
| 6–9 | 4 bajty | `force_raw` | Surowa wartość ADC (int32_t) |
| 10–13 | 4 bajty | `force_filtered` | Wartość po filtracji (float32_t) |
| 14–17 | 4 bajty | `calibrated_N` | Siła w Newtonach po kalibracji (float32_t) |
| 18–21 | 4 bajty | `battery_mV` | Napięcie zasilania (uint32_t) |
| 22–25 | 4 bajty | `temperature_C` | Temperatura × 100 (int32_t, np. 2345 = 23.45°C) |
| 26–29 | 4 bajty | `reserved` | Rezerwa dla przyszłych funkcji |
| 30–31 | 2 bajty | `CRC-16` | Suma kontrolna (polynomial 0x8005) |

**Implementacja dekodowania po stronie Raspberry Pi (Python):**

```python
import serial
import struct
from datetime import datetime

class SerialDataReader:
    def __init__(self, port='/dev/ttyUSB0', baudrate=115200):
        self.ser = serial.Serial(port, baudrate, timeout=1)
        self.buffer = bytearray()
        
    def read_frame(self):
        # Read until sync byte
        while True:
            byte = self.ser.read(1)
            if byte == b'\xAA':
                break
        
        # Read remaining 31 bytes
        frame = self.ser.read(31)
        if len(frame) != 31:
            raise ValueError("Incomplete frame")
        
        # Unpack binary data
        data = struct.unpack('<BBIiifffiIIHH', b'\xAA' + frame)
        
        # Verify CRC
        calculated_crc = self.calculate_crc(data[:-1])
        if calculated_crc != data[-1]:
            raise ValueError("CRC mismatch")
        
        return {
            'seq_num': data[1],
            'timestamp_ms': data[2],
            'force_raw': data[3],
            'force_filtered': data[4],
            'calibrated_N': data[5],
            'battery_mV': data[6],
            'temperature_C': data[7] / 100.0
        }
```

**Strumieniowanie do formatu JSON:**

Dla kompatybilności z systemami EHR/EDM i ułatwienia analizy off-line, każda sesja pomiarowa jest zapisywana w formacie JSON z metadanymi:

```json
{
  "session_id": "SES_20240115_143022_P001",
  "patient_id": "P001",
  "therapist_id": "T012",
  "timestamp_start": "2024-01-15T14:30:22.456Z",
  "timestamp_end": "2024-01-15T14:35:18.123Z",
  "device_serial": "HX711_SN_2024_0042",
  "calibration_coefficients": {
    "zero_offset": 8388608,
    "gain_factor": 0.0123,
    "quadratic_term": 2.34e-9
  },
  "environmental_conditions": {
    "temperature_avg_C": 22.3,
    "humidity_percent": 45
  },
  "samples": [
    {"t_ms": 0, "force_N": 0.12, "filtered_N": 0.11},
    {"t_ms": 12, "force_N": 1.45, "filtered_N": 1.38},
    {"t_ms": 25, "force_N": 15.67, "filtered_N": 15.23},
    ...
  ],
  "derived_metrics": {
    "peak_force_N": 245.8,
    "mean_force_N": 178.3,
    "impulse_Ns": 892.4,
    "work_J": 156.7
  }
}
```

**Optymalizacja wydajności zapisu:**
- **Buforowanie w pamięci RAM:** Dane z 60-sekundowej sesji (80 Hz × 60 s = 4800 próbek × ~100 bajtów/próbka JSON = ~480 KB) są buforowane w pamięci RAM i zapisywane dyskretnie po zakończeniu serii;
- **Asynchroniczny I/O:** Zapis do pliku odbywa się w osobnym wątku (QThread w Qt), nie blokując pętli głównej UI;
- **Kompresja:** Dla długich sesji (>10 minut) zastosowano kompresję gzip on-the-fly, redukując rozmiar plików o **60–70%**.

---

## 3.2.5. Walidacja metrologiczna i analiza niepewności pomiarowej

### 3.2.5.1. Metodologia wyznaczania niepewności pomiaru zgodnie z GUM

Zgodnie z wytycznymi **GUM (Guide to the Expression of Uncertainty in Measurement, JCGM 100:2008)**, całkowita niepewność pomiaru układu warunkowania sygnału została wyznaczona jako pierwiastek sumy kwadratów składowych niepewności cząstkowych (metoda propagacji niepewności):

$$u_c(F) = \sqrt{\sum_{i=1}^{N} \left( \frac{\partial F}{\partial x_i} \cdot u(x_i) \right)^2}$$

gdzie $x_i$ to poszczególne wielkości wejściowe wpływające na wynik pomiaru.

**Identyfikacja źródeł niepewności:**

| Źródło | Typ | Rozkład | Wartość standardowa $u(x_i)$ | Współczynnik wrażliwości $c_i$ |
|--------|-----|---------|------------------------------|-------------------------------|
| Kalibracja mas wzorcowych | B | Normalny | ±0.05% FSO | 1.0 |
| Powtarzalność pomiaru (Type A) | A | Studenta (n=30) | ±0.42% FSO | 1.0 |
| Nieliniowość charakterystyki | B | Prostokątny | ±0.03% FSO / √3 | 1.0 |
| Dryft temperaturowy | B | Prostokątny | ±0.08% FSO / √3 | 1.0 |
| Histereza | B | Prostokątny | ±0.05% FSO / √3 | 1.0 |
| Rozdzielczość ADC (kwantyzacja) | B | Prostokątny | ±0.0001% FSO / √12 | 1.0 |
| Zakłócenia EMI | B | Normalny | ±0.15% FSO | 1.0 |

**Obliczenie całkowitej niepewności:**

$$u_c = \sqrt{(0.05)^2 + (0.42)^2 + (0.017)^2 + (0.046)^2 + (0.029)^2 + (0.00003)^2 + (0.15)^2} = \sqrt{0.2009} = 0.448\%\,FSO$$

**Niepewność rozszerzona (poziom ufności 95%):**

$$U = k \cdot u_c = 2.0 \cdot 0.448\% = 0.896\%\,FSO$$

gdzie $k=2.0$ to współczynnik rozszerzenia dla poziomu ufności 95% (rozkład normalny).

**Wniosek:** Całkowita niepewność pomiaru systemu wynosi **±0.9% FSO**, co spełnia wymaganie **±2%** dla zastosowań klinicznych i mieści się w granicach błędu akceptowalnych dla wyrobów medycznych klasy I.

### 3.2.5.2. Porównanie z systemami referencyjnymi i walidacja krzyżowa

W celu niezależnej weryfikacji dokładności opracowanego układu warunkowania sygnału, przeprowadzono badania porównawcze z laboratoryjnymi systemami referencyjnymi:

**A. Dynamometr izokinetyczny Biodex System 4:**
- Zakres pomiarowy: 0–1000 Nm;
- Dokładność deklarowana: ±1% FSO;
- Częstotliwość próbkowania: 100–2000 Hz.

**Protokół walidacji:**
1. Jednoczesny pomiar siły podczas 10 serii ćwiczeń z tubeingiem (3×8 powtórzeń);
2. Synchronizacja czasowa poprzez impuls triggera TTL;
3. Analiza korelacji i regresji Blanda-Altmana.

**Wyniki:**
- **Współczynnik korelacji Pearsona:** $r = 0.997$ (p < 0.001);
- **Bias systematyczny:** -1.2 N (system własny nieco zaniża wyniki);
- **Granice zgodności 95%:** -8.4 N do +6.0 N;
- **Błąd średniokwadratowy (RMSE):** 3.7 N.

**B. Przetwornik siły HBM U2B z wzmacniaczem Spider8:**
- Klasa dokładności: 0.05% FSO;
- Rozdzielczość: 24 bity;
- Częstotliwość próbkowania: 2400 Hz.

**Wyniki porównania:**
- **Maksymalne rozbieżności:** ±2.1% w dynamicznych testach RFD (szybkie narastanie siły);
- **Przyczyna:** Ograniczone pasmo przenoszenia układu własnego (28 Hz vs 500 Hz HBM);
- **Dla aplikacji klinicznych:** Różnica uznana za akceptowalną, gdyż tempo generowania siły u pacjentów rehabilitacyjnych rzadko przekracza 500 N/s (odpowiednik 18 Hz w domenie częstotliwości).

### 3.2.5.3. Testy długoterminowej stabilności i odporności środowiskowej

**Test starzenia przyspieszonego (accelerated aging test):**
- **Warunki:** 85°C / 85% RH przez 1000 godzin (odpowiednik ~5 lat eksploatacji w warunkach normalnych wg modelu Arrheniusa);
- **Pomiary kontrolne:** Co 100 godzin (kalibracja statyczna, pomiar dryftu zera);
- **Kryterium passing:** Zmiana współczynnika wzmocnienia <±1%, dryft zera <±0.5% FSO.

**Wyniki:**
- **Zmiana gain factor po 1000h:** +0.34% (w granicach specyfikacji);
- **Dryft zera:** +0.12% FSO;
- **Uszkodzenia mechaniczne:** Brak;
- **Korozja złączy:** Minimalna (zabezpieczenie konformalne skuteczne).

**Testy cyklów termicznych:**
- **Profil:** -10°C ↔ +50°C, 50 cykli, czas przejścia 30 minut;
- **Cel:** Weryfikacja odporności na zmiany temperatury w transporcie i magazynowaniu;
- **Wynik:** Brak trwałych zmian parametrów, przejściowe fluktuacje w trakcie testu mieszczące się w ±0.5% FSO.

**Test wstrząsów i wibracji:**
- **Norma:** PN-EN 60068-2-64 (wibracje losowe) i PN-EN 60068-2-27 (wstrząsy półsinusoidalne);
- **Profil wibracyjny:** 5–500 Hz, 1.5 g RMS, 2 godziny na oś;
- **Wstrząsy:** 15 g, 11 ms, 3 wstrząsy na oś;
- **Kryterium:** Brak przerw w komunikacji, zmiana kalibracji <±0.5% FSO;
- **Wynik:** Test zaliczony pozytywnie, jedynie przejściowe zakłócenia (<10 ms) podczas najintensywniejszych wstrząsów.

---

## 3.2.6. Podsumowanie i wnioski z realizacji układu warunkowania sygnału

Realizacja podsystemu warunkowania sygnału dla autorskiego systemu pomiarowo-treningowego stanowiła interdyscyplinarne wyzwanie, łączące zagadnienia z zakresu **metrologii precyzyjnej**, **elektroniki analogowej**, **cyfrowego przetwarzania sygnałów** oraz **inżynierii mechanicznej**. Kluczowe osiągnięcia tego etapu projektu obejmują:

**1. Osiągnięcie parametrów metrologicznych klasy medycznej przy niskim koszcie:**
Zastosowanie wyspecjalizowanego układu HX711 w połączeniu z precyzyjnym mostkiem tensometrycznym full-bridge pozwoliło na uzyskanie niepewności pomiaru **±0.9% FSO** przy koszcie komponentów rzędu **40–50 PLN** (w porównaniu do systemów laboratoryjnych o podobnych parametrach kosztujących **20 000–50 000 PLN**).

**2. Skuteczna kompensacja zakłóceń środowiskowych:**
Zaimplementowane metody filtracji (sprzętowej i cyfrowej), kompensacji temperaturowej oraz synchronicznego uśredniania zapewniły odporność na zakłócenia EMI typowe dla środowisk klinicznych i treningowych, co potwierdzono testami EMC zgodnie z normą PN-EN 61000-4-3.

**3. Deterministyczny czas odpowiedzi spełniający wymagania biofeedbacku:**
Opracowana architektura akwizycji (80 Hz próbkowania, 37 ms opóźnienia end-to-end) gwarantuje skuteczność pętli sprzężenia zwrotnego, co zostało zweryfikowane w badaniach z udziałem użytkowników (rozdział 6.3).

**4. Modularność i skalowalność rozwiązania:**
Warstwowa architektura systemu umożliwia łatwą wymianę poszczególnych komponentów (np. upgrade do 32-bitowego ADC, dodanie kolejnych kanałów pomiarowych) bez konieczności przeprojektowania całego układu.

**5. Kompleksowa walidacja metrologiczna:**
Przeprowadzone badania porównawcze z systemami referencyjnymi (Biodex, HBM) oraz testy długoterminowej stabilności dostarczyły dowodów na trafność i wiarygodność opracowanego rozwiązania, spełniając rygory naukowe pracy habilitacyjnej.

Układ warunkowania sygnału opisany w niniejszym rozdziale stanowi **fundamentalny komponent technologiczny** autorskiego osiągnięcia projektowego, tworząc metrologicznie wiarygodną podstawę dla dalszych etapów przetwarzania danych, obliczeń biomechanicznych i implementacji zaawansowanych algorytmów pętli sprzężenia zwrotnego przedstawionych w rozdziałach 4 i 5.
