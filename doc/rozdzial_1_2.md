# Rozdział 1. Przegląd stanu wiedzy i uwarunkowań prawno-metrologicznych

## 1.2. Metody pomiaru sił napięcia i naprężeń dynamicznych w urządzeniach oporowych

### 1.2.1. Wprowadzenie do metrologii sił w zastosowaniach biomechanicznych

Pomiar sił napięcia i naprężeń dynamicznych w urządzeniach oporowych stanowi fundamentalne wyzwanie metrologiczne na styku inżynierii biomedycznej, fizjoterapii i nauk o sporcie. W kontekście pracy habilitacyjnej dotyczącej autorskiego systemu pomiarowo-treningowego z pętlą sprzężenia zwrotnego, precyzyjne określenie metod pomiarowych, ich ograniczeń, niepewności oraz warunków stosowania ma kluczowe znaczenie dla walidacji całego rozwiązania technologicznego.

**Metrologia sił** w zastosowaniach biomechanicznych różni się istotnie od klasycznych pomiarów przemysłowych ze względu na:
- **Dynamiczny charakter obciążeń** – siły generowane przez układ mięśniowo-szkieletowy człowieka charakteryzują się szybkimi zmianami wartości (rzędu dziesiątek niutonów na milisekundę), obecnością składowych impulsowych oraz szerokim widmem częstotliwościowym (0.1–50 Hz dla ruchów voluntarnych);
- **Nieliniowość układu pomiarowego** – połączenie elastycznego elementu oporowego (tubeing lateksowy) z czujnikiem tensometrycznym tworzy układ o złożonej charakterystyce statycznej i dynamicznej, wymagający zaawansowanej kompensacji matematycznej;
- **Warunki eksploatacyjne** – pomiary przeprowadzane są w środowisku klinicznym lub treningowym, gdzie występują zmienne warunki temperaturowe (15–35°C), wilgotnościowe (30–80% RH), wibracje mechaniczne oraz potencjalne zakłócenia elektromagnetyczne;
- **Wymagania bezpieczeństwa** – urządzenia mające kontakt z pacjentem muszą spełniać rygorystyczne normy bezpieczeństwa elektrycznego (PN-EN 60601-1) oraz kompatybilności elektromagnetycznej (PN-EN 60601-1-2).

Niniejszy podrozdział prezentuje kompleksowy przegląd metod pomiaru sił napięcia i naprężeń dynamicznych stosowanych w urządzeniach oporowych, ze szczególnym uwzględnieniem:
- technik tensometrii oporowej jako dominującej metody w systemach wbudowanych,
- alternatywnych technologii czujnikowych (czujniki piezoelektryczne, pojemnościowe, optyczne, magnetorezystancyjne),
- metod kalibracji statycznej i dynamicznej,
- algorytmów przetwarzania sygnałów i kompensacji błędów systematycznych,
- analizy niepewności pomiarowej zgodnie z przewodnikiem GUM (Guide to the Expression of Uncertainty in Measurement).

---

### 1.2.2. Tensometria oporowa – podstawy teoretyczne i praktyczne

#### 1.2.2.1. Zasada działania tensometrów oporowych

Tensometria oporowa stanowi najpowszechniej stosowaną metodę pomiaru odkształceń mechanicznych i pośrednio – sił działających na element konstrukcyjny. Podstawowym elementem systemu jest **tensometr oporowy** (strain gauge), którego zasada działania bazuje na zjawisku zmiany rezystancji elektrycznej przewodnika poddanego deformacji mechanicznej.

Zmiana rezystancji $\Delta R$ tensometru poddanego odkształceniu $\varepsilon$ opisana jest zależnością:

$$\frac{\Delta R}{R_0} = k \cdot \varepsilon$$

gdzie:
- $R_0$ – rezystancja nominalna tensometru (typowo 120 Ω, 350 Ω, 1000 Ω) [Ω]
- $\Delta R$ – zmiana rezystancji [Ω]
- $k$ – współczynnik tensometryczny (gauge factor) [-]
- $\varepsilon$ – odkształcenie względne ($\varepsilon = \Delta L / L_0$) [-]

**Współczynnik tensometryczny** $k$ jest parametrem materiałowym zależnym od rodzaju folii tensometrycznej. Dla popularnych tensometrów z konstantanu (stop Cu-Ni) wynosi on około 2.0–2.2, podczas gdy dla tensometrów półprzewodnikowych (krzem domieszkowany) może osiągać wartości 50–150, co przekłada się na znacznie wyższą czułość, ale również większą nieliniowość i temperaturową zależność parametru.

Fizyczne podstawy zjawiska obejmują trzy mechanizmy:
1. **Zmiana geometrii przewodnika** – wydłużenie powoduje wzrost długości $L$ i spadek pola przekroju $A$, co zgodnie z zależnością $R = \rho \cdot L / A$ prowadzi do wzrostu rezystancji;
2. **Efekt piezorezystywny** – zmiana rezystywności właściwej $\rho$ materiału pod wpływem naprężeń mechanicznych (dominujący efekt w tensometrach półprzewodnikowych);
3. **Efekty termiczne** – zmiany temperatury wpływają zarówno na rezystywność właściwą, jak i wymiary geometryczne (wymagająca kompensacji).

