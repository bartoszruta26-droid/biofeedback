#ifndef TAB_SAMPLEJOURNALWRITER_HPP
#define TAB_SAMPLEJOURNALWRITER_HPP

#include <QByteArray>
#include <QString>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <mutex>
#include <thread>
#include <utility>

namespace tab {

/**
 * Owns a JSONL sample journal on a dedicated writer thread.
 *
 * append() is non-blocking and bounded. A false return means the caller must
 * stop acquisition and surface an archive error; samples are never silently
 * discarded. flush() and close() are explicit barriers intended for lifecycle
 * boundaries, not the per-sample acquisition path.
 */
class SampleJournalWriter {
public:
    static constexpr std::size_t MaximumQueuedRecords = 8192;

    SampleJournalWriter() = default;
    ~SampleJournalWriter();

    SampleJournalWriter(const SampleJournalWriter&) = delete;
    SampleJournalWriter& operator=(const SampleJournalWriter&) = delete;

    bool open(const QString& path, const QByteArray& optionalHeaderLine);
    bool append(const QByteArray& jsonLine);
    bool flush();
    bool close();
    bool isOpen() const;
    QString errorString() const;

private:
    void run(QString path, QByteArray headerLine);
    bool flushFile();

    mutable std::mutex m_mutex;
    std::condition_variable m_workAvailable;
    std::condition_variable m_stateChanged;
    std::deque<QByteArray> m_queue;
    std::thread m_thread;
    QString m_error;
    bool m_startupComplete{false};
    bool m_startupSucceeded{false};
    bool m_accepting{false};
    bool m_stopping{false};
    bool m_open{false};
    bool m_failed{false};
    std::uint64_t m_flushRequested{0};
    std::uint64_t m_flushCompleted{0};
};

} // namespace tab

#endif // TAB_SAMPLEJOURNALWRITER_HPP
