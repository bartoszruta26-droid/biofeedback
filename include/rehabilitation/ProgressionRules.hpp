#pragma once

#include <algorithm>
#include <string>

namespace biofeedback::rehabilitation {

enum class ProgressionDecision {
    NotEvaluable,
    Hold,
    Review,
    SuggestProgress,
    RegressionReview
};

struct SessionEvidence {
    int painBefore{-1};       // NRS 0..10; -1 means not recorded
    int painAfter{-1};        // NRS 0..10; -1 means not recorded
    int painAt24Hours{-1};    // NRS 0..10; -1 means pending/not recorded
    double completedDoseRatio{-1.0}; // 0..1; -1 means unavailable
    bool movementQualityAcceptable{false};
    bool calibratedMeasurementValid{false};
    bool adverseEvent{false};
    bool redFlag{false};
    bool postOperativeRestrictionConflict{false};
};

struct ProgressionPolicy {
    int maximumPermittedPain{3};
    double minimumCompletedDoseRatio{0.8};
    int minimumQualifyingSessions{2};
    bool requireCalibratedMeasurement{false};
};

struct ProgressionAssessment {
    ProgressionDecision decision{ProgressionDecision::NotEvaluable};
    std::string reason;
    bool requiresClinicianApproval{true};
};

class ProgressionRules final {
public:
    static ProgressionAssessment evaluate(const SessionEvidence& latest,
                                          int consecutiveQualifyingSessions,
                                          const ProgressionPolicy& policy) {
        if (!validPolicy(policy) || consecutiveQualifyingSessions < 0)
            return {ProgressionDecision::NotEvaluable, "INVALID_POLICY_OR_HISTORY", true};

        if (latest.redFlag || latest.adverseEvent || latest.postOperativeRestrictionConflict)
            return {ProgressionDecision::Review,
                    latest.redFlag ? "RED_FLAG_REQUIRES_CLINICAL_REVIEW" :
                    latest.adverseEvent ? "ADVERSE_EVENT_REQUIRES_REVIEW" :
                    "POST_OPERATIVE_RESTRICTION_CONFLICT", true};

        if (latest.painBefore < 0 || latest.painAfter < 0 ||
            latest.painAt24Hours < 0 || latest.completedDoseRatio < 0.0 ||
            latest.completedDoseRatio > 1.0)
            return {ProgressionDecision::NotEvaluable, "INCOMPLETE_SESSION_EVIDENCE", true};

        if (latest.painBefore > 10 || latest.painAfter > 10 || latest.painAt24Hours > 10)
            return {ProgressionDecision::NotEvaluable, "PAIN_SCORE_OUT_OF_RANGE", true};

        if (!latest.movementQualityAcceptable)
            return {ProgressionDecision::Hold, "MOVEMENT_QUALITY_NOT_ACCEPTABLE", true};

        if (latest.painAfter > policy.maximumPermittedPain ||
            latest.painAt24Hours > policy.maximumPermittedPain ||
            latest.painAt24Hours > latest.painBefore)
            return {ProgressionDecision::RegressionReview, "SYMPTOM_RESPONSE_EXCEEDS_POLICY", true};

        if (latest.completedDoseRatio < policy.minimumCompletedDoseRatio)
            return {ProgressionDecision::Hold, "PRESCRIBED_DOSE_NOT_TOLERATED", true};

        if (policy.requireCalibratedMeasurement && !latest.calibratedMeasurementValid)
            return {ProgressionDecision::NotEvaluable, "VALID_CALIBRATED_MEASUREMENT_REQUIRED", true};

        if (consecutiveQualifyingSessions + 1 < policy.minimumQualifyingSessions)
            return {ProgressionDecision::Hold, "INSUFFICIENT_CONSECUTIVE_QUALIFYING_SESSIONS", true};

        return {ProgressionDecision::SuggestProgress,
                "ALL_CONFIGURED_CRITERIA_MET_CLINICIAN_DECISION_REQUIRED", true};
    }

    // This function records only the logical gate; persistence and identity/audit
    // attribution must be performed by the application service.
    static bool clinicianMayApprove(const ProgressionAssessment& assessment,
                                    bool clinicianAuthenticated,
                                    bool explicitApproval) noexcept {
        return assessment.decision == ProgressionDecision::SuggestProgress &&
               clinicianAuthenticated && explicitApproval;
    }

private:
    static bool validPolicy(const ProgressionPolicy& policy) noexcept {
        return policy.maximumPermittedPain >= 0 && policy.maximumPermittedPain <= 10 &&
               policy.minimumCompletedDoseRatio >= 0.0 &&
               policy.minimumCompletedDoseRatio <= 1.0 &&
               policy.minimumQualifyingSessions >= 1;
    }
};

} // namespace biofeedback::rehabilitation