Dla typowego odkształcenia rzędu $1000 \, \mu\varepsilon$ (mikroodkształcenie, czyli $10^{-3}$) i tensometru o $k = 2.0$, względna zmiana rezystancji wynosi zaledwie $0.2\%$, co oznacza, że dla $R_0 = 350 \, \Omega$ zmiana $\Delta R$ to jedynie $0.7 \, \Omega$. Tak niewielkie zmiany wymagają zastosowania wyspecjalizowanych układów warunkowania sygnału, zdolnych do detekcji sygnałów na poziomie mikrowoltów.

#### 1.2.2.2. Mostek Wheatstone'a – konfiguracje i czułość

Ze względu na niewielką wartość zmian rezystancji, tensometry niemal zawsze pracują w konfiguracji **mostka Wheatstone'a**, który umożliwia:
- konwersję zmiany rezystancji na napięcie wyjściowe,
- kompensację temperaturową poprzez zastosowanie tensometrów kompensacyjnych,
- zwiększenie czułości poprzez wykorzystanie wielu czynnych tensometrów.

**Rysunek 1.1.** Schemat mostka Wheatstone'a z czterema tensometrami

```
        V_exc
         |
    +----+----+
    |         |
   R1         R3
    |         |
    +----+----+---- V_out+
    |         |
   R2         R4
    |         |
    +----+----+
         |
        GND
    
    V_out = V_out+ - V_out-
```

Napięcie wyjściowe mostka wyraża się wzorem ogólnym:

$$V_{out} = V_{exc} \cdot \left( \frac{R_2}{R_1 + R_2} - \frac{R_4}{R_3 + R_4} \right)$$

gdzie:
- $V_{exc}$ – napięcie wzbudzenia mostka (typowo 2.5–10 V DC)
- $R_1, R_2, R_3, R_4$ – rezystancje ramion mostka

W stanie równowagi (przy braku obciążenia) mostek jest zrównoważony, tzn. $R_1/R_2 = R_3/R_4$, co daje $V_{out} = 0$. Pod wpływem obciążenia następuje zmiana rezystancji czynnych tensometrów, prowadząc do pojawienia się niezerowego napięcia wyjściowego.

**Tabela 1.3.** Konfiguracje mostka Wheatstone'a i ich parametry

| Konfiguracja | Liczba czynnych tensometrów | Czułość względna | Kompensacja temperaturowa | Zastosowanie |
|--------------|----------------------------|------------------|---------------------------|--------------|
| Ćwierćmostek | 1 | 1× | Częściowa (z tensometrem kompensacyjnym) | Proste układy, koszty minimalne |
| Półmostek | 2 | 2× | Pełna (układ push-pull) | Pomiar zginania, lepsza liniowość |
| Pełny mostek | 4 | 4× | Pełna (wszystkie ramiona aktywne) | Wysoka precyzja, kompensacja sił pasożytniczych |

Dla konfiguracji **pełnego mostka** z czterema aktywnymi tensometrami pracującymi w układzie push-pull (dwa rozciągane, dwa ściskane), napięcie wyjściowe przyjmuje postać:

$$V_{out} = V_{exc} \cdot k \cdot \varepsilon$$

co oznacza czterokrotne zwiększenie czułości w porównaniu do ćwierćmostka. Dodatkowo, wszystkie cztery tensometry podlegają tym samym warunkom temperaturowym, co zapewnia doskonałą kompensację dryfu termicznego.

#### 1.2.2.3. Układy warunkowania sygnału – wzmacniacze instrumentalne

Sygnał wyjściowy z mostka Wheatstone'a ma typowo wartość rzędu **0.5–20 mV** przy pełnym zakresie obciążenia, co wymaga zastosowania wyspecjalizowanego wzmacniacza o następujących parametrach:
- **Wzmocnienie programowalne** (Gain = 10–1000) z wysoką stabilnością,
- **Bardzo niski poziom szumów własnych** (< 1 µV pp w paśmie 0.1–10 Hz),
- **Wysoka impedancja wejściowa** (> 1 GΩ) minimalizująca obciążenie mostka,
- **Wysoki współczynnik tłumienia sygnału wspólnego CMRR** (> 100 dB) eliminujący zakłócenia sieciowe,
- **Niski dryf temperaturowy** (< 0.5 µV/°C).

Klasycznym rozwiązaniem jest **wzmacniacz instrumentalny** (instrumentation amplifier) zbudowany w oparciu o trzy wzmacniacze operacyjne w konfiguracji umożliwiającej precyzyjne wzmocnienie sygnału różnicowego przy jednoczesnym tłumieniu sygnału wspólnego.

**Rysunek 1.2.** Schemat wzmacniacza instrumentalnego trzech wzmacniaczy operacyjnych

