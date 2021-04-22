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

    Reference(const Reference<T>& other)
        : ptr_(other.ptr_)
    {
        ref();
    }

    Reference(Reference<T>&& other)
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

    Reference<T>& operator=(Reference<T> rhs)
    {
        swap(*this, rhs);
        return *this;
    }

    template <typename U>
    Reference<T>& operator=(Reference<U> rhs)
    {
        swap(*this, rhs);
        return *this;
    }

    friend void swap(Reference<T>& first, Reference<T>& second) noexcept
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

}
