#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Hashers.hpp"

namespace rfcommon {

class RFCOMMON_PUBLIC_API FighterHitStatus
{
public:
    typedef uint8_t Type;

    static FighterHitStatus fromValue(Type value);
    static FighterHitStatus makeInvalid();

    ~FighterHitStatus();

    Type value() const { return value_; }
    bool isValid() const { return value_ != Type(-1); }

    bool operator==(FighterHitStatus other) const { return value_ == other.value_; }
    bool operator!=(FighterHitStatus other) const { return value_ != other.value_; }
    bool operator<(FighterHitStatus other) const { return value_ < other.value_; }

    struct Hasher {
        typedef rfcommon::Hasher<Type>::HashType HashType;
        HashType operator()(FighterHitStatus hitStatus) const {
            return rfcommon::Hasher<Type>()(hitStatus.value());
        }
    };

private:
    FighterHitStatus(Type value);

private:
    Type value_;
};

}
