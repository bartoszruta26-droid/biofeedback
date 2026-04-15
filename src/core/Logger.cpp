#include "core/Logger.hpp"
#include "core/ConfigManager.hpp"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sys/stat.h>

namespace biofeedback {

Logger::Logger(ConfigManager& config)
    : m_config(config)
    , m_minLevel(LogLevel::INFO)
    , m_maxSizeBytes(10 * 1024 * 1024)  // 10MB default
    , m_rotateCount(5)
    , m_consoleOutput(true)
{
    m_logPath = config.getString("logging.file", "./logs/biofeedback.log");
    m_maxSizeBytes = config.getInt("logging.max_size_mb", 10) * 1024 * 1024;
    m_rotateCount = config.getInt("logging.rotate_count", 5);
    m_consoleOutput = config.getBool("logging.console_output", true);
    
    std::string levelStr = config.getString("logging.level", "INFO");
    if (levelStr == "DEBUG") m_minLevel = LogLevel::DEBUG;
    else if (levelStr == "WARNING") m_minLevel = LogLevel::WARNING;
    else if (levelStr == "ERROR") m_minLevel = LogLevel::ERROR;
    else if (levelStr == "CRITICAL") m_minLevel = LogLevel::CRITICAL;
    else m_minLevel = LogLevel::INFO;
    
    // Create log directory if needed
    size_t pos = m_logPath.rfind('/');
    if (pos != std::string::npos) {
        std::string dir = m_logPath.substr(0, pos);
        mkdir(dir.c_str(), 0755);
    }
    
    m_logFile.open(m_logPath, std::ios::app);
}

Logger::~Logger() {
    flush();
    if (m_logFile.is_open()) {
        m_logFile.close();
    }
}

void Logger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(LogLevel::WARNING, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}

void Logger::critical(const std::string& message) {
    log(LogLevel::CRITICAL, message);
}

void Logger::setLogLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_minLevel = level;
}

void Logger::flush() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_logFile.is_open()) {
        m_logFile.flush();
    }
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < m_minLevel) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::string timestamp = getTimestamp();
    std::string levelStr = levelToString(level);
    std::string logLine = "[" + timestamp + "] [" + levelStr + "] " + message + "\n";
    
    // Write to file
    if (m_logFile.is_open()) {
        m_logFile << logLine;
        checkRotation();
    }
    
    // Write to console if enabled
    if (m_consoleOutput) {
        std::ostream& os = (level >= LogLevel::WARNING) ? std::cerr : std::cout;
        os << logLine << std::flush;
    }
}

std::string Logger::levelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

std::string Logger::getTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

void Logger::checkRotation() {
    if (!m_logFile.is_open()) {
        return;
    }
    
    m_logFile.seekp(0, std::ios::end);
    std::streampos size = m_logFile.tellp();
    
    if (size > static_cast<std::streampos>(m_maxSizeBytes)) {
        rotate();
    }
}

void Logger::rotate() {
    m_logFile.close();
    
    // Rotate existing logs
    for (int i = m_rotateCount - 1; i > 0; --i) {
        std::string oldPath = m_logPath + "." + std::to_string(i);
        std::string newPath = m_logPath + "." + std::to_string(i + 1);
        rename(oldPath.c_str(), newPath.c_str());
    }
    
    // Rename current log
    rename(m_logPath.c_str(), (m_logPath + ".1").c_str());
    
    // Open new log file
    m_logFile.open(m_logPath, std::ios::out | std::ios::trunc);
}

} // namespace biofeedback
