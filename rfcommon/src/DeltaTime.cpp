#include "rfcommon/DeltaTime.hpp"
#include "rfcommon/Profiler.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
DeltaTime DeltaTime::fromMillis(Type value) 
{
    NOPROFILE();
 
    DeltaTime dt; 
    dt.value_ = value; 
    return dt; 
}

// ----------------------------------------------------------------------------
DeltaTime::~DeltaTime() 
{}

// ----------------------------------------------------------------------------
DeltaTime::DeltaTime() 
    : value_(0)
{}

// ----------------------------------------------------------------------------
DeltaTime::DeltaTime(Type value)
    : value_(value)
{}

}
