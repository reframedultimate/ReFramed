#include "rfcommon/StreamBuffer.hpp"
#include "rfcommon/Endian.hpp"
#include <cassert>
#include <cstring>

namespace rfcommon {

// ----------------------------------------------------------------------------
MemoryBuffer::MemoryBuffer(int bytes)
    : buffer_(bytes)
    , writePtr_(buffer_.data())
    , readPtr_(buffer_.data())
{}

// ----------------------------------------------------------------------------
MemoryBuffer::MemoryBuffer(const void* data, int len)
    : buffer_(static_cast<const unsigned char*>(data), len)
    , writePtr_(buffer_.data() + len)
    , readPtr_(buffer_.data())
{}

// ----------------------------------------------------------------------------
MemoryBuffer& MemoryBuffer::write(const void* data, int len)
{
    unsigned char* end = static_cast<unsigned char*>(get()) + capacity();
    assert(writePtr_ + len <= end);
    std::memcpy(writePtr_, data, len);
    writePtr_ += len;
    return *this;
}

// ----------------------------------------------------------------------------
MemoryBuffer& MemoryBuffer::writeU8(uint8_t value)
{
    unsigned char* end = static_cast<unsigned char*>(get()) + capacity();
    assert(writePtr_ + 1 <= end);
    std::memcpy(writePtr_, &value, 1);
    writePtr_ += 1;
    return *this;
}

// ----------------------------------------------------------------------------
MemoryBuffer& MemoryBuffer::writeLU16(uint16_t value)
{
    unsigned char* end = static_cast<unsigned char*>(get()) + capacity();
    assert(writePtr_ + 2 <= end);
    uint16_t le = toLittleEndian16(value);
    std::memcpy(writePtr_, &le, 2);
    writePtr_ += 2;
    return *this;
}

// ----------------------------------------------------------------------------
MemoryBuffer& MemoryBuffer::writeLU32(uint32_t value)
{
    unsigned char* end = static_cast<unsigned char*>(get()) + capacity();
    assert(writePtr_ + 4 <= end);
    uint32_t le = toLittleEndian32(value);
    std::memcpy(writePtr_, &le, 4);
    writePtr_ += 4;
    return *this;
}

// ----------------------------------------------------------------------------
MemoryBuffer& MemoryBuffer::writeLU64(uint64_t value)
{
    unsigned char* end = static_cast<unsigned char*>(get()) + capacity();
    assert(writePtr_ + 8 <= end);
    uint64_t le = toLittleEndian64(value);
    std::memcpy(writePtr_, &le, 8);
    writePtr_ += 8;
    return *this;
}

// ----------------------------------------------------------------------------
MemoryBuffer& MemoryBuffer::writeLF32(float value)
{
    unsigned char* end = static_cast<unsigned char*>(get()) + capacity();
    assert(writePtr_ + 4 <= end);
    uint32_t le = toLittleEndian32(*reinterpret_cast<uint32_t*>(&value));
    std::memcpy(writePtr_, &le, 4);
    writePtr_ += 4;
    return *this;
}

// ----------------------------------------------------------------------------
MemoryBuffer& MemoryBuffer::writeLF64(double value)
{

    unsigned char* end = static_cast<unsigned char*>(get()) + capacity();
    assert(writePtr_ + 8 <= end);
    uint64_t le = toLittleEndian64(*reinterpret_cast<uint64_t*>(&value));
    std::memcpy(writePtr_, &le, 8);
    writePtr_ += 8;
    return *this;
}

// ----------------------------------------------------------------------------
MemoryBuffer& MemoryBuffer::writeBU16(uint16_t value)
{
    unsigned char* end = static_cast<unsigned char*>(get()) + capacity();
    assert(writePtr_ + 2 <= end);
    uint16_t le = toBigEndian16(value);
    std::memcpy(writePtr_, &le, 2);
    writePtr_ += 2;
    return *this;
}

// ----------------------------------------------------------------------------
MemoryBuffer& MemoryBuffer::writeBU32(uint32_t value)
{
    unsigned char* end = static_cast<unsigned char*>(get()) + capacity();
    assert(writePtr_ + 4 <= end);
    uint32_t le = toBigEndian32(value);
    std::memcpy(writePtr_, &le, 4);
    writePtr_ += 4;
    return *this;
}

// ----------------------------------------------------------------------------
MemoryBuffer& MemoryBuffer::writeBU64(uint64_t value)
{
    unsigned char* end = static_cast<unsigned char*>(get()) + capacity();
    assert(writePtr_ + 8 <= end);
    uint64_t le = toBigEndian64(value);
    std::memcpy(writePtr_, &le, 8);
    writePtr_ += 8;
    return *this;
}

// ----------------------------------------------------------------------------
MemoryBuffer& MemoryBuffer::writeBF32(float value)
{
    unsigned char* end = static_cast<unsigned char*>(get()) + capacity();
    assert(writePtr_ + 4 <= end);
    uint32_t le = toBigEndian32(*reinterpret_cast<uint32_t*>(&value));
    std::memcpy(writePtr_, &le, 4);
    writePtr_ += 4;
    return *this;
}

// ----------------------------------------------------------------------------
MemoryBuffer& MemoryBuffer::writeBF64(double value)
{

    unsigned char* end = static_cast<unsigned char*>(get()) + capacity();
    assert(writePtr_ + 8 <= end);
    uint64_t le = toBigEndian64(*reinterpret_cast<uint64_t*>(&value));
    std::memcpy(writePtr_, &le, 8);
    writePtr_ += 8;
    return *this;
}

// ----------------------------------------------------------------------------
uint8_t MemoryBuffer::readU8(int* error)
{
    if (readPtr_ + 1 > writePtr_)
    {
        *error = 1;
        return 0;
    }
    uint8_t le;
    std::memcpy(&le, readPtr_, 1);
    readPtr_ += 1;
    return le;
}

// ----------------------------------------------------------------------------
uint16_t MemoryBuffer::readLU16(int* error)
{
    if (readPtr_ + 2 > writePtr_)
    {
        *error = 1;
        return 0;
    }
    uint16_t le;
    std::memcpy(&le, readPtr_, 2);
    readPtr_ += 2;
    return fromLittleEndian16(le);
}

// ----------------------------------------------------------------------------
uint32_t MemoryBuffer::readLU32(int* error)
{
    if (readPtr_ + 4 > writePtr_)
    {
        *error = 1;
        return 0;
    }
    uint32_t le;
    std::memcpy(&le, readPtr_, 4);
    readPtr_ += 4;
    return fromLittleEndian32(le);
}

// ----------------------------------------------------------------------------
uint64_t MemoryBuffer::readLU64(int* error)
{
    if (readPtr_ + 8 > writePtr_)
    {
        *error = 1;
        return 0;
    }
    uint64_t le;
    std::memcpy(&le, readPtr_, 8);
    readPtr_ += 8;
    return fromLittleEndian64(le);
}

// ----------------------------------------------------------------------------
float MemoryBuffer::readLF32(int* error)
{
    if (readPtr_ + 4 > writePtr_)
    {
        *error = 1;
        return 0;
    }
    uint32_t le;
    std::memcpy(&le, readPtr_, 4);
    readPtr_ += 4;
    le = fromLittleEndian32(le);
    return *reinterpret_cast<float*>(&le);
}

// ----------------------------------------------------------------------------
double MemoryBuffer::readLF64(int* error)
{
    if (readPtr_ + 8 > writePtr_)
    {
        *error = 1;
        return 0;
    }
    uint64_t le;
    std::memcpy(&le, readPtr_, 8);
    readPtr_ += 8;
    le = fromLittleEndian64(le);
    return *reinterpret_cast<double*>(&le);
}

// ----------------------------------------------------------------------------
uint16_t MemoryBuffer::readBU16(int* error)
{
    if (readPtr_ + 2 > writePtr_)
    {
        *error = 1;
        return 0;
    }
    uint16_t le;
    std::memcpy(&le, readPtr_, 2);
    readPtr_ += 2;
    return fromBigEndian16(le);
}

// ----------------------------------------------------------------------------
uint32_t MemoryBuffer::readBU32(int* error)
{
    if (readPtr_ + 4 > writePtr_)
    {
        *error = 1;
        return 0;
    }
    uint32_t le;
    std::memcpy(&le, readPtr_, 4);
    readPtr_ += 4;
    return fromBigEndian32(le);
}

// ----------------------------------------------------------------------------
uint64_t MemoryBuffer::readBU64(int* error)
{
    if (readPtr_ + 8 > writePtr_)
    {
        *error = 1;
        return 0;
    }
    uint64_t le;
    std::memcpy(&le, readPtr_, 8);
    readPtr_ += 8;
    return fromBigEndian64(le);
}

// ----------------------------------------------------------------------------
float MemoryBuffer::readBF32(int* error)
{
    if (readPtr_ + 4 > writePtr_)
    {
        *error = 1;
        return 0;
    }
    uint32_t le;
    std::memcpy(&le, readPtr_, 4);
    readPtr_ += 4;
    le = fromBigEndian32(le);
    return *reinterpret_cast<float*>(&le);
}

// ----------------------------------------------------------------------------
double MemoryBuffer::readBF64(int* error)
{
    if (readPtr_ + 8 > writePtr_)
    {
        *error = 1;
        return 0;
    }
    uint64_t le;
    std::memcpy(&le, readPtr_, 8);
    readPtr_ += 8;
    le = fromBigEndian64(le);
    return *reinterpret_cast<double*>(&le);
}

}
