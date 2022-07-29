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

    Type value() const;
    bool isValid() const;

    bool operator==(FighterStatus other) const;
    bool operator!=(FighterStatus other) const;
    bool operator<(FighterStatus other) const;

    struct Hasher {
        typedef rfcommon::Hasher<Type>::HashType HashType;
        HashType operator()(FighterStatus fighterStatus) const {
            return rfcommon::Hasher<Type>()(fighterStatus.value());
        }
    };

private:
    FighterStatus();
    FighterStatus(Type value);

private:
    Type value_;
};

}
