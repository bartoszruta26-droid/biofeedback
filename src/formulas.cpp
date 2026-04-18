#include "formulas.hpp"
#include <cmath>
#include <algorithm>
#include <numeric>

// ============================================================================
// BiomechanicalParams - Implementacja
// ============================================================================

BiomechanicalParams::BiomechanicalParams() {
    reset();
}

void BiomechanicalParams::reset() {
    // Kategoria 1: Parametry Siłowe
    F_peak = 0.0; F_mean = 0.0; F_min = 0.0; F_range = 0.0;
    F_set_mean = 0.0; F_set_peak = 0.0; W_total = 0.0; W_rep_avg = 0.0;
    I_impulse = 0.0; I_set_avg = 0.0;
    
    // Kategoria 2: Parametry Czasowe
    T_rep = 0.0; T_conc = 0.0; T_ecc = 0.0; R_CE = 0.0;
    T_rest_rep = 0.0; T_set = 0.0; T_rest_set = 0.0; T_rep_avg = 0.0;
    v_avg = 0.0; f_rep = 0.0;
    
    // Kategoria 3: Parametry Mocy i Energii
    P_peak = 0.0; P_rep_avg = 0.0; P_set_avg = 0.0; E_total = 0.0;
    P_norm = 0.0; RFD = 0.0; RFD_set_avg = 0.0; RFD_max = 0.0;
    E_elastic = 0.0; eta = 0.0;
    
    // Kategoria 4: Parametry Zmienności
    sigma_F = 0.0; CV_F = 0.0; FI_set = 0.0; FI_inter = 0.0;
    MCI = 0.0; PSI = 0.0; Var_T = 0.0; SI = 0.0; H_entropy = 0.0; BPR = 0.0;
    
    // Kategoria 5: Parametry Postępu
    F_grand_mean = 0.0; I_total = 0.0; V_training = 0.0; INT = 0.0;
    D_training = 0.0; SEI = 0.0; FMC = 0.0; ISI = 0.0;
    TUT_avg = 0.0; TUT_total = 0.0;
    
    // Kategoria 6: Parametry Zaawansowane (51-60)
    MQS = 0.0; NML = 0.0; PSE = 0.0;
    TRI = 0.0; IRI = 0.0; TAI = 0.0; MEI = 0.0;
    LBI = 0.0; FRI = 0.0; CTS = 0.0;
    
    // Kategoria 7: Parametry Dodatkowe (61-65)
    FG = 0.0; ESI = 0.0; BRC = 0.0; CEI = 0.0; LPT = 0.0;
}

// ============================================================================
// Formulas - Implementacja
// ============================================================================

Formulas::Formulas(double k_elastic, double patientMass, double sampleRate)
    : k_elastic_(k_elastic)
    , patient_mass_(patientMass)
    , sample_rate_(sampleRate)
    , dt_(1.0 / sampleRate)
{
}

std::vector<double> Formulas::extractForces(const std::vector<SensorSample>& samples) {
    std::vector<double> forces;
    forces.reserve(samples.size());
    for (const auto& s : samples) {
        if (s.isValid) {
            forces.push_back(s.force);
        }
    }
    return forces;
}

std::vector<double> Formulas::extractTimes(const std::vector<SensorSample>& samples) {
    std::vector<double> times;
    times.reserve(samples.size());
    uint32_t baseTime = samples.empty() ? 0 : samples[0].timestamp;
    for (const auto& s : samples) {
        if (s.isValid) {
            times.push_back((s.timestamp - baseTime) / 1000.0);  // Konwersja na sekundy
        }
    }
    return times;
}

double Formulas::calculateDerivative(const std::vector<SensorSample>& samples, size_t index) const {
    if (index == 0 || index >= samples.size() - 1) return 0.0;
    double df = samples[index + 1].force - samples[index - 1].force;
    double dt = (samples[index + 1].timestamp - samples[index - 1].timestamp) / 1000.0;
    return (dt > 0) ? df / dt : 0.0;
}

