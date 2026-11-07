#include "tab/TrainingTab.hpp"
#include "tab/TrainingAnalyticsWidget.hpp"

#include <QApplication>
#include <QDir>
#include <QDebug>
#include <QMessageBox>
#include <QProcess>
#include <QTemporaryDir>
#include <QTimer>

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <cstdlib>

namespace tab {
class TrainingTabProcessRestartRegression {
public:
    static int child(int argc, char** argv)
    {
        QApplication app(argc, argv);
        if (argc != 3) return 90;
        const QString mode = QString::fromLocal8Bit(argv[1]);
        const QString root = QString::fromLocal8Bit(argv[2]);
        if (!QDir().mkpath(root)) return 91;

        if (mode == "--seed-active") return seedActive(root);
        if (mode == "--verify-active") return verifyActive(root);
        if (mode == "--verify-rest" || mode == "--verify-rest-again")
            return verifyRest(root);
        return 92;
    }

private:
    static QVector<tab::ExerciseStats> emptyStats()
    {
        QVector<tab::ExerciseStats> stats(2);
        stats[0].exerciseName = QStringLiteral("Grip A");
        stats[1].exerciseName = QStringLiteral("Grip B");
        return stats;
    }

    static QVector<tab::TrainingExercise> plan()
    {
        QVector<tab::TrainingExercise> result;
        tab::TrainingExercise first;
        first.name = QStringLiteral("Grip A");
        first.seriesCount = 2;
        first.repsPerSeries = 3;
        first.targetForce = 50.0;
        first.restTime = 12.0;
        first.gameType = QStringLiteral("none");
        result.append(first);
        tab::TrainingExercise second = first;
        second.name = QStringLiteral("Grip B");
        second.seriesCount = 3;
        second.repsPerSeries = 4;
        result.append(second);
        return result;
    }

    static void answerRecoveryPrompt()
    {
        // QMessageBox::question() enters its own event loop. Poll until the
        // modal dialog is actually active instead of assuming a zero-delay
        // callback runs after the dialog has been constructed.
        auto* timer = new QTimer(QApplication::instance());
        QObject::connect(timer, &QTimer::timeout, timer, [timer]() {
            auto* dialog = qobject_cast<QMessageBox*>(QApplication::activeModalWidget());
            if (!dialog) return;
            timer->stop();
            dialog->done(QMessageBox::Yes);
            timer->deleteLater();
        });
        timer->start(10);
    }

    static int seedActive(const QString& root)
    {
        tab::TrainingTab training;
        const auto exercises = plan();
        for (const auto& exercise : exercises) training.addExercise(exercise);
        const QString history = QDir(root).filePath(QStringLiteral("training_history.json"));
        training.m_analyticsWidget->setHistoryFilePath(history);
        assert(training.m_analyticsWidget->beginSession());
        training.m_analyticsWidget->setStats(emptyStats(), 12, 9);

        assert(training.m_stateMachine.restore(
            biofeedback::training::SessionState::Paused,
            biofeedback::training::SessionState::Active));
        training.m_currentExerciseIndex = 1;
        training.m_currentSeriesIndex = 0;
        training.m_currentRepIndex = 1;
        training.m_trainingClock.restore(12000, 9000);
        training.m_sessionClock.start();
        training.m_repetitionDetector.setTargetForce(50.0);
        biofeedback::training::RepetitionRecord completed;
        assert(!training.m_repetitionDetector.process(1000, 15.0, true, completed));
        assert(!training.m_repetitionDetector.process(1100, 30.0, true, completed));
        assert(training.m_repetitionDetector.isActive());

        training.m_analyticsWidget->setExecutionState(training.buildExecutionCheckpoint());
        assert(training.m_analyticsWidget->saveSessionCheckpoint());
        // Deliberately bypass destructors: this child models abrupt process death.
        std::_Exit(0);
    }

    static bool load(tab::TrainingTab& training, const QString& root)
    {
        answerRecoveryPrompt();
        training.setCurrentPatientContext(QString(), root);
        return training.m_stateMachine.running();
    }

