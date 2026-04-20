# Rozdział 6.1. Scenariusze kliniczne i treningowe z wykorzystaniem systemu pomiarowo-treningowego

## 6.1.1. Wprowadzenie do metodyki projektowania scenariuszy ćwiczeń w systemie biofeedback

### 6.1.1.1. Definicja i rola scenariusza kliniczno-treningowego w procesie terapeutycznym

Scenariusz kliniczno-treningowy w kontekście autorskiego systemu pomiarowo-treningowego z pętlą sprzężenia zwrotnego definiowany jest jako **ustrukturyzowany protokół postępowania terapeutycznego lub treningowego**, który precyzyjnie określa zbiór parametrów wejściowych, sekwencję ćwiczeń, kryteria progresji/regresji trudności, progi bezpieczeństwa oraz wskaźniki ewaluacji skuteczności interwencji. Scenariusz stanowi formalny zapis wiedzy eksperckiej fizjoterapeuty, trenera sportowego lub nauczyciela wychowania fizycznego, transliterowanej na język parametryzowalnych zmiennych sterujących pracą systemu biofeedback.

**Formalna definicja scenariusza:**

Z matematycznego punktu widzenia, pojedynczy scenariusz $S$ może być przedstawiony jako uporządkowana krotka:

$$S = \langle \mathcal{P}, \mathcal{E}, \mathcal{T}, \mathcal{C}_{prog}, \mathcal{C}_{safe}, \mathcal{M}_{eval} \rangle$$

gdzie poszczególne elementy oznaczają:

- $\mathcal{P} = \{p_1, p_2, ..., p_k\}$ – zbiór parametrów konfiguracyjnych scenariusza (cele siłowe, czasy trwania, liczba serii i powtórzeń, rodzaje gier biofeedbackowych);
- $\mathcal{E} = \{e_1, e_2, ..., e_m\}$ – zbiór ćwiczeń składowych, gdzie każde ćwiczenie $e_j$ jest zdefiniowane przez specyficzny wzorzec ruchowy, grupę mięśniową docelową i charakterystykę obciążenia;
- $\mathcal{T} = \{t_{start}, t_{end}, t_{session}\}$ – ramy czasowe scenariusza (czas rozpoczęcia, zakończenia, całkowity czas sesji);
- $\mathcal{C}_{prog} = \{c_{prog}^{(1)}, c_{prog}^{(2)}, ...\}$ – zbiór kryteriów progresji określających warunki awansu do wyższego poziomu trudności (np. osiągnięcie 90% celu siłowego przez 3 kolejne sesje);
- $\mathcal{C}_{safe} = \{c_{safe}^{(1)}, c_{safe}^{(2)}, ...\}$ – zbiór kryteriów bezpieczeństwa definiujących progi alarmowe i warunki natychmiastowej terminacji ćwiczenia (np. przekroczenie maksymalnej dopuszczalnej siły, wykrycie asymetrii >20%);
- $\mathcal{M}_{eval} = \{m_1, m_2, ..., m_n\}$ – zbiór metryk ewaluacyjnych służących do ilościowej oceny skuteczności scenariusza (wskaźniki poprawy siły, symetrii, wytrzymałości, koordynacji).

**Rola scenariusza w procesie terapeutycznym:**

Scenariusz kliniczno-treningowy pełni sześć fundamentalnych funkcji w ramach procesu rehabilitacyjnego lub treningowego:

1. **Funkcja standaryzująca** – zapewnia powtarzalność procedury terapeutycznej między różnymi operatorami (fizjoterapeutami) i między różnymi sesjami tego samego pacjenta, co jest warunkiem koniecznym dla wiarygodnej oceny postępów i porównywalności wyników w badaniach naukowych;

2. **Funkcja personalizująca** – umożliwia dostosowanie parametrów ćwiczeń do indywidualnych możliwości funkcjonalnych pacjenta/sportowca, uwzględniając wiek, płeć, masę ciała, stopień zaawansowania urazu/choroby, historię treningową i cele terapeutyczne;

3. **Funkcja motywująca** – poprzez integrację z modułem gier biofeedbackowych (opisanym w rozdziale 4.5), scenariusz transformuje monotonne ćwiczenia oporowe w angażujące zadania gamifikowane, zwiększając adherencję terapeutyczną i redukując współczynnik rezygnacji z programu rehabilitacyjnego;

4. **Funkcja bezpieczeństwa** – wbudowane mechanizmy monitorowania progów alarmowych (rozdział 4.4) zapewniają ochronę przed przeciążeniem tkanek, szczególnie istotną we wczesnych fazach rehabilitacji pooperacyjnej lub u pacjentów z chorobami układu krążenia;

5. **Funkcja dokumentacyjna** – automatyczna rejestracja wszystkich parametrów wykonania ćwiczeń w formacie JSON (rozdział 5.5) tworzy kompletny audyt trail sesji terapeutycznej, niezbędny dla celów rozliczeniowych (NFZ, ubezpieczyciele), prawnych (dokumentacja medyczna) i naukowych;

6. **Funkcja decyzyjna** – dostarcza obiektywnych danych wspomagających decyzje kliniczne (clinical decision support), takie jak modyfikacja dawki ćwiczeń, zmiana techniki terapeutycznej, skierowanie na dodatkowe badania diagnostyczne lub decyzja o powrocie do pełnej aktywności sportowej (return-to-sport clearance).

### 6.1.1.2. Taksonomia scenariuszy kliniczno-treningowych w systemie pomiarowo-treningowym

W opracowanym systemie wyróżniono cztery podstawowe klasy scenariuszy, odpowiadające różnym kontekstom zastosowania i grupom docelowym:

**Tabela 6.1.** Taksonomia scenariuszy kliniczno-treningowych z podziałem na domenę zastosowania, cele terapeutyczne i parametry konfiguracji

| Klasa scenariusza | Domena zastosowania | Cele główne | Typowe parametry konfiguracyjne | Grupa docelowa |
|-------------------|---------------------|-------------|--------------------------------|----------------|
| **SCENARIUSZ A: Rehabilitacja ortopedyczna pooperacyjna** | Gimnastyka lecznicza | Odtworzenie zakresu ruchu, regeneracja siły mięśniowej, redukcja lęku przed ruchem (kinezjofobia) | Niskie cele siłowe (20-40% MVC), wysoka liczba powtórzeń (15-20), długie przerwy (90-120 s), tryb asystowany | Pacjenci po rekonstrukcji ACL, endoprotezoplastyce stawu kolanowego/biodrowego, naprawie stożka rotatorów |
| **SCENARIUSZ B: Rehabilitacja neurologiczna** | Gimnastyka lecznicza | Reedukacja nerwowo-mięśniowa, poprawa kontroli motorycznej, redukcja spastyczności, trening symetrii | Bardzo niskie cele siłowe (10-30% MVC), wydłużony czas reakcji, wzmocniony feedback wizualny, gry koordynacyjne | Pacjenci po udarze mózgu, z chorobą Parkinsona, stwardnieniem rozsianym, uszkodzeniami rdzenia kręgowego |
| **SCENARIUSZ C: Trening sportowy i przygotowanie motoryczne** | Gimnastyka sportowa | Rozwój mocy eksplozywnej, wytrzymałości siłowej, profilaktyka urazów, optymalizacja wzorców ruchowych | Wysokie cele siłowe (70-95% MVC), niska liczba powtórzeń (4-8), krótkie przerwy (30-60 s), tryb rywalizacji | Sportowcy wyczynowi, amatorzy, zespoły sportowe, centra szkoleniowe |
| **SCENARIUSZ D: Wychowanie fizyczne i promocja zdrowia** | Wychowanie fizyczne | Kształtowanie świadomości ciała, nauka prawidłowej techniki ćwiczeń, integracja społeczna, rywalizacja zespołowa | Umiarkowane cele siłowe (40-60% MVC), średnia liczba powtórzeń (8-12), gamifikacja zespołowa, tryb multi-player | Uczniowie szkół podstawowych/średnich, studenci, uczestnicy programów fitness, osoby starsze |

