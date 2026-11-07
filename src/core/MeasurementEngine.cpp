#include "core/MeasurementEngine.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>

namespace biofeedback::measurement {

MeasurementEngine::MeasurementEngine(EngineConfig config)
    : m_config(config)
{
    if (!std::isfinite(config.contractionStartThresholdN) ||
        !std::isfinite(config.contractionEndThresholdN) ||
        config.contractionEndThresholdN < 0.0 ||
        config.contractionStartThresholdN <= config.contractionEndThresholdN) {
        throw std::invalid_argument("Invalid contraction thresholds");
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

    if (!m_repetition.empty()) {
        const auto& previous = m_repetition.back();
        if (sample.timestampSeconds <= previous.timestampSeconds ||
            sample.source != previous.source) {
            ++m_rejectedSamples;
            return std::nullopt;
        }
    }

    if (!m_inContraction) {
        if (sample.force < m_config.contractionStartThresholdN) {
            return std::nullopt;
        }
        m_inContraction = true;
        m_repetition.clear();
    }

    m_repetition.push_back(sample);

    if (sample.force <= m_config.contractionEndThresholdN &&
        m_repetition.size() >= 2) {
        auto result = finishRepetition();
        m_repetition.clear();
        m_inContraction = false;
        return result;
    }

    return std::nullopt;
}

void MeasurementEngine::reset()
{
    m_repetition.clear();
    m_inContraction = false;
    m_rejectedSamples = 0;
}

RepetitionStats MeasurementEngine::finishRepetition() const
{
    RepetitionStats stats;
    if (m_repetition.size() < 2) {
        return stats;
    }

    stats.source = m_repetition.front().source;
    stats.sampleCount = m_repetition.size();
    stats.startSeconds = m_repetition.front().timestampSeconds;
    stats.endSeconds = m_repetition.back().timestampSeconds;
    stats.durationSeconds = stats.endSeconds - stats.startSeconds;

    double forceSum = 0.0;
    double peakTime = stats.startSeconds;
    for (const auto& sample : m_repetition) {
        forceSum += sample.force;
        if (sample.force > stats.peakForceN) {
            stats.peakForceN = sample.force;
            peakTime = sample.timestampSeconds;
        }
    }

    stats.averageForceN = forceSum / static_cast<double>(m_repetition.size());
    stats.timeToPeakSeconds = peakTime - stats.startSeconds;

    for (std::size_t i = 1; i < m_repetition.size(); ++i) {
        const auto& a = m_repetition[i - 1];
        const auto& b = m_repetition[i];
        const double dt = b.timestampSeconds - a.timestampSeconds;
        stats.impulseNs += (a.force + b.force) * 0.5 * dt;
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

    return stats;
}

} // namespace biofeedback::measurement