Współczesne aplikacje coraz częściej wykorzystują **specjalizowane układy scalone do tensometrii**, takie jak:
- **HX711** – 24-bitowy przetwornik ADC z wbudowanym wzmacniaczem instrumentalnym (gain 32/64/128), popularny w aplikacjach hobby i prototypowych,
- **ADS1232/ADS124S08** – 24-bitowe ADC Texas Instruments z programmable gain amplifier (PGA) i referencyjnym źródłem napięcia,
- **MAX140/MAX1452** – układy z kompletnym warunkowaniem sygnału (wzmocnienie, filtracja, kompensacja offsetu i gain error),
- **INA125/INA128** – precyzyjne wzmacniacze instrumentalne Analog Devices.

W opracowanym systemie pomiarowo-treningowym zastosowano układ **HX711** ze względu na:
- rozdzielczość 24 bitów zapewniającą teoretyczny zakres dynamiki 16.7 mln kroków,
- wbudowany wzmacniacz instrumentalny z programowalnym wzmocnieniem (32, 64, 128),
- interfejs szeregowy kompatybilny z mikrokontrolerami (Arduino, Raspberry Pi Pico),
- niski pobór prądu (< 1.5 mA w trybie pracy ciągłej),
- dostępność i niski koszt komponentu.

Mimo że HX711 nie spełnia wymogów aplikacji medycznych klasy profesjonalnej (brak certyfikacji, ograniczona dokładność long-term), w połączeniu z odpowiednią kalibracją i algorytmami kompensacji zapewnia wystarczającą precyzję dla celów treningowych i rehabilitacyjnych.

#### 1.2.2.4. Źródła błędów w tensometrii i metody ich kompensacji

Pomiary tensometryczne obarczone są wieloma źródłami błędów systematycznych i losowych, które muszą być identyfikowane i kompensowane:

**A. Błędy temperaturowe**

Zmiany temperatury wpływają na pomiar na trzy sposoby:
1. **Zmiana rezystywności właściwej** tensometru – współczynnik temperaturowy rezystancji (TCR) dla konstantanu wynosi ok. ±20 ppm/°C;
2. **Rozszerzalność cieplna** materiału podłoża i tensometru – różne współczynniki rozszerzalności prowadzą do powstawania odkształceń termicznych;
3. **Zmiana współczynnika tensometrycznego** $k$ – zależność $k(T)$ dla większości materiałów jest nieliniowa.

Kompensację temperaturową realizuje się poprzez:
- zastosowanie **tensometrów kompensacyjnych** umieszczonych na elemencie nieobciążonym, ale w tej samej temperaturze,
- użycie **pełnego mostka** z parami tensometrów pracujących w konfiguracji push-pull,
- implementację **kompensacji programowej** na podstawie pomiaru temperatury otoczenia (termistor lub czujnik cyfrowy DS18B20).

Model kompensacji temperaturowej w systemie:

$$F_{skomp} = F_{zmierz} \cdot [1 + \alpha \cdot (T - T_{ref}) + \beta \cdot (T - T_{ref})^2]$$

gdzie:
- $F_{skomp}$ – siła po kompensacji temperaturowej
- $F_{zmierz}$ – siła zmierzona
- $T$ – aktualna temperatura
- $T_{ref}$ – temperatura odniesienia kalibracji (zwykle 25°C)
- $\alpha, \beta$ – współczynniki temperaturowe wyznaczane eksperymentalnie

**B. Pełzanie (creep)**

Pełzanie to zjawisko zmiany wskazań czujnika przy stałym obciążeniu w czasie, wynikające z relaksacji naprężeń w materiale elementu sprężystego i kleju tensometrycznym. Typowe wartości pełzania dla czujników przemysłowych wynoszą 0.02–0.05% zakresu na godzinę.

Kompensacja pełzania wymaga:
- stosowania materiałów o niskim pełzaniu (stal stopowa 17-4 PH, aluminium 2024-T6),
- odpowiedniego starzenia mechanicznego elementu sprężystego przed kalibracją,
- implementacji algorytmów korekcyjnych opartych na modelu lepkosprężystym.

**C. Histeraza mechaniczna**

Histeraza objawia się różnicą wskazań czujnika przy narastaniu i opadaniu obciążenia. Dla wysokiej jakości czujników tensometrycznych histeraza wynosi < 0.03% zakresu. Minimalizacja wymaga:
- precyzyjnego projektowania elementu sprężystego (unikanie koncentracji naprężeń),
- stosowania tensometrów z folii metalowych zamiast półprzewodnikowych,
- kalibracji dwukierunkowej (loading/unloading).

**D. Zakłócenia elektromagnetyczne (EMI)**

Długie przewody łączące mostek z układem warunkowania działają jak anteny zbierające zakłócenia sieciowe (50/60 Hz) oraz radiowe. Środki zaradcze:
- ekranowanie przewodów (skrętka ekranowana),
- filtrowanie dolnoprzepustowe (RC lub aktywne) z częstotliwością graniczną dobraną do pasma sygnału użytecznego,
- separacja galwaniczna (optoizolatory, transformatory),
- zasilanie bateryjne lub stabilizowane liniowo (nie impulsowo).

