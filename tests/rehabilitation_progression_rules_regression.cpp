#include "rehabilitation/ProgressionRules.hpp"

#include <cassert>
using namespace biofeedback::rehabilitation;

int main() {
    ProgressionPolicy policy;
    policy.maximumPermittedPain = 3;
    policy.minimumCompletedDoseRatio = 0.8;
    policy.minimumQualifyingSessions = 2;

    SessionEvidence good;
    good.painBefore = 2;
    good.painAfter = 2;
    good.painAt24Hours = 1;
    good.completedDoseRatio = 1.0;
    good.movementQualityAcceptable = true;

    auto first = ProgressionRules::evaluate(good, 0, policy);
    assert(first.decision == ProgressionDecision::Hold);
    assert(!ProgressionRules::clinicianMayApprove(first, true, true));

    auto ready = ProgressionRules::evaluate(good, 1, policy);
    assert(ready.decision == ProgressionDecision::SuggestProgress);
    assert(!ProgressionRules::clinicianMayApprove(ready, false, true));
    assert(!ProgressionRules::clinicianMayApprove(ready, true, false));
    assert(ProgressionRules::clinicianMayApprove(ready, true, true));

    auto missing = good;
    missing.painAt24Hours = -1;
    assert(ProgressionRules::evaluate(missing, 5, policy).decision ==
           ProgressionDecision::NotEvaluable);

    auto painWorse = good;
    painWorse.painAt24Hours = 5;
    assert(ProgressionRules::evaluate(painWorse, 5, policy).decision ==
           ProgressionDecision::RegressionReview);

    auto lowDose = good;
    lowDose.completedDoseRatio = 0.5;
    assert(ProgressionRules::evaluate(lowDose, 5, policy).decision ==
           ProgressionDecision::Hold);

    auto poorQuality = good;
    poorQuality.movementQualityAcceptable = false;
    assert(ProgressionRules::evaluate(poorQuality, 5, policy).decision ==
           ProgressionDecision::Hold);

    auto redFlag = good;
    redFlag.redFlag = true;
    assert(ProgressionRules::evaluate(redFlag, 5, policy).decision ==
           ProgressionDecision::Review);

    auto restriction = good;
    restriction.postOperativeRestrictionConflict = true;
    assert(ProgressionRules::evaluate(restriction, 5, policy).decision ==
           ProgressionDecision::Review);

    auto calibratedPolicy = policy;
    calibratedPolicy.requireCalibratedMeasurement = true;
    assert(ProgressionRules::evaluate(good, 5, calibratedPolicy).decision ==
           ProgressionDecision::NotEvaluable);
    good.calibratedMeasurementValid = true;
    assert(ProgressionRules::evaluate(good, 5, calibratedPolicy).decision ==
           ProgressionDecision::SuggestProgress);

    auto invalidPolicy = policy;
    invalidPolicy.minimumCompletedDoseRatio = 2.0;
    assert(ProgressionRules::evaluate(good, 5, invalidPolicy).decision ==
           ProgressionDecision::NotEvaluable);
    return 0;
}
