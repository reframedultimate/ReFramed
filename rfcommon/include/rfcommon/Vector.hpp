#pragma once

#include "rfcommon/config.hpp"
#include <cstdint>
#include <cstddef>
#include <cassert>
#include <utility>
#include <new>

namespace rfcommon {

// ----------------------------------------------------------------------------
class RFCOMMON_PUBLIC_API VectorAlloc
{
public:
    template <typename T>
    char* allocate(size_t count) { return allocate(count, sizeof(T)); }
    static char* allocate(size_t count, size_t size);
    static void deallocate(char* p);
};

// ----------------------------------------------------------------------------
template <typename T, typename S>
class VectorBase
{
public:
    typedef T* Iterator;
    typedef const T* ConstIterator;

    Iterator begin() { return begin_; }
    Iterator end() { return begin_ + count_; }
    ConstIterator begin() const { return begin_; }
    ConstIterator end() const { return begin_ + count_; }
    T* data() { return begin_; }
    const T* data() const { return begin_; }
    S count() const { return count_; }
    S capacity() const { return capacity_; }
    S indexOf(const T& value) { return static_cast<S>(&value - begin_); }
    S indexOf(const T* value) { return static_cast<S>(value - begin_); }

    T& operator[](S i) { return begin_[i]; }
    const T& operator[](S i) const { return begin_[i]; }
    T& front() { return begin_[0]; }
    const T& front() const { return begin_[0]; }
    T& back(S offset=1) { return begin_[count_ - offset]; }
    const T& back(S offset=1) const { return begin_[count_ - offset]; }
    T& at(S pos) { return begin_[pos]; }
    const T& at(S pos) const { return begin_[pos]; }

protected:
    VectorBase()
        : begin_(nullptr)
        , count_(0)
        , capacity_(0)
    {}

    VectorBase(T* begin, S capacity)
        : begin_(begin)
        , count_(0)
        , capacity_(capacity)
    {}

    static S nextPowerOf2(S n)
    {
        S p = 1;
        while (p < n)
            p *= 2;
        return p;
    }

    void relocateElementsTo(T* dst, T* begin, T* end)
    {
        if (dst < begin)
        {
            while (begin != end)
            {
                new (dst++) T(std::move(*begin));
                begin->~T();
                begin++;
            }
        }
        else if (dst > begin)
        {
            dst += end - begin;
            while (end != begin)
            {
                end--;
                new (--dst) T(std::move(*end));
                end->~T();
            }
        }
    }

protected:
    T* begin_;
    S count_;
    S capacity_;
};

// ----------------------------------------------------------------------------
template <typename T, int N, typename S=int32_t>
class SmallVector : public VectorBase<T, S>, private VectorAlloc
{
public:
    typedef typename VectorBase<T, S>::Iterator Iterator;
    typedef typename VectorBase<T, S>::ConstIterator ConstIterator;

    SmallVector()
        : VectorBase<T, S>(reinterpret_cast<T*>(buffer_), N)
    {}

    SmallVector(const SmallVector& other)
        : VectorBase<T, S>(
              other.count_ <= N ? reinterpret_cast<T*>(buffer_) : nullptr,
              other.count_ <= N ? N                             : 0
          )
    {
        insertCopy(0, other.begin_, other.end());
    }

    SmallVector(SmallVector&& other) noexcept
        : SmallVector()
    {
        swap(*this, other);
    }

    SmallVector(std::initializer_list<T> l)
        : SmallVector()
    {
        insertCopy(0, l.begin(), l.end());
    }

    ~SmallVector()
    {
        Iterator end = this->end();
        while (end-- != this->begin_)
            end->~T();

        if (this->capacity_ > N)
            deallocate(reinterpret_cast<char*>(this->begin_));
    }

    static SmallVector makeReserved(S count)
    {
        SmallVector v;
        v.reserve(count);
        return v;
    }

    static SmallVector makeResized(S count)
    {
        SmallVector v;
        v.reserve(count);
        v.resize(count);
        return v;
    }

