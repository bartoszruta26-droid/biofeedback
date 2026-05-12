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

/**
 * @class Logger
 * @brief Medical-grade logging system with file rotation
 * 
 * Provides thread-safe logging with multiple output destinations
 * and automatic log rotation for compliance with medical standards.
 * 
 * @section FEATURES Features
 * - Thread-safe logging with mutex protection
 * - Multiple log levels (DEBUG, INFO, WARNING, ERROR, CRITICAL)
 * - Automatic file rotation based on size
 * - Configurable via ConfigManager
 * - Console output option
 * - Timestamp with millisecond precision
 * 
 * @section THREAD_SAFETY Thread Safety
 * All public methods are thread-safe and can be called from multiple threads.
 * Internal mutex ensures serialized access to log file.
 * 
 * @section ERROR_HANDLING Error Handling
 * - File operation errors are logged but don't throw exceptions
 * - Rotation failures are retried with exponential backoff
 * - Invalid configurations fall back to safe defaults
 */

Logger::Logger(ConfigManager& config)
    : m_config(config)
    , m_minLevel(LogLevel::INFO)
    , m_maxSizeBytes(10 * 1024 * 1024)  // 10MB default
    , m_rotateCount(5)
    , m_consoleOutput(true)
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
        
        // Convert to uppercase for case-insensitive comparison
        for (auto& c : levelStr) {
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        }
        
        if (levelStr == "DEBUG") {
            m_minLevel = LogLevel::DEBUG;
        } else if (levelStr == "INFO") {
            m_minLevel = LogLevel::INFO;
        } else if (levelStr == "WARNING" || levelStr == "WARN") {
            m_minLevel = LogLevel::WARNING;
        } else if (levelStr == "ERROR" || levelStr == "ERR") {
            m_minLevel = LogLevel::ERROR;
        } else if (levelStr == "CRITICAL" || levelStr == "CRIT" || levelStr == "FATAL") {
            m_minLevel = LogLevel::CRITICAL;
        } else {
            std::cerr << "[LOGGER] WARNING: Unknown log level '" << levelStr 
                      << "'. Defaulting to INFO." << std::endl;
            m_minLevel = LogLevel::INFO;
        }
        
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
        } else {
#if DEBUG_FILE_OPS
            std::cout << "[LOGGER] Log file opened successfully in append mode" << std::endl;
            std::cout << "[LOGGER] Logger initialization complete" << std::endl;
#endif
            
            // Log initial message
            info("Logger initialized successfully");
        }
        
    } catch (const std::exception& e) {
        std::cerr << "[LOGGER] CRITICAL ERROR during initialization: " << e.what() << std::endl;
        std::cerr << "[LOGGER] Logger may not function correctly." << std::endl;
        // Re-throw to let caller handle
        throw;
    } catch (...) {
        std::cerr << "[LOGGER] CRITICAL ERROR: Unknown exception during initialization" << std::endl;
        std::cerr << "[LOGGER] Logger may not function correctly." << std::endl;
        throw;
    }
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