double Formulas::calculateSecondDerivative(const std::vector<SensorSample>& samples, size_t index) const {
    if (index == 0 || index >= samples.size() - 2) return 0.0;
    double d2f = samples[index + 1].force - 2.0 * samples[index].force + samples[index - 1].force;
    double dt2 = std::pow((samples[index + 1].timestamp - samples[index].timestamp) / 1000.0, 2);
    return (dt2 > 0) ? d2f / dt2 : 0.0;
}

// ==================== Kategoria 1: Parametry Siłowe ====================

double Formulas::calculatePeakForce(const std::vector<double>& forces) {
    if (forces.empty()) return 0.0;
    return *std::max_element(forces.begin(), forces.end());
}

double Formulas::calculateMeanForce(const std::vector<double>& forces) {
    if (forces.empty()) return 0.0;
    double sum = std::accumulate(forces.begin(), forces.end(), 0.0);
    return sum / forces.size();
}

double Formulas::calculateMinForce(const std::vector<double>& forces) {
    if (forces.empty()) return 0.0;
    return *std::min_element(forces.begin(), forces.end());
}

double Formulas::calculateForceRange(double peakForce, double minForce) {
    return peakForce - minForce;
}

double Formulas::calculateSetMeanForce(const std::vector<double>& repMeanForces) {
    return calculateMeanForce(repMeanForces);
}

double Formulas::calculateSetPeakForce(const std::vector<double>& repPeakForces) {
    return calculatePeakForce(repPeakForces);
}

double Formulas::calculateWork(const std::vector<SensorSample>& samples) const {
    if (samples.size() < 2) return 0.0;
    double work = 0.0;
    for (size_t i = 0; i < samples.size() - 1; ++i) {
        double dF = samples[i + 1].force - samples[i].force;
        work += samples[i].force * dF;
    }
    return work / k_elastic_;
}

double Formulas::calculateAvgWorkPerRep(double totalWork, int numReps) {
    return (numReps > 0) ? totalWork / numReps : 0.0;
}

double Formulas::calculateImpulse(const std::vector<SensorSample>& samples) const {
    if (samples.empty()) return 0.0;
    double impulse = 0.0;
    for (const auto& s : samples) {
        impulse += s.force * dt_;
    }
    return impulse;
}

double Formulas::calculateSetAvgImpulse(const std::vector<double>& repImpulses) {
    return calculateMeanForce(repImpulses);
}

// ==================== Kategoria 2: Parametry Czasowe ====================

double Formulas::calculateRepDuration(uint32_t startTime, uint32_t endTime) {
    return (endTime - startTime) / 1000.0;  // Konwersja ms -> s
}

double Formulas::calculateConcentricTime(uint32_t startTime, uint32_t peakTime) {
    return (peakTime - startTime) / 1000.0;
}

double Formulas::calculateEccentricTime(uint32_t peakTime, uint32_t endTime) {
    return (endTime - peakTime) / 1000.0;
}

double Formulas::calculateCERatio(double concTime, double eccTime) {
    return (eccTime > 0) ? concTime / eccTime : 0.0;
}

double Formulas::calculateInterRepRest(uint32_t prevEndTime, uint32_t nextStartTime) {
    return (nextStartTime > prevEndTime) ? (nextStartTime - prevEndTime) / 1000.0 : 0.0;
}

double Formulas::calculateSetDuration(uint32_t startTime, uint32_t endTime) {
    return (endTime - startTime) / 1000.0;
}

double Formulas::calculateInterSetRest(uint32_t prevSetEnd, uint32_t nextSetStart) {
    return (nextSetStart > prevSetEnd) ? (nextSetStart - prevSetEnd) / 1000.0 : 0.0;
}

double Formulas::calculateAvgRepTime(const std::vector<double>& repTimes) {
    return calculateMeanForce(repTimes);
}

double Formulas::calculateVelocity(double forceRange, double repDuration) const {
    return (repDuration > 0) ? forceRange / (k_elastic_ * repDuration) : 0.0;
}

double Formulas::calculateRepFrequency(int numReps, double setDuration) {
    return (setDuration > 0) ? numReps / setDuration : 0.0;
}

// ==================== Kategoria 3: Parametry Mocy i Energii ====================

