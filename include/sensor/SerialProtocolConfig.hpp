#pragma once
#include <cstddef>
#include <cstdint>

namespace sensor::protocol {
inline constexpr int DefaultBaudRate = 115200;
inline constexpr int DefaultTimeoutMs = 1000;
inline constexpr std::size_t MeasurementPayloadSize = 9;
inline constexpr std::size_t MeasurementFrameSize = 11;
inline constexpr uint8_t Crc8Polynomial = 0x07;
inline constexpr uint8_t Crc8InitialValue = 0xFF;
inline constexpr uint8_t FrameTrailerFirst = 0xAA;
inline constexpr uint8_t FrameTrailerSecond = 0x55;
inline constexpr uint32_t DefaultSampleRateHz = 80;
inline constexpr uint8_t Hx711Gain = 128;

// Versioned ASCII control envelope used by the Pico command processor.
// Measurement frames remain binary and are parsed by ProtocolParser.
inline constexpr int ControlProtocolVersion = 1;
inline constexpr std::size_t MaxControlCommandBytes = 63;
inline constexpr int DefaultControlBaudRate = DefaultBaudRate;
inline constexpr uint8_t PicoUartTxPin = 0;
inline constexpr uint8_t PicoUartRxPin = 1;
inline constexpr uint8_t NanoHx711DataPin = 2;
inline constexpr uint8_t NanoHx711ClockPin = 3;
inline constexpr char ControlPing[] = "PING";
inline constexpr char ControlStatus[] = "STATUS";
inline constexpr char ControlSafe[] = "SAFE";
} // namespace sensor::protocol
