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

    Type count() const { return value_; }

    bool operator==(FighterStocks other) const { return value_ == other.value_; }
    bool operator!=(FighterStocks other) const { return value_ != other.value_; }
    bool operator< (FighterStocks other) const { return value_ < other.value_; }
    bool operator<=(FighterStocks other) const { return value_ <= other.value_; }
    bool operator> (FighterStocks other) const { return value_ > other.value_; }
    bool operator>=(FighterStocks other) const { return value_ >= other.value_; }

private:
    FighterStocks(Type value);

private:
    Type value_;
};

}
