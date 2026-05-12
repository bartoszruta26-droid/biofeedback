#include "core/Logger.hpp"
#include "core/ConfigManager.hpp"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <sys/stat.h>
#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <system_error>
#include <sstream>
#include <thread>

// ============================================================================
// DEBUG FLAGS AND CONFIGURATION
// ============================================================================

/// Enable verbose debug logging for file operations
constexpr bool DEBUG_FILE_OPS = true;

/// Enable detailed rotation debugging
constexpr bool DEBUG_ROTATION = true;

/// Enable mutex lock tracing (use with caution - can be verbose)
constexpr bool DEBUG_MUTEX = false;

/// Maximum number of rotation retries on failure
constexpr int MAX_ROTATION_RETRIES = 3;

/// Delay between rotation retries in milliseconds
constexpr int ROTATION_RETRY_DELAY_MS = 100;

namespace biofeedback {

// ============================================================================
// LoggerStatistics Implementation
// ============================================================================

std::string LoggerStatistics::getSummary() const {
    std::ostringstream oss;
    oss << "=== Logger Statistics Summary ===" << std::endl;
    oss << "VERBOSE messages:  " << verboseCount.load() << std::endl;
    oss << "DEBUG messages:    " << debugCount.load() << std::endl;
    oss << "INFO messages:     " << infoCount.load() << std::endl;
    oss << "WARNING messages:  " << warningCount.load() << std::endl;
    oss << "ERROR messages:    " << errorCount.load() << std::endl;
    oss << "CRITICAL messages: " << criticalCount.load() << std::endl;
    oss << "--------------------------------" << std::endl;
    oss << "TOTAL messages:    " << totalMessages.load() << std::endl;
    oss << "Rotations:         " << rotationCount.load() << std::endl;
    oss << "Write errors:      " << writeErrors.load() << std::endl;
    oss << "Flush operations:  " << flushOperations.load() << std::endl;
    oss << "================================" << std::endl;
    return oss.str();
}

void LoggerStatistics::reset() {
    verboseCount.store(0);
    debugCount.store(0);
    infoCount.store(0);
    warningCount.store(0);
    errorCount.store(0);
    criticalCount.store(0);
    totalMessages.store(0);
    // Don't reset rotationCount, writeErrors, flushOperations as they are cumulative
}

// ============================================================================
// Logger Implementation
// ============================================================================

/**
 * @class Logger
 * @brief Medical-grade logging system with file rotation
 * 
 * Provides thread-safe logging with multiple output destinations
 * and automatic log rotation for compliance with medical standards.
 * 
 * @section FEATURES Features
 * - Thread-safe logging with mutex protection
 * - Multiple log levels (VERBOSE, DEBUG, INFO, WARNING, ERROR, CRITICAL)
 * - Automatic file rotation based on size
 * - Configurable via ConfigManager
 * - Console output option
 * - Timestamp with millisecond precision
 * - Statistics tracking for compliance
 * - Callback support for real-time log monitoring
 * - Graceful error handling without exceptions
 * 
 * @section THREAD_SAFETY Thread Safety
 * All public methods are thread-safe and can be called from multiple threads.
 * Internal mutex ensures serialized access to log file.
 * Atomic counters ensure thread-safe statistics updates.
 * 
 * @section ERROR_HANDLING Error Handling
 * - File operation errors are logged but don't throw exceptions
 * - Rotation failures are retried with exponential backoff
 * - Invalid configurations fall back to safe defaults
 * - Write errors are tracked in statistics
 */

Logger::Logger(ConfigManager& config)
    : m_config(config)
    , m_minLevel(LogLevel::INFO)
    , m_maxSizeBytes(10 * 1024 * 1024)  // 10MB default
    , m_rotateCount(5)
    , m_consoleOutput(true)
    , m_initialized(false)
{
    try {
        // =========================================================================
        // STEP 1: Load configuration parameters with validation
        // =========================================================================
        
#if DEBUG_FILE_OPS
        std::cout << "[LOGGER] Initializing Logger..." << std::endl;
        std::cout << "[LOGGER] Loading configuration parameters..." << std::endl;
#endif
        
        // Load log file path with validation
        m_logPath = config.getString("logging.file", "./logs/biofeedback.log");
        
        // Validate log path is not empty
        if (m_logPath.empty()) {
            std::cerr << "[LOGGER] WARNING: Empty log path provided. Using default." << std::endl;
            m_logPath = "./logs/biofeedback.log";
        }
        
#if DEBUG_FILE_OPS
        std::cout << "[LOGGER] Log path: " << m_logPath << std::endl;
#endif
        
        // Load and validate max size (with bounds checking)
        const int maxSizeMb = config.getInt("logging.max_size_mb", 10);
        if (maxSizeMb < 1) {
            std::cerr << "[LOGGER] WARNING: Invalid max_size_mb (" << maxSizeMb 
                      << "). Using minimum value of 1MB." << std::endl;
            m_maxSizeBytes = 1 * 1024 * 1024;
        } else if (maxSizeMb > 1000) {
            std::cerr << "[LOGGER] WARNING: max_size_mb (" << maxSizeMb 
                      << ") exceeds maximum. Capping at 1000MB." << std::endl;
            m_maxSizeBytes = 1000 * 1024 * 1024;
        } else {
            m_maxSizeBytes = static_cast<size_t>(maxSizeMb) * 1024 * 1024;
        }
        
#if DEBUG_FILE_OPS
        std::cout << "[LOGGER] Max log size: " << (m_maxSizeBytes / (1024 * 1024)) << "MB" << std::endl;
#endif
        
        // Load and validate rotation count
        m_rotateCount = config.getInt("logging.rotate_count", 5);
        if (m_rotateCount < 1) {
            std::cerr << "[LOGGER] WARNING: Invalid rotate_count (" << m_rotateCount 
                      << "). Using minimum value of 1." << std::endl;
            m_rotateCount = 1;
        } else if (m_rotateCount > 100) {
            std::cerr << "[LOGGER] WARNING: rotate_count (" << m_rotateCount 
                      << ") exceeds maximum. Capping at 100." << std::endl;
            m_rotateCount = 100;
        }
        
#if DEBUG_FILE_OPS
        std::cout << "[LOGGER] Rotation count: " << m_rotateCount << std::endl;
#endif
        
        // Load console output setting
        m_consoleOutput = config.getBool("logging.console_output", true);
        
#if DEBUG_FILE_OPS
        std::cout << "[LOGGER] Console output: " << (m_consoleOutput ? "Enabled" : "Disabled") << std::endl;
#endif
        
        // Load and parse log level with case-insensitive comparison
        std::string levelStr = config.getString("logging.level", "INFO");
        m_minLevel = stringToLevel(levelStr, LogLevel::INFO);
        
#if DEBUG_FILE_OPS
        std::cout << "[LOGGER] Minimum log level: " << levelToString(m_minLevel) << std::endl;
#endif
        
        // =========================================================================
        // STEP 2: Create log directory if it doesn't exist
        // =========================================================================
        
#if DEBUG_FILE_OPS
        std::cout << "[LOGGER] Ensuring log directory exists..." << std::endl;
#endif
        
        size_t pos = m_logPath.rfind('/');
        if (pos != std::string::npos) {
            std::string dir = m_logPath.substr(0, pos);
            
            if (!dir.empty()) {
                // Try to create directory
                const int mkdirResult = mkdir(dir.c_str(), 0755);
                
                if (mkdirResult == 0) {
#if DEBUG_FILE_OPS
                    std::cout << "[LOGGER] Created log directory: " << dir << std::endl;
#endif
                } else {
                    // Check if directory already exists (errno == EEXIST)
                    if (errno == EEXIST) {
#if DEBUG_FILE_OPS
                        std::cout << "[LOGGER] Log directory already exists: " << dir << std::endl;
#endif
                    } else {
                        std::cerr << "[LOGGER] ERROR: Failed to create log directory '" << dir 
                                  << "': " << std::strerror(errno) << std::endl;
                        // Continue anyway - file open might still work
                    }
                }
            }
        }
        
        // =========================================================================
        // STEP 3: Open log file with error handling
        // =========================================================================
        
#if DEBUG_FILE_OPS
        std::cout << "[LOGGER] Opening log file: " << m_logPath << std::endl;
#endif
        
        m_logFile.open(m_logPath, std::ios::app);
        
        if (!m_logFile.is_open()) {
            std::cerr << "[LOGGER] ERROR: Failed to open log file '" << m_logPath << "'" << std::endl;
            std::cerr << "[LOGGER] Logging will be disabled for this session." << std::endl;
            // Don't throw - allow application to continue without file logging
            // Console logging will still work if enabled
            m_initialized = false;
        } else {
#if DEBUG_FILE_OPS
            std::cout << "[LOGGER] Log file opened successfully in append mode" << std::endl;
            std::cout << "[LOGGER] Logger initialization complete" << std::endl;
#endif
            
            m_initialized = true;
            
            // Log initial message
            info("Logger initialized successfully");
        }
        
    } catch (const std::exception& e) {
        std::cerr << "[LOGGER] CRITICAL ERROR during initialization: " << e.what() << std::endl;
        std::cerr << "[LOGGER] Logger may not function correctly." << std::endl;
        m_initialized = false;
        // Re-throw to let caller handle
        throw;
    } catch (...) {
        std::cerr << "[LOGGER] CRITICAL ERROR: Unknown exception during initialization" << std::endl;
        std::cerr << "[LOGGER] Logger may not function correctly." << std::endl;
        m_initialized = false;
        throw;
    }
}

Logger::~Logger() {
    try {
        flush();
        if (m_logFile.is_open()) {
            m_logFile.close();
        }
    } catch (...) {
        // Swallow exceptions in destructor - nothing we can do
        std::cerr << "[LOGGER] WARNING: Exception in destructor" << std::endl;
    }
}

Logger::Logger(Logger&& other) noexcept
    : m_config(other.m_config)
    , m_minLevel(other.m_minLevel)
    , m_logPath(std::move(other.m_logPath))
    , m_maxSizeBytes(other.m_maxSizeBytes)
    , m_rotateCount(other.m_rotateCount)
    , m_consoleOutput(other.m_consoleOutput)
    , m_initialized(other.m_initialized)
    , m_statistics(std::move(other.m_statistics))
    , m_callbacks(std::move(other.m_callbacks))
{
    std::lock_guard<std::mutex> lock(other.m_mutex);
    m_logFile = std::move(other.m_logFile);
    other.m_initialized = false;
}

Logger& Logger::operator=(Logger&& other) noexcept {
    if (this != &other) {
        std::lock_guard<std::mutex> lock1(m_mutex);
        std::lock_guard<std::mutex> lock2(other.m_mutex);
        
        if (m_logFile.is_open()) {
            m_logFile.close();
        }
        
        m_config = other.m_config;
        m_logFile = std::move(other.m_logFile);
        m_minLevel = other.m_minLevel;
        m_logPath = std::move(other.m_logPath);
        m_maxSizeBytes = other.m_maxSizeBytes;
        m_rotateCount = other.m_rotateCount;
        m_consoleOutput = other.m_consoleOutput;
        m_initialized = other.m_initialized;
        m_statistics = std::move(other.m_statistics);
        m_callbacks = std::move(other.m_callbacks);
        
        other.m_initialized = false;
    }
    return *this;
}

void Logger::verbose(const std::string& message) {
    log(LogLevel::VERBOSE, message);
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

LogLevel Logger::getLogLevel() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_minLevel;
}

void Logger::setConsoleOutput(bool enabled) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_consoleOutput = enabled;
}

