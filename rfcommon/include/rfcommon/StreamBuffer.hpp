#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Vector.hpp"
#include <cstdint>

namespace rfcommon {

extern template class RFCOMMON_TEMPLATE_API Vector<char>;

class RFCOMMON_PUBLIC_API MemoryBuffer
{
public:
    MemoryBuffer(int bytes);
    MemoryBuffer(const void* data, int len);

    MemoryBuffer& write(const void* data, int len);

    MemoryBuffer& writeU8(uint8_t value);

    MemoryBuffer& writeLU16(uint16_t value);
    MemoryBuffer& writeLU32(uint32_t value);
    MemoryBuffer& writeLU64(uint64_t value);
    MemoryBuffer& writeLF32(float value);
    MemoryBuffer& writeLF64(double value);

    MemoryBuffer& writeBU16(uint16_t value);
    MemoryBuffer& writeBU32(uint32_t value);
    MemoryBuffer& writeBU64(uint64_t value);
    MemoryBuffer& writeBF32(float value);
    MemoryBuffer& writeBF64(double value);

    void seekW(int offset)
        { writePtr_ = buffer_.data() + offset; }

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

    void* get() { buffer_.data(); }
    const void* get() const { return buffer_.data(); }
    int capacity() const { return buffer_.count(); }
    int bytesWritten() const { return static_cast<unsigned char*>(writePtr_) - buffer_.data(); }
    int bytesRead() const { return static_cast<unsigned char*>(writePtr_) - buffer_.data(); }

private:
    Vector<unsigned char> buffer_;
    unsigned char* writePtr_;
    const unsigned char* readPtr_;
};

}