**E. Nieliniowość charakterystyki**

Idealny czujnik siły powinien mieć liniową charakterystykę $F = k \cdot V_{out}$, jednak w rzeczywistości występują odchylenia rzędu 0.05–0.2% zakresu. Kompensacja nieliniowości możliwa jest poprzez:
- kalibrację wielopunktową (minimum 5 punktów w całym zakresie),
- aproksymację wielomianową drugiego lub trzeciego stopnia,
- zastosowanie tabel interpolowanych liniowo lub spline'em.

---

### 1.2.3. Alternatywne technologie pomiaru sił

Mimo dominacji tensometrii oporowej w zastosowaniach przemysłowych i badawczych, rozwój technologiczny przyniósł szereg alternatywnych metod pomiaru sił, z których niektóre znajdują zastosowanie w urządzeniach medycznych i sportowych.

#### 1.2.3.1. Czujniki piezoelektryczne

**Zasada działania:** Wykorzystują zjawisko piezoelektryczne – generowanie ładunku elektrycznego na powierzchni niektórych kryształów (kwarc, turmalin) lub ceramiki (PZT – tytanian-cyrkonian ołowiu) pod wpływem naprężeń mechanicznych.

**Charakterystyka:**
- Bardzo wysoka sztywność – minimalne ugięcie pod obciążeniem,
- Szerokie pasmo przenoszenia (DC do kilkuset kHz),
- Doskonała dynamika – zdolność pomiaru bardzo szybkich zmian siły,
- Brak możliwości pomiaru sił statycznych (ładunek upływa przez rezystancję wejściową wzmacniacza),
- Wysoka czułość (rzędu pC/N).

**Zastosowania w biomechanice:**
- Platformy sił reakcji podłoża (force plates) do analizy chodu i skoków,
- Czujniki uderzeniowe w testach zderzeniowych,
- Dynamometry do pomiaru siły nacisku dłoni, szczęki.

**Ograniczenia w systemach z tubeingiem:**
- Niezdolność do pomiaru sił quasi-statycznych (charakterystycznych dla ćwiczeń rehabilitacyjnych),
- Wymóg stosowania wzmacniaczy ładunkowych o wysokiej impedancji wejściowej,
- Wrażliwość na zmiany temperatury (efekt piroelektryczny).

#### 1.2.3.2. Czujniki pojemnościowe

**Zasada działania:** Pomiar zmiany pojemności kondensatora spowodowanej zmianą odległości między okładkami lub zmiany pola powierzchni pod wpływem siły zewnętrznej.

**Charakterystyka:**
- Wysoka rozdzielczość (możliwość detekcji sił rzędu mili niutonów),
- Niski pobór mocy,
- Odporność na zakłócenia elektromagnetyczne,
- Wymóg stosowania oscylatorów lub układów AC do pomiaru pojemności,
- Wrażliwość na wilgotność i zanieczyszczenia.

**Zastosowania:**
- Mikromanipulatory i robotyka precyzyjna,
- Dotykowe interfejsy człowiek-maszyna,
- Wearables do monitorowania aktywności.

**Potencjał w systemach rehabilitacyjnych:** Możliwość miniaturyzacji i integracji z elastycznymi podłożami (e-textiles), jednak obecnie ograniczona dostępnością komercyjnych rozwiązań o dużym zakresie pomiarowym (> 500 N).

#### 1.2.3.3. Czujniki optyczne i światłowodowe

**Zasada działania:** Wykorzystanie zjawisk modulacji światła (intensywność, faza, polaryzacja, długość fali) pod wpływem odkształceń mechanicznych. Szczególną klasę stanowią czujniki z **siatkami Bragga (FBG – Fiber Bragg Grating)**.

**Charakterystyka FBG:**
- Odporność na zakłócenia EMI/RFI (brak sygnałów elektrycznych),
- Możliwość multiplexowania wielu czujników na jednym światłowodzie,
- Stabilność długoterminowa i odporność na korozję,
- Wysoki koszt interrogatorów optycznych,
- Wymóg specjalistycznej wiedzy przy instalacji.

**Zastosowania medyczne:**
- Monitorowanie sił w implantach ortopedycznych,
- Czujniki wewnątrzczaszkowe,
- Kateteryzacja kardiologyjna.

**Perspektywy w rehabilitacji:** Potencjał zastosowania w środowiskach MRI-compatibile (badania biomechaniczne w rezonansie magnetycznym), jednak obecnie zbyt wysoki koszt dla aplikacji komercyjnych.

#### 1.2.3.4. Czujniki magnetorezystancyjne i Halla

**Zasada działania:** Pomiar zmiany pola magnetycznego spowodowanego przemieszczeniem elementu ferromagnetycznego pod wpływem siły.

