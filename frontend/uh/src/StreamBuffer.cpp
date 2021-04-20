#include "uh/StreamBuffer.hpp"
#include "uh/Endian.hpp"
#include <cassert>
#include <cstring>

namespace uh {

// ----------------------------------------------------------------------------
StreamBuffer::StreamBuffer(int bytes)
    : buffer_(bytes, '\0')
    , writePtr_(buffer_.data())
    , readPtr_(buffer_.data())
{}

// ----------------------------------------------------------------------------
StreamBuffer::StreamBuffer(std::string&& data)
    : buffer_(std::move(data))
    , writePtr_(buffer_.data() + buffer_.size())
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
StreamBuffer& StreamBuffer::writeU16(uint16_t value)
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
StreamBuffer& StreamBuffer::writeU32(uint32_t value)
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
StreamBuffer& StreamBuffer::writeU64(uint64_t value)
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
StreamBuffer& StreamBuffer::writeF32(float value)
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
StreamBuffer& StreamBuffer::writeF64(double value)
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
uint8_t StreamBuffer::readU8()
{
    unsigned char* current = static_cast<unsigned char*>(readPtr_);
    assert(current + 1 <= writePtr_);
    uint8_t le;
    std::memcpy(&le, readPtr_, 1);
    readPtr_ = current + 1;
    return le;
}

// ----------------------------------------------------------------------------
uint16_t StreamBuffer::readU16()
{
    unsigned char* current = static_cast<unsigned char*>(readPtr_);
    assert(current + 2 <= writePtr_);
    uint16_t le;
    std::memcpy(&le, readPtr_, 2);
    readPtr_ = current + 2;
    return fromLittleEndian16(le);
}

// ----------------------------------------------------------------------------
uint32_t StreamBuffer::readU32()
{
    unsigned char* current = static_cast<unsigned char*>(readPtr_);
    assert(current + 4 <= writePtr_);
    uint32_t le;
    std::memcpy(&le, readPtr_, 4);
    readPtr_ = current + 4;
    return fromLittleEndian32(le);
}

// ----------------------------------------------------------------------------
uint64_t StreamBuffer::readU64()
{
    unsigned char* current = static_cast<unsigned char*>(readPtr_);
    assert(current + 8 <= writePtr_);
    uint64_t le;
    std::memcpy(&le, readPtr_, 8);
    readPtr_ = current + 8;
    return fromLittleEndian64(le);
}

// ----------------------------------------------------------------------------
float StreamBuffer::readF32()
{
    unsigned char* current = static_cast<unsigned char*>(readPtr_);
    assert(current + 4 <= writePtr_);
    uint32_t le;
    std::memcpy(&le, readPtr_, 4);
    readPtr_ = current + 4;
    le = fromLittleEndian32(le);
    return *reinterpret_cast<float*>(&le);
}

// ----------------------------------------------------------------------------
double StreamBuffer::readF64()
{
    unsigned char* current = static_cast<unsigned char*>(readPtr_);
    assert(current + 8 <= writePtr_);
    uint64_t le;
    std::memcpy(&le, readPtr_, 8);
    readPtr_ = current + 8;
    le = fromLittleEndian64(le);
    return *reinterpret_cast<double*>(&le);
}

// ----------------------------------------------------------------------------
int StreamBuffer::readBytesLeft() const
{
    const unsigned char* end = static_cast<const unsigned char*>(get()) + size();
    const unsigned char* current = static_cast<const unsigned char*>(readPtr_);
    return end - current;
}

// ----------------------------------------------------------------------------
int StreamBuffer::writeBytesLeft() const
{
    const unsigned char* end = static_cast<const unsigned char*>(get()) + size();
    const unsigned char* current = static_cast<const unsigned char*>(writePtr_);
    return end - current;
}

}
