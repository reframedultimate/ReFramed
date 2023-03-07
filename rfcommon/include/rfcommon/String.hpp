#pragma once

#include "rfcommon/Vector.hpp"
#include <cstring>
#include <cstdlib>

#include <cstdio>

namespace rfcommon {

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
    const S length() const { return this->count_ - 1; }
    bool isEmpty() const { return this->count_ == 1; }
    bool notEmpty() const { return this->count_ > 1; }
    S capacity() const { return this->capacity_ - 1; }

    char& operator[](S i) { return this->begin_[i]; }
    const char& operator[](S i) const { return this->begin_[i]; }
    char& at(S pos) { return this->begin_[pos]; }
    const char& at(S pos) const { return this->begin_[pos]; }

    SmallString()
        : SmallVector<char, N+1, S>(SmallVector<char, N+1, S>::makeResized(1))
    {}

    SmallString(S resizeCount)
        : SmallVector<char, N+1, S>(SmallVector<char, N+1, S>::makeResized(resizeCount + 1))
    {}

    SmallString(S resizeCount, char init)
        : SmallVector<char, N+1, S>(SmallVector<char, N+1, S>::makeResized(resizeCount + 1))
    {
        memset(this->begin_, init, this->count_ - 1);
    }

    SmallString(const char* data, S len)
        : SmallVector<char, N+1, S>(SmallVector<char, N+1, S>::makeResized(len + 1))
    {
        std::memcpy(this->begin_, data, len);
    }

    SmallString(const char* cStr)
        : SmallString(cStr, static_cast<S>(strlen(cStr)))
    {}

    SmallString(const SmallString& other)
        : SmallString(other.data(), other.length())
    {}

    SmallString(SmallString&& other) noexcept
        : SmallString()
    {
        swap(*this, other);
    }

    static SmallString decimal(int value)
    {
        // Special case
        if (value == -2147483648)
            return "-2147483648";

        bool isNegative = value < 0;
        if (isNegative)
            value = -value;

        int div = 1;
        int len = 1;
        while (int64_t(div)*10 <= value)
        {
            len++;
            div *= 10;
        }

        SmallString s(len + isNegative, '0');
        char* p = s.begin_;

        if (isNegative)
            *p++ = '-';

        while (1)
        {
            while (value >= div)
            {
                value -= div;
                (*p)++;
            }

            if (value == 0)
                break;

            div /= 10;
            p++;
        }

        return s;
    }

    SmallString& operator=(SmallString other)
    {
        swap(*this, other);
        return *this;
    }

    bool replaceWith(SmallString other)
    {
        if (*this == other)
            return true;
        swap(*this, other);
        return false;
    }

    SmallString swapWith(SmallString other)
    {
        swap(*this, other);
        return other;
    }

    SmallString copy() const
    {
        return *this;
    }

    SmallString& replaceAll(char find, char replace)
    {
        for (auto& c : *this)
            if (c == find)
                c = replace;
        return *this;
    }

    void resize(S count)
    {
        int nullPos = length();  // number of chars without null terminator
        SmallVector<char, N + 1, S>::resize(count + 1);
        if (nullPos > length())
            nullPos = length();
        this->begin_[count] = '\0';
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

    SmallString& operator+=(const char* rhs)
    {
        const S len = std::strlen(rhs);
        this->count_--;  // string length without null
        this->ensureCapacity(this->count_ + len + 1, this->count_, len + 1);
        std::strcpy(this->begin_ + this->count_, rhs);
        this->count_ += len + 1;
        return *this;
    }

    template <int N2, typename S2>
    friend inline bool operator==(const SmallString<N, S>& lhs, const SmallString<N2, S2>& rhs)
    {
        return lhs.count_ == rhs.count_ && (memcmp(lhs.begin_, rhs.begin_, lhs.count_) == 0);
    }

    friend inline bool operator==(const SmallString<N, S>& lhs, const char* rhs)
    {
        return strcmp(lhs.begin_, rhs) == 0;
    }

    template <int N2, typename S2>
    friend inline bool operator<(const SmallString<N, S>& lhs, const SmallString<N2, S2>& rhs)
    {
        return memcmp(lhs.begin_, rhs.begin_, lhs.count_ < rhs.count_ ? lhs.count_ : rhs.count_) < 0;
    }

    friend inline bool operator<(const SmallString<N, S>& lhs, const char* rhs)
    {
        return strcmp(lhs.begin_, rhs) < 0;
    }

    template <int N2, typename S2>
    friend inline bool operator!=(const SmallString<N, S>& lhs, const SmallString<N2, S2>& rhs) { return !operator==(lhs, rhs); }
    friend inline bool operator!=(const SmallString<N, S>& lhs, const char* rhs) { return !operator==(lhs, rhs); }
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
template <int N, typename S>
inline SmallString<N, S> operator+(const char* lhs, SmallString<N, S> rhs)
{
    SmallString<N, S> s(lhs);
    s += SmallString<N, S>(rhs);
    return s;
}

using String = SmallString<15>;

}
