#ifndef FORMULAS_HPP
#define FORMULAS_HPP

#include <vector>
#include <string>
#include <cstdint>
#include <cmath>
#include <numeric>
#include <algorithm>

/**
 * @brief Struktura reprezentująca pojedynczą próbkę danych z czujnika
 */
struct SensorSample {
    uint32_t timestamp;  // Czas w milisekundach
    double force;        // Siła w Newtonach
    bool isValid;
    
    SensorSample() : timestamp(0), force(0.0), isValid(false) {}
    SensorSample(uint32_t ts, double f) : timestamp(ts), force(f), isValid(true) {}
};

/**
 * @brief Struktura reprezentująca pojedyncze powtórzenie ćwiczenia
 */
struct RepetitionData {
    int repNumber;              // Numer powtórzenia (1-8)
    uint32_t startTime;         // Czas rozpoczęcia powtórzenia [ms]
    uint32_t endTime;           // Czas zakończenia powtórzenia [ms]
    uint32_t peakTime;          // Czas osiągnięcia szczytu [ms]
    double startForce;          // Siła początkowa [N]
    double peakForce;           // Siła szczytowa [N]
    double endForce;            // Siła końcowa [N]
    std::vector<SensorSample> samples;  // Wszystkie próbki powtórzenia
    
    RepetitionData() : repNumber(0), startTime(0), endTime(0), peakTime(0),
                       startForce(0), peakForce(0), endForce(0) {}
};

/**
 * @brief Struktura reprezentującą serię ćwiczeń (8 powtórzeń)
 */
struct SeriesData {
    int seriesNumber;                   // Numer serii (1-3)
    std::vector<RepetitionData> reps;   // Powtórzenia w serii
    uint32_t startTime;                 // Czas rozpoczęcia serii [ms]
    uint32_t endTime;                   // Czas zakończenia serii [ms]
    
    SeriesData() : seriesNumber(0), startTime(0), endTime(0) {}
};

/**
 * @brief Struktura zawierająca wszystkie obliczone parametry biomechaniczne
 */
struct BiomechanicalParams {
    // Kategoria 1: Parametry Siłowe (1-10)
    double F_peak;              // 1. Maksymalna Siła Chwilowa [N]
    double F_mean;              // 2. Średnia Siła z Powtórzenia [N]
    double F_min;               // 3. Minimalna Siła z Powtórzenia [N]
    double F_range;             // 4. Zakres Siły [N]
    double F_set_mean;          // 5. Średnia Siła z Serii [N]
    double F_set_peak;          // 6. Maksymalna Siła z Serii [N]
    double W_total;             // 7. Całkowita Praca Mechaniczna Serii [J]
    double W_rep_avg;           // 8. Średnia Praca na Powtórzenie [J]
    double I_impulse;           // 9. Impuls Siły [N·s]
    double I_set_avg;           // 10. Średni Impuls z Serii [N·s]
    
    // Kategoria 2: Parametry Czasowe (11-20)
    double T_rep;               // 11. Czas Trwania Powtórzenia [s]
    double T_conc;              // 12. Czas Fazy Koncentrycznej [s]
    double T_ecc;               // 13. Czas Fazy Ekscentrycznej [s]
    double R_CE;                // 14. Stosunek Faz
    double T_rest_rep;          // 15. Czas Przerwy Między Powtórzeniami [s]
    double T_set;               // 16. Czas Trwania Serii [s]
    double T_rest_set;          // 17. Czas Przerwy Między Seriami [s]
    double T_rep_avg;           // 18. Średni Czas Powtórzenia w Serii [s]
    double v_avg;               // 19. Tempo Wykonania Ruchu [m/s]
    double f_rep;               // 20. Częstotliwość Powtórzeń [Hz]
    
    // Kategoria 3: Parametry Mocy i Energii (21-30)
    double P_peak;              // 21. Moc Szczytowa [W]
    double P_rep_avg;           // 22. Średnia Moc z Powtórzenia [W]
    double P_set_avg;           // 23. Średnia Moc z Serii [W]
    double E_total;             // 24. Całkowita Energia Zużyta [J]
    double P_norm;              // 25. Moc Znormalizowana do Masy Ciała [W/kg]
    double RFD;                 // 26. Wskaźnik Szybkości Rozwoju Siły [N/s]
    double RFD_set_avg;         // 27. Średni RFD z Serii [N/s]
    double RFD_max;             // 28. Maksymalny RFD [N/s]
    double E_elastic;           // 29. Energia Sprężysta Zmagazynowana [J]
    double eta;                 // 30. Sprawność Mechaniczna [%]
    
