#include "tab/SampleJournalWriter.hpp"
#include "measurement/MeasurementEngine.hpp"

#include <QFile>
#include <QTemporaryDir>
#include <QJsonDocument>
#include <QJsonObject>

#include <cstdlib>
#include <iostream>

namespace {
void require(bool condition, const char* message)
{
    if (!condition) {
        std::cerr << message << std::endl;
        std::exit(1);
    }
}
}

int main()
{
    QTemporaryDir directory;
    require(directory.isValid(), "temporary directory creation failed");

    const QString path = directory.filePath(QStringLiteral("session.jsonl"));
    const QByteArray header =
        "{\"recordType\":\"header\",\"schemaVersion\":1,"
        "\"firstSequence\":1}\n";

    {
        tab::SampleJournalWriter writer;
        QString error;
        require(writer.open(path, header, &error), "journal open failed");
        require(writer.isAccepting(), "writer did not accept after open");

        constexpr int sampleCount = 4000;
        for (int sequence = 1; sequence <= sampleCount; ++sequence) {
            const QByteArray line =
                QByteArray("{\"recordType\":\"sample\",\"sequence\":") +
                QByteArray::number(sequence) + "}\n";
            require(writer.append(line), "sample enqueue unexpectedly failed");
        }
        require(writer.finish(), "writer failed while draining accepted samples");
        require(!writer.isAccepting(), "writer still accepts after finish");
        require(!writer.append("{\"sequence\":4001}\n"),
                "writer accepted a sample after finish");
    }

    QFile file(path);
    require(file.open(QIODevice::ReadOnly | QIODevice::Text), "journal reopen failed");
    const QList<QByteArray> lines = file.readAll().split('\n');
    require(lines.size() == 4002, "journal line count mismatch");
    require(lines.first().contains("\"schemaVersion\":1"),
            "schema-v1 header was not preserved");
    for (int i = 1; i <= 4000; ++i) {
        const QByteArray expected =
            QByteArray("\"sequence\":") + QByteArray::number(i);
        require(lines.at(i).contains(expected.constData()),
                "journal sequence continuity failure");
    }
    file.close();

    // Reopening an existing journal must append records without duplicating its header.
    {
        tab::SampleJournalWriter writer;
        require(writer.open(path, header), "existing journal reopen failed");
        require(writer.append("{\"recordType\":\"sample\",\"sequence\":4001}\n"),
                "append after reopen failed");
        require(writer.finish(), "reopened writer failed to finish");
    }
    require(file.open(QIODevice::ReadOnly | QIODevice::Text), "journal final read failed");
    require(file.readAll().count('\n') == 4002,
            "reopening duplicated the header or lost the final record");
    file.close();

    // The durable commit hook runs before subscribers. Even if the GUI
    // subscriber intentionally discards every update, the archive remains complete.
    const QString committedPath = directory.filePath(QStringLiteral("committed.jsonl"));
    tab::SampleJournalWriter committedWriter;
    const QByteArray committedHeader =
        "{\"recordType\":\"header\",\"schemaVersion\":1,\"firstSequence\":1}\n";
    require(committedWriter.open(committedPath, committedHeader),
            "commit journal open failed");

    biofeedback::measurement::MeasurementEngine engine;
    engine.setCommitHook([&](const biofeedback::measurement::MeasurementSample& sample) {
        if (!sample.isValid()) return true;
        const QJsonObject record{
            {QStringLiteral("recordType"), QStringLiteral("sample")},
            {QStringLiteral("sequence"), static_cast<qint64>(sample.sequence)},
            {QStringLiteral("value"), sample.value},
            {QStringLiteral("unit"), QString::fromStdString(sample.unit)}
        };
        return committedWriter.appendJsonDurably(record);
    });
    engine.subscribe([&](const biofeedback::measurement::MeasurementSample&) {
        // Simulate a GUI presentation queue that cannot display this frame.
    });

    constexpr int committedCount = 200;
    for (int i = 0; i < committedCount; ++i) {
        biofeedback::measurement::MeasurementInput input;
        input.deviceTimestampMs = static_cast<std::uint32_t>(i * 12);
        input.hasTimestamp = true;
        input.rawValue = i;
        input.value = 10.0;
        input.sensorValid = true;
        input.unit = "g";
        input.source = "journal-regression";
        biofeedback::measurement::MeasurementSample sample;
        require(engine.ingest(input, sample), "durable engine ingest was rejected");
    }
    require(committedWriter.finish(), "commit journal failed to finish");
    QFile committedFile(committedPath);
    require(committedFile.open(QIODevice::ReadOnly | QIODevice::Text),
            "commit journal could not be reopened");
    const QList<QByteArray> committedLines = committedFile.readAll().split('\n');
    require(committedLines.size() == committedCount + 2,
            "GUI subscriber backlog changed durable archive sample count");
    for (int i = 1; i <= committedCount; ++i) {
        const QJsonDocument document = QJsonDocument::fromJson(committedLines.at(i));
        require(document.isObject(), "committed journal record is invalid JSON");
        require(document.object().value(QStringLiteral("sequence")).toInt() == i,
                "commit hook did not preserve sequence order");
    }
    committedFile.close();

    tab::SampleJournalWriter failedWriter;
    require(!failedWriter.open(directory.filePath("missing/subdir/journal.jsonl"),
                               header, nullptr),
            "writer unexpectedly opened a path with a missing parent");

    std::cout << "Sample journal writer regression passed" << std::endl;
    return 0;
}