double Formulas::calculatePeakPower(double peakForce, const std::vector<SensorSample>& samples) const {
    if (samples.empty()) return 0.0;
    double maxVel = 0.0;
    for (size_t i = 1; i < samples.size(); ++i) {
        double vel = std::abs(calculateDerivative(samples, i)) / k_elastic_;
        if (vel > maxVel) maxVel = vel;
    }
    return peakForce * maxVel;
}

double Formulas::calculateAvgPower(double work, double duration) {
    return (duration > 0) ? work / duration : 0.0;
}

double Formulas::calculateSetAvgPower(double totalWork, double setDuration) {
    return calculateAvgPower(totalWork, setDuration);
}

double Formulas::calculateTotalEnergy(const std::vector<double>& setWorks) {
    return std::accumulate(setWorks.begin(), setWorks.end(), 0.0);
}

double Formulas::calculateNormalizedPower(double power) const {
    return (patient_mass_ > 0) ? power / patient_mass_ : 0.0;
}

double Formulas::calculateRFD(double peakForce, double startForce, double concTime) {
    return (concTime > 0) ? (peakForce - startForce) / concTime : 0.0;
}

double Formulas::calculateSetAvgRFD(const std::vector<double>& repRFDs) {
    return calculateMeanForce(repRFDs);
}

double Formulas::calculateMaxRFD(const std::vector<double>& repRFDs) {
    if (repRFDs.empty()) return 0.0;
    return *std::max_element(repRFDs.begin(), repRFDs.end());
}

double Formulas::calculateElasticEnergy(double peakForce) const {
    return (k_elastic_ > 0) ? (peakForce * peakForce) / (2.0 * k_elastic_) : 0.0;
}

double Formulas::calculateEfficiency(double work, double metabolicEstimate) const {
    return (metabolicEstimate > 0) ? (work / metabolicEstimate) * 100.0 : 0.0;
}

// ==================== Kategoria 4: Parametry Zmienności ====================

double Formulas::calculateStdDev(const std::vector<double>& values) {
    if (values.size() < 2) return 0.0;
    double mean = calculateMeanForce(values);
    double sqSum = 0.0;
    for (double v : values) {
        sqSum += (v - mean) * (v - mean);
    }
    return std::sqrt(sqSum / (values.size() - 1));
}

double Formulas::calculateCV(double stdDev, double mean) {
    return (mean > 0) ? (stdDev / mean) * 100.0 : 0.0;
}

double Formulas::calculateFatigueIndex(double firstPeak, double lastPeak) {
    return (firstPeak > 0) ? ((firstPeak - lastPeak) / firstPeak) * 100.0 : 0.0;
}

double Formulas::calculateInterSetFatigue(double firstSetPeak, double thirdSetPeak) {
    return calculateFatigueIndex(firstSetPeak, thirdSetPeak);
}

double Formulas::calculateMCI(double timeStdDev, double avgTime) {
    return (avgTime > 0) ? 1.0 - (timeStdDev / avgTime) : 0.0;
}

double Formulas::calculatePSI(double concTime, double eccTime) {
    return (eccTime > 0) ? std::abs(1.0 - (concTime / eccTime)) : 0.0;
}

double Formulas::calculateTimeVariance(const std::vector<double>& repTimes) {
    if (repTimes.size() < 2) return 0.0;
    double mean = calculateAvgRepTime(repTimes);
    double sqSum = 0.0;
    for (double t : repTimes) {
        sqSum += (t - mean) * (t - mean);
    }
    return sqSum / (repTimes.size() - 1);
}

double Formulas::calculateSmoothnessIndex(const std::vector<SensorSample>& samples) const {
    if (samples.size() < 3) return 0.0;
    double sum = 0.0;
    for (size_t i = 1; i < samples.size() - 1; ++i) {
        sum += std::abs(calculateSecondDerivative(samples, i));
    }
    return sum / (samples.size() - 2);
}

