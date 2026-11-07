#include "core/MeasurementEngine.hpp"

#include <algorithm>
#include <cmath>
#include <map>
#include <numeric>
#include <stdexcept>

namespace biofeedback::measurement {
namespace {
double percentile(std::vector<double> values, double fraction)
{
    if (values.empty()) return 0.0;
    std::sort(values.begin(), values.end());
    const double position = fraction * static_cast<double>(values.size() - 1);
    const auto lower = static_cast<std::size_t>(std::floor(position));
    const auto upper = static_cast<std::size_t>(std::ceil(position));
    const double weight = position - static_cast<double>(lower);
    return values[lower] * (1.0 - weight) + values[upper] * weight;
}

double mean(const std::vector<double>& values)
{
    if (values.empty()) return 0.0;
    return std::accumulate(values.begin(), values.end(), 0.0) /
           static_cast<double>(values.size());
}
}

MeasurementEngine::MeasurementEngine(EngineConfig config) : m_config(config)
{
    if (!std::isfinite(config.contractionStartThresholdN) ||
        !std::isfinite(config.contractionEndThresholdN) ||
        config.contractionEndThresholdN < 0.0 ||
        config.contractionStartThresholdN <= config.contractionEndThresholdN ||
        !std::isfinite(config.targetForceN) || config.targetForceN < 0.0 ||
        !std::isfinite(config.targetZoneToleranceFraction) ||
        config.targetZoneToleranceFraction < 0.0 ||
        config.repetitionsPerSeries == 0 || config.repetitionsPerSeries > 1000) {
        throw std::invalid_argument("Invalid measurement engine configuration");
    }
}

std::optional<RepetitionStats> MeasurementEngine::addSample(const Sample& sample)
{
    if (!sample.valid || sample.unit != ForceUnit::Newton ||
        !std::isfinite(sample.timestampSeconds) || !std::isfinite(sample.force) ||
        sample.force < 0.0) {
        ++m_rejectedSamples;
        return std::nullopt;
    }

    if (m_lastSource && sample.source != *m_lastSource) {
        // Never combine sources inside a repetition or partially completed series.
        m_repetition.clear();
        m_currentSeries.clear();
        m_inContraction = false;
        m_lastTimestampSeconds = -1.0;
        m_lastSource = sample.source;
        ++m_rejectedSamples;
        return std::nullopt;
    }
    if (sample.timestampSeconds <= m_lastTimestampSeconds) {
        ++m_rejectedSamples;
        return std::nullopt;
    }

    m_lastTimestampSeconds = sample.timestampSeconds;
    m_lastSource = sample.source;

    if (!m_inContraction) {
        if (sample.force < m_config.contractionStartThresholdN) return std::nullopt;
        m_inContraction = true;
        m_repetition.clear();
    }

    m_repetition.push_back(sample);
    if (sample.force <= m_config.contractionEndThresholdN && m_repetition.size() >= 2) {
        RepetitionStats result = finishRepetition();
        m_currentSeries.push_back(result);
        if (m_currentSeries.size() >= m_config.repetitionsPerSeries) {
            m_completedSeries.push_back(finishSeries());
            m_currentSeries.clear();
        }
        m_repetition.clear();
        m_inContraction = false;
        return result;
    }
    return std::nullopt;
}

std::vector<SeriesStats> MeasurementEngine::takeCompletedSeries()
{
    std::vector<SeriesStats> result;
    result.swap(m_completedSeries);
    return result;
}

void MeasurementEngine::setTargetForceN(double targetForceN)
{
    if (!std::isfinite(targetForceN) || targetForceN < 0.0)
        throw std::invalid_argument("Target force must be finite and non-negative");
    m_config.targetForceN = targetForceN;
}

void MeasurementEngine::reset()
{
    m_repetition.clear();
    m_currentSeries.clear();
    m_completedSeries.clear();
    m_inContraction = false;
    m_rejectedSamples = 0;
    m_lastTimestampSeconds = -1.0;
    m_lastSource.reset();
}

RepetitionStats MeasurementEngine::finishRepetition() const
{
    RepetitionStats stats;
    if (m_repetition.size() < 2) return stats;

    stats.source = m_repetition.front().source;
    stats.sampleCount = m_repetition.size();
    stats.startSeconds = m_repetition.front().timestampSeconds;
    stats.endSeconds = m_repetition.back().timestampSeconds;
    stats.durationSeconds = stats.endSeconds - stats.startSeconds;

    std::vector<double> forces;
    forces.reserve(m_repetition.size());
    double peakTime = stats.startSeconds;
    for (const auto& sample : m_repetition) {
        forces.push_back(sample.force);
        if (sample.force > stats.peakForceN) {
            stats.peakForceN = sample.force;
            peakTime = sample.timestampSeconds;
        }
    }
    stats.averageForceN = mean(forces);
    stats.medianForceN = percentile(forces, 0.50);
    stats.percentile95ForceN = percentile(forces, 0.95);
    double variance = 0.0;
    std::map<long long, std::pair<std::size_t, double>> histogram;
    for (double force : forces) {
        variance += (force - stats.averageForceN) * (force - stats.averageForceN);
        auto& bucket = histogram[static_cast<long long>(std::llround(force))];
        ++bucket.first;
        bucket.second += force;
    }
    stats.standardDeviationN = std::sqrt(variance / static_cast<double>(forces.size()));
    std::size_t modeCount = 0;
    for (const auto& entry : histogram) {
        if (entry.second.first > modeCount) {
            modeCount = entry.second.first;
            stats.modeForceN = entry.second.second / static_cast<double>(entry.second.first);
        }
    }
    stats.timeToPeakSeconds = peakTime - stats.startSeconds;

    const double topThreshold = stats.peakForceN * 0.92;
    const double bottomThreshold = stats.peakForceN * 0.08;
    const double targetLow = m_config.targetForceN * (1.0 - m_config.targetZoneToleranceFraction);
    const double targetHigh = m_config.targetForceN * (1.0 + m_config.targetZoneToleranceFraction);
    for (std::size_t i = 1; i < m_repetition.size(); ++i) {
        const auto& a = m_repetition[i - 1];
        const auto& b = m_repetition[i];
        const double dt = b.timestampSeconds - a.timestampSeconds;
        stats.impulseNs += (a.force + b.force) * 0.5 * dt;
        if (a.force >= topThreshold && b.force >= topThreshold) stats.timeInTop8PercentSeconds += dt;
        if (a.force <= bottomThreshold && b.force <= bottomThreshold) stats.timeInBottom8PercentSeconds += dt;
        if (m_config.targetForceN > 0.0 &&
            a.force >= targetLow && a.force <= targetHigh &&
            b.force >= targetLow && b.force <= targetHigh) {
            stats.timeInTargetZoneSeconds += dt;
        }
    }
    if (stats.timeToPeakSeconds > 0.0) {
        stats.riseRateNPerSecond =
            (stats.peakForceN - m_repetition.front().force) / stats.timeToPeakSeconds;
    }
    const double fallDuration = stats.endSeconds - peakTime;
    if (fallDuration > 0.0) {
        stats.fallRateNPerSecond =
            (stats.peakForceN - m_repetition.back().force) / fallDuration;
    }
    if (stats.durationSeconds > 0.0) {
        stats.targetAccuracyPercent =
            100.0 * stats.timeInTargetZoneSeconds / stats.durationSeconds;
    }
    return stats;
}

SeriesStats MeasurementEngine::finishSeries() const
{
    SeriesStats stats;
    stats.repetitions = m_currentSeries;
    stats.repetitionCount = m_currentSeries.size();
    if (m_currentSeries.empty()) return stats;

    std::vector<double> peaks;
    peaks.reserve(m_currentSeries.size());
    double riseSum = 0.0;
    double fallSum = 0.0;
    double accuracySum = 0.0;
    for (const auto& repetition : m_currentSeries) {
        peaks.push_back(repetition.peakForceN);
        stats.totalImpulseNs += repetition.impulseNs;
        riseSum += repetition.riseRateNPerSecond;
        fallSum += repetition.fallRateNPerSecond;
        accuracySum += repetition.targetAccuracyPercent;
    }
    stats.averagePeakForceN = mean(peaks);
    stats.medianPeakForceN = percentile(peaks, 0.50);
    stats.averageRiseRateNPerSecond = riseSum / static_cast<double>(m_currentSeries.size());
    stats.averageFallRateNPerSecond = fallSum / static_cast<double>(m_currentSeries.size());
    stats.targetAccuracyPercent = accuracySum / static_cast<double>(m_currentSeries.size());

    if (peaks.size() >= 2 && peaks.front() > 0.0) {
        stats.fatigueIndexPercent = 100.0 * (peaks.front() - peaks.back()) / peaks.front();
    }
    const double duration = m_currentSeries.back().endSeconds - m_currentSeries.front().startSeconds;
    if (duration > 0.0) {
        stats.seriesSpeedRepsPerSecond = static_cast<double>(m_currentSeries.size()) / duration;
    }
    double variance = 0.0;
    for (double peak : peaks) variance += (peak - stats.averagePeakForceN) * (peak - stats.averagePeakForceN);
    variance /= static_cast<double>(peaks.size());
    const double stddev = std::sqrt(variance);
    if (stats.averagePeakForceN > 0.0) {
        stats.consistencyScorePercent =
            std::max(0.0, 100.0 - 100.0 * stddev / stats.averagePeakForceN);
    }
    return stats;
}

} // namespace biofeedback::measurement
