#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/watchdog.h"
#include "sensor/SerialProtocolConfig.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace {
uart_inst_t* const kControlUart = uart0;
constexpr uint kUartTxPin = sensor::protocol::PicoUartTxPin;
constexpr uint kUartRxPin = sensor::protocol::PicoUartRxPin;
constexpr uint32_t kBaudRate = sensor::protocol::DefaultControlBaudRate;
constexpr uint32_t kWatchdogTimeoutMs = 2000;
constexpr std::size_t kCommandCapacity = sensor::protocol::MaxControlCommandBytes + 1;

// Fail-closed firmware boundary. No effector GPIO or PWM peripheral is enabled.
void writeReply(const char* message) {
    uart_puts(kControlUart, message);
    uart_puts(kControlUart, "\r\n");
}

void processCommand(const char* command) {
    if (std::strcmp(command, sensor::protocol::ControlPing) == 0) {
        writeReply("OK,PONG,FW=0.2.0,PROTO=1");
    } else if (std::strcmp(command, sensor::protocol::ControlStatus) == 0) {
        writeReply("OK,STATUS,SAFE=1,ACTUATORS=LOCKED,PROTO=1");
    } else if (std::strcmp(command, sensor::protocol::ControlSafe) == 0) {
        writeReply("OK,SAFE,ACTUATORS=LOCKED");
    } else if (std::strncmp(command, "ACTUATOR,", 9) == 0 ||
               std::strncmp(command, "EFFECTOR,", 9) == 0 ||
               std::strncmp(command, "IR,", 3) == 0 ||
               std::strncmp(command, "ELECTRODE,", 10) == 0 ||
               std::strncmp(command, "ANTENNA,", 8) == 0) {
        writeReply(sensor::protocol::ActuatorLockedReply);
    } else {
        writeReply("ERR,UNKNOWN_COMMAND");
    }
}

void configureControlUart() {
    uart_init(kControlUart, kBaudRate);
    gpio_set_function(kUartTxPin, GPIO_FUNC_UART);
    gpio_set_function(kUartRxPin, GPIO_FUNC_UART);
    uart_set_format(kControlUart, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(kControlUart, true);
}
} // namespace

int main() {
    stdio_init_all();
    configureControlUart();
    watchdog_enable(kWatchdogTimeoutMs, true);
    writeReply("BIOFEEDBACK_PICO,FW=0.2.0,SAFE=1,PROTO=1");

    std::array<char, kCommandCapacity> command{};
    std::size_t length = 0;
    bool discardingOversizedLine = false;

    while (true) {
        watchdog_update();
        while (uart_is_readable(kControlUart)) {
            const char ch = static_cast<char>(uart_getc(kControlUart));
            if (ch == '\\r' || ch == '\\n') {
                if (discardingOversizedLine) {
                    writeReply("ERR,COMMAND_TOO_LONG");
                    discardingOversizedLine = false;
                    length = 0;
                    command.fill('\\0');
                } else if (length > 0) {
                    command[length] = '\\0';
                    processCommand(command.data());
                    command.fill('\\0');
                    length = 0;
                }
            } else if (ch >= 0x20 && ch <= 0x7e) {
                if (discardingOversizedLine) continue;
                if (length + 1 < command.size()) {
                    command[length++] = ch;
                } else {
                    command.fill('\\0');
                    length = 0;
                    discardingOversizedLine = true;
                }
            } else {
                command.fill('\\0');
                length = 0;
                discardingOversizedLine = true;
            }
        }
        tight_loop_contents();
    }
}
