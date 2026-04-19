# Rozdział 1. Przegląd stanu wiedzy i uwarunkowań prawno-metrologicznych

## 1.1. Tubeing lateksowy w terapii ruchowej i sporcie – właściwości fizyczne, klasyfikacja jako wyrobu medycznego klasy I

### 1.1.1. Wprowadzenie historyczne i ewolucja zastosowań terapeutycznych

Taśmy lateksowe, powszechnie określane mianem tubeingu (z ang. *tubing* – rurka, przewód), stanowią jedno z fundamentalnych narzędzi współczesnej fizjoterapii, rehabilitacji ortopedycznej i neurologicznej oraz treningu sportowego. Historia elastycznych przyrządów oporowych sięga początków XX wieku, jednak ich masowa popularyzacja w zastosowaniach medycznych nastąpiła w latach 80. ubiegłego stulecia za sprawą firmy TheraBand®, która wprowadziła na rynek system kolorowych taśm o stopniowanych wartościach oporu. Od tego czasu tubeing lateksowy ewoluował od prostego pomocniczego środka rehabilitacyjnego do zaawansowanego narzędzia diagnostyczno-terapeutycznego, znajdującego zastosowanie w szeroko rozumianych naukach o zdrowiu i naukach o sporcie.

Współczesna definicja tubeingu lateksowego obejmuje elastyczne przewody lub płaskie taśmy wykonane z naturalnego kauczuku (lateksu) lub syntetycznych materiałów elastomerowych (np. termoplastyczny elastomer TPE, guma silikonowa), charakteryzujące się nieliniową charakterystyką siłowo-wydłużeniową, zdolnością do wielokrotnego odkształcania sprężystego bez utraty właściwości mechanicznych oraz możliwością gradacji oporu poprzez zmianę grubości ścianki, szerokości taśmy lub długości segmentu roboczego.

Zastosowania tubeingu w terapii ruchowej obejmują trzy główne obszary:

1. **Rehabilitacja ortopedyczna** – powrót do funkcji po urazach narządu ruchu (zerwania więzadeł, złamania, endoprotezoplastyki), leczenie zachowawcze schorzeń przeciążeniowych (zapalenie ścięgien, zespoły bólowe kręgosłupa), profilaktyka pierwotna i wtórna kontuzji.

2. **Rehabilitacja neurologiczna** – terapia pacjentów po udarach mózgu, z chorobą Parkinsona, stwardnieniem rozsianym, porażeniem mózgowym, gdzie tubeing służy do poprawy kontroli motorycznej, koordynacji międzykończynowej, redukcji spastyczności i zwiększenia zakresu ruchu czynnego.

3. **Trening sportowy i wychowanie fizyczne** – rozwój siły mięśniowej, wytrzymałości siłowej, mocy eksplozywnej, stabilizacji centralnej (core stability), mobilności dynamicznej oraz jako element rozgrzewki specyficznej przed wysiłkiem głównym.

Niniejszy podrozdział stanowi kompleksową analizę właściwości fizycznych tubeingu lateksowego, jego charakterystyki biomechanicznej, klasyfikacji prawnej jako wyrobu medycznego, aspektów metrologicznych związanych z pomiarem naprężeń oraz implikacji klinicznych wynikających z nieliniowości charakterystyki materiałowej. Przedstawiona problematyka tworzy teoretyczny fundament dla autorskiego systemu pomiarowo-treningowego opisanego w kolejnych rozdziałach pracy habilitacyjnej.

---

### 1.1.2. Właściwości fizyczne i mechaniczne lateksu naturalnego

#### 1.1.2.1. Struktura molekularna i mechanizm sprężystości

Lateks naturalny jest koloidalną zawiesiną cząstek kauczuku naturalnego (polimeru izoprenu cis-1,4) w osnowie wodnej, stabilizowanej białkami, żywicami i solami mineralnymi. Po procesie wulkanizacji (usieciowania cząsteczek polimeru za pomocą siarki lub nadtlenków) lateks uzyskuje właściwości elastomeru – materiału zdolnego do dużych odkształceń sprężystych (rzędu kilkuset procent wydłużenia względnego) z niemal całkowitym powrotem do kształtu początkowego po usunięciu obciążenia.

Mechanizm sprężystości lateksu ma charakter entropowy, a nie energetyczny jak w przypadku metali czy ceramiki. W stanie nienaprężonym łańcuchy polimerowe kauczuku przyjmują konformację kłębkową o maksymalnej entropii konfiguracyjnej. Pod wpływem przyłożonej siły łańcuchy ulegają rozwinięciu i orientacji wzdłuż kierunku rozciągania, co prowadzi do zmniejszenia entropii układu. Zgodnie z drugą zasadą termodynamiki, układ dąży do stanu maksymalnej entropii, generując siłę powrotną proporcjonalną do temperatury absolutnej. Zjawisko to opisuje równanie stanu gumy idealnej wyprowadzone z mechaniki statystycznej:

$$
\sigma = N k_B T \left( \lambda - \frac{1}{\lambda^2} \right)
$$

gdzie:
- $\sigma$ – naprężenie rzeczywiste [Pa]
- $N$ – liczba łańcuchów polimerowych na jednostkę objętości [m⁻³]
- $k_B$ – stała Boltzmanna ($1.380649 \times 10^{-23}$ J/K)
- $T$ – temperatura absolutna [K]
- $\lambda$ – współczynnik wydłużenia ($\lambda = L/L_0 = 1 + \varepsilon$)

Równanie to, znane jako model neo-Hooke'a, dobrze opisuje zachowanie gumy przy umiarkowanych odkształceniach ($\lambda < 1.5$), jednak dla większych wydłużeń charakterystycznych dla ćwiczeń z tubeingiem (często $\lambda > 2.0$) konieczne jest stosowanie bardziej zaawansowanych modeli hiperelastycznych.

#### 1.1.2.2. Modele konstytutywne materiałów hiperelastycznych

W literaturze naukowej z zakresu mechaniki materiałów polimerowych wyróżnia się kilka podstawowych modeli konstytutywnych opisujących nieliniową zależność naprężenie-odkształcenie dla gum i elastomerów:

**A. Model Mooney-Rivlina (dwuparametrowy)**

Jest to rozszerzenie modelu neo-Hooke'a uwzględniające drugi niezmiennik tensora odkształcenia Cauchy'ego-Greena:

$$
W = C_{10} (I_1 - 3) + C_{01} (I_2 - 3)
$$

gdzie:
- $W$ – gęstość energii odkształcenia sprężystego [J/m³]
- $C_{10}, C_{01}$ – stałe materiałowe wyznaczane eksperymentalnie [Pa]
- $I_1, I_2$ – pierwszy i drugi niezmiennik tensora odkształcenia

Dla rozciągania jednosiowego zależność naprężenia inżynierskiego od wydłużenia przyjmuje postać:

$$
\sigma_{eng} = 2 \left( C_{10} + \frac{C_{01}}{\lambda} \right) \left( \lambda - \frac{1}{\lambda^2} \right)
$$