double Formulas::calculateEntropy(const std::vector<double>& forces, int numBins) {
    if (forces.empty()) return 0.0;
    
    double minF = *std::min_element(forces.begin(), forces.end());
    double maxF = *std::max_element(forces.begin(), forces.end());
    double range = maxF - minF;
    
    if (range <= 0) return 0.0;
    
    std::vector<int> histogram(numBins, 0);
    double binWidth = range / numBins;
    
    for (double f : forces) {
        int bin = static_cast<int>((f - minF) / binWidth);
        if (bin >= numBins) bin = numBins - 1;
        histogram[bin]++;
    }
    
    double entropy = 0.0;
    double n = forces.size();
    for (int count : histogram) {
        if (count > 0) {
            double p = count / n;
            entropy -= p * std::log2(p);
        }
    }
    
    return entropy;
}

double Formulas::calculateBPR(double restStdDev, double restMean) {
    return (restMean > 0) ? 1.0 / (1.0 + (restStdDev / restMean)) : 0.0;
}

// ==================== Kategoria 5: Parametry Postępu ====================

double Formulas::calculateGrandMeanForce(const std::vector<double>& setMeans) {
    return calculateMeanForce(setMeans);
}

double Formulas::calculateTotalImpulse(const std::vector<double>& repImpulses) {
    return std::accumulate(repImpulses.begin(), repImpulses.end(), 0.0);
}

double Formulas::calculateTrainingVolume(const std::vector<std::pair<double, double>>& repData) {
    double volume = 0.0;
    for (const auto& p : repData) {
        volume += p.first * p.second;  // F_mean * F_range
    }
    return volume;
}

double Formulas::calculateIntensity(double grandMean, double maxReference) {
    return (maxReference > 0) ? (grandMean / maxReference) * 100.0 : 0.0;
}

double Formulas::calculateDensity(int numSets, int numReps, double totalTime) {
    return (totalTime > 0) ? (numSets * numReps) / totalTime : 0.0;
}

double Formulas::calculateSEI(double work, double setDuration, double setMeanForce) {
    return (setDuration > 0 && setMeanForce > 0) ? work / (setDuration * setMeanForce) : 0.0;
}

double Formulas::calculateFMC(double thirdSetMean, double firstSetMean) {
    return (firstSetMean > 0) ? (thirdSetMean / firstSetMean) * 100.0 : 0.0;
}

double Formulas::calculateISI(double secondSetPeak, double firstSetPeak) {
    return (firstSetPeak > 0) ? ((secondSetPeak - firstSetPeak) / firstSetPeak) * 100.0 : 0.0;
}

double Formulas::calculateAvgTUT(const std::vector<std::vector<SensorSample>>& repSamples,
                                  double thresholdFactor) const {
    if (repSamples.empty()) return 0.0;
    
    double totalTUT = 0.0;
    for (const auto& reps : repSamples) {
        if (reps.empty()) continue;
        
        double maxF = 0.0;
        for (const auto& s : reps) {
            if (s.force > maxF) maxF = s.force;
        }
        double threshold = maxF * thresholdFactor;
        
        double tut = 0.0;
        for (size_t i = 1; i < reps.size(); ++i) {
            if (reps[i].force > threshold) {
                tut += (reps[i].timestamp - reps[i-1].timestamp) / 1000.0;
            }
        }
        totalTUT += tut;
    }
    
    return totalTUT / repSamples.size();
}

double Formulas::calculateTotalTUT(const std::vector<std::vector<SensorSample>>& repSamples,
                                    double thresholdFactor) const {
    double totalTUT = 0.0;
    for (const auto& reps : repSamples) {
        if (reps.empty()) continue;
        
        double maxF = 0.0;
        for (const auto& s : reps) {
            if (s.force > maxF) maxF = s.force;
        }
        double threshold = maxF * thresholdFactor;
        
        for (size_t i = 1; i < reps.size(); ++i) {
            if (reps[i].force > threshold) {
                totalTUT += (reps[i].timestamp - reps[i-1].timestamp) / 1000.0;
            }
        }
    }
    
    return totalTUT;
}

// ==================== Kategoria 6: Parametry Zaawansowane ====================

double Formulas::calculateMQS(double peakForce, double smoothness, double psi) {
    const double epsilon = 0.001;
    return peakForce / (smoothness * psi + epsilon);
}

double Formulas::calculateNML(double maxRFD, double forceStdDev, double setTime, int numReps) {
    return (numReps > 0) ? maxRFD * forceStdDev * (setTime / numReps) : 0.0;
}

