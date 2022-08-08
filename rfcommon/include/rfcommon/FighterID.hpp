#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Hashers.hpp"
#include <cstdint>

namespace rfcommon {

class RFCOMMON_PUBLIC_API FighterID
{
public:
    typedef uint8_t Type;

    static FighterID fromValue(Type value);
    static FighterID makeInvalid();

    ~FighterID();

    Type value() const { return value_; }
    bool isValid() const { return value_ != Type(-1); }

    bool operator==(FighterID other) const { return value_ == other.value_; }
    bool operator!=(FighterID other) const { return value_ != other.value_; }
    bool operator<(FighterID other) const { return value_ < other.value_; }

    struct Hasher {
        typedef rfcommon::Hasher<Type>::HashType HashType;
        HashType operator()(FighterID fighterID) const {
            return rfcommon::Hasher<Type>()(fighterID.value());
        }
    };

private:
    FighterID(Type value);

private:
    Type value_;
};

}
