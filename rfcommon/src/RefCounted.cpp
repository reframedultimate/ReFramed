#include "rfcommon/RefCounted.hpp"
#include <cassert>

namespace rfcommon {

#ifdef RFCOMMON_REFCOUNTED_DEBUG
std::atomic<int> RefCounted::total_;
#endif

// ----------------------------------------------------------------------------
RefCounted::RefCounted()
    : refs_(0)
{
#ifdef RFCOMMON_REFCOUNTED_DEBUG
    total_++;
#endif
}

// ----------------------------------------------------------------------------
RefCounted::~RefCounted()
{
    assert(refs_ == 0);
    refs_ = -1;
#ifdef RFCOMMON_REFCOUNTED_DEBUG
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
void RefCounted::decRefNoSeppuku()
{
    assert(refs_ > 0);
    refs_--;
}

// ----------------------------------------------------------------------------
void RefCounted::touchRef()
{
    incRef();
    decRef();
}

// ----------------------------------------------------------------------------
void RefCounted::seppuku()
{
    delete this;
}

}
