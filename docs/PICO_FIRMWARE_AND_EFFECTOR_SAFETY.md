# Raspberry Pi Pico firmware — measurement communication status

## Current implementation

The repository contains a Raspberry Pi Pico SDK build target and a bounded UART command loop.

| Function | Current state |
|---|---|
| UART0 communication | Implemented: GP0 TX / GP1 RX, 115200 baud, 8N1 |
| Hardware watchdog | Implemented: 2-second timeout |
| Bounded command parser | Implemented |
| PING / STATUS / SAFE diagnostics | Implemented |
| Unknown command handling | Rejected |
| Sensor acquisition and measurement frames | Not yet implemented in this Pico firmware target |
| Calibration against physical sensors | Not verified |

## Host/Pico diagnostic protocol

- UART: 115200 baud, 8 data bits, no parity, 1 stop bit.
- Commands are bounded printable ASCII lines terminated by CR or LF.
- PING returns `OK,PONG,FW=0.2.0,PROTO=1`.
- STATUS returns `OK,STATUS,READY=1,PROTO=1`.
- SAFE returns `OK,SAFE`.
- Unknown or malformed commands are rejected.

This diagnostic channel is separate from the existing HX711 binary measurement frame (little-endian timestamp, signed raw value, CRC-8, trailer 0xAA 0x55). The host parser and firmware must not conflate diagnostic commands with measurement frames.

## Next engineering work

1. Confirm the exact sensor models and electrical interfaces connected to the Pico.
2. Implement bounded sensor acquisition with explicit sample validity and fault states.
3. Apply calibrated units and timestamp semantics shared with the host measurement pipeline.
4. Add firmware-side protocol tests and host/Pico integration tests.
5. Verify on physical hardware with reference loads and recorded calibration evidence.

A successful firmware build is not proof of physical sensor accuracy or clinical validity.