bool Logger::isConsoleOutputEnabled() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_consoleOutput;
}

void Logger::flush() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_logFile.is_open()) {
        try {
            m_logFile.flush();
            m_statistics.flushOperations.fetch_add(1);
        } catch (...) {
            m_statistics.writeErrors.fetch_add(1);
            std::cerr << "[LOGGER] WARNING: Failed to flush log file" << std::endl;
        }
    }
}

LoggerStatistics Logger::getStatistics() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_statistics;
}

std::string Logger::getStatisticsSummary() const {
    return getStatistics().getSummary();
}

void Logger::resetStatistics() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_statistics.reset();
}

void Logger::registerCallback(const LogCallback& callback, const std::string& id) {
    if (id.empty()) {
        std::cerr << "[LOGGER] WARNING: Cannot register callback with empty ID" << std::endl;
        return;
    }
    
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_callbacks[id] = callback;
}

bool Logger::unregisterCallback(const std::string& id) {
    if (id.empty()) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    return m_callbacks.erase(id) > 0;
}

bool Logger::isInitialized() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_initialized && m_logFile.is_open();
}

std::string Logger::getLogPath() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_logPath;
}

void Logger::log(LogLevel level, const std::string& message) {
    // Quick check without lock for performance
    if (level < m_minLevel) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Double-check after acquiring lock
    if (level < m_minLevel) {
        return;
    }
    
    try {
        std::string timestamp = getTimestamp();
        std::string levelStr = levelToString(level);
        std::string logLine = "[" + timestamp + "] [" + levelStr + "] " + message + "\n";
        
        // Update statistics based on level
        switch (level) {
            case LogLevel::VERBOSE:
                m_statistics.verboseCount.fetch_add(1);
                break;
            case LogLevel::DEBUG:
                m_statistics.debugCount.fetch_add(1);
                break;
            case LogLevel::INFO:
                m_statistics.infoCount.fetch_add(1);
                break;
            case LogLevel::WARNING:
                m_statistics.warningCount.fetch_add(1);
                break;
            case LogLevel::ERROR:
                m_statistics.errorCount.fetch_add(1);
                break;
            case LogLevel::CRITICAL:
                m_statistics.criticalCount.fetch_add(1);
                break;
        }
        m_statistics.totalMessages.fetch_add(1);
        
        // Write to file
        if (m_logFile.is_open()) {
            try {
                m_logFile << logLine;
                checkRotation();
            } catch (...) {
                m_statistics.writeErrors.fetch_add(1);
                std::cerr << "[LOGGER] WARNING: Failed to write to log file" << std::endl;
            }
        }
        
        // Write to console if enabled
        if (m_consoleOutput) {
            std::ostream& os = (level >= LogLevel::WARNING) ? std::cerr : std::cout;
            os << logLine << std::flush;
        }
        
        // Invoke callbacks (with separate lock to avoid deadlock)
        invokeCallbacks(level, message, timestamp);
        
    } catch (const std::exception& e) {
        m_statistics.writeErrors.fetch_add(1);
        std::cerr << "[LOGGER] ERROR in log(): " << e.what() << std::endl;
    } catch (...) {
        m_statistics.writeErrors.fetch_add(1);
        std::cerr << "[LOGGER] ERROR in log(): Unknown exception" << std::endl;
    }
}

