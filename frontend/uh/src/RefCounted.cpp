#include "uh/RefCounted.hpp"
#include <cassert>

namespace uh {

#ifndef DEBUG
std::atomic<int> RefCounted::total_;
#endif

// ----------------------------------------------------------------------------
RefCounted::RefCounted()
    : refs_(0)
{
#ifndef DEBUG
    total_++;
#endif
}

// ----------------------------------------------------------------------------
RefCounted::~RefCounted()
{
    assert(refs_ == 0);
    refs_ = -1;
#ifndef DEBUG
    total_--;
#endif
}

// ----------------------------------------------------------------------------
void RefCounted::incRef()
{
    assert(refs_ >= 0);
    refs_++;
}

// ----------------------------------------------------------------------------
void RefCounted::decRef()
{
    assert(refs_ > 0);
    refs_--;
    if (refs_ == 0)
        seppuku();
}

// ----------------------------------------------------------------------------
void RefCounted::seppuku()
{
    delete this;
}

}
