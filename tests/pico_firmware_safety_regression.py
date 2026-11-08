#!/usr/bin/env python3
"""Static fail-closed checks for the Pico command boundary."""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
source = (root / "firmware/pico/main.cpp").read_text(encoding="utf-8")
cmake = (root / "firmware/pico/CMakeLists.txt").read_text(encoding="utf-8")
protocol = (root / "include/sensor/SerialProtocolConfig.hpp").read_text(encoding="utf-8")

required = [
    "sensor::protocol::DefaultControlBaudRate",
    "sensor::protocol::MaxControlCommandBytes + 1",
    "watchdog_enable(kWatchdogTimeoutMs, true);",
    "watchdog_update();",
    "sensor::protocol::ActuatorLockedReply",
    "std::strcmp(command, sensor::protocol::ControlPing)",
    "std::strcmp(command, sensor::protocol::ControlStatus)",
    "std::strcmp(command, sensor::protocol::ControlSafe)",
    '"IR,"',
    '"ELECTRODE,"',
    '"ANTENNA,"',
    "discardingOversizedLine",
]
for token in required:
    assert token in source, f"missing Pico invariant: {token}"

assert "hardware_watchdog" in cmake and "hardware_uart" in cmake
assert "sensor/SerialProtocolConfig.hpp" in source
assert "target_include_directories(biofeedback_pico" in cmake
assert "ControlProtocolVersion = 1" in protocol

# Assert there are no output-driving APIs or effector pin initializers.
for forbidden in ("gpio_put(", "gpio_init(", "pwm_set_enabled(", "pwm_set_gpio_level("):
    assert forbidden not in source, f"unexpected output API: {forbidden}"
print("pico_firmware_safety_regression: PASS")
