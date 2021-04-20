#pragma once

#include "uh/config.hpp"
#include <string>
#include <cstdint>

namespace uh {

class UH_PUBLIC_API StreamBuffer
{
public:
    StreamBuffer(int bytes);
    StreamBuffer(std::string&& data);

    StreamBuffer& writeU8(uint8_t value);
    StreamBuffer& writeU16(uint16_t value);
    StreamBuffer& writeU32(uint32_t value);
    StreamBuffer& writeU64(uint64_t value);
    StreamBuffer& writeF32(float value);

    uint8_t readU8();
    uint16_t readU16();
    uint32_t readU32();
    uint64_t readU64();
    float readF32();
    double readF64();

    void* get() { return static_cast<void*>(buffer_.data()); }
    int size() { return buffer_.length(); }

private:
    std::string buffer_;
    unsigned char* writePtr_;
    unsigned char* readPtr_;
};

}
