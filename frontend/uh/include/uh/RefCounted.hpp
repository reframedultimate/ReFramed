#pragma once

#include "uh/config.hpp"

#ifndef DEBUG
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

    int refs() const { return refs_; }

#ifndef DEBUG
    static int total() { return total_.load(); }
#endif

protected:
    virtual void seppuku();

private:
    int refs_;

#ifndef DEBUG
    static std::atomic<int> total_;
#endif
};

}
