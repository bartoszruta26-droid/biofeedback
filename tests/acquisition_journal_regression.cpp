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
        for (quint64 sequence = 1; sequence <= 300; ++sequence) {
            assert(journal.append(sequence,
                QJsonObject{{"sequence", static_cast<qint64>(sequence)}, {"value", 12.5}}, &error));
        }
        assert(journal.lastSequence() == 300);
        assert(journal.pendingCount() == 44);
        assert(!journal.append(302, QJsonObject{{"sequence", 302}}, &error));
        assert(journal.flush(&error));
        assert(journal.pendingCount() == 0);
        assert(QDir(path).entryList({QStringLiteral("*.bfj")}, QDir::Files).size() == 2);
    }
    {
        biofeedback::data::AcquisitionJournal recovered(path, key);
        assert(recovered.isReady());
        assert(recovered.lastSequence() == 300);
        QString error;
        const auto records = recovered.readAfter(299, &error);
        assert(error.isEmpty());
        assert(records.size() == 1);
        assert(records.front().sequence == 300);
        assert(records.front().payload.value("value").toDouble() == 12.5);
        assert(recovered.readAfter(300, &error).isEmpty());
    }
    {
        biofeedback::data::AcquisitionJournal wrongKey(path, QString(64, QLatin1Char('Z')));
        assert(!wrongKey.isReady());
    }
    {
        QDir directory(path);
        const QString record = directory.filePath(QStringLiteral("00000000000000000001-00000000000000000256.bfj"));
        QFile file(record);
        assert(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
        file.write("corrupt");
        file.close();
        biofeedback::data::AcquisitionJournal corrupted(path, key);
        assert(!corrupted.isReady());
    }
    return 0;
}
