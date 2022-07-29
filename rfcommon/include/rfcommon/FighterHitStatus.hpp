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

    Type value() const;
    bool isValid() const;

    bool operator==(FighterHitStatus other) const;
    bool operator<(FighterHitStatus other) const;
    bool operator!=(FighterHitStatus other) const;

    struct Hasher {
        typedef rfcommon::Hasher<Type>::HashType HashType;
        HashType operator()(FighterHitStatus hitStatus) const {
            return rfcommon::Hasher<Type>()(hitStatus.value());
        }
    };

private:
    FighterHitStatus();
    FighterHitStatus(Type value);

private:
    Type value_;
};

}
