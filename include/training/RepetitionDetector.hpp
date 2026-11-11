#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace biofeedback::training {

struct RepDetectorConfig {
    double targetForceN{50.0};
    double activationRatio{0.20};
    double releaseRatio{0.10};
    double minimumActivationN{2.0};
    std::uint32_t minimumDurationMs{250};
    std::uint32_t maximumDurationMs{30000};
    std::uint32_t maximumSampleGapMs{500};
};

struct ForceSample {
    std::uint32_t timestampMs{0};
    double forceN{0.0};
};

struct RepetitionRecord {
    std::uint32_t startTimestampMs{0};
    std::uint32_t endTimestampMs{0};
    std::uint32_t durationMs{0};
    std::uint32_t timeToPeakMs{0};
    double peakForceN{0.0};
    double meanForceN{0.0};
    std::size_t sampleCount{0};
    std::vector<ForceSample> samples;
};

struct SeriesSummary {
    std::size_t repetitionCount{0};
    double averagePeakForceN{0.0};
    double maximumPeakForceN{0.0};
    double averageDurationMs{0.0};
    double averageTimeToPeakMs{0.0};
    double peakForceCoefficientOfVariationPercent{0.0};
    double targetAchievementPercent{0.0};
};

// Serializable, versioned snapshot of the detector's in-flight repetition.
// The device timestamp is retained so the detector can continue without
// fabricating a new activation edge after a process restart.
struct RepetitionDetectorSnapshot {
    std::uint32_t version{1};
    bool active{false};
    bool waitingForRelease{false};
    std::uint32_t startTimestampMs{0};
    std::uint32_t lastTimestampMs{0};
    std::uint32_t peakTimestampMs{0};
    double peakForceN{0.0};
    double areaForceMs{0.0};
    double previousForceN{0.0};
    std::size_t sampleCount{0};
    std::vector<ForceSample> samples;
};

class RepetitionDetector final {
public:
    explicit RepetitionDetector(RepDetectorConfig config = {});
    void setTargetForce(double targetForceN);
    void reset() noexcept;
    bool process(std::uint32_t timestampMs, double forceN, bool valid,
                 RepetitionRecord& completed);

    bool isActive() const noexcept { return m_active; }
    double activationThresholdN() const noexcept;
    double releaseThresholdN() const noexcept;
    RepetitionDetectorSnapshot snapshot() const;
    bool restore(const RepetitionDetectorSnapshot& snapshot) noexcept;

private:
    void begin(std::uint32_t timestampMs, double forceN);

    RepDetectorConfig m_config;
    bool m_active{false};
    bool m_waitingForRelease{false};
    std::uint32_t m_startTimestampMs{0};
    std::uint32_t m_lastTimestampMs{0};
    std::uint32_t m_peakTimestampMs{0};
    double m_peakForceN{0.0};
    double m_areaForceMs{0.0};
    double m_previousForceN{0.0};
    std::size_t m_sampleCount{0};
    std::vector<ForceSample> m_samples;
};

SeriesSummary summarizeSeries(const std::vector<RepetitionRecord>& repetitions,
                              double targetForceN) noexcept;

} // namespace biofeedback::training
