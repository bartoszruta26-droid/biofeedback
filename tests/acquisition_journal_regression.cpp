#include "data/AcquisitionJournal.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonObject>
#include <QTemporaryDir>

#include <cassert>

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    QTemporaryDir temporary;
    assert(temporary.isValid());
    const QString key(64, QLatin1Char('K'));
    const QString path = temporary.filePath(QStringLiteral("journal"));

    {
        biofeedback::data::AcquisitionJournal journal(path, key);
        assert(journal.isReady());
        assert(journal.lastSequence() == 0);
        QString error;
        assert(journal.append(1, QJsonObject{{"sequence", 1}, {"value", 12.5}}, &error));
        assert(error.isEmpty());
        assert(journal.lastSequence() == 1);
        assert(!journal.append(3, QJsonObject{{"sequence", 3}}, &error));
    }
    {
        biofeedback::data::AcquisitionJournal recovered(path, key);
        assert(recovered.isReady());
        assert(recovered.lastSequence() == 1);
        QString error;
        const auto records = recovered.readAfter(0, &error);
        assert(error.isEmpty());
        assert(records.size() == 1);
        assert(records.front().sequence == 1);
        assert(records.front().payload.value("value").toDouble() == 12.5);
        assert(recovered.readAfter(1, &error).isEmpty());
    }
    {
        biofeedback::data::AcquisitionJournal wrongKey(path, QString(64, QLatin1Char('Z')));
        assert(!wrongKey.isReady());
        assert(wrongKey.initializationError() == QStringLiteral("JOURNAL_AUTHENTICATION_FAILED"));
    }
    {
        QDir directory(path);
        const QString record = directory.filePath(QStringLiteral("00000000000000000001.bfj"));
        QFile file(record);
        assert(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
        file.write("corrupt");
        file.close();
        biofeedback::data::AcquisitionJournal corrupted(path, key);
        assert(!corrupted.isReady());
    }
    return 0;
}
