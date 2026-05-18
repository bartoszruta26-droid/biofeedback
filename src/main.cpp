/**
 * @file main.cpp
 * @brief Main entry point for Biofeedback Application
 * 
 * Medical-grade biofeedback application for Raspberry Pi 4
 * using Qt5, Arduino Nano, and HX711 strain gauge sensor.
 * 
 * @section DEBUG_INFO Debug Information
 * This file contains extensive error handling, logging, and debugging capabilities.
 * All exceptions are caught and logged appropriately.
 * 
 * @section ERROR_HANDLING Error Handling Strategy
 * - Signal handlers for graceful shutdown (SIGINT, SIGTERM)
 * - Try-catch blocks for all critical operations
 * - Detailed error messages with context information
 * - Fallback mechanisms for critical failures
 * 
 * @author Biofeedback Team
 * @version 1.0.0
 * @date 2024
 */

#include "core/Application.hpp"
#include <iostream>
#include <csignal>
#include <exception>
#include <stdexcept>
#include <system_error>
#include <unistd.h>
#include <cstring>

// ============================================================================
// GLOBAL DEBUG FLAGS AND CONFIGURATION
// ============================================================================

/// Global flag to enable verbose debug output
constexpr bool DEBUG_VERBOSE = true;

/// Global flag to enable detailed exception tracing
constexpr bool DEBUG_EXCEPTION_TRACE = true;

/// Global flag to enable signal handling debug messages
constexpr bool DEBUG_SIGNALS = true;

// ============================================================================
// NAMESPACE ANONYMOUS - Internal linkage for translation unit scope
// ============================================================================

namespace {
    /// Global pointer to application instance for signal handling
    /// @note This is intentionally a raw pointer - we don't own this object
    biofeedback::Application* g_app = nullptr;
    
    /// Flag to track if shutdown is already in progress
    /// Prevents recursive shutdown attempts
    volatile sig_atomic_t g_shutdown_in_progress = 0;
    
    /**
     * @brief Signal handler for graceful application shutdown
     * 
     * This function handles asynchronous signals (SIGINT, SIGTERM) and ensures
     * the application shuts down cleanly. It uses atomic operations to prevent
     * race conditions during signal handling.
     * 
     * @param signum The signal number received
     * 
     * @debug Prints detailed shutdown information when DEBUG_SIGNALS is enabled
     * @error Handles recursive signal attempts gracefully
     * @warning Uses only async-signal-safe functions where possible
     */
    void signalHandler(int signum) {
        // Check for recursive signal (shutdown already in progress)
        if (g_shutdown_in_progress) {
            const char* msg = "[SIGNAL] WARNING: Recursive signal received during shutdown. Forcing immediate exit...\n";
            write(STDERR_FILENO, msg, strlen(msg));
            _exit(signum);  // Use _exit instead of exit for signal safety
        }
        
        // Set shutdown flag atomically
        g_shutdown_in_progress = 1;
        
#if DEBUG_SIGNALS
        const char* header = "\n[DEBUG] ========================================\n";
        const char* activated = "[DEBUG] SIGNAL HANDLER ACTIVATED\n";
        const char* footer = "[DEBUG] ========================================\n";
        
        write(STDOUT_FILENO, header, strlen(header));
        write(STDOUT_FILENO, activated, strlen(activated));
        
        // Provide human-readable signal name
        switch(signum) {
            case SIGINT: {
                const char* sigint_msg = "[DEBUG] Signal type: SIGINT (Keyboard Interrupt/Ctrl+C)\n";
                write(STDOUT_FILENO, sigint_msg, strlen(sigint_msg));
                break;
            }
            case SIGTERM: {
                const char* sigterm_msg = "[DEBUG] Signal type: SIGTERM (Termination Request)\n";
                write(STDOUT_FILENO, sigterm_msg, strlen(sigterm_msg));
                break;
            }
#ifdef SIGQUIT
            case SIGQUIT: {
                const char* sigquit_msg = "[DEBUG] Signal type: SIGQUIT (Quit Request)\n";
                write(STDOUT_FILENO, sigquit_msg, strlen(sigquit_msg));
                break;
            }
#endif
            default: {
                const char* unknown_msg = "[DEBUG] Signal type: Unknown/Other\n";
                write(STDOUT_FILENO, unknown_msg, strlen(unknown_msg));
                break;
            }
        }
        
        const char* app_state = g_app ? "[DEBUG] Application state: Initialized\n" : "[DEBUG] Application state: Not Initialized\n";
        write(STDOUT_FILENO, app_state, strlen(app_state));
        write(STDOUT_FILENO, footer, strlen(footer));
#endif
        
        try {
            if (g_app) {
                const char* interrupt_msg = "\n[SIGNAL] Interrupt signal received. Initiating graceful shutdown...\n";
                write(STDOUT_FILENO, interrupt_msg, strlen(interrupt_msg));
                
                // Attempt graceful shutdown through application
                g_app->shutdown();
                
                const char* success_msg = "[SIGNAL] Graceful shutdown completed successfully.\n";
                write(STDOUT_FILENO, success_msg, strlen(success_msg));
            } else {
                const char* error_msg = "[SIGNAL] ERROR: Application pointer is null. Cannot perform graceful shutdown.\n";
                write(STDERR_FILENO, error_msg, strlen(error_msg));
            }
        } catch (const std::exception& e) {
            const char* exception_msg = "[SIGNAL] ERROR: Exception during signal handling\n";
            write(STDERR_FILENO, exception_msg, strlen(exception_msg));
        } catch (...) {
            const char* unknown_exception_msg = "[SIGNAL] ERROR: Unknown exception during signal handling\n";
            write(STDERR_FILENO, unknown_exception_msg, strlen(unknown_exception_msg));
        }
        
        // Exit with appropriate code
        exit(signum);
    }
    
