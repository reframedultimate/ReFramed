#pragma once

#include "uh/config.hpp"
#include "uh/Vector.hpp"
#include <cstdint>

namespace uh {

template class UH_PUBLIC_API Vector<char>;

class UH_PUBLIC_API StreamBuffer
{
public:
    StreamBuffer(int bytes);
    StreamBuffer(const char* data, int len);

    StreamBuffer& writeU8(uint8_t value);

    StreamBuffer& writeLU16(uint16_t value);
    StreamBuffer& writeLU32(uint32_t value);
    StreamBuffer& writeLU64(uint64_t value);
    StreamBuffer& writeLF32(float value);
    StreamBuffer& writeLF64(double value);

    StreamBuffer& writeBU16(uint16_t value);
    StreamBuffer& writeBU32(uint32_t value);
    StreamBuffer& writeBU64(uint64_t value);
    StreamBuffer& writeBF32(float value);
    StreamBuffer& writeBF64(double value);

    uint8_t readU8(int* error);

    uint16_t readLU16(int* error);
    uint32_t readLU32(int* error);
    uint64_t readLU64(int* error);
    float readLF32(int* error);
    double readLF64(int* error);

    uint16_t readBU16(int* error);
    uint32_t readBU32(int* error);
    uint64_t readBU64(int* error);
    float readBF32(int* error);
    double readBF64(int* error);

    void* get() { return static_cast<void*>(buffer_.data()); }
    const void* get() const { return static_cast<const void*>(buffer_.data()); }
    int size() const { return buffer_.count(); }

private:
    Vector<char> buffer_;
    void* writePtr_;
    void* readPtr_;
};

}
