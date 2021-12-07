#pragma once

#include "uh/config.hpp"

#ifdef UH_REFCOUNTED_DEBUG
#   include <atomic>
#endif

namespace uh {

class UH_PUBLIC_API RefCounted
{
public:
    RefCounted();
    virtual ~RefCounted();

    RefCounted(const RefCounted&) = delete;
    RefCounted& operator=(const RefCounted&) = delete;

    void incRef();
    void decRef();
    void decRefNoSeppuku();

    int refs() const { return refs_; }

#ifdef UH_REFCOUNTED_DEBUG
    static int total() { return total_.load(); }
#endif

protected:
    virtual void seppuku();

private:
    int refs_;

#ifdef UH_REFCOUNTED_DEBUG
    static std::atomic<int> total_;
#endif
};

}
