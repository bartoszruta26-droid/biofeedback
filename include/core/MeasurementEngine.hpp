#pragma once

#include <cstddef>
#include <optional>
#include <vector>

namespace biofeedback::measurement {

enum class SampleSource {
    Device,
    Simulation,
    Replay
};

enum class ForceUnit {
    Newton
};

struct Sample {
    double timestampSeconds{0.0};
    double force{0.0};
    ForceUnit unit{ForceUnit::Newton};
    SampleSource source{SampleSource::Device};
    bool valid{false};
};

struct RepetitionStats {
    SampleSource source{SampleSource::Device};
    std::size_t sampleCount{0};
    double startSeconds{0.0};
    double endSeconds{0.0};
    double durationSeconds{0.0};
    double peakForceN{0.0};
    double timeToPeakSeconds{0.0};
    double averageForceN{0.0};
    double impulseNs{0.0};
    double riseRateNPerSecond{0.0};
    double fallRateNPerSecond{0.0};
};

struct EngineConfig {
    double contractionStartThresholdN{5.0};
    double contractionEndThresholdN{3.0};
};

class MeasurementEngine {
public:
    explicit MeasurementEngine(EngineConfig config = {});

    // Returns a completed repetition when the signal crosses the end threshold.
    // Invalid, non-finite, non-monotonic, or mixed-source samples are rejected.
    std::optional<RepetitionStats> addSample(const Sample& sample);
    void reset();

    bool isInContraction() const noexcept { return m_inContraction; }
    std::size_t rejectedSampleCount() const noexcept { return m_rejectedSamples; }

private:
    EngineConfig m_config;
    std::vector<Sample> m_repetition;
    bool m_inContraction{false};
    std::size_t m_rejectedSamples{0};

    RepetitionStats finishRepetition() const;
};

} // namespace biofeedback::measurement