    /**
     * @brief Print current application state for debugging purposes
     * 
     * Helper function to dump diagnostic information about the application state.
     * Useful for troubleshooting startup/shutdown issues.
     */
    void printDebugState() {
#if DEBUG_VERBOSE
        std::cout << "[DEBUG] Application State:" << std::endl;
        std::cout << "  - Global app pointer: " << (g_app ? "Valid" : "Null") << std::endl;
        std::cout << "  - Shutdown in progress: " << (g_shutdown_in_progress ? "Yes" : "No") << std::endl;
#endif
    }
} // namespace anonymous

// ============================================================================
// MAIN FUNCTION - Entry Point
// ============================================================================

/**
 * @brief Main entry point for the Biofeedback Application
 * 
 * This function serves as the primary entry point for the application. It handles:
 * - Command line argument processing
 * - Signal handler registration for graceful shutdown
 * - Application instantiation and initialization
 * - Global exception handling with detailed error reporting
 * - Resource cleanup on exit
 * 
 * @param argc Number of command line arguments
 * @param argv Array of command line argument strings
 * @return int Exit code (EXIT_SUCCESS on success, EXIT_FAILURE on error)
 * 
 * @exception std::exception Caught and logged with full details
 * @exception ... Any other exceptions caught and reported
 * 
 * @note The application uses RAII for resource management
 * @note Signal handlers ensure cleanup even on external termination requests
 */
