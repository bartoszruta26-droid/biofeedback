#ifndef TAB_SAMPLEJOURNALWRITER_HPP
#define TAB_SAMPLEJOURNALWRITER_HPP

#include <QByteArray>
#include <QFile>
#include <QString>

#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <mutex>
#include <thread>

namespace tab {

/**
 * @brief Bounded, single-owner background writer for the desktop JSONL journal.
 *
 * The worker thread exclusively owns the QFile. append() only performs a
 * bounded in-memory enqueue; disk writes and fsync never run on the GUI thread.
 * A full queue or I/O failure is reported explicitly and rejects further input.
 */
class SampleJournalWriter {
public:
    explicit SampleJournalWriter(std::size_t capacity = 8192);
    ~SampleJournalWriter();

    SampleJournalWriter(const SampleJournalWriter&) = delete;
    SampleJournalWriter& operator=(const SampleJournalWriter&) = delete;

    // Writes and durably flushes the header before returning.
    bool open(const QString& path, const QByteArray& headerLine,
              QString* error = nullptr);
    // Non-blocking bounded enqueue. false means the journal is no longer accepting.
    bool append(const QByteArray& line);
    // Blocks the acquisition producer until this record has been fsynced.
    bool appendDurably(const QByteArray& line);
    // Stops admission, drains accepted records, flushes/fsyncs and joins the worker.
    bool finish();
    bool isAccepting() const;
    QString errorString() const;

private:
    struct PendingRecord {
        QByteArray line;
        std::uint64_t ticket{0};
        bool durable{false};
    };
    bool enqueue(const QByteArray& line, bool durable);
    void run();
    static bool writeDurably(QFile& file, const QByteArray& bytes,
                             QString& error);
    static bool flushDurably(QFile& file, QString& error);
    static bool syncParentDirectory(const QString& path, QString& error);

    const std::size_t m_capacity;
    mutable std::mutex m_mutex;
    std::condition_variable m_changed;
    std::condition_variable m_openCompleted;
    std::condition_variable m_completed;
    std::deque<PendingRecord> m_queue;
    QString m_path;
    QByteArray m_headerLine;
    QString m_error;
    std::thread m_thread;
    bool m_openComplete{false};
    bool m_opened{false};
    bool m_accepting{false};
    bool m_stopRequested{false};
    bool m_failed{false};
    std::uint64_t m_nextTicket{0};
    std::uint64_t m_completedTicket{0};
};

} // namespace tab

#endif // TAB_SAMPLEJOURNALWRITER_HPP