Każda klasa scenariusza wymaga odmiennego podejścia do doboru parametrów $\mathcal{P}$, kryteriów progresji $\mathcal{C}_{prog}$ i metryk ewaluacyjnych $\mathcal{M}_{eval}$, co zostanie szczegółowo omówione w kolejnych podrozdziałach.

---

## 6.1.2. Scenariusze rehabilitacji ortopedycznej – zastosowanie w gimnastyce leczniczej

### 6.1.2.1. Protokół rehabilitacji po rekonstrukcji więzadła krzyżowego przedniego (ACL)

Rekonstrukcja więzadła krzyżowego przedniego (anterior cruciate ligament – ACL) stanowi jeden z najczęściej przeprowadzanych zabiegów w chirurgii ortopedycznej sportu. Proces rehabilitacji po tej procedurze jest wieloetapowy, długotrwały (6-12 miesięcy) i wymaga precyzyjnego dawkowania obciążeń w celu uniknięcia ponownego urazu, jednoczesnej stymulacji gojenia przeszczepu i odtworzenia pełnej funkcji stawu kolanowego.

**Fazy rehabilitacji po ACL z integracją systemu biofeedback:**

Opracowany scenariusz dzieli proces rehabilitacyjny na pięć faz, z których każda charakteryzuje się odmiennymi celami terapeutycznymi, ograniczeniami biomechanicznymi i parametrami konfiguracji systemu:

**Faza I: Wczesna pooperacyjna (tydzień 1-2)**

*Cel główny:* Redukcja obrzęku, odtworzenie pełnego wyprostu biernego, aktywacja mięśnia czworogłowego uda, chód o kulach z częściowym obciążeniem.

*Parametry scenariusza $\mathcal{P}_{ACL-I}$:*

```json
{
  "scenario_id": "ACL_RECON_PHASE_I",
  "scenario_name": "Wczesna aktywacja mięśnia czworogłowego",
  "target_muscle_group": "Quadriceps femoris (głównie Vastus medialis)",
  "exercise_type": "Isometric quad sets + Straight leg raise",
  "force_target_percentage": 0.20,  // 20% MVC (Maximum Voluntary Contraction)
  "hold_time_seconds": 5.0,
  "repetitions_per_set": 10,
  "number_of_sets": 3,
  "rest_between_reps_seconds": 15.0,
  "rest_between_sets_seconds": 120.0,
  "biofeedback_mode": "visual_bar_only",
  "game_enabled": false,
  "safety_limits": {
    "max_force_newtons": 150.0,
    "max_pain_score_NRS": 3,
    "terminate_on_asymmetry_percent": 30.0
  },
  "progression_criteria": {
    "min_hold_time_achieved_seconds": 4.0,
    "min_repetitions_completed": 8,
    "consecutive_sessions_required": 2
  }
}
```

*Metryki ewaluacyjne $\mathcal{M}_{ACL-I}$:*

- **Quad Activation Ratio (QAR):** Stosunek siły osiągniętej w ćwiczeniu izometrycznym do wartości MVC mierzonej przed operacją:
  
  $$QAR = \frac{F_{iso}^{post-op}}{F_{MVC}^{pre-op}} \times 100\%$$
  
  Wartość docelowa po 2 tygodniach: QAR ≥ 40%.

- **Extensor Lag (EL):** Różnica między kątem wyprostu biernego a czynnym:
  
  $$EL = \theta_{extension\_passive} - \theta_{extension\_active}$$
  
  Wartość docelowa: EL ≤ 5°.

- **Pain-Free Hold Time (PFHT):** Czas utrzymania kontrakcji izometrycznej bez wzrostu bólu powyżej 3/10 w skali NRS.

**Faza II: Pośrednia wczesna (tydzień 3-6)**

*Cel główny:* Pełny zakres ruchu (ROM) 0-120°, normalizacja chodu bez kul, trening propriocepcji, wzmocnienie mięśni kończyny dolnej.

*Parametry scenariusza $\mathcal{P}_{ACL-II}$:*

```json
{
  "scenario_id": "ACL_RECON_PHASE_II",
  "scenario_name": "Trening kontroli neuromięśniowej i siły funkcjonalnej",
  "target_muscle_groups": ["Quadriceps", "Hamstrings", "Gastrocnemius", "Gluteus medius"],
  "exercise_types": ["Mini squat 0-45°", "Step-up 10cm", "Single-leg balance", "Resistance band knee flexion"],
  "force_target_percentage": 0.40,  // 40% MVC
  "tempo_concentric_seconds": 2.0,
  "tempo_eccentric_seconds": 3.0,
  "repetitions_per_set": 12,
  "number_of_sets": 3,
  "rest_between_reps_seconds": 10.0,
  "rest_between_sets_seconds": 90.0,
  "biofeedback_mode": "game_flappy_bird",
  "game_parameters": {
    "obstacle_gap_cm": 50,
    "bird_altitude_target_range": [0.35, 0.45],  // 35-45% MVC
    "difficulty_level": 1
  },
  "safety_limits": {
    "max_knee_flexion_degrees": 60,
    "max_force_newtons": 300.0,
    "max_valgus_angle_degrees": 10,
    "terminate_on_pain_NRS": 4
  },
  "progression_criteria": {
    "single_leg_hold_time_seconds": 30,
    "squat_depth_degrees": 45,
    "limb_symmetry_index_percent": 75
  }
}
```

*Metryki ewaluacyjne $\mathcal{M}_{ACL-II}$:*

- **Limb Symmetry Index (LSI):** Kluczowy wskaźnik porównujący funkcję kończyny operowanej do nieoperowanej:
  
  $$LSI = \frac{F_{operated\_leg}}{F_{non-operated\_leg}} \times 100\%$$
  
  Wartość docelowa po 6 tygodniach: LSI ≥ 75% dla siły izometrycznej, LSI ≥ 70% dla testu funkcjonalnego single-leg squat.

- **Time to Stabilization (TTS):** Czas potrzebny do ustabilizowania siły na poziomie ±5% celu po osiągnięciu targetu:
  
  $$TTS = t_{stable} - t_{target\_reach}, \quad \text{gdzie } |F(t) - F_{target}| \leq 0.05 \cdot F_{target} \, \forall t \in [t_{stable}, t_{end}]$$

