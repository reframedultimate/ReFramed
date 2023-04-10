#include "rfcommon/Deserializer.hpp"
#include "rfcommon/Endian.hpp"
#include "rfcommon/Profiler.hpp"
#include <cstring>
#include <cassert>

namespace rfcommon {

// ----------------------------------------------------------------------------
Deserializer::Deserializer(const void* data, uint64_t size)
    : readPtr_(static_cast<const unsigned char*>(data))
    , begin_(static_cast<const unsigned char*>(data))
    , end_(static_cast<const unsigned char*>(data) + size)
{
}

// ----------------------------------------------------------------------------
int8_t Deserializer::readI8()
{
    NOPROFILE();

    if (readPtr_ + 1 > end_)
        return 0;

    return *static_cast<const int8_t*>(static_cast<const void*>(readPtr_++));
}

// ----------------------------------------------------------------------------
uint8_t Deserializer::readU8()
{
    NOPROFILE();

    if (readPtr_ + 1 > end_)
        return 0;

    return *readPtr_++;
}

// ----------------------------------------------------------------------------
uint16_t Deserializer::readLU16()
{
    NOPROFILE();

    if (readPtr_ + 2 > end_)
        return 0;

    uint16_t le;
    std::memcpy(&le, readPtr_, 2);
    readPtr_ += 2;
    return fromLittleEndian16(le);
}

// ----------------------------------------------------------------------------
int16_t Deserializer::readLI16()
{
    NOPROFILE();

    if (readPtr_ + 2 > end_)
        return 0;

    uint16_t le;
    std::memcpy(&le, readPtr_, 2);
    readPtr_ += 2;
    return static_cast<int64_t>(fromLittleEndian16(le));
}

// ----------------------------------------------------------------------------
uint32_t Deserializer::readLU32()
{
    NOPROFILE();

    if (readPtr_ + 4 > end_)
        return 0;

    uint32_t le;
    std::memcpy(&le, readPtr_, 4);
    readPtr_ += 4;
    return fromLittleEndian32(le);
}

// ----------------------------------------------------------------------------
uint64_t Deserializer::readLU64()
{
    NOPROFILE();

    if (readPtr_ + 8 > end_)
        return 0;

    uint64_t le;
    std::memcpy(&le, readPtr_, 8);
    readPtr_ += 8;
    return fromLittleEndian64(le);
}

// ----------------------------------------------------------------------------
float Deserializer::readLF32()
{
    NOPROFILE();

    if (readPtr_ + 4 > end_)
        return 0.0f;

    uint32_t le;
    std::memcpy(&le, readPtr_, 4);
    readPtr_ += 4;
    le = fromLittleEndian32(le);
    return *reinterpret_cast<float*>(&le);
}

// ----------------------------------------------------------------------------
double Deserializer::readLF64()
{
    NOPROFILE();

    if (readPtr_ + 8 > end_)
        return 0.0f;

    uint64_t le;
    std::memcpy(&le, readPtr_, 8);
    readPtr_ += 8;
    le = fromLittleEndian64(le);
    return *reinterpret_cast<double*>(&le);
}

// ----------------------------------------------------------------------------
uint16_t Deserializer::readBU16()
{
    NOPROFILE();

    if (readPtr_ + 2 > end_)
        return 0;

    uint16_t le;
    std::memcpy(&le, readPtr_, 2);
    readPtr_ += 2;
    return fromBigEndian16(le);
}

// ----------------------------------------------------------------------------
uint32_t Deserializer::readBU32()
{
    NOPROFILE();

    if (readPtr_ + 4 > end_)
        return 0;

    uint32_t le;
    std::memcpy(&le, readPtr_, 4);
    readPtr_ += 4;
    return fromBigEndian32(le);
}

// ----------------------------------------------------------------------------
uint64_t Deserializer::readBU64()
{
    NOPROFILE();

    if (readPtr_ + 8 > end_)
        return 0;

    uint64_t le;
    std::memcpy(&le, readPtr_, 8);
    readPtr_ += 8;
    return fromBigEndian64(le);
}

// ----------------------------------------------------------------------------
float Deserializer::readBF32()
{
    NOPROFILE();

    if (readPtr_ + 4 > end_)
        return 0.0f;

    uint32_t le;
    std::memcpy(&le, readPtr_, 4);
    readPtr_ += 4;
    le = fromBigEndian32(le);
    return *reinterpret_cast<float*>(&le);
}

// ----------------------------------------------------------------------------
double Deserializer::readBF64()
{
    NOPROFILE();

    if (readPtr_ + 8 > end_)
        return 0.0f;

    uint64_t le;
    std::memcpy(&le, readPtr_, 8);
    readPtr_ += 8;
    le = fromBigEndian64(le);
    return *reinterpret_cast<double*>(&le);
}

// ----------------------------------------------------------------------------
uint64_t Deserializer::read(void* dst, uint64_t len)
{
    NOPROFILE();

    uint64_t actual = bytesLeft();
    if (len > actual)
        len = actual;
    memcpy(dst, readPtr_, len);
    readPtr_ += len;
    return len;
}

// ----------------------------------------------------------------------------
const void* Deserializer::readFromPtr(int len)
{
    NOPROFILE();

    if (readPtr_ + len > end_)
        return nullptr;
    const void* ptr = static_cast<const void*>(readPtr_);
    readPtr_ += len;
    return ptr;
}

// ----------------------------------------------------------------------------
const void* Deserializer::currentPtr() const
{
    NOPROFILE();

    return readPtr_;
}

// ----------------------------------------------------------------------------
uint64_t Deserializer::bytesRead() const
{
    NOPROFILE();

    return readPtr_ - begin_;
}

// ----------------------------------------------------------------------------
uint64_t Deserializer::bytesLeft() const
{
    NOPROFILE();

    return end_ - readPtr_;
}

// ----------------------------------------------------------------------------
uint64_t Deserializer::bytesTotal() const
{
    NOPROFILE();

    return end_ - begin_;
}

// ----------------------------------------------------------------------------
void Deserializer::seekSet(int64_t offset)
{
    NOPROFILE();

    readPtr_ = begin_ + offset;
    if (readPtr_ > end_)
        readPtr_ = end_;
    if (readPtr_ < begin_)
        readPtr_ = begin_;
}

// ----------------------------------------------------------------------------
void Deserializer::seekCur(int64_t offset)
{
    NOPROFILE();

    readPtr_ += offset;
    if (readPtr_ > end_)
        readPtr_ = end_;
    if (readPtr_ < begin_)
        readPtr_ = begin_;
}

// ----------------------------------------------------------------------------
void Deserializer::seekEnd(int64_t offset)
{
    NOPROFILE();

    readPtr_ = end_ - offset;
    if (readPtr_ > end_)
        readPtr_ = end_;
    if (readPtr_ < begin_)
        readPtr_ = begin_;
}

}
