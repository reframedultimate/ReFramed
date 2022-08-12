#pragma once

#include "rfcommon/config.hpp"
#include <cstdint>

namespace rfcommon {

class RFCOMMON_PUBLIC_API Serializer
{
public:
    Serializer(void* dst, uint64_t maxSize);

    /*!
     * \brief Use this to obtain a block of memory to write to directly.
     *
     * Advances the internal write pointer by the specified number of
     * bytes and returns a pointer to the beginning of that block of memory.
     * You can call this multiple times, as long as the combined number of
     * bytes doesn't exceed the total size.
     */
    void* writeToPtr(int bytes);

    void write(const void* data, int len);

    void writeU8(uint8_t value);

    void writeLU16(uint16_t value);
    void writeLU32(uint32_t value);
    void writeLU64(uint64_t value);
    void writeLF32(float value);
    void writeLF64(double value);

    void writeBU16(uint16_t value);
    void writeBU32(uint32_t value);
    void writeBU64(uint64_t value);
    void writeBF32(float value);
    void writeBF64(double value);

    void seekW(int offset);

    uint64_t bytesWritten() const;

private:
    unsigned char* writePtr_;
    const unsigned char* const begin_;
    const unsigned char* const end_;
};

}
