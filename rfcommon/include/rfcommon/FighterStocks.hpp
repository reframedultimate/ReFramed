#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Hashers.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
class RFCOMMON_PUBLIC_API FighterStocks
{
public:
    typedef uint8_t Type;

    static FighterStocks fromValue(Type value);

    ~FighterStocks();

    Type value() const;

    bool operator==(FighterStocks other) const;
    bool operator!=(FighterStocks other) const;
    bool operator< (FighterStocks other) const;
    bool operator<=(FighterStocks other) const;
    bool operator> (FighterStocks other) const;
    bool operator>=(FighterStocks other) const;

private:
    friend class FighterState;
    FighterStocks();
    FighterStocks(Type value);

private:
    Type value_;
};

}
