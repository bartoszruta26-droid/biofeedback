#include "training/RepetitionDetector.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace biofeedback::training {

RepetitionDetector::RepetitionDetector(RepDetectorConfig config)
    : m_config(config) {
    if (!std::isfinite(m_config.targetForceN) || m_config.targetForceN < 0.0)
        m_config.targetForceN = 0.0;
    if (!std::isfinite(m_config.activationRatio) || m_config.activationRatio <= 0.0)
        m_config.activationRatio = 0.20;
    if (!std::isfinite(m_config.releaseRatio) || m_config.releaseRatio < 0.0 ||
        m_config.releaseRatio >= m_config.activationRatio)
        m_config.releaseRatio = m_config.activationRatio * 0.5;
    if (m_config.minimumDurationMs == 0) m_config.minimumDurationMs = 1;
    if (m_config.maximumDurationMs < m_config.minimumDurationMs)
        m_config.maximumDurationMs = m_config.minimumDurationMs;
    if (m_config.maximumSampleGapMs == 0) m_config.maximumSampleGapMs = 500;
}

void RepetitionDetector::setTargetForce(double targetForceN) {
    m_config.targetForceN = std::isfinite(targetForceN) && targetForceN > 0.0
        ? targetForceN : 0.0;
    reset();
}

double RepetitionDetector::activationThresholdN() const noexcept {
    return std::max(m_config.minimumActivationN,
                    m_config.targetForceN * m_config.activationRatio);
}

double RepetitionDetector::releaseThresholdN() const noexcept {
    return std::max(0.0, activationThresholdN() *
                         (m_config.releaseRatio / m_config.activationRatio));
}

void RepetitionDetector::reset() noexcept {
    m_active = false;
    m_waitingForRelease = false;
    m_startTimestampMs = 0;
    m_lastTimestampMs = 0;
    m_peakTimestampMs = 0;
    m_peakForceN = 0.0;
    m_areaForceMs = 0.0;
    m_previousForceN = 0.0;
    m_sampleCount = 0;
    m_samples.clear();
}

void RepetitionDetector::begin(std::uint32_t timestampMs, double forceN) {
    m_active = true;
    m_startTimestampMs = timestampMs;
    m_lastTimestampMs = timestampMs;
    m_peakTimestampMs = timestampMs;
    m_peakForceN = forceN;
    m_areaForceMs = 0.0;
    m_previousForceN = forceN;
    m_sampleCount = 1;
    m_samples.clear();
    m_samples.push_back({timestampMs, forceN});
}

bool RepetitionDetector::process(std::uint32_t timestampMs, double forceN, bool valid,
                                 RepetitionRecord& completed) {
    if (!valid || !std::isfinite(forceN) || forceN < 0.0) {
        reset();
        m_waitingForRelease = true;
        return false;
    }

    if (!m_active) {
        // After a data gap or invalid sample, require a confirmed baseline
        // before re-arming. Otherwise a sustained contraction could be
        // counted again from its middle as a new, truncated repetition.
        if (m_waitingForRelease) {
            if (forceN <= releaseThresholdN()) m_waitingForRelease = false;
            return false;
        }
        if (forceN >= activationThresholdN()) begin(timestampMs, forceN);
        return false;
    }

    // Unsigned subtraction deliberately handles the 32-bit device clock wrap.
    const std::uint32_t deltaMs = timestampMs - m_lastTimestampMs;
    if (deltaMs == 0 || deltaMs > m_config.maximumSampleGapMs) {
        // A missing/duplicated interval makes the contraction boundary unknown.
        // Discard the partial repetition and wait for a fresh low-to-high edge.
        reset();
        m_waitingForRelease = true;
        return false;
    }

    const std::uint32_t durationMs = timestampMs - m_startTimestampMs;
    if (durationMs > m_config.maximumDurationMs) {
        reset();
        m_waitingForRelease = true;
        return false;
    }

    m_areaForceMs += (m_previousForceN + forceN) * 0.5 * deltaMs;
    m_lastTimestampMs = timestampMs;
    m_previousForceN = forceN;
    ++m_sampleCount;
    m_samples.push_back({timestampMs, forceN});

    if (forceN > m_peakForceN) {
        m_peakForceN = forceN;
        m_peakTimestampMs = timestampMs;
    }

    if (forceN > releaseThresholdN()) return false;

    if (durationMs < m_config.minimumDurationMs) {
        reset();
        return false;
    }

    completed.startTimestampMs = m_startTimestampMs;
    completed.endTimestampMs = timestampMs;
    completed.durationMs = durationMs;
    completed.timeToPeakMs = m_peakTimestampMs - m_startTimestampMs;
    completed.peakForceN = m_peakForceN;
    completed.meanForceN = m_areaForceMs / static_cast<double>(durationMs);
    completed.sampleCount = m_sampleCount;
    completed.samples = m_samples;
    reset();
    return true;
}

SeriesSummary summarizeSeries(const std::vector<RepetitionRecord>& repetitions,
                              double targetForceN) noexcept {
    SeriesSummary result;
    result.repetitionCount = repetitions.size();
    if (repetitions.empty()) return result;

    double sumPeak = 0.0;
    double sumDuration = 0.0;
    double sumTimeToPeak = 0.0;
    double sumSquaredDeviation = 0.0;
    std::size_t targetHits = 0;

    for (const auto& rep : repetitions) {
        sumPeak += rep.peakForceN;
        sumDuration += rep.durationMs;
        sumTimeToPeak += rep.timeToPeakMs;
        result.maximumPeakForceN = std::max(result.maximumPeakForceN, rep.peakForceN);
        if (targetForceN > 0.0 && rep.peakForceN >= targetForceN) ++targetHits;
    }

    const double count = static_cast<double>(repetitions.size());
    result.averagePeakForceN = sumPeak / count;
    result.averageDurationMs = sumDuration / count;
    result.averageTimeToPeakMs = sumTimeToPeak / count;

    for (const auto& rep : repetitions) {
        const double difference = rep.peakForceN - result.averagePeakForceN;
        sumSquaredDeviation += difference * difference;
    }
    const double standardDeviation = std::sqrt(sumSquaredDeviation / count);
    if (result.averagePeakForceN > std::numeric_limits<double>::epsilon()) {
        result.peakForceCoefficientOfVariationPercent =
            standardDeviation / result.averagePeakForceN * 100.0;
    }
    result.targetAchievementPercent = targetForceN > 0.0
        ? static_cast<double>(targetHits) / count * 100.0 : 0.0;
    return result;
}

} // namespace biofeedback::training
