#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Vector.hpp"
#include <cassert>

namespace rfcommon {

template <typename K, typename V, typename S>
class KeyValueRef
{
public:
    KeyValueRef(K& key, V& value) : key_(key), value_(value) {}

    K& key() { return key_; }
    V& value() { return value_; }

    KeyValueRef* operator->() { return this; }

private:
    K& key_;
    V& value_;
};

template <typename K, typename V, typename S>
class ConstKeyValueRef
{
public:
    ConstKeyValueRef(const K& key, const V& value) : key_(key), value_(value) {}

    const K& key() const { return key_; }
    const V& value() const { return value_; }

    const ConstKeyValueRef* operator->() const { return this; }

private:
    const K& key_;
    const V& value_;
};

template <typename K, typename V, int N, typename S>
class LinearMapIterator
{
public:
    LinearMapIterator(SmallVector<K, N, S>& keys, SmallVector<V, N, S>& values, S offset)
        : keyIt_(keys.begin() + offset)
        , valueIt_(values.begin() + offset)
    {}

    LinearMapIterator(const LinearMapIterator& other)
        : keyIt_(other.keyIt_)
        , valueIt_(other.valueIt_)
    {}

    LinearMapIterator& operator=(LinearMapIterator rhs)
    {
        swap(*this, rhs);
        return *this;
    }

    friend void swap(LinearMapIterator& first, LinearMapIterator& second) noexcept
    {
        using std::swap;
        swap(first.keyIt_, second.keyIt_);
        swap(first.valueIt_, second.valueIt_);
    }

    KeyValueRef<K, V, S> operator*() { return KeyValueRef<K, V, S>(*keyIt_, *valueIt_); }
    KeyValueRef<K, V, S> operator->() { return KeyValueRef<K, V, S>(*keyIt_, *valueIt_); }

    LinearMapIterator& operator++()
    {
        keyIt_++;
        valueIt_++;
        return *this;
    }
    LinearMapIterator operator++(int)
    {
        LinearMapIterator tmp(*this);
        operator++();
        return tmp;
    }

    inline bool operator==(const LinearMapIterator& rhs) const { return keyIt_ == rhs.keyIt_; }
    inline bool operator!=(const LinearMapIterator& rhs) const { return !operator==(rhs); }

private:
    typename SmallVector<K, N, S>::Iterator keyIt_;
    typename SmallVector<V, N, S>::Iterator valueIt_;
};

template <typename K, typename V, int N, typename S>
class ConstLinearMapIterator
{
public:
    ConstLinearMapIterator(const SmallVector<K, N, S>& keys, const SmallVector<V, N, S>& values, S offset)
        : keyIt_(keys.begin() + offset)
        , valueIt_(values.begin() + offset)
    {}

    ConstLinearMapIterator(const ConstLinearMapIterator& other)
        : keyIt_(other.keyIt_)
        , valueIt_(other.valueIt_)
    {}

    ConstLinearMapIterator& operator=(ConstLinearMapIterator rhs)
    {
        swap(*this, rhs);
    }

    friend void swap(ConstLinearMapIterator& first, ConstLinearMapIterator& second) noexcept
    {
        using std::swap;
        swap(first.keyIt_, second.keyIt_);
        swap(first.valueIt_, second.valueIt_);
    }

    ConstKeyValueRef<K, V, S> operator*() const { return ConstKeyValueRef<K, V, S>(*keyIt_, *valueIt_); }
    ConstKeyValueRef<K, V, S> operator->() const { return ConstKeyValueRef<K, V, S>(*keyIt_, *valueIt_); }

    ConstLinearMapIterator& operator++()
    {
        keyIt_++;
        valueIt_++;
        return *this;
    }
    ConstLinearMapIterator operator++(int)
    {
        ConstLinearMapIterator tmp(*this);
        operator++();
        return tmp;
    }

    inline bool operator==(const ConstLinearMapIterator& rhs) const { return keyIt_ == rhs.keyIt_; }
    inline bool operator!=(const ConstLinearMapIterator& rhs) const { return !operator==(rhs); }

private:
    typename SmallVector<K, N, S>::ConstIterator keyIt_;
    typename SmallVector<V, N, S>::ConstIterator valueIt_;
};

}
