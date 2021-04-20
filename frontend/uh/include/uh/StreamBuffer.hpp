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
    StreamBuffer& writeF64(double value);

    uint8_t readU8();
    uint16_t readU16();
    uint32_t readU32();
    uint64_t readU64();
    float readF32();
    double readF64();

    int readBytesLeft() const;
    int writeBytesLeft() const;
    void* get() { return static_cast<void*>(buffer_.data()); }
    const void* get() const { return static_cast<const void*>(buffer_.data()); }
    int size() const { return buffer_.size(); }

private:
    std::string buffer_;
    void* writePtr_;
    void* readPtr_;
};

}
