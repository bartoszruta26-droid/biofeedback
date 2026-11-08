# Pico firmware v0.2.0 and effector safety status

## Status as of 2026-11-08

The repository contains a Raspberry Pi Pico SDK build target and a fail-closed UART command loop. This establishes a reproducible firmware boundary.

| Function | Firmware command | Driver | Independent hardware safety | Status |
|---|---|---|---|---|
| IR PWM | Locked | Absent | Not verified | NOT IMPLEMENTED |
| Electrodes | Locked | Absent | Not verified | NOT IMPLEMENTED |
| Antenna | Locked | Absent | Not verified | NOT IMPLEMENTED |
| Pico watchdog | N/A | Implemented | MCU watchdog only | PARTIAL |
| Pico control UART | PING/STATUS/SAFE | Implemented | Wiring not verified | SOFTWARE ONLY |

A UI control, API route, serial command or successful firmware build is not evidence of safe electrical actuation.

## Required hardware evidence
Before implementing an output driver, record the exact board revision, schematic, pin assignment, driver component numbers, supply rails, load impedance across operating conditions, maximum current and voltage, thermal limits, isolation, fuse/current limiting, emergency stop topology and measured fault response. Values must come from the actual design and bench measurements; firmware must not invent them.

## Host/Pico protocol boundary
- UART: 115200 baud, 8 data bits, no parity, 1 stop bit.
- Commands are bounded printable ASCII lines terminated by CR or LF.
- PING returns OK,PONG,FW=0.2.0,PROTO=1.
- STATUS returns OK,STATUS,SAFE=1,ACTUATORS=LOCKED,PROTO=1.
- SAFE returns OK,SAFE,ACTUATORS=LOCKED.
- ACTUATOR, EFFECTOR, IR, ELECTRODE and ANTENNA requests return ERR,ACTUATORS_LOCKED.
- Pico UART0 uses GP0 TX and GP1 RX; the HX711 Nano sketch uses DT=2 and SCK=3.
- The host, configuration and Pico firmware share the versioned protocol constants in SerialProtocolConfig.hpp.
- Unknown or malformed commands are rejected.

This control protocol is separate from the existing HX711 binary measurement frame (little-endian timestamp, signed raw value, CRC-8, trailer 0xAA 0x55). The two protocols must not be conflated. The control protocol is versioned independently from the HX711 measurement frame; no effector driver is exposed by protocol version 1.