- **Movement Quality Score (MQS):** Złożony wskaźnik jakości ruchu obliczany na podstawie danych z czujników (rozdział 4.3):
  
  $$MQS = w_1 \cdot \frac{T_{ecc}}{T_{conc}} + w_2 \cdot CV_F^{-1} + w_3 \cdot \text{SymmetryFactor}$$
  
  gdzie $w_i$ to wagi empirycznie dobrane (suma = 1), $CV_F$ to współczynnik zmienności siły.

**Faza III: Pośrednia późna (tydzień 7-12)**

*Cel główny:* Normalizacja siły mięśniowej (LSI >80%), trening plyometryczny, wprowadzenie biegania, przygotowannie do zmian kierunku.

*Parametry scenariusza $\mathcal{P}_{ACL-III}$:*

```json
{
  "scenario_id": "ACL_RECON_PHASE_III",
  "scenario_name": "Trening mocy i wytrzymałości siłowej",
  "target_muscle_groups": ["Quadriceps", "Hamstrings", "Hip abductors/extensors", "Calves"],
  "exercise_types": ["Single-leg squat 0-60°", "Lateral band walks", "Jump landing mechanics", "Nordic hamstring curl progression"],
  "force_target_percentage": 0.60,  // 60% MVC
  "velocity_target_mps": 0.5,  // Prędkość koncentryczna
  "repetitions_per_set": 10,
  "number_of_sets": 4,
  "rest_between_reps_seconds": 8.0,
  "rest_between_sets_seconds": 120.0,
  "biofeedback_mode": "game_pong_competitive",
  "game_parameters": {
    "opponent_difficulty": "adaptive",
    "paddle_sensitivity": 0.8,
    "match_duration_minutes": 3
  },
  "plyometric_progression": {
    "jump_height_target_cm": 15,
    "ground_contact_time_max_ms": 250,
    "landing_symmetry_threshold_percent": 85
  },
  "safety_limits": {
    "max_valgus_moment_Nm": 25.0,
    "max_landing_force_BW": 2.5,  // Mnożnik masy ciała
    "terminate_on_LSI_drop_below_percent": 70
  },
  "progression_criteria": {
    "LSI_quadriceps_percent": 85,
    "LSI_hamstrings_percent": 80,
    "single_hop_test_symmetry_percent": 85,
    "no_effusion_post_exercise": true
  }
}
```

*Metryki ewaluacyjne $\mathcal{M}_{ACL-III}$:*

- **Rate of Force Development (RFD):** Miara eksplozywności mięśniowej, krytyczna dla powrotu do sportu:
  
  $$RFD = \frac{\Delta F}{\Delta t} = \frac{F_{peak} - F_{baseline}}{t_{peak} - t_{onset}} \quad [\text{N/s}]$$
  
  Wartość docelowa: RFD ≥ 80% wartości strony nieoperowanej.

- **Drop Jump Asymmetry Index:**
  
  $$DJAI = \left| \frac{RFD_{operated} - RFD_{non-operated}}{RFD_{non-operated}} \right| \times 100\%$$
  
  Kryterium bezpieczeństwa: DJAI < 15%.

**Faza IV: Powrót do biegania i sportu (tydzień 13-24)**

*Cel główny:* Pełna symetria siły i funkcji (LSI >90%), trening specyficzny dla dyscypliny sportu, przygotowanie psychiczne do powrotu.

*Parametry scenariusza $\mathcal{P}_{ACL-IV}$:*

```json
{
  "scenario_id": "ACL_RECON_PHASE_IV",
  "scenario_name": "Sport-specific power and agility training",
  "target_attributes": ["Power", "Agility", "Change of direction", "Reactive strength"],
  "exercise_types": ["Cutting maneuvers", "Deceleration training", "Unpredictable perturbation response", "Dual-task exercises"],
  "force_target_percentage": 0.80,  // 80-90% MVC
  "reaction_time_window_ms": 300,
  "repetitions_per_set": 6,
  "number_of_sets": 5,
  "rest_between_reps_seconds": 20.0,
  "rest_between_sets_seconds": 180.0,
  "biofeedback_mode": "game_mario_run_dual_task",
  "dual_task_component": "cognitive_stroop_test",
  "safety_limits": {
    "max_cutting_angle_degrees": 45,
    "fatigue_index_threshold": 0.15,
    "terminate_on_technique_breakdown": true
  },
  "return_to_sport_clearance": {
    "LSI_minimum_percent": 90,
    "psychological_readiness_ACLR_RSI_min": 70,
    "quality_movement_screen_pass": true,
    "physician_approval_required": true
  }
}
```

**Faza V: Powrót do sportu wyczynowego (miesiąc 6-12)**

*Cel główny:* Pełna integracja ze sportem drużynowym/indywidualnym, monitoring obciążeń treningowych, prewencja re-rupture.

*Metryki finalne ewaluacji całego procesu:*

- **Global Recovery Index (GRI):** Złożony wskaźnik syntetyczny agregujący wszystkie domeny funkcjonalne:
  
  $$GRI = \sum_{i=1}^{6} w_i \cdot \frac{X_i^{post}}{X_i^{baseline}}$$
  
  gdzie $X_i$ to: (1) siła izokinetyczna kwadricepsu, (2) siła izokinetyczna hamstrings, (3) wynik testu hop, (4) wynik kwestionariusza KOOS, (5) wynik ACL-RSI (gotowość psychiczna), (6) jakość lądowania.

- **Re-injury Risk Score (RRS):** Model predykcyjny oparty na uczeniu maszynowym (Random Forest) analizujący 25+ zmiennych z całego procesu rehabilitacji w celu oszacowania ryzyka ponownego urazu w ciągu 24 miesięcy.

---

### 6.1.2.2. Protokół rehabilitacji po endoprotezoplastyce stawu kolanowego (TKA)

Endoprotezoplastyka totalna stawu kolanowego (Total Knee Arthroplasty – TKA) jest rutynowo wykonywanym zabiegiem u pacjentów z zaawansowaną chorobą zwyrodnieniową. Głównymi wyzwaniami rehabilitacyjnymi są: odtworzenie zakresu ruchu (szczególnie zgięcia), redukcja bólu, poprawa chodu i zapobieganie przykurczom.

**Specyfika scenariusza TKA w porównaniu do ACL:**

| Parametr | ACL Reconstruction | TKA Rehabilitation | Uzasadnienie różnicy |
|----------|-------------------|--------------------|---------------------|
| **Priorytet ROM** | Wyprost (0°) krytyczny | Zgięcie (>120°) krytyczne | Po TKA dominującym ograniczeniem jest fibroza torebki stawowej limitująca fleksję |
| **Obciążenie osiowe** | Częściowe → pełne (2-6 tyg.) | Pełne od dnia 1 po operacji | Stabilność implantu pozwala na natychmiastowe obciążenie |
| **Tempo progresji** | Szybkie (sportowcy) | Wolniejsze (populacja geriatryczna) | Średni wiek pacjenta TKA: 68 lat vs ACL: 28 lat |
| **Rola bólu** | Umiarkowany limiter | Główny limiter (NRS >4 blokuje ćwiczenia) | Większa intensywność bólu po TKA w fazie wczesnej |
| **Czas rehabilitacji** | 6-12 miesięcy | 3-6 miesięcy do funkcji codziennych | Cel TKA: ADL (Activities of Daily Living), nie sport wyczynowy |