    SmallVector& operator=(SmallVector rhs)
    {
        swap(*this, rhs);
        return *this;
    }

    friend void swap(SmallVector& first, SmallVector& second) noexcept
    {
        using std::swap;

        if (first.capacity_ <= N && second.capacity_ <= N)
        {
            T* firstPtr = first.begin_;
            T* secondPtr = second.begin_;
            for (S idx = 0; idx < first.count_ && idx < second.count_; ++idx)
                swap(*firstPtr++, *secondPtr++);
            while (firstPtr < first.end())
            {
                new (secondPtr++) T(std::move(*firstPtr));
                firstPtr++->~T();
            }
            while (secondPtr < second.end())
            {
                new (firstPtr++) T(std::move(*secondPtr));
                secondPtr++->~T();
            }

            first.begin_ = reinterpret_cast<T*>(first.buffer_);
            second.begin_ = reinterpret_cast<T*>(second.buffer_);
        }
        else if (first.capacity_ <= N)
        {
            // Move elements in small buffer from first into second
            T* firstPtr = first.begin_;
            T* secondPtr = reinterpret_cast<T*>(second.buffer_);
            while (firstPtr != first.end())
            {
                new (secondPtr++) T(std::move(*firstPtr));
                firstPtr++->~T();
            }

            // Move heap buffer to first and update begin pointers
            first.begin_ = second.begin_;
            second.begin_ = reinterpret_cast<T*>(second.buffer_);
        }
        else if (second.capacity_ <= N)
        {
            // Move elements in small buffer from second into first
            T* firstPtr = reinterpret_cast<T*>(first.buffer_);
            T* secondPtr = second.begin_;
            while (secondPtr != second.end())
            {
                new (firstPtr++) T(std::move(*secondPtr));
                secondPtr++->~T();
            }

            // Move heap buffer to second and update begin pointers
            second.begin_ = first.begin_;
            first.begin_ = reinterpret_cast<T*>(first.buffer_);
        }
        else
        {
            swap(first.begin_, second.begin_);
        }

        swap(first.count_, second.count_);
        swap(first.capacity_, second.capacity_);
    }

    void insertCopy(S insertPos, const T* begin, const T* end)
    {
        S insertCount = static_cast<S>(end - begin);
        ensureCapacity(this->count_ + insertCount, insertPos, insertCount);
        T* dst = this->begin_ + insertPos;
        while (begin != end)
            new (dst++) T(*begin++);
        this->count_ += insertCount;
    }

    void insertMove(S insertPos, T* begin, T* end)
    {
        S insertCount = static_cast<S>(end - begin);
        ensureCapacity(this->count_ + insertCount, insertPos, insertCount);
        T* dst = this->begin_ + insertPos;
        while (begin != end)
            new (dst++) T(std::move(*begin++));
        this->count_ += insertCount;
    }

    void insertCopy(T* insertIt, const T* begin, const T* end)
    {
        insertCopy(static_cast<S>(insertIt - this->begin_), begin, end);
    }

    void insertMove(T* insertIt, T* begin, T* end)
    {
        insertMove(static_cast<S>(insertIt - this->begin_), begin, end);
    }

    T& insert(S pos, const T& value)
    {
        insertCopy(this->begin_ + pos, &value, &value + 1);
        return this->at(pos);
    }

    T& insert(T* insertIt, const T& value)
    {
        return insert(static_cast<S>(insertIt - this->begin_), value);
    }

    T& insert(S pos, T&& value)
    {
        insertMove(this->begin_ + pos, &value, &value + 1);
        return this->at(pos);
    }

    T& insert(T* insertIt, T&& value)
    {
        return insert(static_cast<S>(insertIt - this->begin_), std::move(value));
    }

    T& push(const T& value)
    {
        insert(this->count_, value);
        return this->back();
    }

    T& push(T&& value)
    {
        insert(this->count_, std::move(value));
        return this->back();
    }