std::string Logger::levelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::VERBOSE:   return "VERBOSE";
        case LogLevel::DEBUG:     return "DEBUG";
        case LogLevel::INFO:      return "INFO";
        case LogLevel::WARNING:   return "WARNING";
        case LogLevel::ERROR:     return "ERROR";
        case LogLevel::CRITICAL:  return "CRITICAL";
        default:                  return "UNKNOWN";
    }
}

LogLevel Logger::stringToLevel(const std::string& levelStr, LogLevel defaultLevel) const {
    if (levelStr.empty()) {
        return defaultLevel;
    }
    
    // Convert to uppercase for case-insensitive comparison
    std::string upper = levelStr;
    for (auto& c : upper) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    
    if (upper == "VERBOSE") {
        return LogLevel::VERBOSE;
    } else if (upper == "DEBUG") {
        return LogLevel::DEBUG;
    } else if (upper == "INFO") {
        return LogLevel::INFO;
    } else if (upper == "WARNING" || upper == "WARN") {
        return LogLevel::WARNING;
    } else if (upper == "ERROR" || upper == "ERR") {
        return LogLevel::ERROR;
    } else if (upper == "CRITICAL" || upper == "CRIT" || upper == "FATAL") {
        return LogLevel::CRITICAL;
    }
    
    return defaultLevel;
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
    
    try {
        m_logFile.seekp(0, std::ios::end);
        std::streampos size = m_logFile.tellp();
        
        if (size > static_cast<std::streampos>(m_maxSizeBytes)) {
#if DEBUG_ROTATION
            std::cout << "[LOGGER] Log file size (" << size << " bytes) exceeds limit (" 
                      << m_maxSizeBytes << " bytes). Initiating rotation..." << std::endl;
#endif
            rotate();
        }
    } catch (const std::exception& e) {
        m_statistics.writeErrors.fetch_add(1);
        std::cerr << "[LOGGER] ERROR in checkRotation(): " << e.what() << std::endl;
    } catch (...) {
        m_statistics.writeErrors.fetch_add(1);
        std::cerr << "[LOGGER] ERROR in checkRotation(): Unknown exception" << std::endl;
    }
}

