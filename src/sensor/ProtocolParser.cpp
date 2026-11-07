#include "sensor/ProtocolParser.hpp"

#include <algorithm>
#include <limits>

namespace sensor {
namespace {
constexpr std::size_t kPayloadSize = 9;
constexpr std::size_t kFrameSize = 11;
constexpr std::size_t kMaximumBufferedBytes = 4096;

uint32_t readU32Le(const uint8_t* p) noexcept {
    return static_cast<uint32_t>(p[0]) |
           (static_cast<uint32_t>(p[1]) << 8) |
           (static_cast<uint32_t>(p[2]) << 16) |
           (static_cast<uint32_t>(p[3]) << 24);
}
}

uint8_t ProtocolParser::calculateCrc8(const uint8_t* data, std::size_t length) noexcept {
    if (data == nullptr && length != 0) return 0xFF;
    uint8_t crc = 0xFF;
    for (std::size_t i = 0; i < length; ++i) {
        crc ^= data[i];
        for (unsigned bit = 0; bit < 8; ++bit) {
            crc = (crc & 0x80) ? static_cast<uint8_t>((crc << 1) ^ 0x07)
                              : static_cast<uint8_t>(crc << 1);
        }
    }
    return crc;
}

void ProtocolParser::feed(const uint8_t* data, std::size_t length) {
    if (data == nullptr || length == 0) return;
    if (length > kMaximumBufferedBytes) {
        data += length - kMaximumBufferedBytes;
        m_discardedBytes += length - kMaximumBufferedBytes;
        length = kMaximumBufferedBytes;
    }
    m_buffer.insert(m_buffer.end(), data, data + length);
    if (m_buffer.size() > kMaximumBufferedBytes) {
        const auto excess = m_buffer.size() - kMaximumBufferedBytes;
        m_buffer.erase(m_buffer.begin(), m_buffer.begin() + static_cast<std::ptrdiff_t>(excess));
        m_discardedBytes += excess;
    }

    while (m_buffer.size() >= kFrameSize) {
        auto marker = m_buffer.end();
        for (auto it = m_buffer.begin() + 9; it + 1 < m_buffer.end(); ++it) {
            if (*it == 0xAA && *(it + 1) == 0x55) {
                marker = it;
                break;
            }
        }
        if (marker == m_buffer.end()) {
            // Keep the final ten bytes: a frame may be split across feed calls.
            const auto discard = m_buffer.size() - (kFrameSize - 1);
            m_buffer.erase(m_buffer.begin(), m_buffer.begin() + static_cast<std::ptrdiff_t>(discard));
            m_discardedBytes += discard;
            break;
        }

        const auto markerIndex = static_cast<std::size_t>(marker - m_buffer.begin());
        if (markerIndex < 9) {
            m_buffer.erase(m_buffer.begin());
            ++m_discardedBytes;
            continue;
        }
        const auto packetStart = markerIndex - 9;
        const auto* packet = m_buffer.data() + packetStart;
        if (calculateCrc8(packet, 8) != packet[8]) {
            m_buffer.erase(m_buffer.begin(), m_buffer.begin() + static_cast<std::ptrdiff_t>(packetStart + 1));
            ++m_crcErrors;
            m_discardedBytes += packetStart + 1;
            continue;
        }

        // Bytes preceding a CRC-valid frame are synchronization noise.
        m_discardedBytes += packetStart;
        ProtocolFrame frame;
        frame.timestampMs = readU32Le(packet);
        const uint32_t rawBits = readU32Le(packet + 4);
        if (rawBits <= static_cast<uint32_t>(std::numeric_limits<int32_t>::max())) {
            frame.rawValue = static_cast<int32_t>(rawBits);
        } else {
            frame.rawValue = static_cast<int32_t>(static_cast<int64_t>(rawBits) - 0x100000000LL);
        }
        frame.crc = packet[8];
        m_frames.push_back(frame);
        m_buffer.erase(m_buffer.begin(), m_buffer.begin() + static_cast<std::ptrdiff_t>(markerIndex + 2));
    }
}

std::optional<ProtocolFrame> ProtocolParser::takeFrame() {
    if (m_frames.empty()) return std::nullopt;
    ProtocolFrame frame = m_frames.front();
    m_frames.erase(m_frames.begin());
    return frame;
}

void ProtocolParser::reset() noexcept {
    m_buffer.clear();
    m_frames.clear();
    m_crcErrors = 0;
    m_discardedBytes = 0;
}

} // namespace sensor