    template <int N2, typename S2>
    void push(const SmallVector<T, N2, S2>& other)
    {
        insertCopy(this->count_, other.begin(), other.end());
    }

    template <int N2, typename S2>
    void push(SmallVector<T, N2, S2>&& other)
    {
        insertMove(this->count_, other.begin(), other.end());
        other.clear();
    }

    void pop()
    {
        assert(this->count_ > 0);

        this->back().~T();
        this->count_--;
    }

    T popValue()
    {
        assert(this->count_ > 0);

        this->count_--;
        T value = std::move(this->begin_[this->count_]);
        this->begin_[this->count_].~T();
        return value;
    }

    template <typename... Args>
    T& emplace(Args&&... args)
    {
        ensureCapacity(this->count_ + 1, this->count_, 1);
        T* dst = this->begin_ + this->count_;
        new (dst) T(std::forward<Args>(args)...);
        this->count_++;
        return this->back();
    }

    Iterator erase(Iterator it)
    {
        return this->begin() + erase(this->indexOf(it));
    }

    S erase(S pos)
    {
        Iterator it = this->begin() + pos;
        it->~T();
        this->relocateElementsTo(it, it + 1, this->end());
        this->count_--;
        return pos;
    }

    T take(Iterator it)
    {
        return take(this->indexOf(it));
    }

    T take(S pos)
    {
        Iterator it = this->begin() + pos;
        T value(std::move(*it));
        this->relocateElementsTo(it, it + 1, this->end());
        this->count_--;
        return value;
    }

    Iterator findFirst(const T& value)
    {
        Iterator it = this->begin();
        for (; it != this->end(); ++it)
            if (*it == value)
                break;
        return it;
    }

    ConstIterator findFirst(const T& value) const
    {
        ConstIterator it = this->begin();
        for (; it != this->end(); ++it)
            if (*it == value)
                break;
        return it;
    }

    void reserve(S count)
    {
        ensureCapacity(count, this->count_, 0);
    }

    void resize(S count)
    {
        S prevCount = this->count_;
        if (prevCount < count)
        {
            ensureCapacity(count, this->count_, 0);
            while (this->count_ < count)
                emplace();
        }
        else if (prevCount > count)
        {
            while (this->count_ > count)
                pop();

            if (prevCount > N && count > N)
            {
                // Both old and new sizes are on the heap, realloc into smaller
                // buffer
                char* buffer = allocate<T>(count);
                this->relocateElementsTo(reinterpret_cast<T*>(buffer), this->begin_, this->begin_ + count);
                deallocate(reinterpret_cast<char*>(this->begin_));
                this->capacity_ = count;
                this->begin_ = std::launder(reinterpret_cast<T*>(buffer));
            }
            else if (prevCount > N && count <= N)
            {
                // We've shrunk from the heap to the stack, move elements into
                // stack buffer
                this->relocateElementsTo(reinterpret_cast<T*>(buffer_), this->begin_, this->begin_ + count);
                deallocate(reinterpret_cast<char*>(this->begin_));
                this->capacity_ = count;
                this->begin_ = std::launder(reinterpret_cast<T*>(buffer_));
            }
        }
    }

    void clear()
    {
        Iterator end = this->end();
        while (end-- != this->begin_)
            end->~T();

        this->count_ = 0;
    }

