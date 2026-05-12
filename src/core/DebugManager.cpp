#include "core/DebugManager.hpp"
#include <QDebug>
#include <QThread>

namespace core {

// ============================================================================
// DEBUG FLAGS AND CONFIGURATION
// ============================================================================

/// Enable detailed debug logging for internal operations
constexpr bool DEBUG_DEBUGMANAGER = true;

/// Maximum number of messages to track in statistics (prevents overflow)
constexpr int MAX_MESSAGE_COUNT = INT32_MAX;

DebugManager& DebugManager::instance()
{
    static DebugManager instance;
    return instance;
}

DebugManager::DebugManager()
    : m_debugCallback(nullptr)
    , m_minimumLogLevel(DebugLevel::DEBUG)
    , m_consoleOutputEnabled(true)
    , m_totalMessageCount(0)
    , m_criticalCount(0)
{
#if DEBUG_DEBUGMANAGER
    qInfo() << "[DEBUG_MANAGER] Initializing DebugManager singleton...";
#endif
    
    // Initialize message counters for standard types
    QMutexLocker locker(&m_mutex);
    m_messageCounts["VERBOSE"] = 0;
    m_messageCounts["DEBUG"] = 0;
    m_messageCounts["INFO"] = 0;
    m_messageCounts["WARNING"] = 0;
    m_messageCounts["ERROR"] = 0;
    m_messageCounts["CRITICAL"] = 0;
    
#if DEBUG_DEBUGMANAGER
    qInfo() << "[DEBUG_MANAGER] DebugManager initialized successfully";
#endif
}

DebugManager::~DebugManager()
{
#if DEBUG_DEBUGMANAGER
    qInfo() << "[DEBUG_MANAGER] DebugManager shutting down. Total messages:" << m_totalMessageCount.load();
#endif
    unregisterDebugCallback();
}

void DebugManager::registerDebugCallback(std::function<void(const QString&, const QString&)> callback)
{
    QMutexLocker locker(&m_mutex);
    m_debugCallback = callback;
#if DEBUG_DEBUGMANAGER
    qInfo() << "[DEBUG_MANAGER] Debug callback registered";
#endif
}

void DebugManager::unregisterDebugCallback()
{
    QMutexLocker locker(&m_mutex);
    m_debugCallback = nullptr;
#if DEBUG_DEBUGMANAGER
    qInfo() << "[DEBUG_MANAGER] Debug callback unregistered";
#endif
}

void DebugManager::sendDebugMessage(const QString& message, const QString& type)
{
    // Convert type string to DebugLevel
    DebugLevel level = DebugLevel::INFO;
    if (type == "VERBOSE") level = DebugLevel::VERBOSE;
    else if (type == "DEBUG") level = DebugLevel::DEBUG;
    else if (type == "WARNING" || type == "WARN") level = DebugLevel::WARNING;
    else if (type == "ERROR" || type == "ERR") level = DebugLevel::ERROR;
    else if (type == "CRITICAL" || type == "CRIT" || type == "FATAL") level = DebugLevel::CRITICAL;
    else if (type == "DATA") level = DebugLevel::VERBOSE;
    else if (type == "SERIAL") level = DebugLevel::DEBUG;
    else if (type == "ARDUINO") level = DebugLevel::DEBUG;
    
    sendDebugMessage(message, level, type);
}

void DebugManager::sendDebugMessage(const QString& message, DebugLevel level, const QString& source)
{
    // Check minimum log level filter
    if (level < m_minimumLogLevel) {
        return;
    }
    
    // Create debug message with full context
    DebugMessage msg;
    msg.message = message;
    msg.level = level;
    msg.source = source;
    msg.type = levelToString(level);
    msg.timestamp = QDateTime::currentDateTime();
    msg.threadId = static_cast<int>(QThread::currentThreadId());
    
    logMessageInternal(msg);
}

bool DebugManager::hasCallback() const
{
    QMutexLocker locker(&m_mutex);
    return m_debugCallback != nullptr;
}

void DebugManager::setMinimumLogLevel(DebugLevel level)
{
    QMutexLocker locker(&m_mutex);
    m_minimumLogLevel = level;
#if DEBUG_DEBUGMANAGER
    qInfo() << "[DEBUG_MANAGER] Minimum log level set to:" << levelToString(level);
#endif
}

DebugManager::DebugLevel DebugManager::minimumLogLevel() const
{
    QMutexLocker locker(&m_mutex);
    return m_minimumLogLevel;
}

void DebugManager::setConsoleOutputEnabled(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_consoleOutputEnabled = enabled;
#if DEBUG_DEBUGMANAGER
    qInfo() << "[DEBUG_MANAGER] Console output" << (enabled ? "enabled" : "disabled");
#endif
}

bool DebugManager::isConsoleOutputEnabled() const
{
    QMutexLocker locker(&m_mutex);
    return m_consoleOutputEnabled;
}

void DebugManager::clearMessageHistory()
{
    // Note: Currently we don't store full history, just counts
    // This method is here for future expansion
#if DEBUG_DEBUGMANAGER
    qInfo() << "[DEBUG_MANAGER] Message history cleared (counts preserved)";
#endif
}

int DebugManager::getMessageCount(const QString& type) const
{
    QMutexLocker locker(&m_mutex);
    auto it = m_messageCounts.find(type);
    if (it != m_messageCounts.end()) {
        return it->second.load();
    }
    return 0;
}

int DebugManager::getTotalMessageCount() const
{
    return m_totalMessageCount.load();
}

void DebugManager::resetStatistics()
{
    QMutexLocker locker(&m_mutex);
    for (auto& pair : m_messageCounts) {
        pair.second = 0;
    }
    m_totalMessageCount = 0;
    m_criticalCount = 0;
    
#if DEBUG_DEBUGMANAGER
    qInfo() << "[DEBUG_MANAGER] Statistics reset";
#endif
    
    emit statisticsReset();
}

QString DebugManager::getStatisticsSummary() const
{
    QMutexLocker locker(&m_mutex);
    QString summary;
    QTextStream stream(&summary);
    
    stream << "=== Debug Statistics ===" << Qt::endl;
    stream << "Total messages: " << m_totalMessageCount.load() << Qt::endl;
    stream << "Critical errors: " << m_criticalCount.load() << Qt::endl;
    stream << "---" << Qt::endl;
    
    for (const auto& pair : m_messageCounts) {
        stream << pair.first << ": " << pair.second.load() << Qt::endl;
    }
    
    return summary;
}

QString DebugManager::levelToString(DebugLevel level)
{
    switch (level) {
        case DebugLevel::VERBOSE:   return "VERBOSE";
        case DebugLevel::DEBUG:     return "DEBUG";
        case DebugLevel::INFO:      return "INFO";
        case DebugLevel::WARNING:   return "WARNING";
        case DebugLevel::ERROR:     return "ERROR";
        case DebugLevel::CRITICAL:  return "CRITICAL";
        default:                    return "UNKNOWN";
    }
}

void DebugManager::logMessageInternal(const DebugMessage& msg)
{
    QMutexLocker locker(&m_mutex);
    
    // Update statistics
    if (m_totalMessageCount.load() < MAX_MESSAGE_COUNT) {
        m_totalMessageCount++;
    }
    
    auto it = m_messageCounts.find(msg.type);
    if (it != m_messageCounts.end()) {
        if (it->second.load() < MAX_MESSAGE_COUNT) {
            it->second++;
        }
    }
    
    if (msg.level == DebugLevel::CRITICAL) {
        if (m_criticalCount.load() < MAX_MESSAGE_COUNT) {
            m_criticalCount++;
        }
    }
    
    // Emit signal for Qt connections
    emit debugMessageSent(msg.message, msg.type);
    
    // Call registered callback (e.g., for DebugTab)
    if (m_debugCallback) {
        try {
            m_debugCallback(msg.message, msg.type);
        } catch (const std::exception& e) {
            qWarning() << "[DEBUG_MANAGER] Exception in debug callback:" << e.what();
            // Don't propagate - callback errors shouldn't break debugging
        } catch (...) {
            qWarning() << "[DEBUG_MANAGER] Unknown exception in debug callback";
        }
    } else if (m_consoleOutputEnabled) {
        // Fallback to stderr if no callback registered
        QString logLine = QString("[%1] [%2] [%3] %4")
            .arg(msg.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz"))
            .arg(msg.type)
            .arg(msg.source.isEmpty() ? "Unknown" : msg.source)
            .arg(msg.message);
        
        if (msg.level >= DebugLevel::WARNING) {
            qWarning().noquote() << logLine;
        } else {
            qInfo().noquote() << logLine;
        }
    }
    
    // Emit special signal for critical errors
    if (msg.level == DebugLevel::CRITICAL) {
        emit criticalErrorOccurred(msg.message, msg.source);
    }
}

} // namespace core
