#include "sensor/ProtocolParser.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

using sensor::ProtocolParser;

static std::vector<uint8_t> frame(uint32_t timestamp, int32_t raw) {
    std::vector<uint8_t> bytes(11);
    for (int i=0;i<4;++i) bytes[i]=static_cast<uint8_t>(timestamp>>(8*i));
    const uint32_t bits=static_cast<uint32_t>(raw);
    for (int i=0;i<4;++i) bytes[4+i]=static_cast<uint8_t>(bits>>(8*i));
    bytes[8]=ProtocolParser::calculateCrc8(bytes.data(),8);
    bytes[9]=0xAA; bytes[10]=0x55;
    return bytes;
}

int main() {
    ProtocolParser parser;
    auto good=frame(0x12345678u,-123456);
    parser.feed(good.data(),4);
    assert(!parser.takeFrame());
    parser.feed(good.data()+4,good.size()-4);
    auto parsed=parser.takeFrame();
    assert(parsed && parsed->timestampMs==0x12345678u && parsed->rawValue==-123456);

    // Noise before a valid frame must be discarded while preserving synchronization.
    const uint8_t noise[]={0x00,0xFF,0x12,0xAA};
    parser.feed(noise,sizeof(noise));
    auto second=frame(42,987);
    parser.feed(second.data(),second.size());
    parsed=parser.takeFrame();
    assert(parsed && parsed->timestampMs==42 && parsed->rawValue==987);
    assert(parser.discardedByteCount()>=sizeof(noise));

    // A corrupted CRC is rejected; the next intact frame is still recovered.
    auto corrupt=frame(77,111);
    corrupt[5]^=0x40;
    parser.feed(corrupt.data(),corrupt.size());
    parser.feed(second.data(),second.size());
    parsed=parser.takeFrame();
    assert(parsed && parsed->timestampMs==42);
    assert(parser.crcErrorCount()>=1);

    // Multiple frames in one read are delivered in order.
    auto third=frame(43,-1);
    std::vector<uint8_t> batch=second;
    batch.insert(batch.end(),third.begin(),third.end());
    parser.feed(batch.data(),batch.size());
    parsed=parser.takeFrame(); assert(parsed && parsed->timestampMs==42);
    parsed=parser.takeFrame(); assert(parsed && parsed->timestampMs==43 && parsed->rawValue==-1);

    std::cout << "protocol_parser_regression: PASS\n";
}