    // Kategoria 4: Parametry Zmienności i Stabilności (31-40)
    double sigma_F;             // 31. Odchylenie Standardowe Siły [N]
    double CV_F;                // 32. Współczynnik Zmienności Siły [%]
    double FI_set;              // 33. Indeks Zmęczenia Serii [%]
    double FI_inter;            // 34. Indeks Zmęczenia Międzyseryjnego [%]
    double MCI;                 // 35. Współczynnik Jednorodności Ruchu
    double PSI;                 // 36. Indeks Symetrii Faz
    double Var_T;               // 37. Wariancja Czasu Trwania Powtórzeń [s²]
    double SI;                  // 38. Indeks Płynności Ruchu [N/s²]
    double H_entropy;           // 39. Entropia Sygnału Siły [bits]
    double BPR;                 // 40. Indeks Regularności Oddychania
    
    // Kategoria 5: Parametry Postępu i Wydajności (41-50)
    double F_grand_mean;        // 41. Średnia Siła Całkowita z Treningu [N]
    double I_total;             // 42. Całkowity Impuls Treningu [N·s]
    double V_training;          // 43. Objętość Treningowa
    double INT;                 // 44. Intensywność Treningu [%]
    double D_training;          // 45. Gęstość Treningu [rep/s]
    double SEI;                 // 46. Indeks Efektywności Serii
    double FMC;                 // 47. Współczynnik Utrzymania Siły [%]
    double ISI;                 // 48. Indeks Poprawy Międzyseryjnej [%]
    double TUT_avg;             // 49. Średni Czas Pod Napięciem [s]
    double TUT_total;           // 50. Całkowity Czas Pod Napięciem [s]
    
    // Kategoria 6: Parametry Zaawansowane (51-60)
    double MQS;                 // 51. Wskaźnik Jakości Ruchu
    double NML;                 // 52. Indeks Obciążenia Neuromięśniowego
    double PSE;                 // 53. Wskaźnik Stabilności Postawy
    double TRI;                 // 54. Indeks Gotowości Treningowej
    double IRI;                 // 55. Wskaźnik Ryzyka Kontuzji
    double TAI;                 // 56. Indeks Adaptacji Treningowej
    double MEI;                 // 57. Wskaźnik Ekonomii Ruchu
    double LBI;                 // 58. Indeks Zrównoważenia Obciążenia
    double FRI;                 // 59. Wskaźnik Rezerwy Funkcjonalnej
    double CTS;                 // 60. Kompleksowy Wynik Treningowy
    
    // Kategoria 7: Parametry Dodatkowe (61-65)
    double FG;                  // 61. Gradient Zmęczenia
    double ESI;                 // 62. Indeks Eksplozywnej Siły
    double BRC;                 // 63. Współczynnik Powrotu do Bazy
    double CEI;                 // 64. Indeks Kontrolowanego Opuszczania
    double LPT;                 // 65. Wskaźnik Postępu Longitudinalnego
    
    BiomechanicalParams();
    void reset();
};

/**
 * @brief Klasa zawierająca funkcje do obliczania parametrów biomechanicznych
 * 
 * Implementuje wzory z dokumentu MATHEMATICAL_FORMULAS.md
 */
class Formulas {
public:
    /**
     * @brief Konstruktor
     * @param k_elastic Stała sprężystości taśmy [N/m]
     * @param patientMass Masa pacjenta [kg]
     * @param sampleRate Częstotliwość próbkowania [Hz]
     */
    Formulas(double k_elastic = 100.0, double patientMass = 70.0, double sampleRate = 100.0);
    
    /**
     * @brief Oblicza wszystkie parametry biomechaniczne dla serii
     * @param series Dane serii ćwiczeń
     * @return Obliczone parametry biomechaniczne
     */
    BiomechanicalParams calculateAllParams(const SeriesData& series);
    
    /**
     * @brief Oblicza parametry dla pojedynczego powtórzenia
     * @param rep Dane powtórzenia
     * @return Parametry dla powtórzenia
     */
    BiomechanicalParams calculateRepParams(const RepetitionData& rep);
    
