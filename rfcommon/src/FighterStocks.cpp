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
FighterStocks::FighterStocks(Type value) 
    : value_(value) 
{}

}
