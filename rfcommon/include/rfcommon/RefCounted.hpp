#pragma once

#include "rfcommon/config.hpp"

#ifdef RFCOMMON_REFCOUNTED_DEBUG
#   include <atomic>
#endif

namespace rfcommon {

class RFCOMMON_PUBLIC_API RefCounted
{
public:
    RefCounted();
    virtual ~RefCounted();

    RefCounted(const RefCounted&) = delete;
    RefCounted& operator=(const RefCounted&) = delete;

    void incRef();
    void decRef();
    void decRefNoSeppuku();
    void touchRef();

    int refs() const { return refs_; }

#ifdef RFCOMMON_REFCOUNTED_DEBUG
    static int total() { return total_.load(); }
#endif

protected:
    virtual void seppuku();

private:
    int refs_;

#ifdef RFCOMMON_REFCOUNTED_DEBUG
    static std::atomic<int> total_;
#endif
};

}