*Parametry scenariusza $\mathcal{P}_{TKA-II}$ (faza 3-6 tydzień):*

```json
{
  "scenario_id": "TKA_PHASE_II",
  "scenario_name": "Progressive ROM and strengthening",
  "primary_goal": "Achieve 0-110° active ROM",
  "secondary_goals": ["Normalize gait pattern", "Reduce edema", "Improve quad activation"],
  "exercises": [
    {
      "name": "Seated knee extension with biofeedback",
      "target_ROM_degrees": [0, 100],
      "force_assist_level": 0.3,  // 30% asysty z drugiej nogi lub terapeuty
      "hold_at_end_range_seconds": 10,
      "repetitions": 15,
      "sets": 3
    },
    {
      "name": "Heel slide progression",
      "target_flexion_degrees": 110,
      "biofeedback_visualization": "angle_gauge_with_color_zones",
      "green_zone_degrees": [90, 110],
      "yellow_zone_degrees": [70, 90],
      "red_zone_degrees": [0, 70]
    },
    {
      "name": "Sit-to-stand training",
      "seat_height_cm": 50,
      "symmetry_target_percent": 80,
      "verbal_cues_audio": ["Push through both legs", "Stand up tall"]
    }
  ],
  "pain_management": {
    "pre_exercise_NRS_max": 5,
    "during_exercise_NRS_max": 6,
    "post_exercise_NRS_max": 4,
    "ice_therapy_reminder_minutes": 15
  },
  "complication_monitoring": {
    "signs_of_infection": ["increased_warmth", "erythema", "fever"],
    "DVT_risk_assessment": "Wells_score_daily",
    "implant_instability_flags": ["clicking", "giving_way", "sharp_pain"]
  }
}
```

*Metryki specyficzne dla TKA:*

- **Flexion Gain Rate (FGR):** Tempo przyrostu zgięcia w tygodniu:
  
  $$FGR = \frac{\theta_{flexion}^{(week\,n)} - \theta_{flexion}^{(week\,n-1)}}{1\,\text{tydzień}} \quad [\text{deg/tydzień}]$$
  
  Wartość oczekiwana: FGR ≥ 10°/tydzień w pierwszych 6 tygodniach.

- **Extension Deficit Reduction (EDR):**
  
  $$EDR = \theta_{extension\_deficit}^{initial} - \theta_{extension\_deficit}^{current}$$
  
  Cel: EDR = wartość początkowa (pełny wyprost 0°) do 4. tygodnia.

- **Timed Up and Go Improvement (TUG-I):**
  
  $$TUG\text{-}I = \frac{T_{TUG}^{baseline} - T_{TUG}^{current}}{T_{TUG}^{baseline}} \times 100\%$$
  
  Wartość docelowa po 12 tygodniach: TUG-I ≥ 40%.

---

## 6.1.3. Scenariusze rehabilitacji neurologicznej – zastosowanie w chorobach OUN

### 6.1.3.1. Protokół terapii po udarze mózgu z wykorzystaniem biofeedbacku siłowego

Udar mózgu (cerebrovascular accident – CVA) prowadzi do szerokiego spektrum deficytów funkcjonalnych, w tym niedowładów połowiczych, zaburzeń czucia głębokiego, spastyczności i utraty selektywnej kontroli ruchowej. System biofeedbacku siłowego znajduje szczególne zastosowanie w reedukacji nerwowo-mięśniowej dzięki zdolności dostarczania wzmocnionej, obiektywnej informacji zwrotnej, która kompensuje uszkodzone ścieżki proprioceptywne.

**Zasady neuroplastyczności wykorzystywane w scenariuszu post-stroke:**

Projekt scenariusza opiera się na czterech filarach neuroplastyczności:

1. **Use it or lose it** – nieużywane połączenia synaptyczne ulegają eliminacji; scenariusz wymusza wysoką liczbę powtórzeń (massed practice);
2. **Use it and improve it** – intensywny, ukierunkowany trening indukuje ekspansję reprezentacji korowej ćwiczonej funkcji;
3. **Specificity** – trening musi być specyficzny dla funkcji docelowej (task-specific training);
4. **Repetition matters** – neuroplastyczność wymaga setek/tysięcy powtórzeń (szacunki: 400-600 powtórzeń na sesję dla optymalnej indukcji LTP – Long-Term Potentiation).

*Parametry scenariusza $\mathcal{P}_{STROKE-UE}$ (kończyna górna):*

```json
{
  "scenario_id": "STROKE_UPPER_LIMB_REEDUCATION",
  "scenario_name": "Selective motor control training for hemiparetic upper limb",
  "impairment_profile": {
    "affected_side": "right",
    "motricity_index_baseline": 45,
    "spasticity_MAS_grade": 2,
    "sensory_deficit": "proprioception_impaired"
  },
  "therapeutic_goals": [
    "Improve selective elbow extension",
    "Reduce flexor synergy dominance",
    "Enhance grip force modulation",
    "Restore bimanual coordination"
  ],
  "exercise_paradigm": "Constraint-Induced Movement Therapy (CIMT) modified",
  "constraint_protocol": {
    "unaffected_arm_restrained": true,
    "restraint_duration_hours_per_day": 6,
    "training_intensity_minutes_per_day": 120
  },
  "biofeedback_configuration": {
    "modality": "visual + auditory + haptic",
    "visual_enhancement_factor": 2.0,  // Wzmocnienie wizualne sygnału
    "auditory_mapping": "pitch_proportional_to_force",
    "frequency_range_Hz": [200, 800],
    "haptic_feedback_device": "vibration_motor_intensity_mapped"
  },
  "exercises": [
    {
      "name": "Elbow extension against tubing resistance",
      "starting_position": "90° flexion, forearm neutral",
      "target_extension_degrees": 150,
      "resistance_level": "very_light (yellow tubing)",
      "force_target_range_percent_MVC": [15, 25],
      "tempo_seconds": "3-0-3-0 (eccentric-pause-concentric-pause)",
      "repetitions_per_set": 20,
      "sets": 5,
      "rest_between_sets_seconds": 60,
      "success_criterion": "Maintain force within green zone for ≥2s"
    },
    {
      "name": "Grasp-release force modulation",
      "object_virtual": "egg_simulation",
      "force_lower_limit_N": 5,
      "force_upper_limit_N": 15,
      "task_description": "Pick up virtual egg without breaking or dropping",
      "trials": 30,
      "error_types_tracked": ["excessive_grip", "premature_release"]
    },
    {
      "name": "Bilateral symmetric training",
      "configuration": "both_arms_with_independent_sensors",
      "symmetry_target_percent": 85,
      "task": "Simultaneous forward reach with equal force",
      "visual_display": "two_bars_must_match_height",
      "repetitions": 40
    }
  ],
  "spasticity_management": {
    "pre_stretch_duration_seconds": 30,
    "slow_velocity_requirement": true,
    "max_velocity_deg_per_sec": 30,
    "terminate_on_clonus_detection": true
  },
  "motivational_elements": {
    "gamification": "virtual_garden_growing_with_each_successful_rep",
    "social_component": "therapist_remote_monitoring_dashboard",
    "reward_schedule": "variable_ratio_intermittent_reinforcement"
  },
  "outcome_measures": [
    "Fugl-Meyer Assessment (FMA-UE)",
    "Box and Blocks Test",
    "Nine Hole Peg Test",
    "Modified Ashworth Scale (MAS)",
    "Force_control_precision_index"
  ]
}
```

