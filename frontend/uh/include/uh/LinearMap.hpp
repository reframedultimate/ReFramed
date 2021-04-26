#pragma once

#include "uh/config.hpp"
#include "uh/Vector.hpp"
#include "uh/MapIterators.hpp"
#include <algorithm>

namespace uh {

template <typename K, typename V, int N, typename S=int32_t>
class SmallLinearMap
{
public:
    typedef SmallVector<K, N, S> Keys;
    typedef SmallVector<V, N, S> Values;
    typedef LinearMapIterator<K, V, N, S> Iterator;
    typedef ConstLinearMapIterator<K, V, N, S> ConstIterator;

public:
    SmallLinearMap()
    {}

    SmallLinearMap(const SmallLinearMap& other)
        : keys_(other.keys_)
        , values_(other.values_)
    {}

    SmallLinearMap(SmallLinearMap&& other)
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

    Iterator insertOrGet(const K& key, const V& value)
    {
        S offset = std::lower_bound(keys_.begin(), keys_.end(), key) - keys_.begin();
        if (keys_[offset] != key)
        {
            keys_.insert(offset, key);
            values_.insert(offset, value);
        }

        return Iterator(keys_, values_, offset);
    }

    Iterator find(const K& key)
    {
        auto it = std::lower_bound(keys_.begin(), keys_.end(), key);
        return Iterator(keys_, values_, it - keys_.begin());
    }

    ConstIterator find(const K& key) const
    {
        auto it = std::lower_bound(keys_.begin(), keys_.end(), key);
        return ConstIterator(keys_, values_, it - keys_.begin());
    }

    S count() { return keys_.count(); }
    const S count() const { return keys_.count(); }
    Iterator begin() { return Iterator(keys_, values_, 0); }
    ConstIterator begin() const { return ConstIterator(keys_, values_, 0); }
    Iterator end() { return Iterator(keys_, values_, count()); }
    ConstIterator end() const { return ConstIterator(keys_, values_, count()); }

private:
    Keys keys_;
    Values values_;
};

}
