#include "tab/SampleJournalWriter.hpp"

#include <QFile>
#include <QTemporaryDir>

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

    tab::SampleJournalWriter failedWriter;
    require(!failedWriter.open(directory.filePath("missing/subdir/journal.jsonl"),
                               header, nullptr),
            "writer unexpectedly opened a path with a missing parent");

    std::cout << "Sample journal writer regression passed" << std::endl;
    return 0;
}
