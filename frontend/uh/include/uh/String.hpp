#pragma once

#include "uh/Vector.hpp"
#include <cstring>
#include <cstdlib>

namespace uh {

template <int N, typename S=int32_t>
class SmallString : protected SmallVector<char, N+1, S>
{
public:
    char* begin() { return this->begin_; }
    char* end() { return this->begin_ + this->count_ - 1; }
    const char* begin() const { return this->begin_; }
    const char* end() const { return this->begin_ + this->count_ - 1; }
    char* data() { return this->begin_; }
    const char* data() const { return this->begin_; }
    const char* cStr() const { return this->begin_; }
    S count() const { return this->count_ - 1; }
    const S length() const { return this->count_ - 1; }
    S capacity() const { return this->capacity_; }

    char& operator[](S i) { return this->begin_[i]; }
    const char& operator[](S i) const { return this->begin_[i]; }
    char& at(S pos) { return this->begin_[pos]; }
    const char& at(S pos) const { return this->begin_[pos]; }

    SmallString()
        : SmallVector<char, N+1, S>(1)
    {
        this->begin_[0] = '\0';
    }

    SmallString(S resizeCount)
        : SmallVector<char, N+1, S>(resizeCount + 1)
    {}

    SmallString(const char* data, S len)
        : SmallVector<char, N+1, S>(len + 1)
    {
        std::memcpy(this->begin_, data, len);
        this->begin_[len] = '\0';
        this->count_ = len + 1;
    }

    SmallString(const char* cStr)
        : SmallString(cStr, strlen(cStr))
    {}

    SmallString(const SmallString& other)
        : SmallString(other.data(), other.length())
    {
    }

    SmallString(SmallString&& other)
        : SmallString()
    {
        swap(*this, other);
    }

    SmallString& operator=(SmallString other)
    {
        swap(*this, other);
        return *this;
    }

    template <int N2>
    SmallString& operator+=(const SmallString<N2, S>& rhs)
    {
        this->count_--;  // string length without null
        this->ensureCapacity(this->count_ + rhs.length() + 1, this->count_, rhs.length() + 1);
        std::memcpy(this->begin_ + this->count_, rhs.data(), rhs.length() + 1);
        this->count_ += rhs.length() + 1;
        return *this;
    }

    template <int N2, typename S2>
    friend inline bool operator==(const SmallString<N, S>& lhs, const SmallString<N2, S2>& rhs)
    {
        return lhs.count_ == rhs.count_ && (memcmp(lhs.begin_, rhs.begin_, lhs.count_) == 0);
    }

    template <int N2, typename S2>
    friend inline bool operator!=(const SmallString<N, S>& lhs, const SmallString<N2, S2>& rhs)
    {
        return !operator==(lhs, rhs);
    }

    friend inline bool operator==(const SmallString<N, S>& lhs, const char* rhs)
    {
        return strcmp(lhs.begin_, rhs) == 0;
    }
};

template <int N1, int N2, typename S>
inline SmallString<N1, S> operator+(SmallString<N1, S> lhs, const SmallString<N2, S>& rhs)
{
    lhs += rhs;
    return lhs;
}
template <int N, typename S>
inline SmallString<N, S> operator+(SmallString<N, S> lhs, const char* rhs)
{
    lhs += SmallString<N, S>(rhs);
    return lhs;
}

using String = SmallString<0>;

}
