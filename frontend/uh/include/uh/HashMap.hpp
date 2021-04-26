#pragma once

#include "uh/config.hpp"
#include "uh/Vector.hpp"
#include <limits>

namespace uh {

static inline uint32_t hash32_jenkins_oaat(const void* key, int len)
{
    uint32_t hash = 0;
    for(int i = 0; i != len; ++i)
    {
        hash += *(static_cast<const uint8_t*>(key) + i);
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 1);
    hash += (hash << 15);
    return hash;
}

static inline uint32_t hash32_combine(uint32_t lhs, uint32_t rhs)
{
    lhs ^= rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2);
    return lhs;
}

template <typename T, typename H=uint32_t>
struct HashMapHasher
{
    typedef H HashType;
};

template <typename H>
struct HashMapHasher<uint8_t, H>
{
    typedef uint32_t HashType;
    HashType operator()(uint8_t value) {
        return hash32_combine(
            static_cast<uint32_t>(value) << 0,
            hash32_combine(
                static_cast<uint32_t>(value) << 8,
                hash32_combine(
                    static_cast<uint32_t>(value) << 16,
                    static_cast<uint32_t>(value) << 24
                )
            )
        );
    }
};

template <typename H>
struct HashMapHasher<uint16_t, H>
{
    typedef uint32_t HashType;
    HashType operator()(uint16_t value) {
        return hash32_combine(
            static_cast<HashType>(value) << 0,
            static_cast<HashType>(value) << 16
        );
    }
};

template <typename H>
struct HashMapHasher<uint32_t, H>
{
    typedef uint32_t HashType;
    HashType operator()(uint32_t value) {
        return hash32_jenkins_oaat(&value, 4);
    }
};

template <typename H>
struct HashMapHasher<int, H>
{
    typedef H HashType;
    H operator()(int value) { return value; }
};

template <typename K, typename V, typename Hasher=HashMapHasher<K>, typename S=int32_t>
class HashMap
{
    typedef typename Hasher::HashType H;

    enum SlotState
    {
        UNUSED = 0,
        RIP
    };

    H hashWrapper(const K& key) const
    {
        Hasher h;
        H hash = h(key);
        if (hash == UNUSED || hash == RIP)
            return 2;
        return hash;
    }

public:
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

    class Iterator
    {
    public:
        Iterator(Vector<H, S>& table, Vector<K, S>& keys, Vector<V, S>& values, S offset)
            : table_(table)
            , keys_(keys)
            , values_(values)
            , pos_(offset)
        {}

        KeyValueRef operator*() { return KeyValueRef(keys_[pos_], values_[pos_]); }
        KeyValueRef operator->() { return KeyValueRef(keys_[pos_], values_[pos_]); }

        Iterator& operator++()
        {
            while (pos_ != table_.count())
            {
                ++pos_;
                if (table_[pos_] != UNUSED && table_[pos_] != RIP)
                    break;
            }

            return *this;
        }
        Iterator operator++(int)
        {
            Iterator tmp(*this);
            operator++();
            return tmp;
        }

        inline bool operator==(const Iterator& rhs) const { return pos_ == rhs.pos_; }
        inline bool operator!=(const Iterator& rhs) const { return !operator==(rhs); }

    private:
        Vector<H, S>& table_;
        Vector<K, S>& keys_;
        Vector<V, S>& values_;
        S pos_;
    };

    class ConstIterator
    {
    public:
        ConstIterator(const Vector<H, S>& table, const Vector<K, S>& keys, const Vector<V, S>& values, S offset)
            : table_(table)
            , keys_(keys)
            , values_(values)
            , pos_(offset)
        {}

        ConstKeyValueRef operator*() const { return ConstKeyValueRef(keys_[pos_], values_[pos_]); }
        ConstKeyValueRef operator->() const { return ConstKeyValueRef(keys_[pos_], values_[pos_]); }

        ConstIterator& operator++()
        {
            while (pos_ != table_.count())
            {
                ++pos_;
                if (table_[pos_] != UNUSED && table_[pos_] != RIP)
                    break;
            }

            return *this;
        }
        ConstIterator operator++(int)
        {
            Iterator tmp(*this);
            operator++();
            return tmp;
        }

        inline bool operator==(const ConstIterator& rhs) const { return pos_ == rhs.pos_; }
        inline bool operator!=(const ConstIterator& rhs) const { return !operator==(rhs); }

    private:
        const Vector<H, S>& table_;
        const Vector<K, S>& keys_;
        const Vector<V, S>& values_;
        S pos_;
    };

    static S nextPowerOf2(S n)
    {
        S p = 1;
        while (p < n)
            p *= 2;
        return p;
    }

    HashMap(S initialTableSize)
        : table_(initialTableSize)
        , keys_(initialTableSize)
        , values_(initialTableSize)
        , count_(0)
    {}

