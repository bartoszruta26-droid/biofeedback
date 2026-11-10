#include "tab/SampleJournalWriter.hpp"

#include <QFile>
#include <QFileDevice>
#include <QFileInfo>
#include <QJsonDocument>

#include <cerrno>
#include <cstring>

#ifdef Q_OS_UNIX
#include <fcntl.h>
#include <unistd.h>
#endif

#include <algorithm>
#include <utility>

namespace tab {

SampleJournalWriter::SampleJournalWriter(std::size_t capacity)
    : m_capacity(std::max<std::size_t>(1, capacity))
{
}

SampleJournalWriter::~SampleJournalWriter()
{
    finish();
}

bool SampleJournalWriter::open(const QString& path, const QByteArray& headerLine,
                               QString* error)
{
    finish();
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.clear();
        m_path = path;
        m_headerLine = headerLine;
        m_error.clear();
        m_openComplete = false;
        m_opened = false;
        m_accepting = false;
        m_stopRequested = false;
        m_failed = false;
        m_nextTicket = 0;
        m_completedTicket = 0;
    }

    try {
        m_thread = std::thread(&SampleJournalWriter::run, this);
    } catch (...) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_openComplete = true;
        m_failed = true;
        m_error = QStringLiteral("Unable to start sample journal writer thread");
        if (error)
            *error = m_error;
        return false;
    }
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_openCompleted.wait(lock, [this]() { return m_openComplete; });
        if (error)
            *error = m_error;
        if (m_opened)
            return true;
    }

    if (m_thread.joinable())
        m_thread.join();
    return false;
}

bool SampleJournalWriter::append(const QByteArray& line)
{
    return enqueue(line, false);
}

bool SampleJournalWriter::appendDurably(const QByteArray& line)
{
    return enqueue(line, true);
}

bool SampleJournalWriter::appendJson(const QJsonObject& record)
{
    return enqueueJson(record, false);
}

bool SampleJournalWriter::appendJsonDurably(const QJsonObject& record)
{
    return enqueueJson(record, true);
}

bool SampleJournalWriter::enqueueJson(const QJsonObject& record, bool durable)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_accepting || m_failed || record.isEmpty())
        return false;
    if (m_queue.size() >= m_capacity) {
        m_failed = true;
        m_accepting = false;
        m_stopRequested = true;
        m_error = QStringLiteral("Sample journal queue capacity exceeded; acquisition must stop");
        m_changed.notify_one();
        m_completed.notify_all();
        return false;
    }
    const std::uint64_t ticket = ++m_nextTicket;
    try {
        PendingRecord pending;
        pending.ticket = ticket;
        pending.durable = durable;
        pending.jsonObject = record;
        pending.isJsonObject = true;
        m_queue.push_back(std::move(pending));
    } catch (...) {
        m_failed = true;
        m_accepting = false;
        m_stopRequested = true;
        m_error = QStringLiteral("Unable to allocate sample journal queue entry");
        m_changed.notify_one();
        m_completed.notify_all();
        return false;
    }
    m_changed.notify_one();
    if (!durable)
        return true;
    m_completed.wait(lock, [this, ticket] {
        return m_completedTicket >= ticket || m_failed;
    });
    return m_completedTicket >= ticket && !m_failed;
}

bool SampleJournalWriter::enqueue(const QByteArray& line, bool durable)
{
    if (line.isEmpty())
        return false;

    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_accepting || m_failed)
        return false;
    if (m_queue.size() >= m_capacity) {
        m_failed = true;
        m_accepting = false;
        m_stopRequested = true;
        m_error = QStringLiteral("Sample journal queue capacity exceeded; acquisition must stop");
        m_changed.notify_one();
        m_completed.notify_all();
        return false;
    }

    const std::uint64_t ticket = ++m_nextTicket;
    try {
        m_queue.push_back(PendingRecord{line, ticket, durable});
    } catch (...) {
        m_failed = true;
        m_accepting = false;
        m_stopRequested = true;
        m_error = QStringLiteral("Unable to allocate sample journal queue entry");
        m_changed.notify_one();
        m_completed.notify_all();
        return false;
    }
    m_changed.notify_one();

    if (!durable)
        return true;

    m_completed.wait(lock, [this, ticket] {
        return m_completedTicket >= ticket || m_failed;
    });
    return m_completedTicket >= ticket && !m_failed;
}