**Charakterystyka:**
- Bezkontaktowy pomiar przemieszczenia,
- Odporność na zabrudzenia i wilgoć,
- Możliwość pracy w trudnych warunkach środowiskowych,
- Wymóg stosowania magnesów trwałych,
- Wrażliwość na zewnętrzne pola magnetyczne.

**Zastosowania:**
- Joysticki i interfejsy sterowania,
- Zawieszenia aktywne w pojazdach,
- Ergometry rowerowe i wioślarskie.

#### 1.2.3.5. Czujniki hydrauliczne i pneumatyczne

**Zasada działania:** Pomiar ciśnienia cieczy lub gazu w komorze poddawanej obciążeniowi mechanicznemu.

**Charakterystyka:**
- Bardzo duży zakres pomiarowy (do kilku ton),
- Odporność na przeciążenia,
- Samoczynne tłumienie drgań,
- Wolna odpowiedź dynamiczna,
- Ryzyko wycieków medium.

**Zastosowania:**
- Wagi przemysłowe i platformowe,
- Siłowniki hydrauliczne w maszynach budowlanych,
- Rzadziej w biomechanice (ergometry izokinetyczne starszej generacji).

**Tabela 1.4.** Porównanie technologii czujników siły

| Technologia | Zakres typowy [N] | Dokładność [%] | Pasmo [Hz] | Koszt | Przydatność dla tubeingu |
|-------------|-------------------|----------------|------------|-------|--------------------------|
| Tensometryczna | 0–5000 | 0.05–0.5 | DC–500 | Niski–średni | ★★★★★ (wybrana) |
| Piezoelektryczna | 0–10000 | 0.1–1.0 | DC–100k | Wysoki | ★★☆☆☆ (brak DC) |
| Pojemnościowa | 0–100 | 0.5–2.0 | DC–100 | Średni | ★★★☆☆ (mały zakres) |
| Optyczna FBG | 0–2000 | 0.1–0.5 | DC–1000 | Bardzo wysoki | ★★★☆☆ (koszt) |
| Magnetyczna | 0–1000 | 1.0–3.0 | DC–200 | Niski | ★★★☆☆ (histeraza) |
| Hydrauliczna | 100–50000 | 0.5–2.0 | DC–10 | Średni | ★★☆☆☆ (dynamika) |

---

### 1.2.4. Metody kalibracji czujników siły

Kalibracja jest procesem ustalania zależności między wskazaniem przyrządu pomiarowego a wartością rzeczywistą mierzonej wielkości. W kontekście czujników siły stosowanych w systemach rehabilitacyjnych wyróżnia się kalibrację **statyczną** i **dynamiczną**.

#### 1.2.4.1. Kalibracja statyczna

**Cel:** Wyznaczenie charakterystyki przejściowej $F = f(V_{out})$ w warunkach quasi-statycznych (zmiana obciążenia wolniejsza niż czas odpowiedzi układu).

**Procedura:**
1. **Przygotowanie stanowiska:** Czujnik montowany jest w pionowej konfiguracji z możliwością zawieszania odważników wzorcowych lub w poziomej z siłownikiem kontrolowanym.
2. **Wzorcowe źródło siły:** Stosuje się odważniki klasy M1 lub lepszej (błąd < 0.05%), certifikowane przez laboratorium akredytowane (np. GUM w Polsce).
3. **Cykl obciążania:** 
   - Zerowanie czujnika (odciążenie),
   - Stopniowe nakładanie obciążenia (np. 0%, 20%, 40%, 60%, 80%, 100% zakresu),
   - Zarejestrowanie wskazań po ustabilizowaniu (czas oczekiwania 10–30 s na każdy punkt),
   - Stopniowe zdejmowanie obciążenia (histeraza),
   - Powtórzenie cyklu 3–5 razy dla oceny powtarzalności.
4. **Analiza danych:** Dopasowanie funkcji aproksymującej (liniowa, wielomianowa) metodą najmniejszych kwadratów.

**Model liniowy:**

$$F = a \cdot V_{out} + b$$

gdzie współczynniki $a$ (czułość) i $b$ (offset) wyznacza się z regresji liniowej:

$$a = \frac{n \sum (V_i F_i) - \sum V_i \sum F_i}{n \sum V_i^2 - (\sum V_i)^2}$$

$$b = \frac{\sum F_i - a \sum V_i}{n}$$

**Model wielomianowy drugiego stopnia** (dla kompensacji nieliniowości):

$$F = a_2 \cdot V_{out}^2 + a_1 \cdot V_{out} + a_0$$

Współczynniki wyznacza się z układu równań normalnych metody najmniejszych kwadratów.

**Wskaźniki jakości kalibracji:**
- **Współczynnik determinacji $R^2$** – miara dopasowania modelu (powinien być > 0.999),
- **Maksymalny błąd nieliniowości** – maksymalne odchylenie punktu pomiarowego od krzywej aproksymującej,
- **Histeraza** – maksymalna różnica między krzywą loading i unloading,
- **Powtarzalność** – odchylenie standardowe wskazań dla tego samego obciążenia w kolejnych cyklach.