    void resize(S newSize)
    {
        table_.resize(newSize);
        keys_.resize(newSize);
        values_.resize(newSize);

        rehash();
    }

public:
    HashMap()
        : HashMap(128)
    {}

    HashMap(const HashMap& other)
        : table_(other.table_)
        , keys_(other.keys_)
        , values_(other.values_)
        , count_(0)
    {}

    HashMap(HashMap&& other)
        : HashMap(0)
    {
        swap(*this, other);
    }

    friend void swap(HashMap& first, HashMap& second) noexcept
    {
        using std::swap;
        swap(first.table_, second.table_);
        swap(first.keys_, second.keys_);
        swap(first.values_, second.values_);
        swap(first.count_, second.count_);
    }

    HashMap& operator=(HashMap other)
    {
        swap(*this, other);
        return *this;
    }

    void rehash()
    {
        Vector<H, S> newTable(table_.count());

        for (S it = 0; it != table_.count(); ++it)
        {
            if (table_[it] == UNUSED || table_[it] == RIP)
                continue;

            H hash = table_[it];
            S pos = hash % newTable.count();
            S i = 0;
            S lastTombtone = newTable.count();
            while (newTable[pos] != UNUSED)
            {
                if (newTable[pos] == hash && newTable[pos] == RIP)
                    lastTombtone = pos;
                i++;
                pos += i;
                pos = pos % newTable.count();
            }

            if (lastTombtone != newTable.count())
                pos = lastTombtone;

            newTable[pos] = hash;
            if (pos != it)
            {
                std::swap(keys_[it], keys_[pos]);
                std::swap(values_[it], values_[pos]);
            }
        }

        table_ = std::move(newTable);
    }

    Iterator insertOrGet(const K& key, const V& value)
    {
        if (count_ * 100 / table_.count() >= 70)
            resize(table_.count() * 2);

        H hash = hashWrapper(key);
        S pos = hash % table_.count();
        S i = 0;
        S lastTombtone = table_.count();

        while (table_[pos] != UNUSED)
        {
            // If the same hash already exists in this slot, and isn't the
            // result of a hash collision (which we can verify by comparing the
            // origin keys), then we can conclude this key was already inserted
            if (table_[pos] == hash)
            {
                if (keys_[pos] == key)
                    return Iterator(table_, keys_, values_, pos);
            }
            else
            {
                if (table_[pos] == RIP)
                    lastTombtone = pos;
            }

            // Quadratic probing following p(K,i)=(i^2+i)/2. If the hash table
            // size is a power of two, this will visit every slot
            i++;
            pos += i;
            pos = pos % table_.count();
        }

        // It's safe to insert new values at the end of a probing sequence
        if (lastTombtone != table_.count())
            pos = lastTombtone;

        table_[pos] = hash;
        keys_[pos] = key;
        values_[pos] = value;
        count_++;

        return Iterator(table_, keys_, values_, pos);
    }

    void insertReplace(const K& key, const V& value)
    {
    }

    Iterator erase(const K& key)
    {
        S pos = findImpl(key);
        if (pos != count_)
        {
            count_--;
            table_[pos] = RIP;
        }
        return Iterator(table_, keys_, values_, pos);
    }

private:
    S findImpl(const K& key) const
    {
        H hash = hashWrapper(key);
        S pos = hash % table_.count();
        S i = 0;
        while (true)
        {
            if (table_[pos] == hash)
            {
                if (keys_[pos] == key)
                    break;
            }
            else
            {
                if (table_[pos] == UNUSED)
                    return count_;
            }

            // Quadratic probing following p(K,i)=(i^2+i)/2. If the hash table
            // size is a power of two, this will visit every slot
            i++;
            pos += i;
            pos = pos % table_.count();
        }

        return pos;
    }

public:
    Iterator find(const K& key)
    {
        return Iterator(table_, keys_, values_, findImpl(key));
    }

    ConstIterator find(const K& key) const
    {
        return ConstIterator(table_, keys_, values_, findImpl(key));
    }

private:
    S findBeginPos() const
    {
        if (count_ == 0)
            return table_.count();

        for (S pos = 0; pos != table_.count(); ++pos)
            if (table_[pos] != UNUSED && table_[pos] != RIP)
                return pos;

        return table_.count();
    }

public:
    S count() const { return count_; }
    Iterator begin() { return Iterator(table_, keys_, values_, findBeginPos()); }
    ConstIterator begin() const { return ConstIterator(table_, keys_, values_, findBeginPos()); }
    Iterator end() { return Iterator(table_, keys_, values_, table_.count()); }
    ConstIterator end() const { return ConstIterator(table_, keys_, values_, table_.count()); }

private:
    Vector<H, S> table_;
    Vector<K, S> keys_;
    Vector<V, S> values_;
    S count_;
};

}
