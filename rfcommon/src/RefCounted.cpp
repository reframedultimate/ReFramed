#include "rfcommon/RefCounted.hpp"
#include "rfcommon/Profiler.hpp"
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
    NOPROFILE();

    assert(refs_ >= 0);
    refs_++;
}

// ----------------------------------------------------------------------------
void RefCounted::decRef()
{
    NOPROFILE();

    assert(refs_ > 0);
    refs_--;
    if (refs_ == 0)
        seppuku();
}

// ----------------------------------------------------------------------------
void RefCounted::decRefNoSeppuku()
{
    NOPROFILE();

    assert(refs_ > 0);
    refs_--;
}

// ----------------------------------------------------------------------------
void RefCounted::touchRef()
{
    NOPROFILE();

    incRef();
    decRef();
}

// ----------------------------------------------------------------------------
void RefCounted::seppuku()
{
    NOPROFILE();

    delete this;
}

}