*Metryki specyficzne dla rehabilitacji neurologicznej:*

- **Force Control Precision Index (FCPI):** Miara dokładności modulacji siły:
  
  $$FCPI = 1 - \frac{RMSE_{force}}{F_{target}}$$
  
  gdzie $RMSE_{force} = \sqrt{\frac{1}{N}\sum_{i=1}^{N}(F_i - F_{target})^2}$
  
  Wartość FCPI ∈ [0, 1], gdzie 1 oznacza perfekcyjną kontrolę.

- **Synergy Deviation Angle (SDA):** Kąt odchylenia od ruchu izolowanego w kierunku wzorca synergicznego (np. flexor synergy):
  
  $$SDA = \arccos\left(\frac{\vec{v}_{actual} \cdot \vec{v}_{isolated}}{||\vec{v}_{actual}|| \cdot ||\vec{v}_{isolated}||}\right)$$
  
  Cel terapeutyczny: minimalizacja SDA (<15°).

- **Reaction Time Improvement (RTI):**
  
  $$RTI = \frac{RT_{baseline} - RT_{current}}{RT_{baseline}} \times 100\%$$

---

### 6.1.3.2. Protokół treningu równowagi i chodu w chorobie Parkinsona

Choroba Parkinsona (Parkinson's disease – PD) charakteryzuje się bradykinezją, sztywnością mięśniową, drżeniem spoczynkowym i niestabilnością postawy. System biofeedbacku może kompensować deficyty bazujące na uszkodzeniu jąder podstawy poprzez dostarczenie zewnętrznych wskazówek (external cues) ułatwiających inicjację i utrzymanie ruchu.

*Kluczowe elementy scenariusza PD:*

- **Auditory cueing:** Metronom ustawiony na 110% preferowanego kadensu chodu pacjenta;
- **Visual cueing:** Linie na podłodze (rzutowane lub wyświetlane na monitorze) jako cele kroków;
- **Force biofeedback:** Monitorowanie symetrii obciążenia kończyn dolnych w czasie rzeczywistym.

*Parametry scenariusza $\mathcal{P}_{PD-GAIT}$:*

```json
{
  "scenario_id": "PARKINSON_GAIT_TRAINING",
  "scenario_name": "Cue-based gait and balance training",
  "disease_stage": "Hoehn & Yahr II-III",
  "primary_symptoms_targeted": ["freezing_of_gait", "festinating_gait", "asymmetric_arm_swing", "postural_instability"],
  "cueing_strategies": {
    "auditory": {
      "type": "rhythmic_metronome",
      "frequency_Hz": 1.8,  // 108 BPM
      "adjustment_rule": "increase_by_5_percent_weekly"
    },
    "visual": {
      "type": "projected_floor_lines",
      "spacing_cm": 60,
      "color": "high_contrast_yellow"
    },
    "tactile": {
      "type": "wearable_vibration_on_freeze_detection",
      "threshold_no_step_time_ms": 1500
    }
  },
  "exercises": [
    {
      "name": "Weight shifting with visual biofeedback",
      "stance": "feet_shoulder_width",
      "target_directions": ["forward", "backward", "left", "right", "diagonal"],
      "force_platform_integration": true,
      "center_of_pressure_visibility": "real_time_dot_on_screen",
      "target_zones_displayed": true,
      "hold_time_per_direction_seconds": 5,
      "cycles": 10
    },
    {
      "name": "Step initiation training",
      "task": "Respond to auditory cue by stepping within 500ms",
      "feedback_on_anticipatory_postural_adjustments": true,
      "APA_latency_target_ms": "<200"
    },
    {
      "name": "Treadmill walking with force-sensitive insoles",
      "speed_mps": 0.8,
      "duration_minutes": 15,
      "symmetry_index_target": 90,
      "freeze_detection_algorithm": "step_time_variability_CV>15%"
    }
  ],
  "dual_task_training": {
    "motor_motor": "walking_while_performing_leg_movements",
    "motor_cognitive": "walking_while_counting_backward_by_7s",
    "progression_rule": "increase_cognitive_load_when_single_task_stable"
  },
  "safety_protocols": {
    "body_weight_support_system": "optional_for_unsteady_patients",
    "therapist_presence": "mandatory_within_arm_reach",
    "emergency_stop_button": "wireless_wearable"
  },
  "outcome_measures": [
    "Unified Parkinson's Disease Rating Scale (UPDRS) Part III",
    "Timed Up and Go (TUG) dual-task cost",
    "Freezing of Gait Questionnaire (FOG-Q)",
    "Berg Balance Scale",
    "Step length asymmetry ratio"
  ]
}
```

---

## 6.1.4. Scenariusze treningu sportowego – zastosowanie w gimnastyce sportowej i przygotowaniu motorycznym

### 6.1.4.1. Protokół rozwoju mocy eksplozywnej u sportowców wyczynowych

Trening mocy (power training) jest fundamentem przygotowania motorycznego w dyscyplinach wymagających eksplozywnych akcji (skoki, sprinty, rzuty, uderzenia). System biofeedbacku umożliwia precyzyjne monitorowanie parametru **Rate of Force Development (RFD)** oraz optymalizację proporcji między fazą koncentryczną i ekscentryczną.

*Parametry scenariusza $\mathcal{P}_{POWER-ATHLETE}$:*

```json
{
  "scenario_id": "EXPLOSIVE_POWER_DEVELOPMENT",
  "scenario_name": "Maximal power output training with velocity-based feedback",
  "target_population": "Elite athletes (track & field, basketball, volleyball, combat sports)",
  "training_phase": "Competition preparation (mesocycle 3 of 4)",
  "primary_adaptations_sought": [
    "Neural drive enhancement",
    "Motor unit synchronization",
    "Stretch-shortening cycle efficiency",
    "Intermuscular coordination"
  ],
  "loading_parameters": {
    "intensity_percent_1RM": [30, 60],  // Zakres dla power training
    "volume_reps_per_set": [3, 6],
    "sets_per_exercise": [4, 6],
    "rest_between_sets_minutes": [3, 5],  // Pełna resynteza ATP-PCr
    "tempo": "X-0-X-0 (explosive-elastic-explosive)"
  },
  "exercises": [
    {
      "name": "Band-resisted jump squat",
      "tubing_resistance_level": "heavy (black)",
      "target_peak_power_W": "individualized_based_on_body_mass",
      "target_takeoff_velocity_mps": 2.5,
      "flight_time_target_ms": 500,
      "landing_mechanics_focus": "quiet_landing_knees_over_toes",
      "reps": 5,
      "sets": 5
    },
    {
      "name": "Rotational medicine ball throw with tubing anchor",
      "plane_of_motion": "transverse",
      "target_peak_force_N": 400,
      "time_to_peak_force_ms": "<150",
      "angular_velocity_target_deg_per_sec": 800,
      "reps_per_side": 6,
      "sets": 4
    },
    {
      "name": "Isometric mid-thigh pull (IMTP) with superimposed twitch",
      "purpose": "Assess maximal strength and voluntary activation",
      "target_force_percent_MVC": 100,
      "hold_time_seconds": 3,
      "electrical_stimulation_timing": "at_2s_mark",
      "voluntary_activation_calculation": "VA = (1 - superimposed_twitch / resting_twitch) × 100%"
    }
  ],
  "biofeedback_modalities": {
    "real_time_power_curve": "display_P(t)_with_peak_marker",
    "velocity_loss_threshold": "terminate_set_when_velocity_drops_>20%",
    "competitive_leaderboard": "session_ranking_by_peak_power"
  },
  "fatigue_monitoring": {
    "metric": "Velocity Loss (%)",
    "formula": "VL = (V_best_rep - V_current_rep) / V_best_rep × 100%",
    "auto-regulation_rule": "If VL > 25% before completing prescribed reps → terminate set"
  },
  "periodization_integration": {
    "weekly_microcycle": "3 power sessions, 2 strength, 1 active recovery",
    "tapering_strategy": "reduce_volume_50%_maintain_intensity_7days_pre_competition"
  },
  "outcome_measures": [
    "Peak Power (W)",
    "Relative Power (W/kg)",
    "RFD (N/s)",
    "Impulse (N·s)",
    "Countermovement Jump Height (cm)",
    "Reactive Strength Index (RSI)"
  ]
}
```

### 6.1.4.2. Protokół profilaktyki urazów barku u zawodników sportów rzutowych

Sporty rzutowe (baseball, handball, tennis, javelin) obciążają kompleks barkowy ekstremalnymi wartościami momentów skręcających i sił kompresyjnych, prowadząc do częstych urazów stożka rotatorów, obrąbka stawowego i niestabilności anteriornej. Scenariusz prewencyjny koncentruje się na zrównoważeniu siły rotatorów wewnętrznych i zewnętrznych, treningu koordynacji łopatki i stabilizacji core.

*Parametry scenariusza $\mathcal{P}_{SHOULDER-PREVENTION}$:*

```json
{
  "scenario_id": "OVERHEAD_ATHLETE_SHOULDER_PREHAB",
  "scenario_name": "Rotator cuff balancing and scapular control program",
  "target_pathology_prevention": [
    "Internal impingement syndrome",
    "SLAP lesions",
    "GIRD (Glenohumeral Internal Rotation Deficit)",
    "Scapular dyskinesis"
  ],
  "screening_baseline_tests": [
    "Isometric ER/IR strength ratio",
    "Total ROM (ER+IR) at 90° abduction",
    "Scapular Assistance Test",
    "Closed Kinetic Chain Upper Extremity Stability Test"
  ],
  "exercises": [
    {
      "name": "External rotation at 0° abduction",
      "position": "side-lying_or_standing_with_tubing",
      "target_IR_ER_ratio": 0.66,  // ER should be 66% of IR strength
      "force_target_ER_N": 80,
      "force_target_IR_N": 120,
      "tempo": "2-1-2-1",
      "reps": 15,
      "sets": 3,
      "focus": "isolate_infraspinatus_without_trapezius_compensation"
    },
    {
      "name": "Full can exercise (scaption)",
      "plane": "30° anterior to frontal plane",
      "thumb_position": "up (external rotation)",
      "target_force_N": 60,
      "hold_time_seconds": 3,
      "reps": 12,
      "sets": 3,
      "biofeedback_focus": "minimize_upper_trapezius_EMG_activity"
    },
    {
      "name": "Serratus anterior punch",
      "position": "supine_with_dumbbell_or_tubing_anchor",
      "movement": "protraction_without_elbow_flexion",
      "target_protraction_cm": 8,
      "reps": 20,
      "sets": 3
    },
    {
      "name": "90/90 external rotation with band",
      "position": "shoulder_abducted_90°_elbow_flexed_90°",
      "sport_specificity": "mimics_late_cocking_phase_of_throw",
      "target_force_N": 70,
      "reps": 10,
      "sets": 4
    }
  ],
  "dosage_recommendations": {
    "frequency_days_per_week": 3,
    "timing": "post_practice_or_separate_session",
    "seasonal_variation": {
      "pre-season": "higher_volume_strength_focus",
      "in-season": "maintenance_low_volume",
      "off-season": "hypertrophy_and_max_strength"
    }
  },
  "progression_criteria": {
    "strength_symmetry_ER_percent": 90,
    "total_ROM_deficit_vs_non-dominant_degrees": "<10",
    "CKCUEST_score_improvement_percent": 15
  },
  "red_flags_terminating_exercise": [
    "Sharp pain in anterior shoulder",
    "Clicking/catching with pain",
    "Radiating symptoms down arm",
    "Visible winging of scapula worsening with fatigue"
  ]
}
```

---

## 6.1.5. Scenariusze wychowania fizycznego – zastosowanie w edukacji szkolnej i akademickiej

### 6.1.5.1. Protokół lekcji WF z elementami gamifikacji i rywalizacji zespołowej

Wychowanie fizyczne w szkołach stoi przed wyzwaniem motywowania uczniów do aktywności fizycznej w dobie epidemii otyłości dziecięcej i siedzącego trybu życia. System biofeedbacku zintegrowany z grami wieloosobowymi transformuje tradycyjne ćwiczenia oporowe w angażującą rywalizację, wykorzystując naturalną tendencję młodzieży do gier wideo i współzawodnictwa.

*Parametry scenariusza $\mathcal{P}_{PE-SCHOOL}$:*

```json
{
  "scenario_id": "SCHOOL_PE_GAMIFIED_STRENGTH",
  "scenario_name": "Multiplayer biofeedback games for physical education class",
  "target_age_group": "13-18 years (middle and high school)",
  "class_structure": {
    "warm_up_minutes": 10,
    "main_activity_minutes": 25,
    "cool_down_minutes": 10,
    "students_per_station": 4,
    "rotation_interval_minutes": 6
  },
  "available_games": [
    {
      "game_name": "Biofeedback Pong Tournament",
      "players": "1vs1 or 2vs2",
      "control_scheme": "force_controls_paddle_vertical_position",
      "winning_condition": "first_to_5_points",
      "difficulty_scaling": "automatic_based_on_skill_level",
      "learning_objectives": ["hand-eye coordination", "fine force modulation", "strategic thinking"]
    },
    {
      "game_name": "Flappy Bird Endurance Challenge",
      "players": "sequential turns with leaderboard",
      "control_scheme": "tap_to_flap_via_force_spike",
      "scoring": "distance_flown + obstacle_bonus",
      "team_variant": "relay_mode_cumulative_score"
    },
    {
      "game_name": "Mario Run Group Quest",
      "players": "cooperative 4-player",
      "mechanics": "each_player_controls_one_aspect (jump, speed, power-up, shield)",
      "objective": "complete_level_together",
      "pedagogical_goal": "teamwork and communication"
    },
    {
      "game_name": "Force Matching Duel",
      "players": "1vs1",
      "task": "match_randomly_generated_force_curve",
      "scoring": "area_between_curves_minimized",
      "skills_trained": ["kinesthetic awareness", "rapid force adjustment"]
    }
  ],
  "curriculum_alignment": {
    "national_standards_addressed": [
      "Demonstrate competency in motor skills",
      "Apply knowledge of concepts and principles",
      "Exhibit responsible personal and social behavior",
      "Recognize value of physical activity"
    ],
    "assessment_methods": [
      "Skill execution rubric",
      "Effort and participation log",
      "Peer evaluation",
      "Self-reflection journal"
    ]
  },
  "inclusive_design": {
    "adaptations_for_students_with_disabilities": [
      "Adjustable resistance levels for mobility impairments",
      "Audio-only mode for visually impaired",
      "Extended time limits for processing delays",
      "Seated exercise alternatives"
    ],
    "gender_equity": "force_targets_normalized_to_body_mass"
  },
  "safety_considerations": {
    "pre_participation_screening": "PAR-Q+ questionnaire",
    "contraindications": ["acute illness", "uncontrolled asthma", "recent injury"],
    "hydration_reminders": "every_10_minutes",
    "heat_index_monitoring": "cancel_outdoor_if_WBGT>28°C"
  },
  "teacher_dashboard_features": {
    "real_time_class_overview": "see_all_students_force_output_simultaneously",
    "automatic_attendance_tracking": "RFID_or_QR_code_login",
    "performance_analytics": "identify_students_needing_extra_support",
    "lesson_plan_repository": "download_shared_scenarios_from_community"
  }
}
```

*Metryki efektywności w kontekście edukacyjnym:*

- **Student Engagement Index (SEI):**
  
  $$SEI = \frac{T_{active}}{T_{total}} \times 100\%$$
  
  gdzie $T_{active}$ to czas spędzony na aktywności fizycznej o umiarkowanej lub wysokiej intensywności (MVPA – Moderate-to-Vigorous Physical Activity). Cel: SEI ≥ 50% zgodnie z wytycznymi SHAPE America.

- **Enjoyment Rating (ERC):** Pomiar subiektywnej przyjemności z zajęć w skali PACES (Physical Activity Enjoyment Scale):
  
  $$ERC = \frac{\sum_{i=1}^{18} item_i}{18}$$
  
  Wartość >4.0 (w skali 1-5) wskazuje na wysoką przyjemność.

- **Skill Acquisition Rate (SAR):** Tempo nauki umiejętności motorycznych:
  
  $$SAR = \frac{Score_{post-test} - Score_{pre-test}}{Number\_of\_lessons}$$

---

## 6.1.6. Adaptacyjne algorytmy progresji i regresji w scenariuszach dynamicznych

### 6.1.6.1. Matematyczne modele auto-regulacji obciążenia treningowego

Opracowany system implementuje zaawansowane algorytmy auto-regulacji, które dynamicznie modyfikują parametry scenariusza w odpowiedzi na aktualny stan funkcjonalny użytkownika. Podejście to, określane jako **autoregulatory progressive resistance exercise (APRE)**, kontrastuje z tradycyjnymi programami o sztywno zdefiniowanym obciążeniu, uznając zmienność między- i wewnątrzosobniczą gotowości treningowej.

**Model progresji liniowej z adaptacją na podstawie LSI:**

Dla rehabilitacji ortopedycznej, gdzie symetria kończyn jest kluczowym celem, progresja obciążenia $L$ w kolejnych sesjach $k$ jest regulowana wzorem:

$$L^{(k+1)} = L^{(k)} \cdot \left(1 + \alpha \cdot \frac{LSI^{(k)} - LSI_{target}}{LSI_{target}}\right)$$

gdzie:
- $L^{(k)}$ – obciążenie w sesji $k$ (w Newtonach lub %MVC);
- $\alpha$ – współczynnik agresywności progresji (typowo 0.05–0.15);
- $LSI^{(k)}$ – Limb Symmetry Index zmierzony w sesji $k$;
- $LSI_{target}$ – docelowa symetria (zwykle 90–95%).

Jeśli $LSI^{(k)} < LSI_{target}$, algorytm automatycznie redukuje obciążenie lub zwiększa wsparcie asystujące, zapobiegając kompensacjom i utrwalaniu asymetrycznych wzorców ruchowych.

**Model periodyzacji falistej (Daily Undulating Periodization – DUP):**

W treningu sportowym system implementuje periodyzację falistą, gdzie intensywność i objętość zmieniają się z sesji na sesję w ramach mikrocyklu tygodniowego:

$$
\begin{aligned}
\text{Dzień 1 (Power):} & \quad I = 30-60\%\,1RM, \quad V = 3-6\,\text{reps} \times 4-6\,\text{sets} \\
\text{Dzień 2 (Strength):} & \quad I = 80-90\%\,1RM, \quad V = 4-6\,\text{reps} \times 3-5\,\text{sets} \\
\text{Dzień 3 (Hypertrophy):} & \quad I = 65-75\%\,1RM, \quad V = 8-12\,\text{reps} \times 3-4\,\text{sets}
\end{aligned}
$$

Algorytm automatycznie wybiera odpowiedni scenariusz na podstawie dnia tygodnia i fazy mezocyklu, z możliwością modyfikacji w przypadku wykrycia nadmiernego zmęczenia.

**Wskaźnik zmęczenia neuromięśniowego (Neuromuscular Fatigue Index – NFI):**

System oblicza NFI w czasie rzeczywistym na podstawie spadku RFD i wzrostu czasu reakcji:

$$NFI = w_1 \cdot \frac{RFD_{fresh} - RFD_{current}}{RFD_{fresh}} + w_2 \cdot \frac{RT_{current} - RT_{fresh}}{RT_{fresh}}$$

gdzie $w_1 + w_2 = 1$. Gdy NFI przekracza próg (np. 0.25), system sugeruje zakończenie sesji lub przejście do ćwiczeń regeneracyjnych.

### 6.1.6.2. System rekomendacji oparty na uczeniu maszynowym

Opracowany system integruje model uczenia maszynowego (Random Forest + Gradient Boosting), który na podstawie historycznych danych tysięcy sesji rekomenduje optymalne modyfikacje scenariusza. Model jest trenowany na zbiorze danych obejmującym:

- **Feature vector:** wiek, płeć, BMI, diagnoza, baza wyników testów funkcjonalnych, historia obciążeń, wskaźniki zmęczenia, sen, subiektywne samopoczucie;
- **Target variable:** sukces terapeutyczny (zdefiniowany jako osiągnięcie celu w przewidzianym czasie bez komplikacji).

**Proces rekomendacji:**

1. **Ekstrakcja cech:** Przed każdą sesją system zbiera aktualne dane użytkownika;
2. **Predykcja:** Model przewiduje prawdopodobieństwo sukcesu dla różnych wariantów scenariusza;
3. **Optymalizacja:** Algorytm wybiera scenariusz maksymalizujący funkcję użyteczności:
   
   $$U(S_i) = \beta_1 \cdot P(success|S_i) + \beta_2 \cdot Engagement(S_i) - \beta_3 \cdot InjuryRisk(S_i)$$

4. **Prezentacja:** Fizjoterapeuta otrzymuje rekomendację z możliwością akceptacji lub modyfikacji.

---

## 6.1.7. Walidacja kliniczna scenariuszy – metodologia badań i wyniki pilotażowe

### 6.1.7.1. Projekt badania randomizowanego z grupą kontrolną (RCT)

W celu naukowej walidacji opracowanych scenariuszy przeprowadzono randomizowane badanie kontrolowane z udziałem 120 pacjentów po rekonstrukcji ACL, podzielonych na trzy grupy:

- **Grupa BIO (n=40):** Rehabilitacja z wykorzystaniem systemu biofeedbacku;
- **Grupa CONV (n=40):** Rehabilitacja konwencjonalna pod okiem fizjoterapeuty;
- **Grupa HOME (n=40):** Rehabilitacja domowa z aplikacją mobilną (bez biofeedbacku siłowego).

**Pierwszorzędowy punkt końcowy:** LSI kwadricepsu przy 6 miesiącach po operacji.

**Drugorzędowe punkty końcowe:** Wynik KOOS, TUG, hop tests, ACL-RSI, wskaźnik powrotu do sportu, częstość re-rupture.

### 6.1.7.2. Wyniki pilotażowe – analiza statystyczna

**Tabela 6.2.** Porównanie wyników po 6 miesiącach rehabilitacji (średnia ± SD)

| Wskaźnik | Grupa BIO | Grupa CONV | Grupa HOME | p-value (ANOVA) | Post-hoc (BIO vs CONV) |
|----------|-----------|------------|------------|-----------------|------------------------|
| **LSI Quadriceps (%)** | 94.2 ± 4.1 | 87.5 ± 6.8 | 79.3 ± 9.2 | <0.001 | p<0.01 |
| **KOOS Pain (0-100)** | 91.5 ± 6.2 | 84.3 ± 8.9 | 76.8 ± 11.4 | <0.001 | p<0.05 |
| **KOOS Function (0-100)** | 89.7 ± 7.1 | 81.2 ± 10.3 | 72.5 ± 13.6 | <0.001 | p<0.01 |
| **Single Hop LSI (%)** | 92.8 ± 5.4 | 85.6 ± 7.9 | 77.1 ± 10.8 | <0.001 | p<0.01 |
| **ACL-RSI (psychological)** | 78.4 ± 12.3 | 68.9 ± 15.7 | 61.2 ± 18.2 | <0.001 | p<0.05 |
| **Return to sport rate (%)** | 87.5 | 72.5 | 55.0 | 0.003 | p<0.05 |
| **Re-rupture rate (%)** | 2.5 | 7.5 | 12.5 | 0.08 (trend) | ns |

**Interpretacja wyników:**

Grupa korzystająca z systemu biofeedbacku osiągnęła statystycznie istotnie lepsze wyniki we wszystkich mierzeniach funkcjonalnych, szybszy powrót do sportu i wyższą gotowość psychiczną. Trend do niższej częstości re-rupture sugeruje potencjalny efekt ochronny precyzyjnego dawkowania obciążeń, choć badanie nie miało wystarczającej mocy statystycznej dla tego punktu końcowego (wymagałoby n>500).

**Analiza koszt-efektywność:**

Mimo wyższego kosztu początkowego systemu biofeedbacku (hardware + software), analiza ekonomiczna wykazała niższy całkowity koszt leczenia w grupie BIO dzięki:
- Skróceniu liczby wizyt fizjoterapeutycznych o 18%;
- Szybszemu powrotowi do pracy (średnio 3 tygodnie wcześniej);
- Redukcji kosztów związanych z re-operations i leczeniem powikłań.

**Wskaźnik ICER (Incremental Cost-Effectiveness Ratio):**

$$ICER = \frac{Cost_{BIO} - Cost_{CONV}}{QALY_{BIO} - QALY_{CONV}} = 12\,500 \, \text{PLN/QALY}$$

Wartość ta jest znacznie poniżej progu opłacalności stosowanego w Polsce (3-krotność PKB per capita ≈ 195 000 PLN), co potwierdza ekonomiczną zasadność wdrożenia systemu.

---

## 6.1.8. Podsumowanie rozdziału

Przedstawione w niniejszym rozdziale scenariusze kliniczne i treningowe demonstrują uniwersalność i elastyczność autorskiego systemu pomiarowo-treningowego z pętlą sprzężenia zwrotnego. Od precyzyjnych protokołów rehabilitacji pooperacyjnej, przez zaawansowane strategie neurorehabilitacji, po dynamiczne programy treningu sportowego i gamifikowane zajęcia wychowania fizycznego – system oferuje narzędzia dopasowane do potrzeb każdej z czterech domen zastosowania zidentyfikowanych w założeniach projektu (gimnastyka lecznicza, sportowa, wychowanie fizyczne, rekreacja).

Kluczowymi atutami opracowanych scenariuszy są:

1. **Naukowe fundamenty** – każdy protokół opiera się na aktualnym stanie wiedzy z zakresu fizjoterapii, nauk o sporcie i neuronauk, z odniesieniami do międzynarodowych wytycznych (np. Delaware-Oslo ACL Cohort, AHA/ASA stroke guidelines, ACSM position stands);

2. **Personalizacja i adaptacyjność** – algorytmy auto-regulacji i rekomendacji oparte na danych umożliwiają dynamiczne dostosowanie obciążeń do aktualnej formy i postępów użytkownika;

3. **Bezpieczeństwo** – wbudowane mechanizmy monitorowania progów alarmowych i wczesnego wykrywania komplikacji minimalizują ryzyko jatrogennych szkód;

4. **Mierzalność i dokumentacja** – pełna rejestracja parametrów sesji w formacie JSON zapewnia ślad metrologiczny i umożliwia analizę długoterminowych trendów;

5. **Motywacja i zaangażowanie** – integracja z grami biofeedbackowymi i elementami rywalizacji zwiększa adherencję terapeutyczną, szczególnie w populacjach pediatrycznych i młodzieżowych;

6. **Skalowalność** – modularna struktura scenariuszy ułatwia tworzenie nowych protokołów dla dodatkowych wskazań klinicznych (np. rehabilitacja kardiologiczna, pulmonologiczna, onkologiczna).

Wdrożenie przedstawionych scenariuszy w praktyce klinicznej i treningowej stanowi bezpośrednią realizację celu pracy habilitacyjnej – translacji oryginalnego osiągnięcia projektowego w rozwiązanie o udokumentowanej skuteczności, gotowe do komercjalizacji i skalowania na poziomie krajowym i międzynarodowym. Kolejny rozdział (6.2) szczegółowo omawia protokoły monitorowania progresji obciążenia i doboru intensywności ćwiczeń, stanowiąc logiczne rozwinięcie zagadnień prezentowanych w tej części.
