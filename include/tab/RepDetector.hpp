#ifndef TAB_REPDETECTOR_HPP
#define TAB_REPDETECTOR_HPP

#include <cmath>

namespace training {

/**
 * Detects one force repetition using hysteresis thresholds.
 *
 * A repetition starts at or above 80% of the target and completes when
 * force returns to or below 40%. The peak is retained after completion
 * until reset() or the next repetition.
 */
class RepDetector {
public:
    bool update(double force, double target)
    {
        if (!std::isfinite(force) || !std::isfinite(target) || target <= 0.0) {
            reset();
            return false;
        }

        if (!m_inProgress) {
            if (force >= target * 0.80) {
                m_inProgress = true;
                m_peakForce = force;
            }
            return false;
        }

        if (force > m_peakForce) {
            m_peakForce = force;
        }

        if (force <= target * 0.40) {
            m_completedPeakForce = m_peakForce;
            m_inProgress = false;
            m_peakForce = 0.0;
            return true;
        }

        return false;
    }

    void reset()
    {
        m_inProgress = false;
        m_peakForce = 0.0;
        m_completedPeakForce = 0.0;
    }

    bool inProgress() const { return m_inProgress; }
    double peakForce() const { return m_inProgress ? m_peakForce : m_completedPeakForce; }

private:
    bool m_inProgress = false;
    double m_peakForce = 0.0;
    double m_completedPeakForce = 0.0;
};

} // namespace training

#endif // TAB_REPDETECTOR_HPP
