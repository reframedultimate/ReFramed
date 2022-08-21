#include "rfcommon/FighterStocks.hpp"
#include "rfcommon/Profiler.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
FighterStocks FighterStocks::fromValue(Type value)
{
    NOPROFILE();

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