    // ==================== Kategoria 1: Parametry Siłowe ====================
    
    /**
     * @brief 1. Maksymalna Siła Chwilowa (Peak Force)
     * F_peak = max(F(t)) dla t w [t_start, t_end]
     */
    static double calculatePeakForce(const std::vector<double>& forces);
    
    /**
     * @brief 2. Średnia Siła z Powtórzenia (Mean Force)
     * F_mean = (1/M) * sum(F_i)
     */
    static double calculateMeanForce(const std::vector<double>& forces);
    
    /**
     * @brief 3. Minimalna Siła z Powtórzenia (Minimum Force)
     * F_min = min(F(t))
     */
    static double calculateMinForce(const std::vector<double>& forces);
    
    /**
     * @brief 4. Zakres Siły (Force Range)
     * R_F = F_peak - F_min
     */
    static double calculateForceRange(double peakForce, double minForce);
    
    /**
     * @brief 5. Średnia Siła z Serii (Mean Force per Set)
     * F_set_mean = (1/N_reps) * sum(F_mean_n)
     */
    static double calculateSetMeanForce(const std::vector<double>& repMeanForces);
    
    /**
     * @brief 6. Maksymalna Siła z Serii (Maximum Force per Set)
     * F_set_peak = max(F_peak_n)
     */
    static double calculateSetPeakForce(const std::vector<double>& repPeakForces);
    
    /**
     * @brief 7. Całkowita Praca Mechaniczna Serii (Total Work)
     * W = (1/k_elastic) * sum(F_i * (F_{i+1} - F_i))
     */
    double calculateWork(const std::vector<SensorSample>& samples) const;
    
    /**
     * @brief 8. Średnia Praca na Powtórzenie (Average Work per Rep)
     * W_rep_avg = W / N_reps
     */
    static double calculateAvgWorkPerRep(double totalWork, int numReps);
    
    /**
     * @brief 9. Impuls Siły (Force Impulse)
     * I = sum(F_i * dt)
     */
    double calculateImpulse(const std::vector<SensorSample>& samples) const;
    
    /**
     * @brief 10. Średni Impuls z Serii (Average Impulse per Set)
     * I_set_avg = (1/N_reps) * sum(I_n)
     */
    static double calculateSetAvgImpulse(const std::vector<double>& repImpulses);
    
    // ==================== Kategoria 2: Parametry Czasowe ====================
    
    /**
     * @brief 11. Czas Trwania Powtórzenia (Repetition Duration)
     * T_rep = t_end - t_start
     */
    static double calculateRepDuration(uint32_t startTime, uint32_t endTime);
    
    /**
     * @brief 12. Czas Fazy Koncentrycznej (Concentric Phase Time)
     * T_conc = t_peak - t_start
     */
    static double calculateConcentricTime(uint32_t startTime, uint32_t peakTime);
    
    /**
     * @brief 13. Czas Fazy Ekscentrycznej (Eccentric Phase Time)
     * T_ecc = t_end - t_peak
     */
    static double calculateEccentricTime(uint32_t peakTime, uint32_t endTime);
    
    /**
     * @brief 14. Stosunek Faz (Concentric-Eccentric Ratio)
     * R_CE = T_conc / T_ecc
     */
    static double calculateCERatio(double concTime, double eccTime);
    
    /**
     * @brief 15. Czas Przerwy Między Powtórzeniami (Inter-Repetition Rest)
     * T_rest_rep = t_start_{n+1} - t_end_n
     */
    static double calculateInterRepRest(uint32_t prevEndTime, uint32_t nextStartTime);
    
    /**
     * @brief 16. Czas Trwania Serii (Set Duration)
     * T_set = t_end_last - t_start_first
     */
    static double calculateSetDuration(uint32_t startTime, uint32_t endTime);
    
    /**
     * @brief 17. Czas Przerwy Między Seriami (Inter-Set Rest)
     * T_rest_set = t_start_{s+1} - t_end_s
     */
    static double calculateInterSetRest(uint32_t prevSetEnd, uint32_t nextSetStart);
    
    /**
     * @brief 18. Średni Czas Powtórzenia w Serii (Average Rep Time)
     * T_rep_avg = (1/N_reps) * sum(T_n)
     */
    static double calculateAvgRepTime(const std::vector<double>& repTimes);
    