double Formulas::calculatePSE(const std::vector<SensorSample>& samples) const {
    if (samples.size() < 2) return 0.0;
    
    std::vector<double> derivatives;
    for (size_t i = 1; i < samples.size(); ++i) {
        derivatives.push_back(std::abs(calculateDerivative(samples, i)));
    }
    
    double meanDeriv = calculateMeanForce(derivatives);
    double stdDevDeriv = calculateStdDev(derivatives);
    
    return (meanDeriv > 0) ? 1.0 / (1.0 + (stdDevDeriv / meanDeriv)) : 0.0;
}

// ==================== Kategoria 6: Parametry Zaawansowane (54-60) ====================

double Formulas::calculateTRI(double currentSetPeak, double historicalPeak, double cvForce) {
    if (historicalPeak <= 0) return 0.0;
    double consistencyFactor = 1.0 - (cvForce / 100.0);
    if (consistencyFactor < 0) consistencyFactor = 0;
    return (currentSetPeak / historicalPeak) * consistencyFactor;
}

double Formulas::calculateIRI(double rfd, double peakForce, double concTime, double eccTime) {
    if (peakForce <= 0 || eccTime <= 0) return 0.0;
    return (rfd / peakForce) * (concTime / eccTime);
}

double Formulas::calculateTAI(double thirdSetPower, double firstSetPower, double interSetFatigue) {
    if (firstSetPower <= 0 || interSetFatigue <= 0) return 0.0;
    double powerChange = (thirdSetPower - firstSetPower) / firstSetPower;
    return powerChange * (1.0 / interSetFatigue);
}

double Formulas::calculateMEI(double work, double totalImpulse, double setMeanForce) {
    if (totalImpulse <= 0 || setMeanForce <= 0) return 0.0;
    return work / (totalImpulse * setMeanForce);
}

double Formulas::calculateLBI(double work1, double work2, double work3) {
    double avgWork = (work1 + work2 + work3) / 3.0;
    if (avgWork <= 0) return 0.0;
    double sumDiff = std::abs(work1 - work2) + std::abs(work2 - work3) + std::abs(work1 - work3);
    return 1.0 - (sumDiff / (3.0 * avgWork));
}

double Formulas::calculateFRI(double peakForce, double maxSensorForce) {
    if (maxSensorForce <= 0) return 0.0;
    return ((maxSensorForce - peakForce) / maxSensorForce) * 100.0;
}

double Formulas::calculateCTS(double grandMeanForce, double refForce, double cvForce,
                              double mci, double interSetFatigue,
                              double w1, double w2, double w3, double w4) {
    if (refForce <= 0) return 0.0;
    
    // Normalizacja składników do [0, 1]
    double forceComponent = grandMeanForce / refForce;
    if (forceComponent > 1.0) forceComponent = 1.0;
    
    double consistencyComponent = 1.0 - (cvForce / 100.0);
    if (consistencyComponent < 0) consistencyComponent = 0;
    if (consistencyComponent > 1.0) consistencyComponent = 1.0;
    
    double mciComponent = mci;
    if (mciComponent < 0) mciComponent = 0;
    if (mciComponent > 1.0) mciComponent = 1.0;
    
    double fatigueComponent = 1.0 - (interSetFatigue / 100.0);
    if (fatigueComponent < 0) fatigueComponent = 0;
    if (fatigueComponent > 1.0) fatigueComponent = 1.0;
    
    return w1 * forceComponent + w2 * consistencyComponent + 
           w3 * mciComponent + w4 * fatigueComponent;
}

// ==================== Kategoria 7: Parametry Dodatkowe (61-65) ====================

double Formulas::calculateFG(double fatigueIndex1, double fatigueIndex2, double fatigueIndex3) {
    return (fatigueIndex1 + fatigueIndex2 + fatigueIndex3) / 3.0;
}

double Formulas::calculateESI(double peakForce, double concTime) {
    return (concTime > 0) ? peakForce / concTime : 0.0;
}

double Formulas::calculateBRC(double minForce, double baselineForce) {
    return (baselineForce > 0) ? minForce / baselineForce : 0.0;
}