#### 1.2.4.2. Kalibracja dynamiczna

**Cel:** Wyznaczenie charakterystyki częstotliwościowej i czasowej odpowiedzi układu pomiarowego na zmieniające się w czasie obciążenie.

**Metody:**

**A. Skok jednostkowy (step response)**
- Nagłe przyłożenie znanego obciążenia (np. upuszczenie odważnika na platformę),
- Rejestracja przebiegu czasowego,
- Wyznaczenie parametrów: czas narastania, czas ustawienia, przeregulowanie, stała czasowa.

Model odpowiedzi pierwszego rzędu:

$$F(t) = F_0 \cdot (1 - e^{-t/\tau})$$

gdzie $\tau$ to stała czasowa układu.

**B. Wzbudzenie sinusoidalne**
- Generowanie siły o zmieniającej się sinusoidalnie wartości $F(t) = F_0 \cdot \sin(2\pi f t)$,
- Pomiar amplitudy i fazy sygnału wyjściowego dla różnych częstotliwości $f$,
- Wyznaczenie charakterystyki amplitudowej i fazowej (wykres Bodego).

**C. Wzbudzenie impulsowe**
- Uderzenie młotka modalnego z wbudowanym czujnikiem siły,
- Analiza widmowa (FFT) odpowiedzi układu,
- Identyfikacja częstotliwości rezonansowych.

**Znaczenie dla systemu z tubeingiem:** Ze względu na lepkosprężyste właściwości lateksu, kalibracja dynamiczna jest kluczowa dla ćwiczeń o szybkim tempie. Należy wyznaczyć charakterystykę dla częstotliwości 0.1–5 Hz (typowe tempo ćwiczeń rehabilitacyjnych).

#### 1.2.4.3. Kalibracja in situ i autokalibracja

W systemach wbudowanych pracujących w warunkach klinicznych korzystne jest posiadanie procedur **kalibracji na miejscu (in situ)** bez konieczności demontażu czujnika.

**Metody autokalibracji:**
- **Wzorcowanie zerem** – automatyczne zerowanie przy braku obciążenia (przed każdym treningiem),
- **Wzorcowanie jednym punktem** – użycie znanego obciążenia kalibracyjnego (np. ciężarek 5 kg dostarczony z urządzeniem),
- **Kalibracja wzorcem wewnętrznym** – zastosowanie dodatkowego czujnika referencyjnego o znanej charakterystyce,
- **Kalibracja przez użytkownika** – procedura prowadzona przez fizjoterapeutę z użyciem dostarczonych odważników.

W opracowanym systemie zastosowano procedurę **dwupunktowej kalibracji użytkownika**:
1. Zerowanie – rejestracja wartości offsetu przy odciążonym tubeingu,
2. Obciążenie wzorcowe – zawieszenie znanego ciężaru (np. 10 kg = 98.1 N) i rejestracja wskazania,
3. Obliczenie czułości: $k = (F_{wzorzec} - 0) / (V_{obciążenie} - V_{offset})$,
4. Zapis parametrów w pamięci EEPROM mikrokontrolera.

---

### 1.2.5. Przetwarzanie sygnałów pomiarowych i algorytmy filtracji

Sygnał z czujnika tensometrycznego wymaga zaawansowanego przetwarzania w celu ekstrakcji użytecznych informacji biomechanicznych i eliminacji zakłóceń.

#### 1.2.5.1. Filtracja dolnoprzepustowa

**Cel:** Usunięcie składowych wysokoczęstotliwościowych (szum termiczny, zakłócenia sieciowe 50/60 Hz, EMI) przy zachowaniu sygnału użytecznego o częstotliwości < 10 Hz.

**Filtry analogowe:**
- **RC first-order:** $H(s) = \frac{1}{1 + sRC}$, nachylenie -20 dB/dekadę,
- **Sallen-Key second-order:** możliwość strojenia dobroci Q, nachylenie -40 dB/dekadę.

**Filtry cyfrowe:**
- **FIR (Finite Impulse Response):** stabilne, liniowa charakterystyka fazowa, wymagają więcej obliczeń,
- **IIR (Infinite Impulse Response):** efektywne obliczeniowo, nieliniowa faza, ryzyko niestabilności.

**Filtr Butterwortha drugiego rzędu** (często stosowany w biomechanice):

$$H(z) = \frac{b_0 + b_1 z^{-1} + b_2 z^{-2}}{1 + a_1 z^{-1} + a_2 z^{-2}}$$

Współczynniki $a_i, b_i$ dobiera się w zależności od częstotliwości granicznej $f_c$ i częstotliwości próbkowania $f_s$.

Dla systemu z próbkowaniem 100 Hz i $f_c = 10$ Hz, typowe współczynniki wynoszą:
- $b_0 = 0.0675, b_1 = 0.1350, b_2 = 0.0675$
- $a_1 = -1.1430, a_2 = 0.4128$

#### 1.2.5.2. Filtracja adaptacyjna i wygładzanie