    /**
     * @brief 19. Tempo Wykonania Ruchu (Movement Velocity)
     * v_avg = R_F / (k_elastic * T_rep)
     */
    double calculateVelocity(double forceRange, double repDuration) const;
    
    /**
     * @brief 20. Częstotliwość Powtórzeń (Repetition Frequency)
     * f_rep = N_reps / T_set
     */
    static double calculateRepFrequency(int numReps, double setDuration);
    
    // ==================== Kategoria 3: Parametry Mocy i Energii ====================
    
    /**
     * @brief 21. Moc Szczytowa (Peak Power)
     * P_peak = F_peak * v_peak
     */
    double calculatePeakPower(double peakForce, const std::vector<SensorSample>& samples) const;
    
    /**
     * @brief 22. Średnia Moc z Powtórzenia (Average Power per Rep)
     * P_rep_avg = W / T_rep
     */
    static double calculateAvgPower(double work, double duration);
    
    /**
     * @brief 23. Średnia Moc z Serii (Average Power per Set)
     * P_set_avg = W_total / T_set
     */
    static double calculateSetAvgPower(double totalWork, double setDuration);
    
    /**
     * @brief 24. Całkowita Energia Zużyta (Total Energy)
     * E_total = sum(W_s)
     */
    static double calculateTotalEnergy(const std::vector<double>& setWorks);
    
    /**
     * @brief 25. Moc Znormalizowana do Masy Ciała (Normalized Power)
     * P_norm = P / m_patient
     */
    double calculateNormalizedPower(double power) const;
    
    /**
     * @brief 26. Wskaźnik Szybkości Rozwoju Siły (RFD)
     * RFD = (F_peak - F_start) / T_conc
     */
    static double calculateRFD(double peakForce, double startForce, double concTime);
    
    /**
     * @brief 27. Średni RFD z Serii (Average RFD per Set)
     */
    static double calculateSetAvgRFD(const std::vector<double>& repRFDs);
    
    /**
     * @brief 28. Maksymalny RFD (Maximum RFD)
     */
    static double calculateMaxRFD(const std::vector<double>& repRFDs);
    
    /**
     * @brief 29. Energia Sprężysta Zmagazynowana (Elastic Potential Energy)
     * E_elastic = (F_peak^2) / (2 * k_elastic)
     */
    double calculateElasticEnergy(double peakForce) const;
    
    /**
     * @brief 30. Sprawność Mechaniczna (Mechanical Efficiency)
     * eta = W / E_metabolic_est * 100%
     */
    double calculateEfficiency(double work, double metabolicEstimate) const;
    
    // ==================== Kategoria 4: Parametry Zmienności ====================
    
    /**
     * @brief 31. Odchylenie Standardowe Siły (Force Standard Deviation)
     */
    static double calculateStdDev(const std::vector<double>& values);
    
    /**
     * @brief 32. Współczynnik Zmienności Siły (Coefficient of Variation)
     * CV_F = (sigma_F / F_mean) * 100%
     */
    static double calculateCV(double stdDev, double mean);
    
    /**
     * @brief 33. Indeks Zmęczenia Serii (Fatigue Index within Set)
     * FI_set = ((F_peak_1 - F_peak_N) / F_peak_1) * 100%
     */
    static double calculateFatigueIndex(double firstPeak, double lastPeak);
    
    /**
     * @brief 34. Indeks Zmęczenia Międzyseryjnego (Inter-Set Fatigue Index)
     */
    static double calculateInterSetFatigue(double firstSetPeak, double thirdSetPeak);
    
    /**
     * @brief 35. Współczynnik Jednorodności Ruchu (Movement Consistency Index)
     * MCI = 1 - (sigma_T / T_rep_avg)
     */
    static double calculateMCI(double timeStdDev, double avgTime);
    
    /**
     * @brief 36. Indeks Symetrii Faz (Phase Symmetry Index)
     * PSI = |1 - (T_conc / T_ecc)|
     */
    static double calculatePSI(double concTime, double eccTime);
    
    /**
     * @brief 37. Wariancja Czasu Trwania Powtórzeń (Rep Duration Variance)
     */
    static double calculateTimeVariance(const std::vector<double>& repTimes);
    