void Logger::rotate() {
    if (!m_logFile.is_open()) {
        return;
    }
    
    m_logFile.close();
    
    bool rotationSuccess = false;
    int retryCount = 0;
    
    while (!rotationSuccess && retryCount < MAX_ROTATION_RETRIES) {
        try {
#if DEBUG_ROTATION
            std::cout << "[LOGGER] Attempting log rotation (attempt " << (retryCount + 1) 
                      << " of " << MAX_ROTATION_RETRIES << ")" << std::endl;
#endif
            
            // Rotate existing logs
            for (int i = m_rotateCount - 1; i > 0; --i) {
                std::string oldPath = m_logPath + "." + std::to_string(i);
                std::string newPath = m_logPath + "." + std::to_string(i + 1);
                
                // Check if old file exists before renaming
                struct stat buffer;
                if (stat(oldPath.c_str(), &buffer) == 0) {
                    const int renameResult = rename(oldPath.c_str(), newPath.c_str());
                    if (renameResult != 0) {
                        std::cerr << "[LOGGER] WARNING: Failed to rename '" << oldPath 
                                  << "' to '" << newPath << "': " << std::strerror(errno) << std::endl;
                    }
                }
            }
            
            // Rename current log
            const int renameResult = rename(m_logPath.c_str(), (m_logPath + ".1").c_str());
            if (renameResult != 0) {
                std::cerr << "[LOGGER] WARNING: Failed to rename current log file: " 
                          << std::strerror(errno) << std::endl;
            }
            
            // Open new log file
            m_logFile.open(m_logPath, std::ios::out | std::ios::trunc);
            
            if (m_logFile.is_open()) {
#if DEBUG_ROTATION
                std::cout << "[LOGGER] Log rotation completed successfully" << std::endl;
#endif
                rotationSuccess = true;
                m_statistics.rotationCount.fetch_add(1);
                
                // Log rotation event
                std::string timestamp = getTimestamp();
                std::string logLine = "[" + timestamp + "] [INFO] Log file rotated\n";
                m_logFile << logLine;
                m_statistics.infoCount.fetch_add(1);
                m_statistics.totalMessages.fetch_add(1);
            } else {
                std::cerr << "[LOGGER] ERROR: Failed to open new log file after rotation" << std::endl;
            }
            
        } catch (const std::exception& e) {
            m_statistics.writeErrors.fetch_add(1);
            std::cerr << "[LOGGER] ERROR during rotation: " << e.what() << std::endl;
        } catch (...) {
            m_statistics.writeErrors.fetch_add(1);
            std::cerr << "[LOGGER] ERROR during rotation: Unknown exception" << std::endl;
        }
        
        if (!rotationSuccess && ++retryCount < MAX_ROTATION_RETRIES) {
#if DEBUG_ROTATION
            std::cout << "[LOGGER] Rotation failed. Retrying in " << ROTATION_RETRY_DELAY_MS 
                      << "ms..." << std::endl;
#endif
            std::this_thread::sleep_for(std::chrono::milliseconds(ROTATION_RETRY_DELAY_MS));
        }
    }
    
    if (!rotationSuccess) {
        std::cerr << "[LOGGER] ERROR: Log rotation failed after " << MAX_ROTATION_RETRIES 
                  << " attempts. Continuing with original file." << std::endl;
        // Try to reopen the original file
        m_logFile.open(m_logPath, std::ios::app);
    }
}

void Logger::invokeCallbacks(LogLevel level, const std::string& message, const std::string& timestamp) {
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    
    if (m_callbacks.empty()) {
        return;
    }
    
    for (const auto& [id, callback] : m_callbacks) {
        try {
            callback(level, message, timestamp);
        } catch (const std::exception& e) {
            std::cerr << "[LOGGER] WARNING: Callback '" << id << "' threw exception: " 
                      << e.what() << std::endl;
            // Continue with other callbacks
        } catch (...) {
            std::cerr << "[LOGGER] WARNING: Callback '" << id << "' threw unknown exception" 
                      << std::endl;
            // Continue with other callbacks
        }
    }
}

} // namespace biofeedback