**Średnia ruchoma (Moving Average):**

$$y[n] = \frac{1}{N} \sum_{i=0}^{N-1} x[n-i]$$

Prosta implementacja, skuteczna redukcja szumu białego, ale wprowadza opóźnienie grupowe $N/2$ próbek.

**Filtr Savitzky'ego-Golaya:**
- Aproksymacja wielomianowa w oknie ruchomym,
- Zachowanie kształtu sygnału (lepsze niż średnia ruchoma dla pików),
- Możliwość jednoczesnego wygładzania i wyznaczania pochodnych.

**Filtr Kalmana:**
- Optymalny estymator stanu dla układów liniowych z szumem Gaussa,
- Łączy model dynamiki układu z pomiarami,
- Wymaga znajomości macierzy kowariancji szumu procesu i pomiaru.

Równania filtra Kalmana dla modelu siły jako stałej z szumem:

**Predykcja:**
$$\hat{x}_{k|k-1} = \hat{x}_{k-1|k-1}$$
$$P_{k|k-1} = P_{k-1|k-1} + Q$$

**Aktualizacja:**
$$K_k = \frac{P_{k|k-1}}{P_{k|k-1} + R}$$
$$\hat{x}_{k|k} = \hat{x}_{k|k-1} + K_k (z_k - \hat{x}_{k|k-1})$$
$$P_{k|k} = (1 - K_k) P_{k|k-1}$$

gdzie:
- $\hat{x}$ – estymata siły,
- $P$ – kowariancja błędu estymacji,
- $Q$ – kowariancja szumu procesu,
- $R$ – kowariancja szumu pomiaru,
- $K$ – wzmocnienie Kalmana,
- $z_k$ – pomiar siły.

#### 1.2.5.3. Detekcja zdarzeń i segmentacja powtórzeń

W analizie ćwiczeń z tubeingiem kluczowa jest automatyczna detekcja:
- początku i końca serii ćwiczeń,
- poszczególnych powtórzeń (repetitions),
- faz koncentrycznej i ekscentrycznej.

**Algorytm detekcji powtórzeń:**

1. **Wstępne przetwarzanie:** Filtracja dolnoprzepustowa 5 Hz, usunięcie trendu.
2. **Detekcja pików:** Znalezienie lokalnych maksimów sygnału siły z progiem minimalnej amplitudy (np. 10% MVC – Maximum Voluntary Contraction).
3. **Segmentacja:** Podział sygnału na odcinki między kolejnymi minimami (początek każdego powtórzenia).
4. **Walidacja:** Odrzucenie fałszywych detekcji na podstawie:
   - minimalnego czasu trwania powtórzenia (> 0.5 s),
   - minimalnej amplitudy (> 5% zakresu),
   - symetrii fazy wzrostu i opadania.

**Pseudokod:**

```
for each sample i:
    if F[i] > threshold AND F[i] > F[i-1] AND F[i] > F[i+1]:
        candidate_peak = i
        if time_since_last_peak > min_rep_time:
            if peak_amplitude > min_amplitude:
                register_repetition(candidate_peak)
```

---

### 1.2.6. Analiza niepewności pomiarowej zgodnie z GUM

Ocena niepewności pomiarowej jest wymagana dla urządzeń stosowanych w zastosowaniach medycznych i naukowych. Zgodnie z **Guide to the Expression of Uncertainty in Measurement (GUM)**, niepewność dzieli się na składniki typu A (oceniane statystycznie) i typu B (oceniane innymi metodami).

#### 1.2.6.1. Model pomiaru

Model matematyczny pomiaru siły:

$$F = k \cdot (V_{out} - V_{offset}) \cdot C_T \cdot C_{NL}$$

gdzie:
- $k$ – czułość układu [N/V],
- $V_{out}$ – napięcie wyjściowe mostka [V],
- $V_{offset}$ – napięcie offsetu (odczyt zerowy) [V],
- $C_T$ – współczynnik korekcyjny temperatury [-],
- $C_{NL}$ – współczynnik korekcyjny nieliniowości [-].

#### 1.2.6.2. Identyfikacja źródeł niepewności

**Tabela 1.5.** Źródła niepewności pomiaru siły

| Źródło | Typ | Metoda oceny | Rozkład | Współczynnik wrażliwości |
|--------|-----|--------------|---------|--------------------------|
| Powtarzalność wskazań | A | Statystyczna (odchylenie std.) | Normalny | $c_1 = 1$ |
| Odważniki wzorcowe | B | Certyfikat kalibracji | Normalny | $c_2 = 1$ |
| Rozdzielczość ADC | B | Specyfikacja producenta | Prostokątny | $c_3 = 1$ |
| Dryf temperaturowy | B | Testy środowiskowe | Prostokątny | $c_4 = \partial F/\partial T$ |
| Nieliniowość | B | Kalibracja wielopunktowa | Trapezoidalny | $c_5 = \max(\delta_{NL})$ |
| Histeraza | B | Cykle loading/unloading | Prostokątny | $c_6 = \max(\delta_{Hyst})$ |
| Zakłócenia EMI | B | Testy kompatybilności | Normalny | $c_7 = \sigma_{EMI}$ |

