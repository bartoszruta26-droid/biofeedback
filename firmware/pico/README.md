# Biofeedback — Raspberry Pi Pico firmware

Firmware provides the Raspberry Pi Pico serial communication foundation for the Biofeedback measurement system.

## Implemented

- UART0 communication on GP0 TX / GP1 RX, 115200 baud, 8N1.
- Hardware watchdog with a 2-second timeout.
- Bounded printable-ASCII command parser.
- PING, STATUS and SAFE diagnostic commands.
- Explicit rejection of unknown commands.
- No therapeutic-output control interface.

## Build

Set `PICO_SDK_PATH` to a checked-out Raspberry Pi Pico SDK.

    cmake -S firmware/pico -B build/pico
    cmake --build build/pico --parallel

## Measurement integration

The Pico firmware target is currently a communication foundation. Sensor acquisition, calibration and measurement-frame production must be implemented against the actual selected sensor hardware and the shared measurement protocol. Do not treat the diagnostic command channel as a measurement stream or as evidence that a sensor is calibrated.
