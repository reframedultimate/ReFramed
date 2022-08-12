#include "rfcommon/Serializer.hpp"
#include "rfcommon/Endian.hpp"
#include <cassert>
#include <cstring>

namespace rfcommon {

// ----------------------------------------------------------------------------
Serializer::Serializer(void* dst, uint64_t maxSize)
    : writePtr_(static_cast<unsigned char*>(dst))
    , begin_(static_cast<unsigned char*>(dst))
    , end_(static_cast<unsigned char*>(dst) + maxSize)
{
}

// ----------------------------------------------------------------------------
void* Serializer::writeToPtr(int bytes)
{
    assert(writePtr_ + bytes <= end_);

    void* begin = writePtr_;
    writePtr_ += bytes;
    return begin;
}

// ----------------------------------------------------------------------------
void Serializer::writeU8(uint8_t value)
{
    assert(writePtr_ + 1 <= end_);
    *writePtr_++ = value;
}

// ----------------------------------------------------------------------------
void Serializer::writeLU16(uint16_t value)
{
    assert(writePtr_ + 2 <= end_);

    uint16_t le = toLittleEndian16(value);
    std::memcpy(writePtr_, &le, 2);
    writePtr_ += 2;
}

// ----------------------------------------------------------------------------
void Serializer::writeLU32(uint32_t value)
{
    assert(writePtr_ + 4 <= end_);

    uint32_t le = toLittleEndian32(value);
    std::memcpy(writePtr_, &le, 4);
    writePtr_ += 4;
}

// ----------------------------------------------------------------------------
void Serializer::writeLU64(uint64_t value)
{
    assert(writePtr_ + 8 <= end_);

    uint64_t le = toLittleEndian64(value);
    std::memcpy(writePtr_, &le, 8);
    writePtr_ += 8;
}

// ----------------------------------------------------------------------------
void Serializer::writeLF32(float value)
{
    assert(writePtr_ + 4 <= end_);

    uint32_t le = toLittleEndian32(*reinterpret_cast<uint32_t*>(&value));
    std::memcpy(writePtr_, &le, 4);
    writePtr_ += 4;
}

// ----------------------------------------------------------------------------
void Serializer::writeLF64(double value)
{

    assert(writePtr_ + 8 <= end_);

    uint64_t le = toLittleEndian64(*reinterpret_cast<uint64_t*>(&value));
    std::memcpy(writePtr_, &le, 8);
    writePtr_ += 8;
}

// ----------------------------------------------------------------------------
void Serializer::writeBU16(uint16_t value)
{
    assert(writePtr_ + 2 <= end_);

    uint16_t le = toBigEndian16(value);
    std::memcpy(writePtr_, &le, 2);
    writePtr_ += 2;
}

// ----------------------------------------------------------------------------
void Serializer::writeBU32(uint32_t value)
{
    assert(writePtr_ + 4 <= end_);

    uint32_t le = toBigEndian32(value);
    std::memcpy(writePtr_, &le, 4);
    writePtr_ += 4;
}

// ----------------------------------------------------------------------------
void Serializer::writeBU64(uint64_t value)
{
    assert(writePtr_ + 8 <= end_);

    uint64_t le = toBigEndian64(value);
    std::memcpy(writePtr_, &le, 8);
    writePtr_ += 8;
}

// ----------------------------------------------------------------------------
void Serializer::writeBF32(float value)
{
    assert(writePtr_ + 4 <= end_);

    uint32_t le = toBigEndian32(*reinterpret_cast<uint32_t*>(&value));
    std::memcpy(writePtr_, &le, 4);
    writePtr_ += 4;
}

// ----------------------------------------------------------------------------
void Serializer::writeBF64(double value)
{

    assert(writePtr_ + 8 <= end_);

    uint64_t le = toBigEndian64(*reinterpret_cast<uint64_t*>(&value));
    std::memcpy(writePtr_, &le, 8);
    writePtr_ += 8;
}

// ----------------------------------------------------------------------------
void Serializer::seekW(int offset)
{
    writePtr_ = const_cast<unsigned char*>(begin_ + offset);
}

// ----------------------------------------------------------------------------
uint64_t Serializer::bytesWritten() const
{
    return writePtr_ - begin_;
}

}
