#!/usr/bin/env python3
"""Static fail-closed checks for the Pico command boundary."""
from pathlib import Path
import re

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
    'std::strcmp(command, sensor::protocol::ControlPing)',
    'std::strcmp(command, sensor::protocol::ControlStatus)',
    'std::strcmp(command, sensor::protocol::ControlSafe)',
    '"IR,"',
    '"ELECTRODE,"',
    '"ANTENNA,"',
    'discardingOversizedLine',
]
for token in required:
    assert token in source, f"missing Pico invariant: {token}"

assert "hardware_watchdog" in cmake and "hardware_uart" in cmake
assert "sensor/SerialProtocolConfig.hpp" in source
assert "ControlProtocolVersion = 1" in protocol
assert not re.search(r"gpio_put\\s*\\(|pwm_set_\\w+_enabled\\s*\\(.*true", source), (
    "actuator output operation must not be enabled in this scaffold"
)
assert not re.search(r"gpio_init\\s*\\(", source), (
    "no GPIO output must be configured by the safe scaffold"
)
print("pico_firmware_safety_regression: PASS")