double Formulas::calculateCEI(const std::vector<SensorSample>& samples, double peakForce,
                               uint32_t peakTime, uint32_t endTime) const {
    if (samples.empty() || peakForce <= 0) return 0.0;
    
    // Znajdź próbki w fazie ekscentrycznej (od peakTime do endTime)
    std::vector<SensorSample> eccentricSamples;
    for (const auto& s : samples) {
        if (s.timestamp >= peakTime && s.timestamp <= endTime) {
            eccentricSamples.push_back(s);
        }
    }
    
    if (eccentricSamples.size() < 2) return 0.0;
    
    double integral = 0.0;
    for (size_t i = 0; i < eccentricSamples.size() - 1; ++i) {
        double avgF = (eccentricSamples[i].force + eccentricSamples[i+1].force) / 2.0;
        double dt = (eccentricSamples[i+1].timestamp - eccentricSamples[i].timestamp) / 1000.0;
        integral += (avgF / peakForce) * dt;
    }
    
    double tEcc = (endTime - peakTime) / 1000.0;
    return (tEcc > 0) ? integral / tEcc : 0.0;
}

double Formulas::calculateLPT(double currentCTS, double previousCTS) {
    if (previousCTS <= 0) return 0.0;
    return ((currentCTS - previousCTS) / previousCTS) * 100.0;
}

BiomechanicalParams Formulas::calculateRepParams(const RepetitionData& rep) {
    BiomechanicalParams params;
    
    std::vector<double> forces = extractForces(rep.samples);
    if (forces.empty()) return params;
    
    // Parametry siłowe
    params.F_peak = calculatePeakForce(forces);
    params.F_mean = calculateMeanForce(forces);
    params.F_min = calculateMinForce(forces);
    params.F_range = calculateForceRange(params.F_peak, params.F_min);
    
    // Parametry czasowe
    params.T_rep = calculateRepDuration(rep.startTime, rep.endTime);
    params.T_conc = calculateConcentricTime(rep.startTime, rep.peakTime);
    params.T_ecc = calculateEccentricTime(rep.peakTime, rep.endTime);
    params.R_CE = calculateCERatio(params.T_conc, params.T_ecc);
    
    // Praca i impuls
    params.W_total = calculateWork(rep.samples);
    params.I_impulse = calculateImpulse(rep.samples);
    
    // Moc
    params.P_peak = calculatePeakPower(params.F_peak, rep.samples);
    params.P_rep_avg = calculateAvgPower(params.W_total, params.T_rep);
    
    // RFD
    params.RFD = calculateRFD(params.F_peak, rep.startForce, params.T_conc);
    
    // Energia sprężysta
    params.E_elastic = calculateElasticEnergy(params.F_peak);
    
    // Zmienność (dla pojedynczego powtórzenia ograniczone)
    params.SI = calculateSmoothnessIndex(rep.samples);
    params.PSI = calculatePSI(params.T_conc, params.T_ecc);
    params.H_entropy = calculateEntropy(forces);
    
    return params;
}

