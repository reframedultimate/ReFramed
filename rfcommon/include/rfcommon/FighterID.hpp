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

    Type value() const;
    bool isValid() const;

    bool operator==(FighterID other) const;
    bool operator!=(FighterID other) const;
    bool operator<(FighterID other) const;

    struct Hasher {
        typedef rfcommon::Hasher<Type>::HashType HashType;
        HashType operator()(FighterID fighterID) const {
            return rfcommon::Hasher<Type>()(fighterID.value());
        }
    };

private:
    FighterID();
    FighterID(Type value);

private:
    Type value_;
};

}
