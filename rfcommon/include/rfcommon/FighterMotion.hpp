#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Hashers.hpp"

namespace rfcommon {

class RFCOMMON_PUBLIC_API FighterMotion
{
public:
    typedef uint64_t Type;

    static FighterMotion fromHexString(const char* hex);
    static FighterMotion fromValue(Type value);
    static FighterMotion fromParts(uint8_t upper, uint32_t lower);
    static FighterMotion makeInvalid();

    ~FighterMotion();

    uint8_t upper() const { return (value_ >> 32) & 0xFF; }
    uint32_t lower() const { return value_ & 0xFFFFFFFF; }
    Type value() const { return value_; }
    bool isValid() const { return value_ != 0; }
    String toHex() const;

    bool operator==(FighterMotion other) const { return value_ == other.value_; }
    bool operator!=(FighterMotion other) const { return value_ != other.value_; }
    bool operator<(FighterMotion other) const { return value_ < other.value_; }

    // Motion values are actually 40 bits in length, where the lower 32 bits are
    // a crc32 checksum of the original string, and the upper byte is the length
    // of that original string. For our purposes, it should be enough to use the
    // lower 32 bits for our hashmap
    struct Hasher {
        typedef uint32_t HashType;
        HashType operator()(FighterMotion motion) const {
            return motion.lower();
        }
    };

private:
    FighterMotion(Type value);

private:
    Type value_;
};

}