    /**
     * @brief 38. Indeks Płynności Ruchu (Smoothness Index)
     * SI = (1/(M-2)) * sum(|(F_{i+1} - 2F_i + F_{i-1}) / dt^2|)
     */
    double calculateSmoothnessIndex(const std::vector<SensorSample>& samples) const;
    
    /**
     * @brief 39. Entropia Sygnału Siły (Force Signal Entropy)
     * H = -sum(p_k * log2(p_k))
     */
    static double calculateEntropy(const std::vector<double>& forces, int numBins = 10);
    
    /**
     * @brief 40. Indeks Regularności Oddychania (Breathing Pattern Regularity)
     * BPR = 1 / (1 + (sigma_rest / mean_rest))
     */
    static double calculateBPR(double restStdDev, double restMean);
    
    // ==================== Kategoria 5: Parametry Postępu ====================
    
    /**
     * @brief 41. Średnia Siła Całkowita z Treningu (Grand Mean Force)
     */
    static double calculateGrandMeanForce(const std::vector<double>& setMeans);
    
    /**
     * @brief 42. Całkowity Impuls Treningu (Total Training Impulse)
     */
    static double calculateTotalImpulse(const std::vector<double>& repImpulses);
    
    /**
     * @brief 43. Objętość Treningowa (Training Volume)
     * V = sum(F_mean * F_range)
     */
    static double calculateTrainingVolume(const std::vector<std::pair<double, double>>& repData);
    
    /**
     * @brief 44. Intensywność Treningu (Training Intensity)
     * INT = (F_grand_mean / F_max_reference) * 100%
     */
    static double calculateIntensity(double grandMean, double maxReference);
    
    /**
     * @brief 45. Gęstość Treningu (Training Density)
     * D = (N_sets * N_reps) / T_total
     */
    static double calculateDensity(int numSets, int numReps, double totalTime);
    
    /**
     * @brief 46. Indeks Efektywności Serii (Set Efficiency Index)
     * SEI = W / (T_set * F_set_mean)
     */
    static double calculateSEI(double work, double setDuration, double setMeanForce);
    
    /**
     * @brief 47. Współczynnik Utrzymania Siły (Force Maintenance Coefficient)
     * FMC = (F_set_3 / F_set_1) * 100%
     */
    static double calculateFMC(double thirdSetMean, double firstSetMean);
    
    /**
     * @brief 48. Indeks Poprawy Międzyseryjnej (Inter-Set Improvement Index)
     * ISI = ((F_set_2 - F_set_1) / F_set_1) * 100%
     */
    static double calculateISI(double secondSetPeak, double firstSetPeak);
    
    /**
     * @brief 49. Średni Czas Pod Napięciem (Average Time Under Tension)
     */
    double calculateAvgTUT(const std::vector<std::vector<SensorSample>>& repSamples, 
                           double thresholdFactor = 0.5) const;
    
    /**
     * @brief 50. Całkowity Czas Pod Napięciem (Total Time Under Tension)
     */
    double calculateTotalTUT(const std::vector<std::vector<SensorSample>>& repSamples,
                             double thresholdFactor = 0.5) const;
    
    // ==================== Kategoria 6: Parametry Zaawansowane ====================
    
    /**
     * @brief 51. Wskaźnik Jakości Ruchu (Movement Quality Score)
     * MQS = F_peak / (SI * PSI + epsilon)
     */
    static double calculateMQS(double peakForce, double smoothness, double psi);
    
    /**
     * @brief 52. Indeks Obciążenia Neuromięśniowego (Neuromuscular Load Index)
     * NML = RFD_max * sigma_F * (T_set / N_reps)
     */
    static double calculateNML(double maxRFD, double forceStdDev, double setTime, int numReps);
    
    /**
     * @brief 53. Wskaźnik Stabilności Postawy (Postural Stability Estimate)
     */
    double calculatePSE(const std::vector<SensorSample>& samples) const;
    
    /**
     * @brief 54. Indeks Gotowości Treningowej (Training Readiness Index)
     * TRI = (F_peak_set / F_peak_historical) * (1 - CV_F/100)
     */
    static double calculateTRI(double currentSetPeak, double historicalPeak, double cvForce);
    
