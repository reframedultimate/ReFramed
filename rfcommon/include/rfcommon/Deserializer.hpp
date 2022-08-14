#pragma once

#include "rfcommon/config.hpp"
#include <cstdint>

namespace rfcommon {

class RFCOMMON_PUBLIC_API Deserializer
{
public:
    Deserializer(const void* data, uint64_t size);

    uint8_t readU8();

    uint16_t readLU16();
    uint32_t readLU32();
    uint64_t readLU64();
    float readLF32();
    double readLF64();

    uint16_t readBU16();
    uint32_t readBU32();
    uint64_t readBU64();
    float readBF32();
    double readBF64();

    uint64_t read(void* dst, uint64_t len);

    const void* readFromPtr(int len);
    const void* currentPtr() const;

    uint64_t bytesRead() const;
    uint64_t bytesLeft() const;
    uint64_t bytesTotal() const;

    void seekSet(int64_t offset);
    void seekCur(int64_t offset);
    void seekEnd(int64_t offset);
    int64_t currentOffset() const;

private:
    const unsigned char* readPtr_;
    const unsigned char* const begin_;
    const unsigned char* const end_;
};

}
