#include "uh/StreamBuffer.hpp"
#include "uh/Endian.hpp"
#include <cassert>

namespace uh {

// ----------------------------------------------------------------------------
StreamBuffer::StreamBuffer(int bytes)
    : buffer_(bytes)
    , writePtr_(buffer_.data())
    , readPtr_(&buffer_.data())
{}

// ----------------------------------------------------------------------------
StreamBuffer::StreamBuffer(std::string&& data)
    : buffer_(std::move(data))
    , writePtr_(buffer_.data() + buffer_.size())
    , readPtr_(buffer.data)
{}

// ----------------------------------------------------------------------------
StreamBuffer& StreamBuffer::writeU8(uint8_t value)
{
    assert(writePtr_ + 1 <= buffer_.data() + buffer_.length());
    *writePtr_++ = value;
    return *this;
}

// ----------------------------------------------------------------------------
StreamBuffer& StreamBuffer::writeU16(uint16_t value)
{
    assert(writePtr_ + 2 <= buffer_.data() + buffer_.length());
    uint16_t le = toLittleEndian16(value);
    memcpy(writePtr_, &le, 2);
    writePtr_ += 2;
    return *this;
}

// ----------------------------------------------------------------------------
StreamBuffer& StreamBuffer::writeU32(uint32_t value)
{
    assert(writePtr_ + 4 <= buffer_.data() + buffer_.length());
    uint32_t le = toLittleEndian32(value);
    memcpy(writePtr_, &le, 4);
    writePtr_ += 4;
    return *this;
}

// ----------------------------------------------------------------------------
StreamBuffer& StreamBuffer::writeU64(uint64_t value)
{
    assert(writePtr_ + 8 <= buffer_.data() + buffer_.length());
    uint64_t le = toLittleEndian64(value);
    memcpy(writePtr_, &le, 8);
    writePtr_ += 8;
    return *this;
}

// ----------------------------------------------------------------------------
StreamBuffer& StreamBuffer::writeF32(float value)
{
    assert(writePtr_ + 4 <= buffer_.data() + buffer_.length());
    uint32_t le = toLittleEndian32(*static_cast<uint32_t*>(&value));
    memcpy(writePtr_, &le, 4);
    writePtr_ += 4;
    return *this;
}

// ----------------------------------------------------------------------------
uint8_t StreamBuffer::readU8()
{
    assert(readPtr_ < writePtr_);
    return *readPtr_++;
}

// ----------------------------------------------------------------------------
uint16_t StreamBuffer::readU16()
{
    assert(readPtr_ + 2 < writePtr_);
    uint16_t le;
    memcpy(&le, readPtr_, 2);
    readPtr_ += 2;
    return fromLittleEndian16(le);
}

// ----------------------------------------------------------------------------
uint32_t StreamBuffer::readU32()
{
    assert(readPtr_ + 4 < writePtr_);
    uint32_t le;
    memcpy(&le, readPtr_, 4);
    readPtr_ += 4;
    return fromLittleEndian32(le);
}

// ----------------------------------------------------------------------------
uint64_t StreamBuffer::readU64()
{
    assert(readPtr_ + 8 < writePtr_);
    uint64_t le;
    memcpy(&le, readPtr_, 8);
    readPtr_ += 8;
    return fromLittleEndian64(le);
}

// ----------------------------------------------------------------------------
float StreamBuffer::readF32()
{
    assert(readPtr_ + 4 < writePtr_);
    uint32_t le;
    memcpy(&le, readPtr_, 4);
    readPtr_ += 4;
    le = fromLittleEndian32(le);
    return *static_cast<float*>(&le);
}

// ----------------------------------------------------------------------------
double StreamBuffer::readF64()
{
    assert(readPtr_ + 8 < writePtr_);
    uint64_t le;
    memcpy(&le, readPtr_, 8);
    readPtr_ += 8;
    le = fromLittleEndian64(le);
    return *static_cast<double*>(&le);
}

}
