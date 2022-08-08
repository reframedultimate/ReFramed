#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Hashers.hpp"

namespace rfcommon {

class RFCOMMON_PUBLIC_API FighterStatus
{
public:
    typedef uint16_t Type;

    static FighterStatus fromValue(Type type);
    static FighterStatus makeInvalid();

    Type value() const { return value_; }
    bool isValid() const { return value_ != Type(-1); }

    bool operator==(FighterStatus other) const { return value_ == other.value_; }
    bool operator!=(FighterStatus other) const { return value_ != other.value_; }
    bool operator<(FighterStatus other) const { return value_ < other.value_; }

    struct Hasher {
        typedef rfcommon::Hasher<Type>::HashType HashType;
        HashType operator()(FighterStatus fighterStatus) const {
            return rfcommon::Hasher<Type>()(fighterStatus.value());
        }
    };

private:
    FighterStatus(Type value);

private:
    Type value_;
};

}
