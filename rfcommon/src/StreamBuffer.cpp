#include "rfcommon/StreamBuffer.hpp"
#include "rfcommon/Endian.hpp"
#include <cassert>
#include <cstring>

namespace rfcommon {

// ----------------------------------------------------------------------------
StreamBuffer::StreamBuffer(int bytes)
    : buffer_(Vector<unsigned char>::makeResized(bytes))
    , writePtr_(buffer_.data())
    , readPtr_(buffer_.data())
{}

// ----------------------------------------------------------------------------
StreamBuffer::StreamBuffer(const void* data, int len)
    : buffer_(static_cast<const unsigned char*>(data), len)
    , writePtr_(buffer_.data() + len)
    , readPtr_(buffer_.data())
{}

// ----------------------------------------------------------------------------
StreamBuffer& StreamBuffer::write(const void* data, int len)
{
    std::memcpy(writeToPtr(len), data, len);
    return *this;
}

// ----------------------------------------------------------------------------
void* StreamBuffer::writeToPtr(int bytes)
{
    const unsigned char* end = static_cast<const unsigned char*>(get()) + capacity();
    assert(writePtr_ + bytes <= end);
    void* begin = writePtr_;
    writePtr_ += bytes;
    return begin;
}

// ----------------------------------------------------------------------------
StreamBuffer& StreamBuffer::writeU8(uint8_t value)
{
    const unsigned char* end = static_cast<const unsigned char*>(get()) + capacity();
    assert(writePtr_ + 1 <= end);
    std::memcpy(writePtr_, &value, 1);
    writePtr_ += 1;
    return *this;
}

// ----------------------------------------------------------------------------
StreamBuffer& StreamBuffer::writeLU16(uint16_t value)
{
    const unsigned char* end = static_cast<const unsigned char*>(get()) + capacity();
    assert(writePtr_ + 2 <= end);
    uint16_t le = toLittleEndian16(value);
    std::memcpy(writePtr_, &le, 2);
    writePtr_ += 2;
    return *this;
}

// ----------------------------------------------------------------------------
StreamBuffer& StreamBuffer::writeLU32(uint32_t value)
{
    const unsigned char* end = static_cast<const unsigned char*>(get()) + capacity();
    assert(writePtr_ + 4 <= end);
    uint32_t le = toLittleEndian32(value);
    std::memcpy(writePtr_, &le, 4);
    writePtr_ += 4;
    return *this;
}

// ----------------------------------------------------------------------------
StreamBuffer& StreamBuffer::writeLU64(uint64_t value)
{
    const unsigned char* end = static_cast<const unsigned char*>(get()) + capacity();
    assert(writePtr_ + 8 <= end);
    uint64_t le = toLittleEndian64(value);
    std::memcpy(writePtr_, &le, 8);
    writePtr_ += 8;
    return *this;
}

// ----------------------------------------------------------------------------
StreamBuffer& StreamBuffer::writeLF32(float value)
{
    const unsigned char* end = static_cast<const unsigned char*>(get()) + capacity();
    assert(writePtr_ + 4 <= end);
    uint32_t le = toLittleEndian32(*reinterpret_cast<uint32_t*>(&value));
    std::memcpy(writePtr_, &le, 4);
    writePtr_ += 4;
    return *this;
}

// ----------------------------------------------------------------------------
StreamBuffer& StreamBuffer::writeLF64(double value)
{

    const unsigned char* end = static_cast<const unsigned char*>(get()) + capacity();
    assert(writePtr_ + 8 <= end);
    uint64_t le = toLittleEndian64(*reinterpret_cast<uint64_t*>(&value));
    std::memcpy(writePtr_, &le, 8);
    writePtr_ += 8;
    return *this;
}

// ----------------------------------------------------------------------------
StreamBuffer& StreamBuffer::writeBU16(uint16_t value)
{
    const unsigned char* end = static_cast<const unsigned char*>(get()) + capacity();
    assert(writePtr_ + 2 <= end);
    uint16_t le = toBigEndian16(value);
    std::memcpy(writePtr_, &le, 2);
    writePtr_ += 2;
    return *this;
}

// ----------------------------------------------------------------------------
StreamBuffer& StreamBuffer::writeBU32(uint32_t value)
{
    const unsigned char* end = static_cast<const unsigned char*>(get()) + capacity();
    assert(writePtr_ + 4 <= end);
    uint32_t le = toBigEndian32(value);
    std::memcpy(writePtr_, &le, 4);
    writePtr_ += 4;
    return *this;
}

// ----------------------------------------------------------------------------
StreamBuffer& StreamBuffer::writeBU64(uint64_t value)
{
    const unsigned char* end = static_cast<const unsigned char*>(get()) + capacity();
    assert(writePtr_ + 8 <= end);
    uint64_t le = toBigEndian64(value);
    std::memcpy(writePtr_, &le, 8);
    writePtr_ += 8;
    return *this;
}

// ----------------------------------------------------------------------------
StreamBuffer& StreamBuffer::writeBF32(float value)
{
    const unsigned char* end = static_cast<const unsigned char*>(get()) + capacity();
    assert(writePtr_ + 4 <= end);
    uint32_t le = toBigEndian32(*reinterpret_cast<uint32_t*>(&value));
    std::memcpy(writePtr_, &le, 4);
    writePtr_ += 4;
    return *this;
}

// ----------------------------------------------------------------------------
StreamBuffer& StreamBuffer::writeBF64(double value)
{

    const unsigned char* end = static_cast<const unsigned char*>(get()) + capacity();
    assert(writePtr_ + 8 <= end);
    uint64_t le = toBigEndian64(*reinterpret_cast<uint64_t*>(&value));
    std::memcpy(writePtr_, &le, 8);
    writePtr_ += 8;
    return *this;
}

// ----------------------------------------------------------------------------
void StreamBuffer::seekW(int offset)
{
    writePtr_ = buffer_.data() + offset;
}

// ----------------------------------------------------------------------------
uint8_t StreamBuffer::readU8(int* error)
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
uint16_t StreamBuffer::readLU16(int* error)
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
uint32_t StreamBuffer::readLU32(int* error)
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
uint64_t StreamBuffer::readLU64(int* error)
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
float StreamBuffer::readLF32(int* error)
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
double StreamBuffer::readLF64(int* error)
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
uint16_t StreamBuffer::readBU16(int* error)
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
uint32_t StreamBuffer::readBU32(int* error)
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
uint64_t StreamBuffer::readBU64(int* error)
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
float StreamBuffer::readBF32(int* error)
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
double StreamBuffer::readBF64(int* error)
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