int main(int argc, char* argv[]) {
    // =========================================================================
    // PHASE 1: INITIALIZATION AND SETUP
    // =========================================================================
    
#if DEBUG_VERBOSE
    std::cout << "\n[MAIN] =============================================" << std::endl;
    std::cout << "[MAIN] BIOFEEDBACK APPLICATION STARTUP" << std::endl;
    std::cout << "[MAIN] Version: 1.0.0" << std::endl;
    std::cout << "[MAIN] Build Date: " << __DATE__ << " " << __TIME__ << std::endl;
    std::cout << "[MAIN] Compiler: " 
#if defined(__GNUC__)
              << "GCC " << __GNUC__ << "." << __GNUC_MINOR__
#elif defined(_MSC_VER)
              << "MSVC " << _MSC_VER
#else
              << "Unknown"
#endif
              << std::endl;
    std::cout << "[MAIN] Arguments: " << argc << " parameters" << std::endl;
    
    // Log command line arguments (excluding program name)
    for (int i = 1; i < argc; ++i) {
        std::cout << "[MAIN]   Arg[" << i << "]: " << argv[i] << std::endl;
    }
    std::cout << "[MAIN] =============================================\n" << std::endl;
#endif
    
    try {
        // =====================================================================
        // STEP 1: Register Signal Handlers
        // =====================================================================
        
#if DEBUG_VERBOSE
        std::cout << "[MAIN] Step 1: Registering signal handlers..." << std::endl;
#endif
        
        // Register handler for keyboard interrupt (Ctrl+C)
        if (std::signal(SIGINT, signalHandler) == SIG_ERR) {
            std::cerr << "[MAIN] ERROR: Failed to register SIGINT handler" << std::endl;
            return EXIT_FAILURE;
        }
#if DEBUG_SIGNALS
        std::cout << "[DEBUG] SIGINT handler registered successfully" << std::endl;
#endif
        
        // Register handler for termination request
        if (std::signal(SIGTERM, signalHandler) == SIG_ERR) {
            std::cerr << "[MAIN] ERROR: Failed to register SIGTERM handler" << std::endl;
            return EXIT_FAILURE;
        }
#if DEBUG_SIGNALS
        std::cout << "[DEBUG] SIGTERM handler registered successfully" << std::endl;
#endif

#ifdef SIGQUIT
        // Register handler for quit request (optional)
        if (std::signal(SIGQUIT, signalHandler) == SIG_ERR) {
            std::cerr << "[MAIN] WARNING: Failed to register SIGQUIT handler" << std::endl;
            // Non-fatal, continue execution
        }
#if DEBUG_SIGNALS
        else {
            std::cout << "[DEBUG] SIGQUIT handler registered successfully" << std::endl;
        }
#endif
#endif
        
#if DEBUG_VERBOSE
        std::cout << "[MAIN] Signal handlers registered successfully\n" << std::endl;
#endif
        
        // =====================================================================
        // STEP 2: Create Application Instance
        // =====================================================================
        
#if DEBUG_VERBOSE
        std::cout << "[MAIN] Step 2: Creating application instance..." << std::endl;
        printDebugState();
#endif
        
        // Create application with RAII semantics
        biofeedback::Application app(argc, argv);
        g_app = &app;
        
#if DEBUG_VERBOSE
        std::cout << "[DEBUG] Application instance created at address: " << &app << std::endl;
        std::cout << "[DEBUG] Global pointer updated" << std::endl;
        printDebugState();
#endif
        
        // =====================================================================
        // STEP 3: Display Startup Banner
        // =====================================================================
        
        std::cout << "\n╔══════════════════════════════════════════════════════╗" << std::endl;
        std::cout << "║     BIOFEEDBACK APPLICATION v1.0.0                   ║" << std::endl;
        std::cout << "║     Medical-Grade Biofeedback System                 ║" << std::endl;
        std::cout << "║     © 2024 Biofeedback Team                          ║" << std::endl;
        std::cout << "╚══════════════════════════════════════════════════════╝\n" << std::endl;
        
        std::cout << "Initializing system components..." << std::endl;
        
#if DEBUG_VERBOSE
        std::cout << "[DEBUG] Proceeding to application run loop...\n" << std::endl;
#endif
        
        // =====================================================================
        // STEP 4: Execute Application Main Loop
        // =====================================================================
        
#if DEBUG_VERBOSE
        std::cout << "[MAIN] Step 4: Entering application run loop..." << std::endl;
#endif
        
        const int exitCode = app.run();
        
#if DEBUG_VERBOSE
        std::cout << "[DEBUG] Application run loop exited with code: " << exitCode << std::endl;
#endif
        
        // =====================================================================
        // PHASE 2: CLEANUP AND SHUTDOWN
        // =====================================================================
        
#if DEBUG_VERBOSE
        std::cout << "\n[MAIN] =============================================" << std::endl;
        std::cout << "[MAIN] APPLICATION SHUTDOWN SEQUENCE" << std::endl;
        std::cout << "[MAIN] Exit code from run(): " << exitCode << std::endl;
#endif
        
        // Clear global pointer before destruction
        g_app = nullptr;
        
#if DEBUG_VERBOSE
        printDebugState();
        std::cout << "[MAIN] Global application pointer cleared" << std::endl;
        std::cout << "[MAIN] Application object will be destroyed via RAII" << std::endl;
        std::cout << "[MAIN] =============================================\n" << std::endl;
#endif
        
        return exitCode;
        
    } // End of main try block
    
    // =========================================================================
    // EXCEPTION HANDLING - Standard Exceptions
    // =========================================================================
    
    catch (const std::exception& e) {
        // Handle standard library exceptions with detailed diagnostics
        
        std::cerr << "\n╔══════════════════════════════════════════════════════╗" << std::endl;
        std::cerr << "║           FATAL ERROR - STANDARD EXCEPTION           ║" << std::endl;
        std::cerr << "╚══════════════════════════════════════════════════════╝\n" << std::endl;
        
        std::cerr << "[FATAL] An unhandled standard exception occurred!" << std::endl;
        std::cerr << "[FATAL] Exception type: std::exception" << std::endl;
        std::cerr << "[FATAL] Exception message: " << e.what() << std::endl;
        
#if DEBUG_EXCEPTION_TRACE
        std::cerr << "\n[DEBUG] Exception Details:" << std::endl;
        std::cerr << "  - Demangled type: " << typeid(e).name() << std::endl;
        std::cerr << "  - Stack unwinding: In progress..." << std::endl;
        std::cerr << "  - Application state: " << (g_app ? "Active" : "Destroyed") << std::endl;
#endif
        
        // Additional type-specific information
        if (dynamic_cast<const std::runtime_error*>(&e)) {
            std::cerr << "[DEBUG] Category: Runtime Error (recoverable in some cases)" << std::endl;
        } else if (dynamic_cast<const std::logic_error*>(&e)) {
            std::cerr << "[DEBUG] Category: Logic Error (programming mistake)" << std::endl;
        } else if (dynamic_cast<const std::bad_alloc*>(&e)) {
            std::cerr << "[DEBUG] Category: Memory Allocation Failure (critical)" << std::endl;
        } else if (dynamic_cast<const std::system_error*>(&e)) {
            std::cerr << "[DEBUG] Category: System Error (OS-level issue)" << std::endl;
        }
        
        std::cerr << "\n[FATAL] Application cannot continue. Exiting..." << std::endl;
        
        // Cleanup before exit
        g_app = nullptr;
        
        return EXIT_FAILURE;
    }
    
    // =========================================================================
    // EXCEPTION HANDLING - Unknown Exceptions
    // =========================================================================
    
    catch (...) {
        // Catch-all handler for any non-standard exceptions
        
        std::cerr << "\n╔══════════════════════════════════════════════════════╗" << std::endl;
        std::cerr << "║          FATAL ERROR - UNKNOWN EXCEPTION             ║" << std::endl;
        std::cerr << "╚══════════════════════════════════════════════════════╝\n" << std::endl;
        
        std::cerr << "[FATAL] An unknown/unexpected exception occurred!" << std::endl;
        std::cerr << "[FATAL] Exception type: Unknown (not derived from std::exception)" << std::endl;
        std::cerr << "[FATAL] This may indicate:" << std::endl;
        std::cerr << "  - Hardware failure" << std::endl;
        std::cerr << "  - Memory corruption" << std::endl;
        std::cerr << "  - Third-party library issue" << std::endl;
        std::cerr << "  - Undefined behavior in code" << std::endl;
        
#if DEBUG_EXCEPTION_TRACE
        std::cerr << "\n[DEBUG] Diagnostic Information:" << std::endl;
        std::cerr << "  - Current function: main()" << std::endl;
        std::cerr << "  - Application state: " << (g_app ? "Active" : "Destroyed") << std::endl;
        std::cerr << "  - Shutdown in progress: " << (g_shutdown_in_progress ? "Yes" : "No") << std::endl;
#endif
        
        std::cerr << "\n[FATAL] Application cannot continue. Emergency exit..." << std::endl;
        
        // Emergency cleanup
        g_app = nullptr;
        g_shutdown_in_progress = 1;
        
        return EXIT_FAILURE;
    }
    
    // =========================================================================
    // NORMAL EXIT PATH (should not reach here due to return in try block)
    // =========================================================================
    
    // This code should never execute, but included for completeness
    g_app = nullptr;
    return EXIT_SUCCESS;
}
