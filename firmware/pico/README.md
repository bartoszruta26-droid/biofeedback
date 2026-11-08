# Biofeedback — Raspberry Pi Pico firmware foundation

This is a fail-closed firmware foundation, not a release-ready actuator controller.

## Implemented
- UART0 control channel: GP0 TX / GP1 RX, 115200 baud, 8N1.
- Hardware watchdog: 2 seconds, serviced by the main control loop.
- Bounded command parser (63 printable characters).
- PING, STATUS and SAFE commands.
- All ACTUATOR and EFFECTOR commands are rejected.
- No GPIO is configured as an actuator output.

## Build
Set PICO_SDK_PATH to a checked-out Raspberry Pi Pico SDK.

    cmake -S firmware/pico -B build/pico
    cmake --build build/pico --parallel

## Required before enabling any effector
Do not enable outputs by changing firmware flags alone. A reviewed hardware design and implementation must supply:
1. Exact board revision and GPIO pin map, including boot/debug pin conflicts.
2. Driver topology and galvanic/isolation boundaries.
3. Measured load impedance and operating envelope for every output.
4. Independent hardware current, voltage and temperature limits and interlocks.
5. Hardware emergency stop and de-energized state on reset, watchdog, brownout, cable disconnect and host loss.
6. Output readback or independent fault detection where applicable.
7. Bench verification with non-patient dummy loads and recorded measurements.
8. Command authentication or physically isolated control link, plus replay/sequence protection before remote actuation.

IR PWM, electrodes and antenna are intentionally not implemented or enabled.
No clinical or therapeutic output may be connected based on this scaffold.
