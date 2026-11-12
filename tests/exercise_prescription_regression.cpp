#include "tab/ExercisePrescription.hpp"
#include <QJsonObject>
#include <cstdio>
#include <cstdlib>

#define REQUIRE(condition) do { if (!(condition)) { \
    std::fprintf(stderr, "Exercise prescription regression failed at %s:%d: %s\\n", __FILE__, __LINE__, #condition); \
    std::abort(); } } while (false)

int main()
{
    QString error;
    tab::ExercisePrescription legacy;
    REQUIRE(tab::ExercisePrescription::fromJson(QJsonObject{}, legacy, &error));
    REQUIRE(error.isEmpty());
    REQUIRE(legacy.orderIndex == -1);
    REQUIRE(legacy.setsTarget == 0 && legacy.repsTarget == 0);
    REQUIRE(legacy.holdSeconds == 0 && legacy.restSeconds == 0);
    REQUIRE(legacy.resistanceValue == 0.0 && legacy.rpeTarget == -1.0);
    REQUIRE(legacy.phase.isEmpty() && legacy.painPolicyId.isEmpty());

    const QJsonObject complete{
        {QStringLiteral("orderIndex"), 3},
        {QStringLiteral("phase"), QStringLiteral("strengthening")},
        {QStringLiteral("permittedRom"), QStringLiteral("0-90 deg")},
        {QStringLiteral("permittedLoad"), QStringLiteral("pain-free")},
        {QStringLiteral("setsTarget"), 3},
        {QStringLiteral("repsTarget"), 12},
        {QStringLiteral("holdSeconds"), 5},
        {QStringLiteral("restSeconds"), 45},
        {QStringLiteral("resistanceValue"), 2.5},
        {QStringLiteral("resistanceUnit"), QStringLiteral("kg")},
        {QStringLiteral("tempo"), QStringLiteral("3-1-2")},
        {QStringLiteral("rpeTarget"), 6.0},
        {QStringLiteral("painPolicyId"), QStringLiteral("pain-monitoring-v1")},
        {QStringLiteral("progressionRuleId"), QStringLiteral("linear-v1")}
    };
    tab::ExercisePrescription full;
    REQUIRE(tab::ExercisePrescription::fromJson(complete, full, &error));
    REQUIRE(full.orderIndex == 3 && full.setsTarget == 3 && full.repsTarget == 12);
    REQUIRE(full.holdSeconds == 5 && full.restSeconds == 45);
    REQUIRE(full.resistanceValue == 2.5 && full.rpeTarget == 6.0);
    REQUIRE(full.phase == QStringLiteral("strengthening"));
    REQUIRE(full.progressionRuleId == QStringLiteral("linear-v1"));

    tab::ExercisePrescription rejected;
    REQUIRE(!tab::ExercisePrescription::fromJson(
        QJsonObject{{QStringLiteral("setsTarget"), 101}}, rejected, &error));
    REQUIRE(!error.isEmpty());
    REQUIRE(!tab::ExercisePrescription::fromJson(
        QJsonObject{{QStringLiteral("rpeTarget"), 11.0}}, rejected, &error));
    REQUIRE(!tab::ExercisePrescription::fromJson(
        QJsonObject{{QStringLiteral("holdSeconds"), 1.5}}, rejected, &error));
    REQUIRE(!tab::ExercisePrescription::fromJson(
        QJsonObject{{QStringLiteral("phase"), 42}}, rejected, &error));
    return 0;
}
