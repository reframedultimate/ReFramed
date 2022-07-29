#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Vector.hpp"
#include <cstdint>

namespace rfcommon {

class RFCOMMON_PUBLIC_API StreamBuffer
{
public:
    StreamBuffer(int bytes);
    StreamBuffer(const void* data, int len);

    StreamBuffer& write(const void* data, int len);

    /*!
     * \brief Use this to obtain a block of memory to write to directly.
     *
     * Advances the internal write pointer by the specified number of
     * bytes and returns a pointer to the beginning of that block of memory.
     * You can call this multiple times, as long as the combined number of
     * bytes doesn't exceed the total size of the stream buffer.
     */
    void* writeToPtr(int bytes);

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

    void seekW(int offset);

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
