#include "tab/SampleJournalWriter.hpp"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <iostream>

int main()
{
    QTemporaryDir directory;
    if (!directory.isValid()) {
        std::cerr << "temporary directory unavailable\n";
        return 1;
    }

    const QString path = directory.filePath(QStringLiteral("session.jsonl"));
    tab::SampleJournalWriter writer;
    const QByteArray header = QByteArrayLiteral("{\"recordType\":\"header\",\"schemaVersion\":1}\n");
    if (!writer.open(path, header)) {
        std::cerr << "writer open failed: " << writer.errorString().toStdString() << "\n";
        return 2;
    }

    constexpr int firstBatch = 600;
    constexpr int secondBatch = 400;
    for (int sequence = 1; sequence <= firstBatch; ++sequence) {
        const QByteArray line = QByteArrayLiteral("{\"recordType\":\"sample\",\"sequence\":")
            + QByteArray::number(sequence) + QByteArrayLiteral("}\n");
        if (!writer.append(line)) {
            std::cerr << "append failed at " << sequence << "\n";
            writer.close();
            return 3;
        }
    }
    if (!writer.flush()) {
        std::cerr << "flush barrier failed: " << writer.errorString().toStdString() << "\n";
        writer.close();
        return 4;
    }

    for (int sequence = firstBatch + 1; sequence <= firstBatch + secondBatch; ++sequence) {
        const QByteArray line = QByteArrayLiteral("{\"recordType\":\"sample\",\"sequence\":")
            + QByteArray::number(sequence) + QByteArrayLiteral("}\n");
        if (!writer.append(line)) {
            std::cerr << "append failed at " << sequence << "\n";
            writer.close();
            return 5;
        }
    }
    if (!writer.close()) {
        std::cerr << "close failed: " << writer.errorString().toStdString() << "\n";
        return 6;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::cerr << "journal could not be reopened\n";
        return 7;
    }
    const QByteArray headerLine = file.readLine();
    const QJsonDocument headerDoc = QJsonDocument::fromJson(headerLine);
    if (!headerDoc.isObject() ||
        headerDoc.object().value(QStringLiteral("recordType")).toString() != QStringLiteral("header"))
        return 8;

    int expected = 1;
    while (!file.atEnd()) {
        const QJsonDocument document = QJsonDocument::fromJson(file.readLine());
        if (!document.isObject()) {
            std::cerr << "invalid JSONL record at " << expected << "\n";
            return 9;
        }
        const QJsonObject object = document.object();
        if (object.value(QStringLiteral("recordType")).toString() != QStringLiteral("sample") ||
            object.value(QStringLiteral("sequence")).toInt() != expected) {
            std::cerr << "sample order mismatch at " << expected << "\n";
            return 10;
        }
        ++expected;
    }
    if (expected != firstBatch + secondBatch + 1) {
        std::cerr << "unexpected sample count: " << (expected - 1) << "\n";
        return 11;
    }

    std::cout << "asynchronous sample journal regression passed\n";
    return 0;
}