    void clearCompact()
    {
        clear();
        if (this->capacity_ > N)
        {
            deallocate(reinterpret_cast<char*>(this->begin_));
            this->capacity_ = N;
            this->begin_ = std::launder(reinterpret_cast<T*>(buffer_));
        }
        else
        {
            this->capacity_ = N;
        }
    }

protected:
    void ensureCapacity(S requiredCapacity, S insertPos, S insertCount)
    {
        if (requiredCapacity > this->capacity_)
        {
            S newCapacity = this->nextPowerOf2(requiredCapacity);
            char* buffer = allocate<T>(newCapacity);

            this->relocateElementsTo(reinterpret_cast<T*>(buffer),
                               this->begin_,
                               this->begin_ + insertPos);
            this->relocateElementsTo(reinterpret_cast<T*>(buffer) + insertPos + insertCount,
                               this->begin_ + insertPos,
                               this->begin_ + this->count_);

            if (this->capacity_ > N)
                deallocate(reinterpret_cast<char*>(this->begin_));

            this->capacity_ = newCapacity;
            this->begin_ = std::launder(reinterpret_cast<T*>(buffer));
        }
        else
        {
            this->relocateElementsTo(this->begin_ + insertPos + insertCount,
                               this->begin_ + insertPos,
                               this->begin_ + this->count_);
        }
    }

private:
    char buffer_[sizeof(T) * N];
};

// ----------------------------------------------------------------------------
template <typename T, typename S=int32_t>
class Vector : public VectorBase<T, S>, private VectorAlloc
{
public:
    typedef typename VectorBase<T, S>::Iterator Iterator;
    typedef typename VectorBase<T, S>::ConstIterator ConstIterator;

    Vector()
        : VectorBase<T, S>()
    {}

    Vector(const Vector& other)
        : Vector()
    {
        insertCopy(0, other.begin_, other.end());
    }

    Vector(Vector&& other) noexcept
        : Vector()
    {
        swap(*this, other);
    }

    Vector(std::initializer_list<T> l)
        : Vector()
    {
        insertCopy(0, l.begin(), l.end());
    }

    template <int N>
    Vector(const SmallVector<T, N>& other)
        : Vector()
    {
        insertCopy(0, other.begin(), other.end());
    }

    Vector(const T* data, S len)
        : VectorBase<T, S>()
    {
        reserve(len);
        insertCopy(this->begin_, data, data + len);
    }

    ~Vector()
    {
        Iterator end = this->end();
        while (end-- != this->begin_)
            end->~T();

        deallocate(reinterpret_cast<char*>(this->begin_));
    }

    static Vector makeReserved(S count)
    {
        Vector v;
        v.reserve(count);
        return v;
    }

    static Vector makeResized(S count)
    {
        Vector v;
        v.reserve(count);
        v.resize(count);
        return v;
    }

    Vector& operator=(Vector rhs)
    {
        swap(*this, rhs);
        return *this;
    }

    friend void swap(Vector& first, Vector& second) noexcept
    {
        using std::swap;
        swap(first.begin_, second.begin_);
        swap(first.count_, second.count_);
        swap(first.capacity_, second.capacity_);
    }

    void insertCopy(S insertPos, const T* begin, const T* end)
    {
        S insertCount = static_cast<S>(end - begin);
        ensureCapacity(this->count_ + insertCount, insertPos, insertCount);
        T* dst = this->begin_ + insertPos;
        while (begin != end)
            new (dst++) T(*begin++);
        this->count_ += insertCount;
    }

    void insertMove(S insertPos, T* begin, T* end)
    {
        S insertCount = static_cast<S>(end - begin);
        ensureCapacity(this->count_ + insertCount, insertPos, insertCount);
        T* dst = this->begin_ + insertPos;
        while (begin != end)
            new (dst++) T(std::move(*begin++));
        this->count_ += insertCount;
    }

    void insertCopy(T* insertIt, const T* begin, const T* end)
    {
        insertCopy(static_cast<S>(insertIt - this->begin_), begin, end);
    }

    void insertMove(T* insertIt, T* begin, T* end)
    {
        insertMove(static_cast<S>(insertIt - this->begin_), begin, end);
    }

    Iterator insert(S pos, const T& value)
    {
        insertCopy(this->begin_ + pos, &value, &value + 1);
        return &this->at(pos);
    }

    Iterator insert(S pos, T&& value)
    {
        insertMove(this->begin_ + pos, &value, &value + 1);
        return &this->at(pos);
    }

    Iterator push(const T& value)
    {
        insert(this->count_, value);
        return &this->back();
    }

