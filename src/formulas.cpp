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
    availability.fill(ParameterAvailability::InsufficientData);
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

const char* BiomechanicalParams::parameterUnit(std::size_t index) noexcept
{
    static constexpr const char* units[ParameterCount] = {
        "N", "N", "N", "N", "N", "N", "J", "J", "N·s", "N·s", "s", "s", "s", "—", "s", "s", "s", "s", "m/s", "Hz", "W", "W", "W", "J", "W/kg", "N/s", "N/s", "N/s", "J", "%", "N", "%", "%", "%", "—", "—", "s²", "N/s²", "bit", "—", "N", "N·s", "N²", "%", "powt./s", "—", "%", "%", "s", "s", "—", "—", "—", "—", "—", "—", "—", "—", "%", "—", "—", "—", "—", "—", "%"
    };
    return index < ParameterCount ? units[index] : "";
}

std::array<double, BiomechanicalParams::ParameterCount> BiomechanicalParams::values() const noexcept
{
    return {F_peak, F_mean, F_min, F_range, F_set_mean, F_set_peak, W_total, W_rep_avg, I_impulse, I_set_avg, T_rep, T_conc, T_ecc, R_CE, T_rest_rep, T_set, T_rest_set, T_rep_avg, v_avg, f_rep, P_peak, P_rep_avg, P_set_avg, E_total, P_norm, RFD, RFD_set_avg, RFD_max, E_elastic, eta, sigma_F, CV_F, FI_set, FI_inter, MCI, PSI, Var_T, SI, H_entropy, BPR, F_grand_mean, I_total, V_training, INT, D_training, SEI, FMC, ISI, TUT_avg, TUT_total, MQS, NML, PSE, TRI, IRI, TAI, MEI, LBI, FRI, CTS, FG, ESI, BRC, CEI, LPT};
}

void BiomechanicalParams::assignValues(
    const std::array<double, BiomechanicalParams::ParameterCount>& v) noexcept
{
    F_peak = v[0];
    F_mean = v[1];
    F_min = v[2];
    F_range = v[3];
    F_set_mean = v[4];
    F_set_peak = v[5];
    W_total = v[6];
    W_rep_avg = v[7];
    I_impulse = v[8];
    I_set_avg = v[9];
    T_rep = v[10];
    T_conc = v[11];
    T_ecc = v[12];
    R_CE = v[13];
    T_rest_rep = v[14];
    T_set = v[15];
    T_rest_set = v[16];
    T_rep_avg = v[17];
    v_avg = v[18];
    f_rep = v[19];
    P_peak = v[20];
    P_rep_avg = v[21];
    P_set_avg = v[22];
    E_total = v[23];
    P_norm = v[24];
    RFD = v[25];
    RFD_set_avg = v[26];
    RFD_max = v[27];
    E_elastic = v[28];
    eta = v[29];
    sigma_F = v[30];
    CV_F = v[31];
    FI_set = v[32];
    FI_inter = v[33];
    MCI = v[34];
    PSI = v[35];
    Var_T = v[36];
    SI = v[37];
    H_entropy = v[38];
    BPR = v[39];
    F_grand_mean = v[40];
    I_total = v[41];
    V_training = v[42];
    INT = v[43];
    D_training = v[44];
    SEI = v[45];
    FMC = v[46];
    ISI = v[47];
    TUT_avg = v[48];
    TUT_total = v[49];
    MQS = v[50];
    NML = v[51];
    PSE = v[52];
    TRI = v[53];
    IRI = v[54];
    TAI = v[55];
    MEI = v[56];
    LBI = v[57];
    FRI = v[58];
    CTS = v[59];
    FG = v[60];
    ESI = v[61];
    BRC = v[62];
    CEI = v[63];
    LPT = v[64];
}

const char* BiomechanicalParams::parameterName(std::size_t index) noexcept
{
    static constexpr const char* names[ParameterCount] = {
        "F_peak", "F_mean", "F_min", "F_range", "F_set_mean", "F_set_peak", "W_total", "W_rep_avg", "I_impulse", "I_set_avg", "T_rep", "T_conc", "T_ecc", "R_CE", "T_rest_rep", "T_set", "T_rest_set", "T_rep_avg", "v_avg", "f_rep", "P_peak", "P_rep_avg", "P_set_avg", "E_total", "P_norm", "RFD", "RFD_set_avg", "RFD_max", "E_elastic", "eta", "sigma_F", "CV_F", "FI_set", "FI_inter", "MCI", "PSI", "Var_T", "SI", "H_entropy", "BPR", "F_grand_mean", "I_total", "V_training", "INT", "D_training", "SEI", "FMC", "ISI", "TUT_avg", "TUT_total", "MQS", "NML", "PSE", "TRI", "IRI", "TAI", "MEI", "LBI", "FRI", "CTS", "FG", "ESI", "BRC", "CEI", "LPT"
    };
    return index < ParameterCount ? names[index] : "";
}

