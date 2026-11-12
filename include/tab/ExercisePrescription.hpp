#ifndef TAB_EXERCISE_PRESCRIPTION_HPP
#define TAB_EXERCISE_PRESCRIPTION_HPP

#include <QJsonObject>
#include <QJsonValue>
#include <QString>
#include <cmath>

namespace tab {

// Optional dose and execution parameters. Missing JSON keys are deliberately
// neutral so legacy outlines remain readable without migration.
struct ExercisePrescription {
    int orderIndex{-1};
    QString phase;
    QString permittedRom;
    QString permittedLoad;
    int setsTarget{0};
    int repsTarget{0};
    int holdSeconds{0};
    int restSeconds{0};
    double resistanceValue{0.0};
    QString resistanceUnit;
    QString tempo;
    double rpeTarget{-1.0};
    QString painPolicyId;
    QString progressionRuleId;

    static bool fromJson(const QJsonObject& object,
                         ExercisePrescription& result,
                         QString* error = nullptr)
    {
        ExercisePrescription parsed;
        const auto fail = [error](const QString& message) {
            if (error) *error = message;
            return false;
        };
        const auto integer = [&object, &fail](const char* key, int min, int max,
                                               int fallback, int& out) {
            const QString name = QString::fromLatin1(key);
            const QJsonValue value = object.value(name);
            if (value.isUndefined()) { out = fallback; return true; }
            if (!value.isDouble()) return fail(name + QStringLiteral(" must be numeric"));
            const double n = value.toDouble();
            if (!std::isfinite(n) || std::floor(n) != n || n < min || n > max)
                return fail(name + QStringLiteral(" is outside the supported integer range"));
            out = static_cast<int>(n);
            return true;
        };
        const auto string = [&object, &fail](const char* key, QString& out) {
            const QString name = QString::fromLatin1(key);
            const QJsonValue value = object.value(name);
            if (value.isUndefined()) { out.clear(); return true; }
            if (!value.isString() || value.toString().size() > 256)
                return fail(name + QStringLiteral(" must be a string of at most 256 characters"));
            out = value.toString();
            return true;
        };

        if (!integer("orderIndex", 0, 100000, -1, parsed.orderIndex) ||
            !integer("setsTarget", 0, 100, 0, parsed.setsTarget) ||
            !integer("repsTarget", 0, 10000, 0, parsed.repsTarget) ||
            !integer("holdSeconds", 0, 86400, 0, parsed.holdSeconds) ||
            !integer("restSeconds", 0, 86400, 0, parsed.restSeconds) ||
            !string("phase", parsed.phase) ||
            !string("permittedRom", parsed.permittedRom) ||
            !string("permittedLoad", parsed.permittedLoad) ||
            !string("resistanceUnit", parsed.resistanceUnit) ||
            !string("tempo", parsed.tempo) ||
            !string("painPolicyId", parsed.painPolicyId) ||
            !string("progressionRuleId", parsed.progressionRuleId))
            return false;

        const QJsonValue resistance = object.value(QStringLiteral("resistanceValue"));
        if (!resistance.isUndefined()) {
            const double n = resistance.toDouble(std::numeric_limits<double>::quiet_NaN());
            if (!resistance.isDouble() || !std::isfinite(n) || n < 0.0 || n > 1000000.0)
                return fail(QStringLiteral("resistanceValue must be between 0 and 1000000"));
            parsed.resistanceValue = n;
        }
        const QJsonValue rpe = object.value(QStringLiteral("rpeTarget"));
        if (!rpe.isUndefined()) {
            const double n = rpe.toDouble(std::numeric_limits<double>::quiet_NaN());
            if (!rpe.isDouble() || !std::isfinite(n) || n < 0.0 || n > 10.0)
                return fail(QStringLiteral("rpeTarget must be between 0 and 10"));
            parsed.rpeTarget = n;
        }
        result = parsed;
        if (error) error->clear();
        return true;
    }
};

} // namespace tab

#endif // TAB_EXERCISE_PRESCRIPTION_HPP
