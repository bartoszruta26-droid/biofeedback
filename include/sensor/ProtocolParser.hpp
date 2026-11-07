#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace sensor {

struct ProtocolFrame {
    uint32_t timestampMs{0};
    int32_t rawValue{0};
    uint8_t crc{0};
};

// Incremental parser for the existing v1 wire format:
// timestamp[4] LE, raw value[4] LE, CRC-8[1], trailer 0xAA 0x55.
// CRC-8 uses polynomial 0x07 and initial value 0xFF.
class ProtocolParser {
public:
    void feed(const uint8_t* data, std::size_t length);
    std::optional<ProtocolFrame> takeFrame();
    void reset() noexcept;

    std::size_t crcErrorCount() const noexcept { return m_crcErrors; }
    std::size_t discardedByteCount() const noexcept { return m_discardedBytes; }

    static uint8_t calculateCrc8(const uint8_t* data, std::size_t length) noexcept;

private:
    std::vector<uint8_t> m_buffer;
    std::vector<ProtocolFrame> m_frames;
    std::size_t m_crcErrors{0};
    std::size_t m_discardedBytes{0};
};

} // namespace sensor
