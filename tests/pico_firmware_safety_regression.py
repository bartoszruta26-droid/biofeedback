#!/usr/bin/env python3
"""Static fail-closed checks for the initial Pico firmware boundary."""
from pathlib import Path
import re

source = Path("firmware/pico/main.cpp").read_text(encoding="utf-8")
cmake = Path("firmware/pico/CMakeLists.txt").read_text(encoding="utf-8")

required = [
    'constexpr uint32_t kBaudRate = 115200;',
    'constexpr uint32_t kWatchdogTimeoutMs = 2000;',
    'constexpr std::size_t kCommandCapacity = 64;',
    'watchdog_enable(kWatchdogTimeoutMs, true);',
    'watchdog_update();',
    'writeReply("ERR,ACTUATORS_LOCKED");',
]
for token in required:
    assert token in source, f"missing fail-closed invariant: {token}"

assert 'uart_init(kControlUart, kBaudRate);' in source
assert 'uart_set_format(kControlUart, 8, 1, UART_PARITY_NONE);' in source
assert 'gpio_set_function(kUartTxPin, GPIO_FUNC_UART);' in source
assert 'gpio_set_function(kUartRxPin, GPIO_FUNC_UART);' in source
assert not re.search(r'gpio_put\s*\(|pwm_set_.*enabled\s*\(.*true', source), \
    "actuator output operation must not be enabled in this scaffold"
assert "hardware_watchdog" in cmake and "hardware_uart" in cmake
assert "PICO_SDK_PATH" in cmake
print("pico_firmware_safety_regression: PASS")