BiomechanicalParams Formulas::calculateAllParams(const SeriesData& series) {
    BiomechanicalParams params;
    
    if (series.reps.empty()) return params;
    
    std::vector<double> repPeakForces;
    std::vector<double> repMeanForces;
    std::vector<double> repImpulses;
    std::vector<double> repTimes;
    std::vector<double> repRFDs;
    std::vector<std::vector<SensorSample>> allRepSamples;
    
    // Oblicz parametry dla każdego powtórzenia
    for (const auto& rep : series.reps) {
        BiomechanicalParams repParams = calculateRepParams(rep);
        repPeakForces.push_back(repParams.F_peak);
        repMeanForces.push_back(repParams.F_mean);
        repImpulses.push_back(repParams.I_impulse);
        repTimes.push_back(repParams.T_rep);
        repRFDs.push_back(repParams.RFD);
        allRepSamples.push_back(rep.samples);
        
        params.W_total += repParams.W_total;
    }
    
    // Średnie parametry siłowe serii
    params.F_set_mean = calculateSetMeanForce(repMeanForces);
    params.F_set_peak = calculateSetPeakForce(repPeakForces);
    params.F_peak = params.F_set_peak;  // Maksymalna z wszystkich
    params.F_mean = params.F_set_mean;
    
    // Praca średnia na powtórzenie
    params.W_rep_avg = calculateAvgWorkPerRep(params.W_total, series.reps.size());
    
    // Impuls średni
    params.I_set_avg = calculateSetAvgImpulse(repImpulses);
    params.I_impulse = params.I_set_avg;
    
    // Parametry czasowe serii
    params.T_set = calculateSetDuration(series.startTime, series.endTime);
    params.T_rep_avg = calculateAvgRepTime(repTimes);
    params.f_rep = calculateRepFrequency(series.reps.size(), params.T_set);
    
    // Średni czas koncentryczny i ekscentryczny
    double totalConc = 0.0, totalEcc = 0.0;
    for (const auto& rep : series.reps) {
        totalConc += (rep.peakTime - rep.startTime) / 1000.0;
        totalEcc += (rep.endTime - rep.peakTime) / 1000.0;
    }
    params.T_conc = totalConc / series.reps.size();
    params.T_ecc = totalEcc / series.reps.size();
    params.R_CE = calculateCERatio(params.T_conc, params.T_ecc);
    
    // Moc
    params.P_set_avg = calculateSetAvgPower(params.W_total, params.T_set);
    params.P_rep_avg = calculateAvgPower(params.W_total / series.reps.size(), params.T_rep_avg);
    params.P_peak = calculatePeakPower(params.F_set_peak, series.reps.back().samples);
    
    // RFD
    params.RFD_set_avg = calculateSetAvgRFD(repRFDs);
    params.RFD_max = calculateMaxRFD(repRFDs);
    params.RFD = params.RFD_set_avg;
    
    // Energia
    params.E_total = params.W_total;  // Dla jednej serii
    params.E_elastic = calculateElasticEnergy(params.F_set_peak);
    params.P_norm = calculateNormalizedPower(params.P_set_avg);
    
    // Zmienność
    params.sigma_F = calculateStdDev(repMeanForces);
    params.CV_F = calculateCV(params.sigma_F, params.F_set_mean);
    params.FI_set = calculateFatigueIndex(repPeakForces.front(), repPeakForces.back());
    params.Var_T = calculateTimeVariance(repTimes);
    params.MCI = calculateMCI(std::sqrt(params.Var_T), params.T_rep_avg);
    
    // Średni SI i PSI
    double totalSI = 0.0, totalPSI = 0.0;
    for (const auto& rep : series.reps) {
        totalSI += calculateSmoothnessIndex(rep.samples);
        double tConc = (rep.peakTime - rep.startTime) / 1000.0;
        double tEcc = (rep.endTime - rep.peakTime) / 1000.0;
        totalPSI += calculatePSI(tConc, tEcc);
    }
    params.SI = totalSI / series.reps.size();
    params.PSI = totalPSI / series.reps.size();
    
    // Entropia ze wszystkich próbek
    std::vector<double> allForces;
    for (const auto& rep : series.reps) {
        auto forces = extractForces(rep.samples);
        allForces.insert(allForces.end(), forces.begin(), forces.end());
    }
    params.H_entropy = calculateEntropy(allForces);
    
    // Jakość ruchu
    params.MQS = calculateMQS(params.F_peak, params.SI, params.PSI);
    
    // Obciążenie neuromięśniowe
    params.NML = calculateNML(params.RFD_max, params.sigma_F, params.T_set, series.reps.size());
    
    // Stabilność postawy
    std::vector<SensorSample> allSamples;
    for (const auto& rep : series.reps) {
        allSamples.insert(allSamples.end(), rep.samples.begin(), rep.samples.end());
    }
    params.PSE = calculatePSE(allSamples);
    
    // Czas pod napięciem
    params.TUT_avg = calculateAvgTUT(allRepSamples);
    params.TUT_total = calculateTotalTUT(allRepSamples);

    // ==================== Kategoria 6: Parametry Zaawansowane (54-60) ====================

    // 54. Indeks Gotowości Treningowej (TRI) - wymaga historycznego maksimum
    // Dla pojedynczej serii używamy F_set_peak jako currentSetPeak
    // F_peak_historical musi być przekazane zewnętrznie lub wczytane z historii pacjenta
    double historicalPeak = params.F_set_peak * 1.1;  // Przyjmujemy 110% aktualnego jako historyczne maksimum
    params.TRI = calculateTRI(params.F_set_peak, historicalPeak, params.CV_F);

    // 55. Wskaźnik Ryzyka Kontuzji (IRI) - średni dla serii
    double totalIRI = 0.0;
    for (const auto& rep : series.reps) {
        double rfd = calculateRFD(rep.peakForce, rep.startForce, 
                                  (rep.peakTime - rep.startTime) / 1000.0);
        double iri = calculateIRI(rfd, rep.peakForce,
                                  (rep.peakTime - rep.startTime) / 1000.0,
                                  (rep.endTime - rep.peakTime) / 1000.0);
        totalIRI += iri;
    }
    params.IRI = totalIRI / series.reps.size();

    // 56. Indeks Adaptacji Treningowej (TAI) - wymaga danych z wielu serii
    // Dla pojedynczej serii przyjmujemy domyślną wartość
    // TAI wymaga porównania mocy między serią 1 a 3
    params.TAI = 0.0;  // Wymaga danych z co najmniej 3 serii do obliczenia

    // 57. Wskaźnik Ekonomii Ruchu (MEI)
    params.MEI = calculateMEI(params.W_total, params.I_impulse * series.reps.size(), params.F_set_mean);

    // 58. Indeks Zrównoważenia Obciążenia (LBI) - wymaga danych z 3 serii
    // Dla pojedynczej serii przyjmujemy idealne zrównoważenie
    params.LBI = 1.0;  // Wymaga danych z co najmniej 3 serii do obliczenia

    // 59. Wskaźnik Rezerwy Funkcjonalnej (FRI)
    double maxSensorForce = 500.0;  // Maksymalna siła czujnika [N] - powinna być konfiguracją
    params.FRI = calculateFRI(params.F_set_peak, maxSensorForce);

    // 60. Kompleksowy Wynik Treningowy (CTS)
    double refForce = historicalPeak;  // Siła referencyjna
    params.CTS = calculateCTS(params.F_set_mean, refForce, params.CV_F, 
                              params.MCI, params.FI_inter);

    // ==================== Kategoria 7: Parametry Dodatkowe (61-65) ====================

    // 61. Gradient Zmęczenia (FG) - wymaga danych z 3 serii
    // Dla pojedynczej serii używamy FI_set jako wszystkich trzech wartości
    params.FG = calculateFG(params.FI_set, params.FI_set, params.FI_set);

    // 62. Indeks Eksplozywnej Siły (ESI) - średni dla serii
    double totalESI = 0.0;
    for (const auto& rep : series.reps) {
        double esi = calculateESI(rep.peakForce, (rep.peakTime - rep.startTime) / 1000.0);
        totalESI += esi;
    }
    params.ESI = totalESI / series.reps.size();

    // 63. Współczynnik Powrotu do Bazy (BRC) - średni dla serii
    double baselineForce = 10.0;  // Siła bazowa/spoczynkowa [N] - powinna być zmierzona przed ćwiczeniem
    double totalBRC = 0.0;
    for (const auto& rep : series.reps) {
        double brc = calculateBRC(rep.endForce, baselineForce);
        totalBRC += brc;
    }
    params.BRC = totalBRC / series.reps.size();

    // 64. Indeks Kontrolowanego Opuszczania (CEI) - średni dla serii
    double totalCEI = 0.0;
    for (const auto& rep : series.reps) {
        double cei = calculateCEI(rep.samples, rep.peakForce, rep.peakTime, rep.endTime);
        totalCEI += cei;
    }
    params.CEI = totalCEI / series.reps.size();

    // 65. Wskaźnik Postępu Longitudinalnego (LPT) - wymaga poprzedniej sesji
    // Dla pojedynczej sesji przyjmujemy 0 (brak porównania)
    params.LPT = 0.0;  // Wymaga danych z poprzedniej sesji treningowej

    return params;
}