    Iterator push(T&& value)
    {
        insert(this->count_, std::move(value));
        return &this->back();
    }

    void push(const Vector& other)
    {
        insertCopy(this->count_, other.begin(), other.end());
    }

    void push(Vector&& other)
    {
        insertMove(this->count_, other.begin(), other.end());
        other.clear();
    }

    void pop()
    {
        assert(this->count_ > 0);

        this->back().~T();
        this->count_--;
    }

    T popValue()
    {
        assert(this->count_ > 0);

        this->count_--;
        T value = std::move(this->begin_[this->count_]);
        this->begin_[this->count_].~T();
        return value;
    }

    template <typename... Args>
    T& emplace(Args&&... args)
    {
        ensureCapacity(this->count_ + 1, this->count_, 1);
        T* dst = this->begin_ + this->count_;
        new (dst) T(std::forward<Args>(args)...);
        this->count_++;
        return this->back();
    }

    Iterator erase(Iterator it)
    {
        return this->begin() + erase(this->indexOf(it));
    }

    S erase(S pos)
    {
        Iterator it = this->begin() + pos;
        it->~T();
        this->relocateElementsTo(it, it + 1, this->end());
        this->count_--;
        return pos;
    }

    T take(Iterator it)
    {
        return take(this->indexOf(it));
    }

    T take(S pos)
    {
        Iterator it = this->begin() + pos;
        T value(std::move(*it));
        this->relocateElementsTo(it, it + 1, this->end());
        this->count_--;
        return value;
    }

    Iterator findFirst(const T& value)
    {
        Iterator it = this->begin();
        for (; it != this->end(); ++it)
            if (*it == value)
                break;
        return it;
    }

    ConstIterator findFirst(const T& value) const
    {
        ConstIterator it = this->begin();
        for (; it != this->end(); ++it)
            if (*it == value)
                break;
        return it;
    }

    void reserve(S count)
    {
        ensureCapacity(count, this->count_, 0);
    }

    void resize(S count)
    {
        S prevCount = this->count_;
        if (prevCount < count)
        {
            ensureCapacity(count, this->count_, 0);
            while (this->count_ < count)
                emplace();
        }
        else if (prevCount > count)
        {
            while (this->count_ > count)
                pop();

            // realloc into smaller buffer
            char* buffer = allocate<T>(count);
            this->relocateElementsTo(reinterpret_cast<T*>(buffer), this->begin_, this->begin_ + count);
            deallocate(reinterpret_cast<char*>(this->begin_));
            this->capacity_ = count;
            this->begin_ = std::launder(reinterpret_cast<T*>(buffer));
        }
    }

    void clear()
    {
        Iterator end = this->end();
        while (end-- != this->begin_)
            end->~T();

        this->count_ = 0;
    }

    void clearCompact()
    {
        clear();
        deallocate(reinterpret_cast<char*>(this->begin_));
        this->begin_ = nullptr;
        this->capacity_ = 0;
    }

protected:
    void ensureCapacity(S requiredCapacity, S insertPos, S insertCount)
    {
        if (requiredCapacity > this->capacity_)
        {
            S newCapacity = this->nextPowerOf2(requiredCapacity);
            char* buffer = allocate<T>(newCapacity);

            this->relocateElementsTo(reinterpret_cast<T*>(buffer),
                               this->begin_,
                               this->begin_ + insertPos);
            this->relocateElementsTo(reinterpret_cast<T*>(buffer) + insertPos + insertCount,
                               this->begin_ + insertPos,
                               this->begin_ + this->count_);

            if (this->begin_)
                deallocate(reinterpret_cast<char*>(this->begin_));

            this->capacity_ = newCapacity;
            this->begin_ = reinterpret_cast<T*>(buffer);
        }
        else
        {
            this->relocateElementsTo(this->begin_ + insertPos + insertCount,
                               this->begin_ + insertPos,
                               this->begin_ + this->count_);
        }
    }
};

}