// ============================================================================
// Formulas - Implementacja
// ============================================================================

Formulas::Formulas(double k_elastic, double patientMass, double sampleRate)
    : k_elastic_(std::isfinite(k_elastic) && k_elastic > 0.0 ? k_elastic : 0.0)
    , patient_mass_(std::isfinite(patientMass) && patientMass > 0.0 ? patientMass : 0.0)
    , sample_rate_(std::isfinite(sampleRate) && sampleRate > 0.0 ? sampleRate : 0.0)
{
}

std::vector<double> Formulas::extractForces(const std::vector<SensorSample>& samples) {
    std::vector<double> forces;
    forces.reserve(samples.size());
    for (const auto& s : samples) {
        if (s.isValid && std::isfinite(s.force))
            forces.push_back(s.force);
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
    const auto& before = samples[index - 1];
    const auto& current = samples[index];
    const auto& after = samples[index + 1];
    if (!before.isValid || !current.isValid || !after.isValid ||
        !std::isfinite(before.force) || !std::isfinite(after.force))
        return 0.0;

    const std::uint32_t deltaMs = after.timestamp - before.timestamp;
    if (deltaMs == 0 || deltaMs >= 0x80000000u) return 0.0;
    const double dt = static_cast<double>(deltaMs) / 1000.0;
    return (after.force - before.force) / dt;
}

double Formulas::calculateSecondDerivative(const std::vector<SensorSample>& samples, size_t index) const {
    if (index == 0 || index >= samples.size() - 1) return 0.0;
    const auto& before = samples[index - 1];
    const auto& current = samples[index];
    const auto& after = samples[index + 1];
    if (!before.isValid || !current.isValid || !after.isValid ||
        !std::isfinite(before.force) || !std::isfinite(current.force) ||
        !std::isfinite(after.force))
        return 0.0;

    const std::uint32_t leftMs = current.timestamp - before.timestamp;
    const std::uint32_t rightMs = after.timestamp - current.timestamp;
    if (leftMs == 0 || rightMs == 0 ||
        leftMs >= 0x80000000u || rightMs >= 0x80000000u)
        return 0.0;

    const double left = static_cast<double>(leftMs) / 1000.0;
    const double right = static_cast<double>(rightMs) / 1000.0;
    const double leftSlope = (current.force - before.force) / left;
    const double rightSlope = (after.force - current.force) / right;
    return 2.0 * (rightSlope - leftSlope) / (left + right);
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
    if (samples.size() < 2 || !std::isfinite(k_elastic_) || k_elastic_ <= 0.0)
        return 0.0;
    double work = 0.0;
    for (size_t i = 0; i + 1 < samples.size(); ++i) {
        if (!samples[i].isValid || !samples[i + 1].isValid ||
            !std::isfinite(samples[i].force) || !std::isfinite(samples[i + 1].force))
            continue;
        const double dF = samples[i + 1].force - samples[i].force;
        work += samples[i].force * dF;
    }
    return work / k_elastic_;
}

double Formulas::calculateAvgWorkPerRep(double totalWork, int numReps) {
    return (numReps > 0) ? totalWork / numReps : 0.0;
}

double Formulas::calculateImpulse(const std::vector<SensorSample>& samples) const {
    if (samples.size() < 2) return 0.0;

    // Integrate the measured force against each actual device timestamp.
    // A configured nominal sample rate is not a substitute for observed dt:
    // USB/serial buffering and dropped frames make the interval non-uniform.
    double impulse = 0.0;
    for (std::size_t i = 0; i + 1 < samples.size(); ++i) {
        const auto& current = samples[i];
        const auto& next = samples[i + 1];
        if (!current.isValid || !next.isValid ||
            !std::isfinite(current.force) || !std::isfinite(next.force))
            continue;

        // Unsigned subtraction handles a single uint32 device-clock wrap.
        const std::uint32_t deltaMs = next.timestamp - current.timestamp;
        if (deltaMs == 0 || deltaMs >= 0x80000000u)
            continue;

        const double dt = static_cast<double>(deltaMs) / 1000.0;
        impulse += 0.5 * (current.force + next.force) * dt;
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
    if (!std::isfinite(forceRange) || !std::isfinite(repDuration) ||
        !std::isfinite(k_elastic_) || k_elastic_ <= 0.0 || repDuration <= 0.0)
        return 0.0;
    return forceRange / (k_elastic_ * repDuration);
}

double Formulas::calculateRepFrequency(int numReps, double setDuration) {
    return (setDuration > 0) ? numReps / setDuration : 0.0;
}

// ==================== Kategoria 3: Parametry Mocy i Energii ====================

double Formulas::calculatePeakPower(double peakForce, const std::vector<SensorSample>& samples) const {
    if (samples.size() < 3 || !std::isfinite(peakForce) ||
        !std::isfinite(k_elastic_) || k_elastic_ <= 0.0)
        return 0.0;
    double maxVel = 0.0;
    for (size_t i = 1; i + 1 < samples.size(); ++i) {
        const double velocity = std::abs(calculateDerivative(samples, i)) / k_elastic_;
        if (std::isfinite(velocity))
            maxVel = std::max(maxVel, velocity);
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
    return (std::isfinite(power) && std::isfinite(patient_mass_) && patient_mass_ > 0.0)
        ? power / patient_mass_ : 0.0;
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
    return (std::isfinite(peakForce) && std::isfinite(k_elastic_) && k_elastic_ > 0.0)
        ? (peakForce * peakForce) / (2.0 * k_elastic_) : 0.0;
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
    std::size_t validDerivatives = 0;
    for (size_t i = 1; i + 1 < samples.size(); ++i) {
        if (!samples[i - 1].isValid || !samples[i].isValid || !samples[i + 1].isValid)
            continue;
        const double derivative = calculateSecondDerivative(samples, i);
        if (!std::isfinite(derivative)) continue;
        sum += std::abs(derivative);
        ++validDerivatives;
    }
    return validDerivatives > 0 ? sum / static_cast<double>(validDerivatives) : 0.0;
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
    std::size_t validRepetitions = 0;
    for (const auto& reps : repSamples) {
        if (reps.size() < 2) continue;

        double maxF = 0.0;
        for (const auto& sample : reps) {
            if (sample.isValid && std::isfinite(sample.force))
                maxF = std::max(maxF, sample.force);
        }
        if (maxF <= 0.0) continue;
        const double threshold = maxF * thresholdFactor;

        double tut = 0.0;
        for (size_t i = 1; i < reps.size(); ++i) {
            if (!reps[i - 1].isValid || !reps[i].isValid ||
                reps[i - 1].force <= threshold || reps[i].force <= threshold)
                continue;
            const std::uint32_t deltaMs = reps[i].timestamp - reps[i - 1].timestamp;
            if (deltaMs == 0 || deltaMs >= 0x80000000u) continue;
            tut += static_cast<double>(deltaMs) / 1000.0;
        }
        totalTUT += tut;
        ++validRepetitions;
    }

    return validRepetitions > 0
        ? totalTUT / static_cast<double>(validRepetitions) : 0.0;
}

double Formulas::calculateTotalTUT(const std::vector<std::vector<SensorSample>>& repSamples,
                                    double thresholdFactor) const {
    double totalTUT = 0.0;
    for (const auto& reps : repSamples) {
        if (reps.empty()) continue;
        
        double maxF = 0.0;
        for (const auto& sample : reps) {
            if (sample.isValid && std::isfinite(sample.force))
                maxF = std::max(maxF, sample.force);
        }
        if (maxF <= 0.0) continue;
        const double threshold = maxF * thresholdFactor;

        for (size_t i = 1; i < reps.size(); ++i) {
            if (!reps[i - 1].isValid || !reps[i].isValid ||
                reps[i - 1].force <= threshold || reps[i].force <= threshold)
                continue;
            const std::uint32_t deltaMs = reps[i].timestamp - reps[i - 1].timestamp;
            if (deltaMs == 0 || deltaMs >= 0x80000000u) continue;
            totalTUT += static_cast<double>(deltaMs) / 1000.0;
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
    if (forces.size() < 3) return params;
    
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
    params.v_avg = calculateVelocity(params.F_range, params.T_rep);
    
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
    
    // A single repetition can support these metrics directly. Values that
    // require calibrated mechanics or additional context remain unavailable.
    for (std::size_t index : {0u, 1u, 2u, 3u, 10u, 11u, 12u, 13u,
                              25u, 35u, 37u, 38u})
        params.setAvailability(index, ParameterAvailability::Available);
    const bool hasElasticCalibration = std::isfinite(k_elastic_) && k_elastic_ > 0.0;
    const auto calibrationStatus = hasElasticCalibration
        ? ParameterAvailability::Available : ParameterAvailability::RequiresCalibration;
    for (std::size_t index : {6u, 18u, 20u, 21u, 28u})
        params.setAvailability(index, calibrationStatus);

    return params;
}

BiomechanicalParams Formulas::calculateAllParams(const SeriesData& series) {
    BiomechanicalParams params;
    
    if (series.reps.empty()) return params;

    const std::uint32_t setDurationMs = series.endTime - series.startTime;
    if (setDurationMs == 0 || setDurationMs >= 0x80000000u) {
        params.availability.fill(ParameterAvailability::InvalidInput);
        return params;
    }
    for (const auto& rep : series.reps) {
        const std::uint32_t durationMs = rep.endTime - rep.startTime;
        const std::uint32_t concentricMs = rep.peakTime - rep.startTime;
        const std::uint32_t eccentricMs = rep.endTime - rep.peakTime;
        const auto validSamples = std::count_if(
            rep.samples.begin(), rep.samples.end(),
            [](const SensorSample& sample) {
                return sample.isValid && std::isfinite(sample.force);
            });
        if (validSamples < 3 || durationMs == 0 || durationMs >= 0x80000000u ||
            concentricMs > durationMs || eccentricMs > durationMs) {
            params.availability.fill(ParameterAvailability::InvalidInput);
            return params;
        }
    }

    std::vector<double> repPeakForces;
    std::vector<double> repMeanForces;
    std::vector<double> repMinForces;
    std::vector<double> repImpulses;
    std::vector<double> repTimes;
    std::vector<double> repRFDs;
    std::vector<double> repPeakPowers;
    std::vector<std::vector<SensorSample>> allRepSamples;
    
    // Oblicz parametry dla każdego powtórzenia
    for (const auto& rep : series.reps) {
        BiomechanicalParams repParams = calculateRepParams(rep);
        repPeakForces.push_back(repParams.F_peak);
        repMeanForces.push_back(repParams.F_mean);
        repMinForces.push_back(repParams.F_min);
        repImpulses.push_back(repParams.I_impulse);
        repTimes.push_back(repParams.T_rep);
        repRFDs.push_back(repParams.RFD);
        repPeakPowers.push_back(repParams.P_peak);
        allRepSamples.push_back(rep.samples);
        
        params.W_total += repParams.W_total;
    }
    
    // Średnie parametry siłowe serii
    params.F_set_mean = calculateSetMeanForce(repMeanForces);
    params.F_set_peak = calculateSetPeakForce(repPeakForces);
    params.F_peak = params.F_set_peak;  // Maksymalna z wszystkich
    params.F_mean = params.F_set_mean;
    params.F_min = *std::min_element(repMinForces.begin(), repMinForces.end());
    params.F_range = calculateForceRange(params.F_peak, params.F_min);
    
    // Praca średnia na powtórzenie
    params.W_rep_avg = calculateAvgWorkPerRep(params.W_total, series.reps.size());
    
    // Impuls średni
    params.I_set_avg = calculateSetAvgImpulse(repImpulses);
    params.I_impulse = params.I_set_avg;
    
    // Parametry czasowe serii
    params.T_set = calculateSetDuration(series.startTime, series.endTime);
    params.T_rep_avg = calculateAvgRepTime(repTimes);
    params.f_rep = calculateRepFrequency(series.reps.size(), params.T_set);
    params.v_avg = calculateVelocity(params.F_range, params.T_rep_avg);
    
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
    params.P_peak = calculateSetPeakForce(repPeakPowers);
    
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

    // Historical readiness cannot be inferred from the current set.

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

    // TAI requires at least three comparable sets.

    // 57. Wskaźnik Ekonomii Ruchu (MEI)
    params.MEI = calculateMEI(params.W_total, params.I_impulse * series.reps.size(), params.F_set_mean);

    // LBI requires three comparable sets.

    // FRI requires the configured, calibrated sensor force limit.

    // CTS requires a real reference force and inter-set fatigue context.

    // ==================== Kategoria 7: Parametry Dodatkowe (61-65) ====================

    // FG requires fatigue values from three separate sets.

    // 62. Indeks Eksplozywnej Siły (ESI) - średni dla serii
    double totalESI = 0.0;
    for (const auto& rep : series.reps) {
        double esi = calculateESI(rep.peakForce, (rep.peakTime - rep.startTime) / 1000.0);
        totalESI += esi;
    }
    params.ESI = totalESI / series.reps.size();

    // BRC requires a measured resting baseline before the exercise.

    // 64. Indeks Kontrolowanego Opuszczania (CEI) - średni dla serii
    double totalCEI = 0.0;
    for (const auto& rep : series.reps) {
        double cei = calculateCEI(rep.samples, rep.peakForce, rep.peakTime, rep.endTime);
        totalCEI += cei;
    }
    params.CEI = totalCEI / series.reps.size();

    // LPT requires a previous comparable session.


    // Explicitly mark only metrics backed by this set's inputs. A value left
    // at its reset value is not evidence that the measured quantity was zero.
    for (std::size_t index : {4u, 5u, 8u, 9u, 11u, 12u, 13u, 15u, 17u, 19u,
                              25u, 26u, 27u, 30u, 31u, 32u, 34u, 35u, 36u,
                              37u, 38u, 48u, 49u, 50u, 51u, 54u, 61u, 63u})
        params.setAvailability(index, ParameterAvailability::Available);

    // Mechanics-derived values are unavailable until the elastic system is calibrated.
    const bool hasElasticCalibration = std::isfinite(k_elastic_) && k_elastic_ > 0.0;
    const bool hasPatientMass = std::isfinite(patient_mass_) && patient_mass_ > 0.0;
    const auto elasticStatus = hasElasticCalibration
        ? ParameterAvailability::Available : ParameterAvailability::RequiresCalibration;
    for (std::size_t index : {6u, 7u, 20u, 21u, 22u, 23u, 28u, 56u})
        params.setAvailability(index, elasticStatus);
    params.setAvailability(2, ParameterAvailability::Available);
    params.setAvailability(3, ParameterAvailability::Available);
    params.setAvailability(18, elasticStatus);
    params.setAvailability(24, hasElasticCalibration && hasPatientMass
        ? ParameterAvailability::Available : ParameterAvailability::RequiresCalibration);

    // Explicit external dependencies; do not fabricate proxy values.
    params.setAvailability(14, ParameterAvailability::InsufficientData);
    params.setAvailability(16, ParameterAvailability::InsufficientData);
    params.setAvailability(18, ParameterAvailability::RequiresCalibration);
    params.setAvailability(29, ParameterAvailability::RequiresAdditionalSensor);
    params.setAvailability(33, ParameterAvailability::InsufficientData);
    params.setAvailability(39, ParameterAvailability::RequiresAdditionalSensor);
    params.setAvailability(40, ParameterAvailability::InsufficientData);
    params.setAvailability(41, ParameterAvailability::InsufficientData);
    params.setAvailability(42, ParameterAvailability::InsufficientData);
    params.setAvailability(43, ParameterAvailability::RequiresCalibration);
    params.setAvailability(44, ParameterAvailability::InsufficientData);
    params.setAvailability(46, ParameterAvailability::InsufficientData);
    params.setAvailability(47, ParameterAvailability::InsufficientData);
    params.setAvailability(52, ParameterAvailability::RequiresAdditionalSensor);
    params.setAvailability(53, ParameterAvailability::RequiresHistory);
    params.setAvailability(55, ParameterAvailability::RequiresHistory);
    params.setAvailability(57, ParameterAvailability::InsufficientData);
    params.setAvailability(58, ParameterAvailability::RequiresCalibration);
    params.setAvailability(59, ParameterAvailability::RequiresHistory);
    params.setAvailability(60, ParameterAvailability::InsufficientData);
    params.setAvailability(62, ParameterAvailability::RequiresCalibration);
    params.setAvailability(64, ParameterAvailability::RequiresHistory);
    return params;
}
