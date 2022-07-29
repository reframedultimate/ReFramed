#include "rfcommon/FighterStocks.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
FighterStocks FighterStocks::fromValue(Type value)
{
    return FighterStocks(value);
}

// ----------------------------------------------------------------------------
FighterStocks::~FighterStocks()
{}

// ----------------------------------------------------------------------------
FighterStocks::Type FighterStocks::value() const 
{ 
    return value_;
}

// ----------------------------------------------------------------------------
bool FighterStocks::operator==(FighterStocks other) const 
{ 
    return value_ == other.value_; 
}

// ----------------------------------------------------------------------------
bool FighterStocks::operator!=(FighterStocks other) const 
{
    return value_ != other.value_; 
}

// ----------------------------------------------------------------------------
bool FighterStocks::operator< (FighterStocks other) const
{ 
    return value_ < other.value_;
}

// ----------------------------------------------------------------------------
bool FighterStocks::operator<=(FighterStocks other) const 
{
    return value_ <= other.value_;
}

// ----------------------------------------------------------------------------
bool FighterStocks::operator> (FighterStocks other) const 
{ 
    return value_ > other.value_;
}

// ----------------------------------------------------------------------------
bool FighterStocks::operator>=(FighterStocks other) const
{ 
    return value_ >= other.value_; 
}

// ----------------------------------------------------------------------------
FighterStocks::FighterStocks() 
    : value_(0)
{}

// ----------------------------------------------------------------------------
FighterStocks::FighterStocks(Type value) 
    : value_(value) 
{}

}
