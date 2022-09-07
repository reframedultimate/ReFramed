#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/MapIterators.hpp"
#include <algorithm>

namespace rfcommon {

template <typename K, typename V, int N, typename S=int32_t>
class SmallLinearMap
{
public:
    typedef SmallVector<K, N, S> KeyContainer;
    typedef SmallVector<V, N, S> ValueContainer;
    typedef LinearMapIterator<K, V, N, S> Iterator;
    typedef ConstLinearMapIterator<K, V, N, S> ConstIterator;

public:
    SmallLinearMap()
    {}

    SmallLinearMap(const SmallLinearMap& other)
        : keys_(other.keys_)
        , values_(other.values_)
    {}

    SmallLinearMap(SmallLinearMap&& other) noexcept
        : SmallLinearMap()
    {
        swap(*this, other);
    }

    SmallLinearMap& operator=(SmallLinearMap rhs)
    {
        swap(*this, rhs);
        return *this;
    }

    friend void swap(SmallLinearMap& first, SmallLinearMap& second)
    {
        using std::swap;
        swap(first.keys_, second.keys_);
        swap(first.values_, second.values_);
    }

    Iterator insertIfNew(const K& key, const V& value)
    {
        S offset = static_cast<S>(std::lower_bound(keys_.begin(), keys_.end(), key) - keys_.begin());
        if (offset == keys_.count() || keys_[offset] != key)
        {
            keys_.insert(offset, key);
            values_.insert(offset, value);
        }

        return end();
    }

    Iterator insertOrGet(const K& key, const V& value)
    {
        S offset = static_cast<S>(std::lower_bound(keys_.begin(), keys_.end(), key) - keys_.begin());
        if (offset == keys_.count() || keys_[offset] != key)
        {
            keys_.insert(offset, key);
            values_.insert(offset, value);
        }

        return Iterator(keys_, values_, offset);
    }

    Iterator findKey(const K& key)
    {
        S offset = static_cast<S>(std::lower_bound(keys_.begin(), keys_.end(), key) - keys_.begin());
        if (offset == keys_.count() || keys_[offset] != key)
            return end();
        return Iterator(keys_, values_, offset);
    }

    ConstIterator findKey(const K& key) const
    {
        S offset = static_cast<S>(std::lower_bound(keys_.begin(), keys_.end(), key) - keys_.begin());
        if (offset == keys_.count() || keys_[offset] != key)
            return end();
        return ConstIterator(keys_, values_, offset);
    }

    Iterator findValue(const V& value)
    {
        S i = 0;
        for (; i != values_.count(); ++i)
            if (values_[i] == value)
                break;
        return Iterator(keys_, values_, i);
    }

    ConstIterator findValue(const V& value) const
    {
        S i = 0;
        for (; i != values_.count(); ++i)
            if (values_[i] == value)
                break;
        return ConstIterator(keys_, values_, i);
    }

    void clear()
    {
        keys_.clear();
        values_.clear();
    }

    void clearCompact()
    {
        keys_.clearCompact();
        values_.clearCompact();
    }

    S count() { return keys_.count(); }
    S count() const { return keys_.count(); }
    Iterator begin() { return Iterator(keys_, values_, 0); }
    ConstIterator begin() const { return ConstIterator(keys_, values_, 0); }
    Iterator end() { return Iterator(keys_, values_, count()); }
    ConstIterator end() const { return ConstIterator(keys_, values_, count()); }

private:
    KeyContainer keys_;
    ValueContainer values_;
};

}
