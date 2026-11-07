#pragma once

#include <cstddef>
#include <optional>
#include <vector>

namespace biofeedback::measurement {

enum class SampleSource { Device, Simulation, Replay };
enum class ForceUnit { Newton };

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
    double averageForceN{0.0};
    double medianForceN{0.0};
    double modeForceN{0.0};
    double standardDeviationN{0.0};
    double percentile95ForceN{0.0};
    double timeToPeakSeconds{0.0};
    double impulseNs{0.0};
    double riseRateNPerSecond{0.0};
    double fallRateNPerSecond{0.0};
    double timeInTop8PercentSeconds{0.0};
    double timeInBottom8PercentSeconds{0.0};
    double timeInTargetZoneSeconds{0.0};
    double targetAccuracyPercent{0.0};
};

struct SeriesStats {
    std::size_t repetitionCount{0};
    std::vector<RepetitionStats> repetitions;
    double averagePeakForceN{0.0};
    double medianPeakForceN{0.0};
    double maximumPeakForceN{0.0};
    double minimumPeakForceN{0.0};
    double averageForceN{0.0};
    double averageTimeToPeakSeconds{0.0};
    double averageRepetitionDurationSeconds{0.0};
    double fatigueIndexPercent{0.0};
    double totalImpulseNs{0.0};
    double averageRiseRateNPerSecond{0.0};
    double averageFallRateNPerSecond{0.0};
    double seriesSpeedRepsPerSecond{0.0};
    double consistencyScorePercent{0.0};
    double targetAccuracyPercent{0.0};
};

struct EngineConfig {
    double contractionStartThresholdN{5.0};
    double contractionEndThresholdN{3.0};
    double targetForceN{0.0};
    double targetZoneToleranceFraction{0.08};
    std::size_t repetitionsPerSeries{8};
};

class MeasurementEngine {
public:
    explicit MeasurementEngine(EngineConfig config = {});

    std::optional<RepetitionStats> addSample(const Sample& sample);
    std::vector<SeriesStats> takeCompletedSeries();
    void setTargetForceN(double targetForceN);
    bool setRepetitionsPerSeries(std::size_t repetitionsPerSeries);
    void reset();

    bool isInContraction() const noexcept { return m_inContraction; }
    std::size_t rejectedSampleCount() const noexcept { return m_rejectedSamples; }
    bool lastSampleAccepted() const noexcept { return m_lastSampleAccepted; }

private:
    EngineConfig m_config;
    std::vector<Sample> m_repetition;
    std::vector<RepetitionStats> m_currentSeries;
    std::vector<SeriesStats> m_completedSeries;
    bool m_inContraction{false};
    std::size_t m_rejectedSamples{0};
    bool m_lastSampleAccepted{false};
    double m_lastTimestampSeconds{-1.0};
    std::optional<SampleSource> m_lastSource;

    RepetitionStats finishRepetition() const;
    SeriesStats finishSeries() const;
};

} // namespace biofeedback::measurement
