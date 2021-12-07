#include "uh/StreamBuffer.hpp"
#include "uh/Endian.hpp"
#include <cassert>
#include <cstring>

namespace uh {

// ----------------------------------------------------------------------------
StreamBuffer::StreamBuffer(int bytes)
    : buffer_(bytes)
    , writePtr_(buffer_.data())
    , readPtr_(buffer_.data())
{}

// ----------------------------------------------------------------------------
StreamBuffer::StreamBuffer(const char* data, int len)
    : buffer_(data, len)
    , writePtr_(buffer_.data() + len)
    , readPtr_(buffer_.data())
{}

// ----------------------------------------------------------------------------
StreamBuffer& StreamBuffer::writeU8(uint8_t value)
{
    unsigned char* end = static_cast<unsigned char*>(get()) + size();
    unsigned char* current = static_cast<unsigned char*>(writePtr_);
    assert(current + 1 <= end);
    std::memcpy(writePtr_, &value, 1);
    writePtr_ = current + 1;
    return *this;
}

// ----------------------------------------------------------------------------
StreamBuffer& StreamBuffer::writeLU16(uint16_t value)
{
    unsigned char* end = static_cast<unsigned char*>(get()) + size();
    unsigned char* current = static_cast<unsigned char*>(writePtr_);
    assert(current + 2 <= end);
    uint16_t le = toLittleEndian16(value);
    std::memcpy(writePtr_, &le, 2);
    writePtr_ = current + 2;
    return *this;
}

// ----------------------------------------------------------------------------
StreamBuffer& StreamBuffer::writeLU32(uint32_t value)
{
    unsigned char* end = static_cast<unsigned char*>(get()) + size();
    unsigned char* current = static_cast<unsigned char*>(writePtr_);
    assert(current + 4 <= end);
    uint32_t le = toLittleEndian32(value);
    std::memcpy(writePtr_, &le, 4);
    writePtr_ = current + 4;
    return *this;
}

// ----------------------------------------------------------------------------
StreamBuffer& StreamBuffer::writeLU64(uint64_t value)
{
    unsigned char* end = static_cast<unsigned char*>(get()) + size();
    unsigned char* current = static_cast<unsigned char*>(writePtr_);
    assert(current + 8 <= end);
    uint64_t le = toLittleEndian64(value);
    std::memcpy(writePtr_, &le, 8);
    writePtr_ = current + 8;
    return *this;
}

// ----------------------------------------------------------------------------
StreamBuffer& StreamBuffer::writeLF32(float value)
{
    unsigned char* end = static_cast<unsigned char*>(get()) + size();
    unsigned char* current = static_cast<unsigned char*>(writePtr_);
    assert(current + 4 <= end);
    uint32_t le = toLittleEndian32(*reinterpret_cast<uint32_t*>(&value));
    std::memcpy(writePtr_, &le, 4);
    writePtr_ = current + 4;
    return *this;
}

// ----------------------------------------------------------------------------
StreamBuffer& StreamBuffer::writeLF64(double value)
{

    unsigned char* end = static_cast<unsigned char*>(get()) + size();
    unsigned char* current = static_cast<unsigned char*>(writePtr_);
    assert(current + 8 <= end);
    uint64_t le = toLittleEndian64(*reinterpret_cast<uint64_t*>(&value));
    std::memcpy(writePtr_, &le, 8);
    writePtr_ = current + 8;
    return *this;
}

// ----------------------------------------------------------------------------
StreamBuffer& StreamBuffer::writeBU16(uint16_t value)
{
    unsigned char* end = static_cast<unsigned char*>(get()) + size();
    unsigned char* current = static_cast<unsigned char*>(writePtr_);
    assert(current + 2 <= end);
    uint16_t le = toBigEndian16(value);
    std::memcpy(writePtr_, &le, 2);
    writePtr_ = current + 2;
    return *this;
}

// ----------------------------------------------------------------------------
StreamBuffer& StreamBuffer::writeBU32(uint32_t value)
{
    unsigned char* end = static_cast<unsigned char*>(get()) + size();
    unsigned char* current = static_cast<unsigned char*>(writePtr_);
    assert(current + 4 <= end);
    uint32_t le = toBigEndian32(value);
    std::memcpy(writePtr_, &le, 4);
    writePtr_ = current + 4;
    return *this;
}

// ----------------------------------------------------------------------------
StreamBuffer& StreamBuffer::writeBU64(uint64_t value)
{
    unsigned char* end = static_cast<unsigned char*>(get()) + size();
    unsigned char* current = static_cast<unsigned char*>(writePtr_);
    assert(current + 8 <= end);
    uint64_t le = toBigEndian64(value);
    std::memcpy(writePtr_, &le, 8);
    writePtr_ = current + 8;
    return *this;
}

// ----------------------------------------------------------------------------
StreamBuffer& StreamBuffer::writeBF32(float value)
{
    unsigned char* end = static_cast<unsigned char*>(get()) + size();
    unsigned char* current = static_cast<unsigned char*>(writePtr_);
    assert(current + 4 <= end);
    uint32_t le = toBigEndian32(*reinterpret_cast<uint32_t*>(&value));
    std::memcpy(writePtr_, &le, 4);
    writePtr_ = current + 4;
    return *this;
}

// ----------------------------------------------------------------------------
StreamBuffer& StreamBuffer::writeBF64(double value)
{

    unsigned char* end = static_cast<unsigned char*>(get()) + size();
    unsigned char* current = static_cast<unsigned char*>(writePtr_);
    assert(current + 8 <= end);
    uint64_t le = toBigEndian64(*reinterpret_cast<uint64_t*>(&value));
    std::memcpy(writePtr_, &le, 8);
    writePtr_ = current + 8;
    return *this;
}

// ----------------------------------------------------------------------------
uint8_t StreamBuffer::readU8(int* error)
{
    unsigned char* current = static_cast<unsigned char*>(readPtr_);
    if (current + 1 > writePtr_)
    {
        *error = 1;
        return 0;
    }
    uint8_t le;
    std::memcpy(&le, readPtr_, 1);
    readPtr_ = current + 1;
    return le;
}

// ----------------------------------------------------------------------------
uint16_t StreamBuffer::readLU16(int* error)
{
    unsigned char* current = static_cast<unsigned char*>(readPtr_);
    if (current + 2 > writePtr_)
    {
        *error = 1;
        return 0;
    }
    uint16_t le;
    std::memcpy(&le, readPtr_, 2);
    readPtr_ = current + 2;
    return fromLittleEndian16(le);
}

// ----------------------------------------------------------------------------
uint32_t StreamBuffer::readLU32(int* error)
{
    unsigned char* current = static_cast<unsigned char*>(readPtr_);
    if (current + 4 > writePtr_)
    {
        *error = 1;
        return 0;
    }
    uint32_t le;
    std::memcpy(&le, readPtr_, 4);
    readPtr_ = current + 4;
    return fromLittleEndian32(le);
}

// ----------------------------------------------------------------------------
uint64_t StreamBuffer::readLU64(int* error)
{
    unsigned char* current = static_cast<unsigned char*>(readPtr_);
    if (current + 8 > writePtr_)
    {
        *error = 1;
        return 0;
    }
    uint64_t le;
    std::memcpy(&le, readPtr_, 8);
    readPtr_ = current + 8;
    return fromLittleEndian64(le);
}

// ----------------------------------------------------------------------------
float StreamBuffer::readLF32(int* error)
{
    unsigned char* current = static_cast<unsigned char*>(readPtr_);
    if (current + 4 > writePtr_)
    {
        *error = 1;
        return 0;
    }
    uint32_t le;
    std::memcpy(&le, readPtr_, 4);
    readPtr_ = current + 4;
    le = fromLittleEndian32(le);
    return *reinterpret_cast<float*>(&le);
}

// ----------------------------------------------------------------------------
double StreamBuffer::readLF64(int* error)
{
    unsigned char* current = static_cast<unsigned char*>(readPtr_);
    if (current + 8 > writePtr_)
    {
        *error = 1;
        return 0;
    }
    uint64_t le;
    std::memcpy(&le, readPtr_, 8);
    readPtr_ = current + 8;
    le = fromLittleEndian64(le);
    return *reinterpret_cast<double*>(&le);
}

// ----------------------------------------------------------------------------
uint16_t StreamBuffer::readBU16(int* error)
{
    unsigned char* current = static_cast<unsigned char*>(readPtr_);
    if (current + 2 > writePtr_)
    {
        *error = 1;
        return 0;
    }
    uint16_t le;
    std::memcpy(&le, readPtr_, 2);
    readPtr_ = current + 2;
    return fromBigEndian16(le);
}

// ----------------------------------------------------------------------------
uint32_t StreamBuffer::readBU32(int* error)
{
    unsigned char* current = static_cast<unsigned char*>(readPtr_);
    if (current + 4 > writePtr_)
    {
        *error = 1;
        return 0;
    }
    uint32_t le;
    std::memcpy(&le, readPtr_, 4);
    readPtr_ = current + 4;
    return fromBigEndian32(le);
}

// ----------------------------------------------------------------------------
uint64_t StreamBuffer::readBU64(int* error)
{
    unsigned char* current = static_cast<unsigned char*>(readPtr_);
    if (current + 8 > writePtr_)
    {
        *error = 1;
        return 0;
    }
    uint64_t le;
    std::memcpy(&le, readPtr_, 8);
    readPtr_ = current + 8;
    return fromBigEndian64(le);
}

// ----------------------------------------------------------------------------
float StreamBuffer::readBF32(int* error)
{
    unsigned char* current = static_cast<unsigned char*>(readPtr_);
    if (current + 4 > writePtr_)
    {
        *error = 1;
        return 0;
    }
    uint32_t le;
    std::memcpy(&le, readPtr_, 4);
    readPtr_ = current + 4;
    le = fromBigEndian32(le);
    return *reinterpret_cast<float*>(&le);
}

// ----------------------------------------------------------------------------
double StreamBuffer::readBF64(int* error)
{
    unsigned char* current = static_cast<unsigned char*>(readPtr_);
    if (current + 8 > writePtr_)
    {
        *error = 1;
        return 0;
    }
    uint64_t le;
    std::memcpy(&le, readPtr_, 8);
    readPtr_ = current + 8;
    le = fromBigEndian64(le);
    return *reinterpret_cast<double*>(&le);
}

}
