#include "tab/SampleJournalWriter.hpp"

#include <QFile>
#include <QFileDevice>
#include <algorithm>
#include <cerrno>
#ifdef Q_OS_UNIX
#include <unistd.h>
#endif

namespace tab {

SampleJournalWriter::~SampleJournalWriter()
{
    close();
}

bool SampleJournalWriter::open(const QString& path, const QByteArray& optionalHeaderLine)
{
    close();
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.clear();
        m_error.clear();
        m_startupComplete = false;
        m_startupSucceeded = false;
        m_accepting = true;
        m_stopping = false;
        m_open = false;
        m_failed = false;
        m_flushRequested = 0;
        m_flushCompleted = 0;
    }

    try {
        m_thread = std::thread(&SampleJournalWriter::run, this, path, optionalHeaderLine);
    } catch (...) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_accepting = false;
        m_failed = true;
        m_error = QStringLiteral("Nie można uruchomić wątku zapisu dziennika");
        return false;
    }

    std::unique_lock<std::mutex> lock(m_mutex);
    m_stateChanged.wait(lock, [this] { return m_startupComplete; });
    return m_startupSucceeded;
}

bool SampleJournalWriter::append(const QByteArray& jsonLine)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_accepting || m_failed || !m_open ||
        m_queue.size() >= MaximumQueuedRecords)
        return false;
    m_queue.push_back(jsonLine);
    m_workAvailable.notify_one();
    return true;
}

bool SampleJournalWriter::flush()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_failed) return false;
    if (!m_open) return true;
    const std::uint64_t request = ++m_flushRequested;
    m_workAvailable.notify_one();
    m_stateChanged.wait(lock, [this, request] {
        return m_flushCompleted >= request || m_failed || !m_open;
    });
    return !m_failed && m_flushCompleted >= request;
}

bool SampleJournalWriter::close()
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_thread.joinable()) return !m_failed;
        m_accepting = false;
        m_stopping = true;
        m_workAvailable.notify_one();
    }
    m_thread.join();
    std::lock_guard<std::mutex> lock(m_mutex);
    m_open = false;
    return !m_failed;
}

bool SampleJournalWriter::isOpen() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_open && !m_failed;
}

QString SampleJournalWriter::errorString() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_error;
}

bool SampleJournalWriter::flushFile(QFile& file)
{
    if (!file.flush()) return false;
#ifdef Q_OS_UNIX
    if (::fsync(file.handle()) != 0) return false;
#endif
    return true;
}

void SampleJournalWriter::run(QString path, QByteArray headerLine)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_failed = true;
        m_error = file.errorString();
        m_startupComplete = true;
        m_stateChanged.notify_all();
        return;
    }

    if (!headerLine.isEmpty() &&
        (file.write(headerLine) != headerLine.size() || !flushFile(file))) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_failed = true;
        m_error = file.errorString().isEmpty()
            ? QStringLiteral("Nie można trwale zapisać nagłówka dziennika")
            : file.errorString();
        m_startupComplete = true;
        m_stateChanged.notify_all();
        file.close();
        return;
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_open = true;
        m_startupSucceeded = true;
        m_startupComplete = true;
        m_stateChanged.notify_all();
    }

    std::size_t recordsSinceFlush = 0;
    for (;;) {
        QByteArray line;
        bool doFlush = false;
        bool shouldStop = false;
        std::uint64_t flushTarget = 0;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_workAvailable.wait(lock, [this] {
                return !m_queue.empty() || m_stopping ||
                       m_flushRequested > m_flushCompleted || m_failed;
            });
            if (m_failed) break;
            if (!m_queue.empty()) {
                line = std::move(m_queue.front());
                m_queue.pop_front();
                m_stateChanged.notify_all();
            } else {
                doFlush = m_flushRequested > m_flushCompleted || m_stopping;
                flushTarget = m_flushRequested;
                shouldStop = m_stopping;
            }
        }

        if (!line.isEmpty()) {
            if (file.write(line) != line.size()) {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_failed = true;
                m_accepting = false;
                m_error = file.errorString();
                m_queue.clear();
                m_stateChanged.notify_all();
                break;
            }
            if (++recordsSinceFlush >= 32) {
                if (!flushFile(file)) {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    m_failed = true;
                    m_accepting = false;
                    m_error = file.errorString().isEmpty()
                        ? QStringLiteral("Błąd synchronizacji dziennika")
                        : file.errorString();
                    m_queue.clear();
                    m_stateChanged.notify_all();
                    break;
                }
                recordsSinceFlush = 0;
            }
            continue;
        }

        if (doFlush) {
            if (!flushFile(file)) {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_failed = true;
                m_accepting = false;
                m_error = file.errorString().isEmpty()
                    ? QStringLiteral("Błąd synchronizacji dziennika")
                    : file.errorString();
                m_queue.clear();
                m_stateChanged.notify_all();
                break;
            }
            recordsSinceFlush = 0;
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_flushCompleted = std::max(m_flushCompleted, flushTarget);
                m_stateChanged.notify_all();
            }
        }
        if (shouldStop) break;
    }

    file.close();
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_open = false;
        m_accepting = false;
        m_stateChanged.notify_all();
    }
}

} // namespace tab