bool SampleJournalWriter::finish()
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_thread.joinable())
            return !m_failed;
        m_accepting = false;
        m_stopRequested = true;
    }
    m_changed.notify_one();
    m_thread.join();

    std::lock_guard<std::mutex> lock(m_mutex);
    return !m_failed;
}

bool SampleJournalWriter::isAccepting() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_accepting && !m_failed;
}

QString SampleJournalWriter::errorString() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_error;
}

bool SampleJournalWriter::syncParentDirectory(const QString& path, QString& error)
{
#ifdef Q_OS_UNIX
    const QByteArray directoryPath =
        QFile::encodeName(QFileInfo(path).absolutePath());
    const int directoryFd = ::open(directoryPath.constData(), O_RDONLY);
    if (directoryFd < 0) {
        error = QStringLiteral("Unable to open journal directory for fsync: %1")
                    .arg(QString::fromLocal8Bit(std::strerror(errno)));
        return false;
    }

    const int result = ::fsync(directoryFd);
    const int savedErrno = errno;
    ::close(directoryFd);
    if (result != 0) {
        error = QStringLiteral("Unable to fsync journal directory: %1")
                    .arg(QString::fromLocal8Bit(std::strerror(savedErrno)));
        return false;
    }
#else
    Q_UNUSED(path);
    Q_UNUSED(error);
#endif
    return true;
}

bool SampleJournalWriter::flushDurably(QFile& file, QString& error)
{
    if (!file.flush()) {
        error = file.errorString();
        return false;
    }
#ifdef Q_OS_UNIX
    if (::fsync(file.handle()) != 0) {
        error = QStringLiteral("fsync failed for sample journal");
        return false;
    }
#endif
    return true;
}

bool SampleJournalWriter::writeDurably(QFile& file, const QByteArray& bytes,
                                       QString& error)
{
    if (file.write(bytes) != bytes.size()) {
        error = file.errorString();
        return false;
    }
    return flushDurably(file, error);
}

void SampleJournalWriter::run()
{
    QFile file(m_path);
    const bool newFile = !QFile::exists(m_path);
    bool opened = file.open(QIODevice::WriteOnly | QIODevice::Append);
    QString error;

    if (!opened) {
        error = file.errorString();
    } else if (newFile) {
        // Restrict access before the first patient-linked byte is written.
        if (!QFile::setPermissions(m_path,
                                   QFileDevice::ReadOwner | QFileDevice::WriteOwner)) {
            error = QStringLiteral("Unable to restrict sample journal file permissions");
            opened = false;
        } else if (!writeDurably(file, m_headerLine, error) ||
                   !syncParentDirectory(m_path, error)) {
            opened = false;
        }
    }

    if (!opened && file.isOpen()) {
        file.close();
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_openComplete = true;
        m_opened = opened;
        m_accepting = opened;
        if (!opened) {
            m_failed = true;
            m_error = error.isEmpty()
                ? QStringLiteral("Unable to open sample journal") : error;
            m_stopRequested = true;
        }
    }
    m_openCompleted.notify_all();

    if (!opened)
        return;

    unsigned int pendingSinceSync = 0;
    for (;;) {
        PendingRecord record;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_changed.wait(lock, [this]() {
                return m_stopRequested || !m_queue.empty();
            });
            if (m_queue.empty() && m_stopRequested)
                break;
            if (m_queue.empty())
                continue;
            record = std::move(m_queue.front());
            m_queue.pop_front();
        }

        QByteArray line = record.isJsonObject
            ? QJsonDocument(record.jsonObject).toJson(QJsonDocument::Compact) + '\\n'
            : std::move(record.line);
        if (file.write(line) != line.size()) {
            error = file.errorString();
            std::lock_guard<std::mutex> lock(m_mutex);
            m_failed = true;
            m_accepting = false;
            m_stopRequested = true;
            m_error = error;
            m_queue.clear();
            m_completed.notify_all();
            break;
        }

        const bool syncRequired = record.durable || ++pendingSinceSync >= 8;
        if (syncRequired) {
            if (!flushDurably(file, error)) {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_failed = true;
                m_accepting = false;
                m_stopRequested = true;
                m_error = error;
                m_queue.clear();
                m_completed.notify_all();
                break;
            }
            pendingSinceSync = 0;
        }
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_completedTicket = std::max(m_completedTicket, record.ticket);
            m_completed.notify_all();
        }
    }

    if (!flushDurably(file, error)) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_failed = true;
        m_error = error;
    }
    file.close();

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_accepting = false;
        m_completed.notify_all();
    }
}

} // namespace tab