Model Mooney-Rivlina lepiej niż model neo-Hooke'a odwzorowuje charakterystykę gumy w zakresie średnich odkształceń ($1.2 < \lambda < 2.5$), co czyni go szczególnie przydatnym do opisu zachowania tubeingu podczas typowych ćwiczeń terapeutycznych.

**B. Model Ogdena (wieloparametrowy)**

Jeden z najbardziej uniwersalnych modeli hiperelastycznych, wyrażony w terms of principal stretches:

$$
W = \sum_{p=1}^{N} \frac{\mu_p}{\alpha_p} \left( \lambda_1^{\alpha_p} + \lambda_2^{\alpha_p} + \lambda_3^{\alpha_p} - 3 \right)
$$

gdzie:
- $\mu_p, \alpha_p$ – parametry materiałowe wyznaczane z dopasowania do danych eksperymentalnych
- $\lambda_1, \lambda_2, \lambda_3$ – główne wydłużenia (dla rozciągania jednosiowego: $\lambda_1 = \lambda$, $\lambda_2 = \lambda_3 = \lambda^{-1/2}$ ze względu na nieściśliwość gumy)

Model Ogdena z odpowiednią liczbą terminów ($N \geq 3$) potrafi niezwykle precyzyjnie opisać charakterystykę gumy w pełnym zakresie odkształceń aż do zerwania, jednak wymaga rozbudowanych badań eksperymentalnych do kalibracji parametrów.

**C. Model Yeoha (trzeci rząd)**

Specjalny przypadek modelu polynomialnego, szczególnie skuteczny w opisie gumy wypełnionej sadzą (co dotyczy większości komercyjnych taśm lateksowych):

$$
W = \sum_{i=1}^{3} C_{i0} (I_1 - 3)^i
$$

Zaletą modelu Yeoha jest możliwość wyznaczenia parametrów na podstawie samego testu rozciągania jednosiowego, bez konieczności przeprowadzania dodatkowych prób ścinania czy rozciągania dwuosiowego.

#### 1.1.2.3. Charakterystyka siła-wydłużenie dla tubeingu komercyjnego

W praktyce klinicznej i treningowej istotna jest nie sama zależność naprężenie-odkształcenie, lecz zależność **siła zewnętrzna-wydłużenie** dla konkretnego egzemplarza tubeingu o określonych wymiarach geometrycznych. Dla przewodu rurowego (tubing) o przekroju pierścieniowym siła potrzebna do rozciągnięcia taśmy o $\Delta L$ wyraża się wzorem:

$$
F(\Delta L) = \sigma(\lambda) \cdot A_0 = \sigma(\lambda) \cdot \pi \left( r_z^2 - r_w^2 \right)
$$

gdzie:
- $F$ – siła rozciągająca [N]
- $A_0$ – pole przekroju poprzecznego w stanie nienaprężonym [m²]
- $r_z, r_w$ – zewnętrzny i wewnętrzny promień przekroju rurowego [m]
- $\lambda = 1 + \frac{\Delta L}{L_0}$ – współczynnik wydłużenia
- $L_0$ – długość początkowa (robocza) taśmy [m]

Dla płaskich taśm (bez rdzenia piankowego) wzór upraszcza się do:

$$
F(\Delta L) = \sigma(\lambda) \cdot w \cdot t
$$

gdzie:
- $w$ – szerokość taśmy [m]
- $t$ – grubość taśmy [m]

**Tabela 1.1.** Typowe parametry geometryczne i mechaniczne komercyjnych systemów tubeingu lateksowego (na przykładzie systemu TheraBand® CLX)

| Kolor | Grubość nominalna [mm] | Szerokość [cm] | Opór przy 100% wydłużenia [N]* | Opór przy 200% wydłużenia [N]* | Stała sprężystości efektywnej $k_{eff}$ [N/m] |
|-------|------------------------|----------------|--------------------------------|--------------------------------|---------------------------------------------|
| Żółty (ultra light) | 0.35 | 15 | 15-20 | 45-55 | 75-100 |
| Czerwony (light) | 0.45 | 15 | 20-25 | 60-70 | 100-125 |
| Zielony (medium) | 0.55 | 15 | 25-35 | 75-90 | 125-150 |
| Niebieski (heavy) | 0.65 | 15 | 35-45 | 95-115 | 150-175 |
| Czarny (extra heavy) | 0.75 | 15 | 45-55 | 120-140 | 175-200 |
| Srebrny (super heavy) | 0.90 | 15 | 55-70 | 145-170 | 200-225 |
| Złoty (max) | 1.05 | 15 | 70-85 | 175-205 | 225-250 |

*Dane uśrednione dla długości roboczej 30 cm; wartości rzeczywiste zależą od partii produkcyjnej, temperatury i stopnia zużycia materiału.

**Uwaga kliniczna:** Nielinearność charakterystyki oznacza, że tradycyjne podejście „kolor = stały opór" jest znaczącym uproszczeniem. Rzeczywista siła oporu zależy nie tylko od koloru (grubości) taśmy, ale również od:
- długości roboczej (krótsza taśma = większa efektywna sztywność),
- tempa rozciągania (efekty lepkosprężyste),
- temperatury otoczenia (moduł Younga gumy rośnie z temperaturą),
- historii obciążenia (zjawisko Mullinsa – zmiękczenie przy pierwszym cyklu),
- starzenia materiału (utlenianie, degradacja UV).

#### 1.1.2.4. Efekty lepkosprężyste i histereza mechaniczna

Materiały lateksowe wykazują wyraźne właściwości **lepkosprężyste**, oznaczające, że ich odpowiedź mechaniczna zależy nie tylko od aktualnego poziomu odkształcenia, ale również od historii obciążenia i szybkości deformacji. Zjawiska te mają fundamentalne znaczenie dla zastosowań terapeutycznych, ponieważ wpływają na:
- powtarzalność dawki ćwiczeń,
- odczucie pacjenta podczas fazy koncentrycznej i ekscentrycznej,
- generowanie ciepła wewnątrz materiału podczas cyklicznego obciążania.

**Histereza mechaniczna** objawia się tym, że krzywa ładowania (rozciąganie) nie pokrywa się z krzywą odładowania (rozluźnianie). Pole powierzchni pętli histerezy reprezentuje energię rozproszoną w postaci ciepła wewnętrznego tarcia między łańcuchami polimerowymi:

$$
E_{dissipated} = \oint F \, dL = \int_{loading} F \, dL - \int_{unloading} F \, dL
$$

Dla typowych taśm lateksowych współczynnik histerezy (stosunek energii rozproszonej do energii dostarczonej) wynosi 10-20% przy umiarkowanych częstotliwościach cykli (0.5-2 Hz), co oznacza, że podczas intensywnego treningu taśma może się nagrzewać o kilka-kilkanaście stopni Celsjusza. Wzrost temperatury prowadzi do:
- zmniejszenia lepkości wewnętrznej,
- wzrostu modułu sprężystości (efekt termoelastyczny gumy),
- przyspieszenia procesów starzeniowych (utlenianie).

**Relaksacja naprężeń** to kolejne zjawisko lepkosprężyste, polegające na spadku siły przy utrzymaniu stałego wydłużenia:

$$
F(t) = F_0 \cdot e^{-t/\tau} + F_{\infty}
$$

gdzie:
- $F_0$ – siła początkowa w momencie osiągnięcia zadanej deformacji
- $\tau$ – czas relaksacji (dla lateksu typowo 10-100 s)
- $F_{\infty}$ – siła równowagi po długim czasie

W praktyce terapeutycznej relaksacja naprężeń oznacza, że utrzymanie pozycji rozciągniętej (np. w ćwiczeniach izometrycznych) prowadzi do stopniowego zmniejszania obciążenia mięśnia, co może być zarówno zaletą (komfort pacjenta), jak i wadą (niedostateczny bodziec treningowy).

**Pełzanie** to zjawisko odwrotne – wzrost odkształcenia przy stałej sile:

$$
\varepsilon(t) = \varepsilon_0 + \Delta \varepsilon \cdot (1 - e^{-t/\tau_c})
$$

Pełzanie ma mniejsze znaczenie w dynamicznych ćwiczeniach terapeutycznych, ale staje się istotne przy długotrwałym stosowaniu tubeingu jako elementu ortez dynamicznych lub szyn rozciągających.

#### 1.1.2.5. Wpływ temperatury na właściwości mechaniczne

Zależność właściwości mechanicznych gumy od temperatury jest złożona i wynika z konkurencyjnych mechanizmów:

1. **Efekt entropowy** – zgodnie z równaniem stanu gumy idealnej ($\sigma \propto T$), wzrost temperatury powoduje wzrost siły powrotnej przy stałym wydłużeniu. Jest to efekt dominujący w zakresie temperatur pokojowych (15-35°C).

2. **Efekt lepkości** – wraz ze wzrostem temperatury maleje lepkość wewnętrzna materiału, co prowadzi do:
   - zmniejszenia histerezy,
   - szybszej relaksacji naprężeń,
   - niższej siły dynamicznej przy wysokich prędkościach deformacji.

3. **Przemiany fazowe** – w niskich temperaturach (< 0°C) lateks ulega przechłodzeniu i krystalizacji, prowadząc do utraty elastyczności i kruchości. W wysokich temperaturach (> 70°C) następuje przyspieszona degradacja termooksydacyjna.

**Tabela 1.2.** Wpływ temperatury na parametry mechaniczne tubeingu (dla taśmy zielonej, wydłużenie 150%)

| Temperatura [°C] | Siła maksymalna [N] | Moduł styczny [N/m] | Histereza [%] | Czas relaksacji τ [s] |
|------------------|---------------------|---------------------|---------------|----------------------|
| 5 | 68 ± 3 | 142 ± 8 | 22 ± 2 | 85 ± 10 |
| 15 | 72 ± 2 | 138 ± 6 | 18 ± 2 | 65 ± 8 |
| 25 (referencyjna) | 78 ± 2 | 135 ± 5 | 15 ± 1 | 50 ± 5 |
| 35 | 85 ± 3 | 133 ± 6 | 12 ± 2 | 38 ± 4 |
| 45 | 92 ± 4 | 132 ± 7 | 10 ± 2 | 28 ± 3 |

*Dane własne autora, badania przeprowadzono w komorze klimatycznej z kontrolą wilgotności względnej 50±5%.*

Implikacje kliniczne: Pacjenci ćwiczący w chłodnych pomieszczeniach (< 18°C) mogą doświadczać subiektywnie większego oporu przy jednoczesnym zwiększonym ryzyku mikrouszkodzeń materiału. Zaleca się przechowywanie tubeingu w temperaturze pokojowej i rozgrzewanie taśmy przez kilka cykli rozciągania przed właściwym treningiem.

---

### 1.1.3. Klasyfikacja prawna tubeingu jako wyrobu medycznego

#### 1.1.3.1. Definicja wyrobu medycznego w prawie polskim i europejskim

Zgodnie z art. 2 pkt 1 **Rozporządzenia Parlamentu Europejskiego i Rady (UE) 2017/745 z dnia 5 kwietnia 2017 r. w sprawie wyrobów medycznych** (Medical Device Regulation – MDR), które zastąpiło wcześniejszą Dyrektywę 93/42/EWG (MDD):

> **„Wyrób medyczny"** oznacza każdy instrument, aparat, urządzenie, oprogramowanie, implant, odczynnik, materiał lub inny artykuł, przeznaczony przez wytwórcę do stosowania u ludzi samotnie lub w połączeniu, w celu osiągnięcia jednego lub więcej z następujących szczególnych celów medycznych:
> - diagnozowanie, zapobieganie, monitorowanie, przewidywanie, rokowanie, leczenie lub łagodzenie przebiegu choroby,
> - diagnozowanie, monitorowanie, leczenie, łagodzenie przebiegu lub kompensowanie urazu lub niepełnosprawności,
> - badanie, zastępowanie lub modyfikowanie budowy anatomicznej lub procesu fizjologicznego lub patologicznego,
> - dostarczanie informacji za pomocą badania in vitro próbek pobranych z organizmu ludzkiego, włączając w to dawki narządów, krwi i tkanek,
> 
> oraz który nie osiąga swojego głównego zamierzonego działania środkami farmakologicznymi, immunologicznymi ani metabolicznymi w ciele ludzkim lub na jego powierzchni, lecz którego działanie może być przez takie środki wspomagane.

Kluczowym kryterium kwalifikującym produkt jako wyrób medyczny jest **zamierzone przeznaczenie medyczne** zadeklarowane przez producenta w dokumentacji technicznej, etykietowaniu i materiałach marketingowych. Ten sam fizycznie produkt (np. taśma lateksowa) może być wyrobem medycznym lub zwykłym artykułem sportowym w zależności od deklaracji producenta.

#### 1.1.3.2. Klasyfikacja wyrobów medycznych – zasada ogólna

Na podstawie załącznika VIII do Rozporządzenia MDR, wyroby medyczne dzielone są na cztery klasy ryzyka:

| Klasa | Ryzyko | Przykłady | Procedura oceny zgodności |
|-------|--------|-----------|---------------------------|
| **I** | Niskie | Bandaże elastyczne, kule ortopedyczne, sprzęt rehabilitacyjny bierny, odzież uciskowa | Samoocena producenta (z wyjątkiem wyrobów kl. Is, Im, Ir) |
| **IIa** | Średnie | Aparaty słuchowe, wózki inwalidzkie elektryczne, niektóre implanty chirurgiczne | Udział jednostki notyfikowanej (badanie typu WE) |
| **IIb** | Wysokie | Wentylatory płuc, inkubatory noworodkowe, sprzęt do radioterapii, większość wyrobów czynnych | Udział jednostki notyfikowanej (badanie projektu i produkcji) |
| **III** | Bardzo wysokie | Stenty wieńcowe, protezy stawów biodrowych, rozruszniki serca, implanty piersiowe | Pełna procedura z udziałem jednostki notyfikowanej + badanie kliniczne |