    /**
     * @brief 55. Wskaźnik Ryzyka Kontuzji (Injury Risk Indicator)
     * IRI = (RFD / F_peak) * (T_conc / T_ecc)
     */
    static double calculateIRI(double rfd, double peakForce, double concTime, double eccTime);
    
    /**
     * @brief 56. Indeks Adaptacji Treningowej (Training Adaptation Index)
     * TAI = ((P_set_3 - P_set_1) / P_set_1) * (1 / FI_inter)
     */
    static double calculateTAI(double thirdSetPower, double firstSetPower, double interSetFatigue);
    
    /**
     * @brief 57. Wskaźnik Ekonomii Ruchu (Movement Economy Index)
     * MEI = W / (I_total * F_set_mean)
     */
    static double calculateMEI(double work, double totalImpulse, double setMeanForce);
    
    /**
     * @brief 58. Indeks Zrównoważenia Obciążenia (Load Balance Index)
     * LBI = 1 - (|W1-W2| + |W2-W3| + |W1-W3|) / (3 * W_avg)
     */
    static double calculateLBI(double work1, double work2, double work3);
    
    /**
     * @brief 59. Wskaźnik Rezerwy Funkcjonalnej (Functional Reserve Index)
     * FRI = ((F_max_sensor - F_peak_set) / F_max_sensor) * 100%
     */
    static double calculateFRI(double peakForce, double maxSensorForce);
    
    /**
     * @brief 60. Kompleksowy Wynik Treningowy (Composite Training Score)
     * CTS = w1*(F_grand/F_ref) + w2*(1-CV_F) + w3*MCI + w4*(1-FI_inter)
     */
    static double calculateCTS(double grandMeanForce, double refForce, double cvForce, 
                               double mci, double interSetFatigue,
                               double w1 = 0.25, double w2 = 0.25, double w3 = 0.25, double w4 = 0.25);
    
    // ==================== Kategoria 7: Parametry Dodatkowe ====================
    
    /**
     * @brief 61. Gradient Zmęczenia (Fatigue Gradient)
     * FG = (FI_set_1 + FI_set_2 + FI_set_3) / 3
     */
    static double calculateFG(double fatigueIndex1, double fatigueIndex2, double fatigueIndex3);
    
    /**
     * @brief 62. Indeks Eksplozywnej Siły (Explosive Strength Index)
     * ESI = F_peak / T_conc
     */
    static double calculateESI(double peakForce, double concTime);
    
    /**
     * @brief 63. Współczynnik Powrotu do Bazy (Baseline Recovery Coefficient)
     * BRC = F_min / F_baseline
     */
    static double calculateBRC(double minForce, double baselineForce);
    
    /**
     * @brief 64. Indeks Kontrolowanego Opuszczania (Controlled Eccentric Index)
     * CEI = (1/T_ecc) * integral(F(t)/F_peak dt) od t_peak do t_end
     */
    double calculateCEI(const std::vector<SensorSample>& samples, double peakForce, 
                        uint32_t peakTime, uint32_t endTime) const;
    
    /**
     * @brief 65. Wskaźnik Postępu Longitudinalnego (Longitudinal Progress Tracker)
     * LPT = ((CTS_current - CTS_previous) / CTS_previous) * 100%
     */
    static double calculateLPT(double currentCTS, double previousCTS);
    
    // Gettery i settery
    void setElasticConstant(double k) { k_elastic_ = k; }
    void setPatientMass(double m) { patient_mass_ = m; }
    void setSampleRate(double rate) { sample_rate_ = rate; }
    
    double getElasticConstant() const { return k_elastic_; }
    double getPatientMass() const { return patient_mass_; }
    double getSampleRate() const { return sample_rate_; }
    
private:
    double k_elastic_;      // Stała sprężystości taśmy [N/m]
    double patient_mass_;   // Masa pacjenta [kg]
    double sample_rate_;    // Częstotliwość próbkowania [Hz]
    double dt_;             // Interwał próbkowania [s]
    
    // Funkcje pomocnicze
    static std::vector<double> extractForces(const std::vector<SensorSample>& samples);
    static std::vector<double> extractTimes(const std::vector<SensorSample>& samples);
    double calculateDerivative(const std::vector<SensorSample>& samples, size_t index) const;
    double calculateSecondDerivative(const std::vector<SensorSample>& samples, size_t index) const;
};

#endif // FORMULAS_HPP