#### 1.2.6.3. Obliczanie niepewności złożonej

Niepewność standardowa pojedynczych składników:

- **Typ A:** $u_A = s / \sqrt{n}$ (odchylenie standardowe średniej)
- **Typ B (rozkład normalny):** $u_B = U / k$ (gdzie $k=2$ dla 95% poziomu ufności)
- **Typ B (rozkład prostokątny):** $u_B = a / \sqrt{3}$ (gdzie $a$ to połowa szerokości przedziału)

Niepewność złożona:

$$u_c(F) = \sqrt{\sum_{i=1}^{n} \left( \frac{\partial F}{\partial x_i} \right)^2 \cdot u^2(x_i)}$$

Dla modelu addytywnego upraszcza się do:

$$u_c(F) = \sqrt{u_A^2 + u_{wzorce}^2 + u_{ADC}^2 + u_T^2 + u_{NL}^2 + u_{Hyst}^2 + u_{EMI}^2}$$

#### 1.2.6.4. Niepewność rozszerzona

Niepewność rozszerzona $U$ definiowana jest jako:

$$U = k_p \cdot u_c(F)$$

gdzie $k_p$ to współczynnik rozszerzenia:
- $k_p = 2$ dla poziomu ufności ~95% (zalecane w większości aplikacji),
- $k_p = 3$ dla poziomu ufności ~99% (aplikacje krytyczne).

**Przykład obliczeniowy dla systemu z tubeingiem:**

Zakładając:
- $u_A = 0.15$ N (powtarzalność),
- $u_{wzorce} = 0.05$ N (certyfikat odważników),
- $u_{ADC} = 0.08$ N (rozdzielczość 24-bit),
- $u_T = 0.10$ N (dryf temperaturowy),
- $u_{NL} = 0.12$ N (nieliniowość),
- $u_{Hyst} = 0.06$ N (histeraza),

$$u_c(F) = \sqrt{0.15^2 + 0.05^2 + 0.08^2 + 0.10^2 + 0.12^2 + 0.06^2} = 0.24 \text{ N}$$

Dla $k_p = 2$:

$$U = 2 \cdot 0.24 = 0.48 \text{ N}$$

Oznacza to, że przy wskazaniu 100 N, rzeczywista wartość siły mieści się w przedziale $100 \pm 0.48$ N z prawdopodobieństwem 95%.

---

### 1.2.7. Implementacja w systemie pomiarowo-treningowym

W opracowanym autorskim systemie pomiarowo-treningowym zastosowano hybrydowe podejście łączące:
- **Hardware:** Mostek pełny z czterema tensometrami foilowymi 350 Ω, zasilanie 5 V, układ HX711 z gain = 128, próbkowanie 80 Hz.
- **Software:** Filtr Butterwortha 4. rzędu z $f_c = 8$ Hz, filtr Kalmana do estymacji siły chwilowej, algorytm detekcji powtórzeń z adaptacyjnym progiem.
- **Kalibracja:** Procedura dwupunktowa z możliwością rozszerzenia do pięciu punktów, kompensacja temperaturowa na podstawie czujnika DS18B20.
- **Walidacja:** Porównanie z laboratoryjnym dynamometrem Izokinetycznym Biodex System 4, uzyskana zgodność w granicach ±1.2% zakresu.

Szczegóły implementacji oprogramowania, schematy elektryczne i wyniki walidacji przedstawiono w rozdziałach 3–5 niniejszej pracy.

---

### 1.2.8. Podsumowanie

Podrozdział 1.2 stanowi kompleksowe omówienie metod pomiaru sił napięcia i naprężeń dynamicznych w urządzeniach oporowych, ze szczególnym uwzględnieniem tensometrii oporowej jako technologii wybranej do autorskiego systemu pomiarowo-treningowego. Przedstawiono:
- podstawy teoretyczne tensometrii i mostka Wheatstone'a,
- układy warunkowania sygnału i źródła błędów pomiarowych,
- alternatywne technologie czujnikowe i ich porównanie,
- procedury kalibracji statycznej i dynamicznej,
- algorytmy przetwarzania sygnałów i filtracji cyfrowej,
- metodykę oceny niepewności pomiarowej zgodnie z GUM.

Wykazano, że pomimo istnienia zaawansowanych technologii alternatywnych (piezoelektryki, sensory optyczne), tensometria oporowa pozostaje optymalnym wyborem dla aplikacji rehabilitacyjnych ze względu na doskonały stosunek precyzji do kosztu, możliwość pomiaru sił statycznych i quasi-statycznych oraz łatwość integracji z systemami wbudowanymi.

Przedstawiona wiedza teoretyczna stanowi fundament dla rozdziałów praktycznych pracy, w których opisano realizację konstrukcyjną, oprogramowanie i walidację metrologiczną opracowanego systemu.