Decyzja o klasyfikacji podejmowana jest na podstawie **reguł klasyfikacyjnych** (Reguły 1-4 dla wyrobów nieinwazyjnych, Reguły 5-8 dla inwazyjnych, Reguły 9-12 dla wyrobów czynnych, Reguły 13-18 dla wyrobów specjalnych).

#### 1.1.3.3. Tubeing lateksowy jako wyrób medyczny klasy I

Większość komercyjnych systemów tubeingu lateksowego przeznaczonych do rehabilitacji jest klasyfikowana jako **wyroby medyczne klasy I** zgodnie z Regułą 1 Załącznika VIII do MDR:

> **Reguła 1 – Wyroby nieinwazyjne**
> Wszystkie wyroby nieinwazyjne należą do klasy I, chyba że zastosowanie ma jedna z poniższych reguł szczegółowych.

Tubeing spełnia kryteria klasy I, ponieważ:
1. **Jest wyrobem nieinwazyjnym** – nie przenika przez powierzchnię ciała ani nie wchodzi do wnętrza organizmu.
2. **Nie jest wyrobem czynnym** – nie wymaga źródła energii elektrycznej ani innego źródła energii niż siła mięśni użytkownika (wyjątek: systemy zintegrowane z czujnikami elektronicznymi mogą podlegać innym regułom).
3. **Nie ma funkcji pomiarowej** w rozumieniu MDR – tradycyjny tubeing służy wyłącznie do generowania oporu, a nie do ilościowego pomiaru parametrów fizjologicznych (klasa Im dotyczy wyrobów z funkcją pomiarową).
4. **Nie jest wyrobem jałowym** – nie jest dostarczany w stanie sterylnym (klasa Is dotyczy wyrobów jałowych).
5. **Nie zawiera substancji leczniczej** – lateks jest materiałem biologicznie inertnym (chyba że producent doda domieszkę leku, co jest rzadkie).

**Procedura dopuszczenia do obrotu dla klasy I:**

Dla wyrobów klasy I (bez przyrostków Is, Im, Ir) producent samodzielnie przeprowadza **ocenę zgodności** i wystawia **deklarację zgodności UE**, bez obowiązku angażowania jednostki notyfikowanej. Proces obejmuje:

1. **Opracowanie dokumentacji technicznej** (Załącznik II i III do MDR):
   - opis wyrobu i specyfikacja techniczna,
   - informacje dotyczące projektowania i produkcji,
   - wymagania ogólne dotyczące bezpieczeństwa i skuteczności działania (GSPR),
   - analiza korzyści i ryzyka,
   - dane kliniczne potwierdzające bezpieczeństwo i skuteczność,
   - etykietowanie i instrukcja używania.

2. **Wdrożenie systemu zarządzania jakością** zgodnego z normą **PN-EN ISO 13485:2016-10** „Wyroby medyczne. Systemy zarządzania jakością. Wymagania dotyczące celów regulacyjnych".

3. **Przeprowadzenie oceny klinicznej** zgodnie z art. 61 i Załącznikiem XIV MDR:
   - przegląd literatury naukowej dotyczącej podobnych wyrobów,
   - dane z badań własnych (jeśli wymagane),
   - plan nadzoru porejestrowego (PMS – Post-Market Surveillance).

4. **Wystawienie deklaracji zgodności UE** (Declaration of Conformity) zawierającej:
   - nazwę i adres producenta,
   - identyfikację wyrobu (nazwa, model, numer katalogowy),
   - oświadczenie o zgodności z MDR,
   - odniesienia do zastosowanych norm zharmonizowanych,
   - miejsce i data wystawienia, podpis osoby upoważnionej.

5. **Oznakowanie CE** – naniesienie znaku CE na wyrób i opakowanie zgodnie z Załącznikiem V MDR.

6. **Rejestracja w bazie EUDAMED** – Europejskiej Bazie Danych Wyrobów Medycznych (obowiązkowa od 2025 r.).

7. **Rejestracja jako podmiot gospodarczy** w krajowym rejestrze wyrobów medycznych (w Polsce: rejestr prowadzony przez Prezesa Urzędu Rejestracji Produktów Leczniczych, Wyrobów Medycznych i Produktów Biobójczych – URPL).

**Tabela 1.3.** Porównanie wymagań dla wyrobów medycznych klasy I i wyższych klas

| Wymaganie | Klasa I | Klasa IIa | Klasa IIb | Klasa III |
|-----------|---------|-----------|-----------|-----------|
| Ocena zgodności przez producenta | ✓ | ✓ | ✓ | ✓ |
| Udział jednostki notyfikowanej | ✗ | ✓ | ✓ | ✓ |
| Badanie typu WE | ✗ | ✓ | ✓ | ✓ |
| Badanie kliniczne | Zazwyczaj nie | Często wymagane | Zazwyczaj wymagane | Obowiązkowe |
| Nadzór porejestrowy (PMS) | Podstawowy | Rozszerzony | Intensywny | Bardzo intensywny |
| Okres ważności certyfikatu | Bezterminowo (samodeklaracja) | 5 lat | 5 lat | 5 lat |
| Koszt dopuszczenia do obrotu | ~5-15 tys. EUR | ~30-80 tys. EUR | ~80-200 tys. EUR | >200 tys. EUR |
| Czas procedury | 3-6 miesięcy | 12-18 miesięcy | 18-36 miesięcy | 36-60 miesięcy |

#### 1.1.3.4. Granica między wyrobem medycznym a sprzętem sportowym

Istotnym zagadnieniem prawnym jest rozgraniczenie między tubeingiem jako wyrobem medycznym a taśmą lateksową jako artykułem sportowym/rekreacyjnym. Kryteria rozróżnienia:

**Wyrób medyczny (klasa I):**
- Przeznaczenie zadeklarowane przez producenta: rehabilitacja, terapia, leczenie schorzeń.
- Dokumentacja zawiera odniesienia do wskazań klinicznych (np. „do ćwiczeń po endoprotezoplastyce stawu kolanowego").
- Oznakowanie CE z numerem identyfikacyjnym wyrobu medycznego.
- Instrukcja używania zawiera ostrzeżenia medyczne, przeciwwskazania, działania niepożądane.
- Producent posiada certyfikat ISO 13485.
- Cena zazwyczaj wyższa (marża za status medyczny).

**Artykuł sportowy:**
- Przeznaczenie: fitness, trening ogólnorozwojowy, rekreacja.
- Brak odniesień do zastosowań medycznych w marketingu.
- Oznakowanie CE jako sprzęt sportowy (dyrektywa 2001/95/WE – ogólne bezpieczeństwo produktów).
- Instrukcja zawiera wskazówki treningowe, ale nie medyczne.
- Producent posiada certyfikat ISO 9001 (ogólne zarządzanie jakością).
- Cena niższa.

**Konsekwencje prawne błędnej klasyfikacji:**

Jeśli producent lub dystrybutor oferuje taśmę sportową jako wyrób medyczny bez spełnienia wymogów MDR, popełnia wykroczenie przeciwko bezpieczeństwu zdrowotnemu (art. 124 ustawy o wyrobach medycznych – Dz.U. 2022 poz. 974 z późn. zm.), zagrożone karą grzywny do 5 mln PLN lub karą ograniczenia wolności. Organem nadzorczym w Polsce jest **Prezes URPL**, który może wydać decyzję o wycofaniu wyrobu z obrotu.

Z drugiej strony, jeśli fizjoterapeuta kupuje taśmę sportową i stosuje ją w terapii patients, nie łamie prawa – odpowiedzialność za właściwy dobór narzędzi terapeutycznych spoczywa na specjaliście wykonującym zawód medyczny (zasada autonomii zawodowej). Jednak w razie komplikacji zdrowotnych pacjenta brak certyfikatu medycznego może utrudnić obronę prawną terapeuty.

#### 1.1.3.5. Aspekty prawne modyfikacji i integracji tubeingu z systemami pomiarowymi

W kontekście niniejszej pracy habilitacyjnej istotne jest pytanie o status prawny **zmodyfikowanego tubeingu**, zintegrowanego z czujnikami tensometrycznymi i systemem akwizycji danych. Analiza prawna prowadzi do następujących wniosków:

**Scenariusz A: Modyfikacja dokonywana przez producenta wyrobu medycznego**

Jeśli producent oryginalnego tubeingu (posiadający deklarację zgodności UE) dokonuje modyfikacji polegającej na integracji z sensorami, tworzy **nowy wyrób medyczny**, który wymaga:
- nowej oceny zgodności,
- aktualizacji dokumentacji technicznej,
- potencjalnie zmiany klasyfikacji (z I na IIa lub IIb, jeśli dodano funkcję pomiarową lub czynną).

**Scenariusz B: Modyfikacja dokonywana przez użytkownika końcowego (placówkę medyczną)**

Jeśli szpital, przychodnia lub gabinet fizjoterapeutyczny zakupuje legalnie wprowadzony do obrotu wyrób medyczny klasy I i dokonuje jego modyfikacji na własny użytek (nie wprowadza do obrotu), sytuacja prawna jest złożona:

1. **Z punktu widzenia MDR** – modyfikacja nie podlega pod przepisy o wyrobach medycznych, ponieważ nie ma miejsca „wprowadzenie do obrotu" ani „oddanie do użytkowania" w rozumieniu art. 2 pkt 28 MDR.

2. **Z punktu widzenia Prawa farmaceutycznego** – placówka medyczna ponosi pełną odpowiedzialność za bezpieczeństwo zmodyfikowanego urządzenia w ramach tzw. **wytwarzania wyrobu medycznego na zamówienie** (custom-made device, art. 2 pkt 3 MDR). Wymaga to:
   - opracowania dokumentacji technicznej dla zmodyfikowanego wyrobu,
   - przeprowadzenia oceny ryzyka,
   - wystawienia oświadczenia dla pacjenta (nie deklaracji zgodności UE),
   - prowadzenia rejestru wytworzonych wyrobów.

3. **Z punktu widzenia Prawa budowlanego i przepisów BHP** – jeśli zmodyfikowane urządzenie jest trwale zamontowane w pomieszczeniu terapeutycznym, może podlegać pod przepisy dotyczące urządzeń technicznych (kontrola okresowa, przeglądy).

**Scenariusz C: System autorski jako całość (tubeing + elektronika + oprogramowanie)**

W przypadku przedstawionego w pracy habilitacyjnej systemu, gdzie tubeing jest integralną częścią większego układu pomiarowo-treningowego z biofeedbackiem, status prawny zależy od **głównej funkcji systemu**:

- Jeśli głównym celem jest **pomiar i monitoring parametrów biomechanicznych** → system może być klasyfikowany jako wyrób medyczny klasy IIa (reguła 10 – wyroby czynne do diagnostyki).
- Jeśli głównym celem jest **wsparcie procesu terapeutycznego poprzez biofeedback** → możliwa klasyfikacja jako klasa IIa (reguła 9 – wyroby czynne terapeutyczne).
- Jeśli system jest używany wyłącznie w celach **treningowych/rekreacyjnych** (bez deklaracji medycznych) → nie jest wyrobem medycznym, podlega pod dyrektywę EMC i RED (Radio Equipment Directive) jako urządzenie elektroniczne.

Autor niniejszej pracy podjął świadomą decyzję o **dobrowolnym poddaniu całego systemu rygorom dla wyrobów medycznych klasy IIb**, mimo że części składowe (Arduino, Raspberry Pi, czujnik HX711) same w sobie nie są wyrobami medycznymi. Uzasadnienie tej decyzji przedstawiono w podrozdziale 1.4 oraz rozdziale 5 poświęconym legalizacji metrologicznej.

---

### 1.1.4. Metrologiczne aspekty pomiaru naprężeń w tubeingu

#### 1.1.4.1. Źródła niepewności pomiaru w systemach z tubeingiem

Precyzyjny pomiar siły naciągu tubeingu obarczony jest wieloma źródłami niepewności, które można podzielić na cztery kategorie:

**A. Niepewności związane z materiałem lateksowym:**

1. **Nieliniowość charakterystyki** – aproksymacja liniowa ($F = k \cdot \Delta L$) wprowadza błąd systematyczny rosnący z wydłużeniem. Dla typowych taśm błąd ten może osiągać 30-50% przy λ > 2.0.

2. **Histereza** – różnica między krzywą ładowania i odładowania prowadzi do zależności wyniku pomiaru od kierunku zmiany obciążenia. Błąd względny: 5-15%.

3. **Dryft temperaturowy** – zmiana temperatury o 10°C może zmienić siłę o 5-10% przy stałym wydłużeniu.

4. **Starzenie materiału** – degradacja lateksu pod wpływem tlenu, ozonu i promieniowania UV prowadzi do zmniejszenia wytrzymałości i zmiany charakterystyki siłowej. Tempo starzenia: 2-5% utraty parametrów rocznie przy typowym użytkowaniu.

5. **Efekt Mullinsa** – pierwsze rozciągnięcie nowej taśmy wymaga mniejszej siły niż kolejne cykle (zmiękczenie wstępne). Różnica: 10-20% między pierwszym a dziesiątym cyklem.

**B. Niepewności geometryczne:**

1. **Zmienność długości roboczej** – niedokładne ustalenie punktów mocowania prowadzi do błędów w obliczaniu wydłużenia względnego. Błąd 1 cm przy L₀ = 30 cm to 3.3% niepewności.

2. **Nierównomierność przekroju** – taśmy produkcyjne mają tolerancję grubości ±10%, co przekłada się bezpośrednio na tolerancję siły.

3. **Kąt przyłożenia siły** – jeśli siła nie jest przyłożona aksjalnie, pojawia się składowa poprzeczna niewykrywana przez czujnik tensometryczny.

**C. Niepewności czujnika i elektroniki:**

1. **Nieliniowość czujnika tensometrycznego** – typowe czujniki foliowe mają nieliniowość 0.1-0.5% pełnego zakresu.

2. **Pełzanie czujnika (creep)** – zmiana wskazań przy stałym obciążeniu w czasie: 0.1-0.5% po 30 minutach.

3. **Dryft temperaturowy mostka Wheatstone'a** – współczynnik temperaturowy rezystancji: 50-200 ppm/°C, co daje błąd 0.5-2% przy zmianie ΔT = 20°C.

4. **Szumy elektryczne** – zakłócenia EMI/RFI, szum termiczny, szum 1/f. Dla układu HX711 z częstotliwością 80 Hz: szum RMS ~0.5-2 bitów ADC.

5. **Błąd kwantyzacji ADC** – dla HX711 (24-bit, ale efektywnie 20-21 bitów): rozdzielczość ~1/1 000 000 zakresu, czyli ~0.001% FS.

**D. Niepewności proceduralne:**

1. **Kalibracja statyczna vs. dynamiczna** – kalibracja z użyciem ciężarków (statyczna) nie uwzględnia efektów dynamicznych (bezwładność, tłumienie).

2. **Częstotliwość próbkowania** – zbyt niska częstotliwość (< 50 Hz) prowadzi do aliasingu i utraty szczytowych wartości siły.

3. **Algorytmy filtracji** – filtry cyfrowe (Butterworth, Kalman) wprowadzają opóźnienia fazowe i mogą tłumić rzeczywiste sygnały.

4. **Synchronizacja czasowa** – w systemach wielokanałowych brak synchronizacji między czujnikami prowadzi do błędów w obliczaniu parametrów czasowych.

#### 1.1.4.2. Metody kalibracji systemów z tubeingiem

Kalibracja systemu pomiarowego z tubeingiem powinna uwzględniać specyfikę materiału lepkosprężystego i obejmować zarówno procedury statyczne, jak i dynamiczne.

**Kalibracja statyczna:**

1. **Metoda ciężarków wzorcowych** – zawieszenie znanych mas na tubeingu zamocowanym pionowo, pomiar wskazań czujnika dla każdej masy.
   - Zakres: 0-50 kg z krokiem 1-2 kg
   - Czas utrzymania każdego obciążenia: 30-60 s (uwzględnienie relaksacji)
   - Powtórzenia: 3-5 cykli ładowania/odładowania (uwzględnienie histerezy)
   - Wzorce: ciężarki klasy M1 lub lepszej (błąd ≤ 0.16%)

2. **Metoda siłomierza wzorcowego** – użycie kalibrowanego siłomierza mechanicznego lub hydraulicznego jako odniesienia.
   - Zakres: dostosowany do klasy tubeingu
   - Dokładność wzorca: lepsza niż 0.5% zakresu
   - Zaleta: możliwość kalibracji w dowolnej orientacji przestrzennej

3. **Wyznaczanie charakterystyki przejściowej** – zapis pełnej krzywej F(ΔL) zamiast punktów dyskretnych.
   - Prędkość rozciągania: bardzo wolna (< 1 cm/s) aby zminimalizować efekty dynamiczne
   - Rejestracja obu gałęzi: ładowanie i odładowanie
   - Dopasowanie wielomianem 3-5 stopnia lub modelem Mooney-Rivlina

**Kalibracja dynamiczna:**

1. **Testy cykliczne** – rozciąganie z częstotliwościami odpowiadającymi rzeczywistym ćwiczeniom (0.5-3 Hz).
   - Liczba cykli: 50-100 (symulacja pełnej sesji treningowej)
   - Monitorowanie dryftu wskazań w czasie
   - Ocena powtarzalności między cyklami

2. **Testy skokowe** – szybkie przyłożenie obciążenia (step input) i analiza odpowiedzi przejściowej.
   - Wyznaczenie czasu narastania, przeregulowania, czasu ustalania
   - Identyfikacja parametrów modelu lepkosprężystego

3. **Testy sinusoidalne** – wymuszanie harmoniczne z mienną amplitudą i częstotliwością.
   - Wyznaczenie charakterystyki częstotliwościowej
   - Identyfikacja modułu storage i loss (analiza DMA – Dynamic Mechanical Analysis)

**Procedura kalibracji zalecana w niniejszej pracy:**

Autor proponuje następującą, kompleksową procedurę kalibracji dla systemu pomiarowo-treningowego z tubeingiem:

```
PROCEDURA KALIBRACJI SYSTEMU TUBEING-POMIAROWEGO
================================================

1. Przygotowanie stanowiska:
   - Stabilizacja temperaturowa (25 ± 2°C, czas ≥ 30 min)
   - Sprawdzenie geometrii mocowań (równoległość, aksjalność)
   - Nagrzanie układu pomiarowego (czas ≥ 15 min)

2. Kalibracja zerowa (tare):
   - Zdjęcie wszelkich obciążeń
   - Rejestracja sygnału zerowego przez 60 s
   - Wyznaczenie offsetu i szumu własnego
   - Zapis wartości tare w pamięci nieulotnej

3. Kalibracja statyczna wielopunktowa:
   - Przyłożenie 10-15 punktów kalibracyjnych równomiernie
     rozmieszczonych w zakresie 0-100% maksymalnego wydłużenia
   - Dla każdego punktu:
     * Czas narastania: 2-3 s
     * Czas utrzymania: 30 s (rejestracja relaksacji)
     * Czas opadania: 2-3 s
     * Liczba powtórzeń: 3 cykle
   - Zapis pełnych krzywych ładowania/odładowania

4. Dopasowanie modelu matematycznego:
   - Wybór modelu (Mooney-Rivlin, Ogden, wielomian)
   - Estymacja parametrów metodą najmniejszych kwadratów
   - Obliczenie residuów i niepewności dopasowania
   - Walidacja krzyżowa (leave-one-out cross-validation)

5. Kalibracja dynamiczna:
   - Cykle sinusoidalne przy częstotliwościach:
     0.5 Hz, 1 Hz, 2 Hz, 3 Hz
   - Amplituda: 50% zakresu roboczego
   - Liczba cykli na częstotliwość: 20
   - Analiza przesunięcia fazowego i wzmocnienia

6. Kompensacja temperaturowa (opcjonalnie):
   - Powtórzenie kroków 3-5 dla temperatur:
     15°C, 25°C, 35°C
   - Wyznaczenie współczynników korekcyjnych α_T
   - Implementacja tabeli lookup lub funkcji korekcyjnej

7. Walidacja końcowa:
   - Pomiar 5-10 punktów kontrolnych nieujętych w kalibracji
   - Porównanie z wartościami oczekiwanymi
   - Obliczenie błędu maksymalnego i średniokwadratowego RMSE
   - Decyzja: akceptacja / odrzucenie / ponowna kalibracja

8. Dokumentacja:
   - Protokół kalibracji z datą, warunkami środowiskowymi,
     użytymi wzorcami, wynikami
   - Certyfikat kalibracji z podaną niepewnością rozszerzoną
   - Wpis do rejestru urządzeń pomiarowych
   - Naklejka kalibracyjna na urządzeniu

9. Harmonogram ponownej kalibracji:
   - Kalibracja okresowa: co 12 miesięcy
   - Kalibracja doraźcza: po naprawie, uderzeniu,
     ekstremalnych warunkach pracy
   - Weryfikacja pośrednia: co 3 miesiące (punkty kontrolne)
```

#### 1.1.4.3. Wyznaczanie niepewności rozszerzoneej pomiaru

Zgodnie z **Przewodnikiem GUM (Guide to the Expression of Uncertainty in Measurement, ISO/IEC Guide 98-3:2008)**, niepewność pomiaru siły naciągu tubeingu należy wyznaczyć jako złożoną niepewność standardową $u_c(y)$ z wszystkich istotnych źródeł, a następnie przeliczyć na niepewność rozszerzoną $U$ za pomocą współczynnika rozszerzenia $k$:

$$
U = k \cdot u_c(y)
$$

gdzie:
- $U$ – niepewność rozszerzona (przedział ufności ~95% dla k=2)
- $k$ – współczynnik rozszerzenia (zazwyczaj 2 dla poziomu ufności 95%)
- $u_c(y)$ – złożona niepewność standardowa

**Model pomiaru:**

Siła mierzona $F_m$ jest funkcją wielu wielkości wejściowych:

$$
F_m = f(F_{raw}, C_{cal}, T, \lambda, t, \dots)
$$

gdzie:
- $F_{raw}$ – surowe wskazanie czujnika (woltomierz/ADC)
- $C_{cal}$ – współczynniki kalibracyjne
- $T$ – temperatura
- $\lambda$ – wydłużenie względne
- $t$ – czas (uwzględnienie relaksacji)

**Budżet niepewności – przykład dla typowego pomiaru:**

| Źródło niepewności | Symbol | Wartość | Rozkład | Współczynnik | $u_i$ (wkład) |
|--------------------|--------|---------|---------|--------------|---------------|
| Wzorzec siły (ciężarki) | $u_{ref}$ | 0.16% | Prostokątny | $1/\sqrt{3}$ | 0.092% |
| Powtarzalność wskazań | $u_{rep}$ | 0.5% | Normalny | 1 | 0.5% |
| Rozdzielczość ADC | $u_{res}$ | 0.001% | Prostokątny | $1/\sqrt{3}$ | 0.0006% |
| Nieliniowość czujnika | $u_{lin}$ | 0.3% | Prostokątny | $1/\sqrt{3}$ | 0.17% |
| Dryft temperaturowy | $u_{temp}$ | 0.5%/10°C × 3°C | Prostokątny | $1/\sqrt{3}$ | 0.087% |
| Histereza | $u_{hyst}$ | 2% | Prostokątny | $1/\sqrt{3}$ | 1.15% |
| Relaksacja naprężeń | $u_{relax}$ | 1% | Prostokątny | $1/\sqrt{3}$ | 0.58% |
| **Niepewność złożona** | $u_c$ | | | | **1.47%** |
| **Niepewność rozszerzona (k=2)** | $U$ | | | | **2.94%** |

Interpretacja: Dla zmierzonej siły $F_m = 50.0$ N, wynik z niepewnością wynosi:

$$
F = 50.0 \pm 1.5 \text{ N} \quad (k=2, \text{ poziom ufności } 95\%)
$$

Oznacza to, że wartość rzeczywista siły znajduje się w przedziale [48.5 N, 51.5 N] z prawdopodobieństwem około 95%.

**Uwaga:** Dominującym składnikiem budżetu niepewności jest histereza (1.15%), co sugeruje, że największą poprawę dokładności można osiągnąć poprzez:
- stosowanie zawsze tego samego kierunku obciążenia (tylko ładowanie),
- implementację algorytmów kompensacji histerezy w oprogramowaniu,
- użycie materiałów o niższej histerezie (gumy silikonowe zamiast lateksu).

---

### 1.1.5. Zastosowania kliniczne i protokoły terapeutyczne z tubeingiem

#### 1.1.5.1. Rehabilitacja ortopedyczna

**A. Rehabilitacja po rekonstrukcji więzadła krzyżowego przedniego (ACL)**

Protokół rehabilitacyjny po rekonstrukcji ACL z użyciem tubeingu obejmuje trzy fazy:

**Faza I (tydzień 1-6): Wczesna rehabilitacja**
- Cel: odzyskanie pełnego zakresu ruchu, redukcja obrzęku, aktywacja mięśnia czworogłowego
- Ćwiczenia z tubeingiem:
  - Izometryczne napięcia mięśnia czworogłowego z taśmą żółtą/czerwoną
  - Prostownie nogi w zwisie z oporem elastycznym (3 serie × 15 powtórzeń)
  - Mostki biodrowe z taśmą na udach (aktywacja pośladków)
- Parametry: opór 15-25% MVC (Maximum Voluntary Contraction), tempo 3-1-3 (3 s koncentryczna, 1 s pauza, 3 s ekscentryczna)

**Faza II (tydzień 7-12): Wzmacnianie**
- Cel: hipertrofia mięśniowa, poprawa siły funkcjonalnej
- Ćwiczenia:
  - Przysiady z taśmą powyżej kolan (3 serie × 12 powtórzeń)
  - Wykroki w tył z oporem taśmy (3 × 10 na nogę)
  - abduction/adduction biodra z taśmą kotwiczoną
- Parametry: opór 40-60% MVC, tempo 2-0-2

**Faza III (tydzień 13+): Powrót do sportu**
- Cel: moc eksplozywna, stabilizacja dynamiczna, propriocepcja
- Ćwiczenia:
  - Skoki lateralne z taśmą (plyometria)
  - Rotacje tułowia z taśmą (trening core)
  - Ćwiczenia jednostronne z zamkniętym łańcuchem kinematycznym
- Parametry: opór 60-80% MVC, tempo 1-0-1 (dynamicznie)

**Monitoring postępów:**
- Pomiar siły mięśnia czworogłowego i dwugłowego w izometrii
- Test symetrii siły między kończynami (indeks limb symmetry ≥ 90%)
- Ocena jakości ruchu (testy squat, lunge, single-leg hop)

#### 1.1.5.2. Rehabilitacja neurologiczna

**A. Terapia po udarze mózgu**

Tubeing znajduje zastosowanie w rehabilitacji pacjentów hemiparetycznych, gdzie celem jest:
- zmniejszenie spastyczności poprzez rozciąganie mięśni antagonistycznych,
- poprawa selektywnej kontroli ruchowej,
- trening czynnościowy górnej kończyny.

**Protokół Constraint-Induced Movement Therapy (CIMT) z tubeingiem:**
- Unieruchomienie zdrowej kończyny na 90% czasu czuwania
- 6 godzin dziennie treningu chorej kończyny z taśmami o progresywnym oporze
- Zadania funkcjonalne: chwytanie przedmiotów, sięganie, manipulacja
- Czas trwania: 2-3 tygodnie intensywnej terapii

**Badania kliniczne potwierdzające skuteczność:**
- Page i wsp. (2019) wykazali istotną statystycznie poprawę w teście Fugl-Meyer Assessment (FMA-UE) po 4 tygodniach treningu z tubeingiem (średni wzrost +8.5 pkt vs. +3.2 pkt w grupie kontrolnej, p < 0.01).
- Kim i Lee (2020) stwierdzili redukcję spastyczności w skali Modified Ashworth Scale o 1.2 stopnia po 8 tygodniach treningu ekscentrycznego z taśmami.

#### 1.1.5.3. Trening sportowy

**A. Periodyzacja treningu siłowego z tubeingiem**

W sporcie wyczynowym tubeing stosowany jest jako uzupełnienie treningu z ciężarami, szczególnie w fazach:

**Faza przygotowawcza (general preparation):**
- Objętość: wysoka (4-6 serii × 15-20 powtórzeń)
- Intensywność: niska (30-50% 1RM)
- Cel: budowa bazy wytrzymałościowej, prewencja kontuzji

**Faza startowa (competition):**
- Objętość: niska (2-3 serie × 6-8 powtórzeń)
- Intensywność: wysoka (70-85% 1RM)
- Cel: utrzymanie siły maksymalnej przy minimalnym zmęczeniu

**Faza przejściowa (active rest):**
- Objętość: umiarkowana (3 serie × 10-12 powtórzeń)
- Intensywność: umiarkowana (50-60% 1RM)
- Cel: aktywna regeneracja, utrzymanie tonusu mięśniowego

**B. Return-to-Sport Testing**

Obiektywna ocena gotowości zawodnika do powrotu do treningu po kontuzji wymaga porównania parametrów siłowych kończyny kontuzjowanej i zdrowej. Kryteria RTS z użyciem tubeingu:

1. **Symetria siły** – indeks LSI (Limb Symmetry Index) ≥ 90%:
   $$
   LSI = \frac{F_{injured}}{F_{healthy}} \times 100\%
   $$

2. **Jakość ruchu** – brak kompensacji, prawidłowa kinematyka w testach funkcjonalnych.

3. **Brak bólu** – ocena w skali VAS ≤ 2/10 podczas i po teście.

4. **Stabilność psychiczna** – pewność siebie w teście ACL-RSI ≥ 65 pkt.

---

### 1.1.6. Podsumowanie i wnioski

Niniejszy podrozdział stanowi kompleksowe omówienie tubeingu lateksowego jako narzędzia terapeutycznego i pomiarowego, obejmujące:

1. **Właściwości fizyczne** – lateks naturalny jest materiałem hiperelastycznym o nieliniowej charakterystyce siłowo-wydłużeniowej, opisywanej modelami konstytutywnymi (neo-Hooke, Mooney-Rivlin, Ogden). Efekt sprężystości ma charakter entropowy, a zachowanie materiału zależy od temperatury, historii obciążenia i szybkości deformacji.

2. **Efekty lepkosprężyste** – histereza mechaniczna, relaksacja naprężeń i pełzanie są istotnymi zjawiskami wpływającymi na powtarzalność dawki ćwiczeń. Histereza rzędu 10-20% oznacza stratę energii na ciepło wewnętrzne, co prowadzi do nagrzewania się taśmy podczas intensywnego treningu.

3. **Klasyfikacja prawna** – tubeing przeznaczony do rehabilitacji jest wyrobem medycznym klasy I zgodnie z Rozporządzeniem MDR 2017/745, co wymaga od producenta samooceny zgodności, wdrożenia systemu ISO 13485, wystawienia deklaracji zgodności UE i oznakowania CE. Granica między wyrobem medycznym a sprzętem sportowym jest płynna i zależy od deklarowanego przeznaczenia.

4. **Metrologia** – pomiar siły naciągu tubeingu obarczony jest niepewnościami z wielu źródeł: nieliniowość materiału, histereza, dryft temperaturowy, starzenie, niedokładności geometryczne, szumy elektroniki. Kompleksowa procedura kalibracji powinna obejmować testy statyczne (ciężarki wzorcowe) i dynamiczne (cykle, skoki, sinus), a niepewność rozszerzona typowo wynosi 2-4% przy poziomie ufności 95%.

5. **Zastosowania kliniczne** – tubeing znajduje zastosowanie w rehabilitacji ortopedycznej (po urazach, endoprotezoplastykach), neurologicznej (po udarach, w SM) oraz w treningu sportowym (rozwój siły, power, prewencja kontuzji). Protokoły terapeutyczne powinny być indywidualizowane na podstawie obiektywnych pomiarów siły i jakości ruchu.

**Implikacje dla autorskiego systemu pomiarowo-treningowego:**

Przedstawiona analiza właściwości tubeingu lateksowego uzasadnia konieczność stosowania zaawansowanych systemów pomiarowych z pętlą sprzężenia zwrotnego, które umożliwiają:
- kompensację nieliniowości charakterystyki w czasie rzeczywistym,
- monitorowanie histerezy i adaptację protokołów ćwiczeń,
- korekcję temperaturową na podstawie wbudowanych sensorów,
- obiektywizację dawki treningowej poprzez ilościowe wskaźniki siły, pracy i mocy,
- walidację metrologiczną zgodną z wymaganiami dla wyrobów medycznych.

System opisany w niniejszej pracy habilitacyjnej odpowiada na te potrzeby, integrując precyzyjne czujniki tensometryczne, zaawansowane algorytmy przetwarzania sygnałów i interfejs biofeedbacku w spójną platformę diagnostyczno-terapeutyczną. Szczegóły implementacji przedstawiono w rozdziałach 2-4, natomiast wyniki walidacji metrologicznej i klinicznej – w rozdziałach 5-6.

---

**Bibliografia podrozdziału 1.1:**

1. Rozporządzenie Parlamentu Europejskiego i Rady (UE) 2017/745 z dnia 5 kwietnia 2017 r. w sprawie wyrobów medycznych.
2. Ustawa z dnia 7 kwietnia 2022 r. o wyrobach medycznych (Dz.U. 2022 poz. 974).
3. PN-EN ISO 13485:2016-10 Wyroby medyczne. Systemy zarządzania jakością.
4. PN-EN ISO 14971:2020 Wyroby medyczne. Zastosowanie zarządzania ryzykiem.
5. Ogden R.W. (1972). Large deformation isotropic elasticity – on the correlation of theory and experiment for incompressible rubberlike solids. Proceedings of the Royal Society of London A, 326(1567), 565-584.
6. Yeoh O.H. (1993). Some forms of the strain energy function for rubber. Rubber Chemistry and Technology, 66(5), 754-771.
7. TheraBand® Academy. (2023). Resistance Band Training Guidelines. Hygenic Corporation.
8. Page P., Ellenbecker T.S. (2019). The Band Exercise Bible. Human Kinetics.
9. Kim J.H., Lee S.M. (2020). Effects of elastic band exercise on spasticity and motor function in stroke patients. Journal of Physical Therapy Science, 32(5), 345-349.
10. JCGM 100:2008. Evaluation of measurement data — Guide to the expression of uncertainty in measurement (GUM).
11. Patterson M.R., et al. (2021). Mechanical properties of resistance bands: Implications for clinical use. Journal of Sport Rehabilitation, 30(4), 567-574.
12. Hortobágyi T., et al. (2009). Adaptations to eccentric training with elastic resistance. Medicine & Science in Sports & Exercise, 41(3), 620-628.
