#pragma once

#include "uh/config.hpp"
#include <utility>
#include <cassert>

namespace uh {

template <typename T>
class Reference
{
public:
    Reference()
        : ptr_(nullptr)
    {}

    Reference(const Reference& other)
        : ptr_(other.ptr_)
    {
        ref();
    }

    Reference(Reference&& other) noexcept
        : Reference()
    {
        swap(*this, other);
    }

    //! Allow implicit upcasting
    template <typename U>
    Reference(const Reference<U>& rhs)
        : ptr_(rhs.ptr_)
    {
        ref();
    }

    Reference(T* ptr)
        : ptr_(ptr)
    {
        ref();
    }

    ~Reference()
    {
        unref();
    }

    Reference& operator=(Reference rhs)
    {
        swap(*this, rhs);
        return *this;
    }

    template <typename U>
    Reference& operator=(Reference<U> rhs)
    {
        swap(*this, rhs);
        return *this;
    }

    friend void swap(Reference& first, Reference& second) noexcept
    {
        using std::swap;
        swap(first.ptr_, second.ptr_);
    }

    T* operator->() const
    {
        assert(ptr_);
        return ptr_;
    }

    T& operator*() const
    {
        assert(ptr_);
        return *ptr_;
    }

    operator T*() const
    {
        return ptr_;
    }

    bool isNull() const
    {
        return ptr_ == nullptr;
    }

    bool notNull() const
    {
        return ptr_ != nullptr;
    }

    operator bool() const
    {
        return notNull();
    }

    T* get() const
    {
        return ptr_;
    }

    T* steal()
    {
        T* ret = ptr_;
        ptr_ = nullptr;
        return ret;
    }

    T* detach()
    {
        T* ret = ptr_;
        if (ptr_)
        {
            ptr_->decRefNoSeppuku();
            ptr_ = nullptr;
        }
        return ret;
    }

    int refs() const
    {
        return ptr_ ? ptr_->refs() : 0;
    }

    void reset()
    {
        unref();
    }

private:
    void ref()
    {
        if (ptr_)
            ptr_->incRef();
    }

    void unref()
    {
        if (ptr_)
        {
            ptr_->decRef();
            ptr_ = nullptr;
        }
    }

    T* ptr_;
};

// The following ensures that any clients that use uh::Reference will import
// the explicit template instantiations from libuh instead of instantiating
// new versions, causing multiple definition linker errors.
class GameSession;
class DataSetFilter;

extern template class UH_TEMPLATE_API Reference<GameSession>;
extern template class UH_TEMPLATE_API Reference<DataSetFilter>;

}
