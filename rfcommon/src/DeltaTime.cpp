#include "rfcommon/DeltaTime.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
DeltaTime DeltaTime::fromMillis(Type value) 
{ 
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