    static int verifyActive(const QString& root)
    {
        tab::TrainingTab training;
        assert(load(training, root));
        assert(training.isTrainingPaused());
        assert(training.m_exercisePlan.size() == 2);
        assert(training.m_exercisePlan[1].name == QStringLiteral("Grip B"));
        assert(training.m_currentExerciseIndex == 1);
        assert(training.m_currentSeriesIndex == 0);
        assert(training.m_currentRepIndex == 1);
        assert(training.m_trainingClock.sessionMilliseconds() == 12000);
        assert(training.m_trainingClock.activeMilliseconds() == 9000);

        const auto detector = training.m_repetitionDetector.snapshot();
        assert(detector.active);
        assert(detector.sampleCount == 2);
        assert(detector.samples.size() == 2);
        assert(detector.samples[0].timestampMs == 1000);
        assert(detector.samples[1].timestampMs == 1100);

        assert(training.m_stateMachine.resume());
        biofeedback::training::RepetitionRecord completed;
        assert(!training.m_repetitionDetector.process(1200, 45.0, true, completed));
        assert(!training.m_repetitionDetector.process(1400, 60.0, true, completed));
        assert(training.m_repetitionDetector.process(1600, 0.0, true, completed));
        training.onRepetitionDetected(completed);
        assert(training.m_exerciseStats[1].completedReps == 1);
        assert(training.m_exerciseStats[1].repetitions.size() == 1);
        assert(training.m_exerciseStats[1].repetitions[0].repetitionIndex == 2);
        assert(training.m_currentRepIndex == 2);

        // Persist a paused rest with a non-round millisecond remainder.
        assert(training.m_stateMachine.restore(
            biofeedback::training::SessionState::Paused,
            biofeedback::training::SessionState::Resting,
            biofeedback::training::RestTarget::NextSeries));
        training.m_restDurationMs = 12345;
        training.m_restClock.invalidate();
        assert(training.saveExecutionCheckpoint());
        std::_Exit(0);
    }

    static int verifyRest(const QString& root)
    {
        tab::TrainingTab training;
        assert(load(training, root));
        assert(training.isTrainingPaused());
        assert(training.m_stateMachine.resting());
        assert(training.m_stateMachine.restTarget() ==
               biofeedback::training::RestTarget::NextSeries);
        assert(training.m_restDurationMs == 12345);
        assert(training.m_restTimeRemaining == 13);
        assert(training.m_currentExerciseIndex == 1);
        assert(training.m_currentSeriesIndex == 0);
        assert(training.m_currentRepIndex == 2);
        assert(training.m_exerciseStats[1].completedReps == 1);
        assert(training.m_exerciseStats[1].repetitions.size() == 1);
        assert(training.m_exerciseStats[1].repetitions[0].repetitionIndex == 2);
        assert(training.saveExecutionCheckpoint());
        std::_Exit(0);
    }
};
} // namespace tab

int main(int argc, char** argv)
{
    if (argc > 1)
        return tab::TrainingTabProcessRestartRegression::child(argc, argv);

    QApplication app(argc, argv);
    QTemporaryDir temporary;
    assert(temporary.isValid());
    const QString root = temporary.path();
    const QString executable = QCoreApplication::applicationFilePath();

    const QStringList stages = {
        QStringLiteral("--seed-active"),
        QStringLiteral("--verify-active"),
        QStringLiteral("--verify-rest"),
        QStringLiteral("--verify-rest-again")
    };
    for (const QString& stage : stages) {
        QProcess process;
        process.start(executable, {stage, root});
        assert(process.waitForStarted(10000));
        assert(process.waitForFinished(30000));
        if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
            qCritical().noquote() << "Restart stage failed:" << stage
                                  << "exit" << process.exitCode()
                                  << "stderr:" << process.readAllStandardError()
                                  << "stdout:" << process.readAllStandardOutput();
            return 1;
        }
    }
    return 0;
}
