#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/String.hpp"
#include <limits>
#include <cassert>

namespace rfcommon {

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

template <>
struct HashMapHasher<uint8_t, uint32_t>
{
    typedef uint32_t HashType;
    HashType operator()(uint8_t value) const {
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

template <>
struct HashMapHasher<uint16_t, uint32_t>
{
    typedef uint32_t HashType;
    HashType operator()(uint16_t value) const {
        return hash32_combine(
            static_cast<HashType>(value) << 0,
            static_cast<HashType>(value) << 16
        );
    }
};

template <>
struct HashMapHasher<uint32_t, uint32_t>
{
    typedef uint32_t HashType;
    HashType operator()(uint32_t value) const {
        return hash32_jenkins_oaat(&value, 4);
    }
};

template <>
struct HashMapHasher<int, uint32_t>
{
    typedef uint32_t HashType;
    uint32_t operator()(int value) const {
        return hash32_jenkins_oaat(&value, 4);
    }
};

template <>
struct HashMapHasher<String, uint32_t>
{
    typedef uint32_t HashType;
    uint32_t operator()(const String& s) const {
        return hash32_jenkins_oaat(s.data(), s.length());
    }
};

template <typename P>
struct HashMapHasher<P*, uint32_t>
{
    typedef uint32_t HashType;
    uint32_t operator()(P* p) const {
        return hash32_combine(
            static_cast<uint32_t>(reinterpret_cast<size_t>(p) / sizeof(P*)),
            static_cast<uint32_t>((reinterpret_cast<size_t>(p) / sizeof(P*)) >> 32)
        );
    }
};

class RFCOMMON_PUBLIC_API HashMapAlloc
{
public:
    template <typename T>
    char* allocate(size_t count) { return allocate(count, sizeof(T)); }
    static char* allocate(size_t count, size_t size);
    static void deallocate(char* p);
};

template <typename K, typename V, typename Hasher=HashMapHasher<K>, typename S=int32_t>
class HashMap : private HashMapAlloc
{
    using H = typename Hasher::HashType;
    using TableContainer = Vector<H, S>;

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

    class KeyValueRef
    {
    public:
        KeyValueRef(K& key, V& value) : key_(key), value_(value) {}

        K& key() { return key_; }
        V& value() { return value_; }
        const K& key() const { return key_; }
        const V& value() const { return value_; }

        KeyValueRef* operator->() { return this; }
        const ConstKeyValueRef* operator->() const { return this; }

    private:
        K& key_;
        V& value_;
    };

    class Iterator
    {
    public:
        Iterator(Vector<H, S>& table, K* keys, V* values, S offset)
            : table_(table)
            , keys_(keys)
            , values_(values)
            , pos_(offset)
        {}

        KeyValueRef operator*()
        {
            assert(pos_ != table_.count());
            return KeyValueRef(keys_[pos_], values_[pos_]);
        }

        KeyValueRef operator->()
        {
            assert(pos_ != table_.count());
            return KeyValueRef(keys_[pos_], values_[pos_]);
        }

        ConstKeyValueRef operator*() const
        {
            assert(pos_ != table_.count());
            return ConstKeyValueRef(keys_[pos_], values_[pos_]);
        }

        ConstKeyValueRef operator->() const
        {
            assert(pos_ != table_.count());
            return ConstKeyValueRef(keys_[pos_], values_[pos_]);
        }

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
        friend class HashMap;
        Vector<H, S>& table_;
        K* keys_;
        V* values_;
        S pos_;
    };

    class ConstIterator
    {
    public:
        ConstIterator(const Vector<H, S>& table, const K* keys, const V* values, S offset)
            : table_(table)
            , keys_(keys)
            , values_(values)
            , pos_(offset)
        {}

        ConstKeyValueRef operator*() const
        {
            assert(pos_ != table_.count());
            return ConstKeyValueRef(keys_[pos_], values_[pos_]);
        }

        ConstKeyValueRef operator->() const
        {
            assert(pos_ != table_.count());
            return ConstKeyValueRef(keys_[pos_], values_[pos_]);
        }

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
            ConstIterator tmp(*this);
            operator++();
            return tmp;
        }

        inline bool operator==(const ConstIterator& rhs) const { return pos_ == rhs.pos_; }
        inline bool operator!=(const ConstIterator& rhs) const { return !operator==(rhs); }

    private:
        friend class HashMap;
        const Vector<H, S>& table_;
        const K* keys_;
        const V* values_;
        S pos_;
    };

    static S nextPowerOf2(S n)
    {
        S p = 1;
        while (p < n)
            p *= 2;
        return p;
    }

    void resize(S newCount)
    {
        K* newKeys = reinterpret_cast<K*>(allocate<K>(newCount));
        V* newValues = reinterpret_cast<V*>(allocate<V>(newCount));

        for (S pos = 0; pos != table_.count(); ++pos)
        {
            if (table_[pos] == UNUSED || table_[pos] == RIP)
                continue;

            new (newKeys + pos) K(std::move(keys_[pos]));
            new (newValues + pos) V(std::move(values_[pos]));
            keys_[pos].~K();
            values_[pos].~V();
        }

        if (table_.count())
        {
            deallocate(reinterpret_cast<char*>(keys_));
            deallocate(reinterpret_cast<char*>(values_));
        }

        table_.resize(newCount);
        keys_ = newKeys;
        values_ = newValues;

        rehash();
    }

public:
    HashMap()
        : keys_(nullptr)
        , values_(nullptr)
        , count_(0)
    {}

    HashMap(S initialTableSize)
        : table_(nextPowerOf2(initialTableSize))
        , keys_(nullptr)
        , values_(nullptr)
        , count_(0)
    {
        keys_ = reinterpret_cast<K*>(allocate<K>(table_.count()));
        values_ = reinterpret_cast<V*>(allocate<V>(table_.count()));
    }

    HashMap(const HashMap& other)
        : table_(other.table_)
        , keys_(reinterpret_cast<K*>(allocate<K>(table_.count())))
        , values_(reinterpret_cast<V*>(allocate<V>(table_.count())))
        , count_(other.count_)
    {
        for (S pos = 0; pos != table_.count(); ++pos)
        {
            if (table_[pos] == UNUSED || table_[pos] == RIP)
                continue;

            new (keys_ + pos) K(other.keys_[pos]);
            new (values_ + pos) V(other.values_[pos]);
        }
    }

    HashMap(HashMap&& other)
        : HashMap(0)
    {
        swap(*this, other);
    }

    ~HashMap()
    {
        clear();
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

        for (S oldPos = 0; oldPos != table_.count(); ++oldPos)
        {
            if (table_[oldPos] == UNUSED || table_[oldPos] == RIP)
                continue;

            H hash = table_[oldPos];
            S newPos = hash & (newTable.count() - 1);
            S i = 0;
            S lastTombtone = newTable.count();
            while (newTable[newPos] != UNUSED)
            {
                if (newTable[newPos] == hash && newTable[newPos] == RIP)
                    lastTombtone = newPos;
                i++;
                newPos += i;
                newPos = newPos & (newTable.count() - 1);
            }

            if (lastTombtone != newTable.count())
                newPos = lastTombtone;

            newTable[newPos] = hash;
            if (newPos != oldPos)
            {
                new (keys_ + newPos) K(std::move(keys_[oldPos]));
                new (values_ + newPos) V(std::move(values_[oldPos]));
                keys_[oldPos].~K();
                values_[oldPos].~V();
            }
        }

        table_ = std::move(newTable);
    }

private:
    bool findFreeInsertSlot(const K& key, H& hash, S& pos)
    {
        assert(table_.count());

        S i = 0;
        S lastTombtone = table_.count();
        hash = hashWrapper(key);
        pos = hash & (table_.count() - 1);

        while (table_[pos] != UNUSED)
        {
            // If the same hash already exists in this slot, and isn't the
            // result of a hash collision (which we can verify by comparing the
            // origin keys), then we can conclude this key was already inserted
            if (table_[pos] == hash)
            {
                if (keys_[pos] == key)
                    return false;
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
            pos = pos & (table_.count() - 1);
        }

        // It's safe to insert new values at the end of a probing sequence
        if (lastTombtone != table_.count())
            pos = lastTombtone;

        return true;
    }

    template <typename T, typename U>
    T* construct(T* dst, U&& src, std::enable_if_t<std::is_rvalue_reference<U&&>::value>*_=0)
    {
        (void)_;
        return new (dst) T(std::move(src));
    }

    template <typename T, typename U>
    T* construct(T* dst, U&& src, std::enable_if_t<!std::is_rvalue_reference<U&&>::value>*_=0)
    {
        (void)_;
        return new (dst) T(src);
    }

public:
    /*!
     * \brief If the key does not exist, inserts the key/value pair and returns
     * an interator to the new insertion. If the key does exist, returns end()
     * and nothing is inserted.
     */
    template <typename KK, typename VV>
    Iterator insertNew(KK&& key, VV&& value)
    {
        if (table_.count() == 0)
            resize(128);
        else if (count_ * 100 / table_.count() >= 70)
            resize(table_.count() * 2);

        H hash;
        S pos;
        if (findFreeInsertSlot(key, hash, pos))
        {
            table_[pos] = hash;
            construct(keys_ + pos, std::forward<KK&&>(key));
            construct(values_ + pos, std::forward<VV&&>(value));
            count_++;
            return Iterator(table_, keys_, values_, pos);
        }

        return end();
    }

    template <typename KK, typename VV>
    Iterator insertOrGet(KK&& key, VV&& value)
    {
        if (table_.count() == 0)
            resize(128);
        else if (count_ * 100 / table_.count() >= 70)
            resize(table_.count() * 2);

        H hash;
        S pos;
        if (findFreeInsertSlot(key, hash, pos))
        {
            table_[pos] = hash;
            construct(keys_ + pos, std::forward<KK&&>(key));
            construct(values_ + pos, std::forward<VV&&>(value));
            count_++;
        }

        return Iterator(table_, keys_, values_, pos);
    }

    template <typename KK, typename VV>
    Iterator insertReplace(KK&& key, VV&& value)
    {
        if (table_.count() == 0)
            resize(128);
        else if (count_ * 100 / table_.count() >= 70)
            resize(table_.count() * 2);

        H hash;
        S pos;
        if (findFreeInsertSlot(key, hash, pos))
            return end();

        table_[pos] = hash;
        construct(keys_ + pos, std::forward<KK&&>(key));
        construct(values_ + pos, std::forward<VV&&>(value));
        count_++;
        return Iterator(table_, keys_, values_, pos);
    }

    template <typename K1, typename K2>
    Iterator reinsert(K1&& oldKey, K2&& newKey)
    {
        if (table_.count() == 0)
            return end();

        S oldPos = findImpl(oldKey);
        assert(oldPos != table_.count());

        H newHash;
        S newPos;
        if (findFreeInsertSlot(newKey, newHash, newPos) == false)
            return end();

        table_[newPos] = newHash;
        construct(keys_ + newPos, std::forward<K2&&>(newKey));
        construct(values_ + newPos, std::move(values_[oldPos]));

        table_[oldPos] = RIP;
        keys_[oldPos].~K();
        values_[oldPos].~V();
        return Iterator(table_, keys_, values_, newPos);
    }

    S erase(const K& key)
    {
        S pos = findImpl(key);
        if (pos != table_.count())
        {
            count_--;
            table_[pos] = RIP;
            keys_[pos].~K();
            values_[pos].~V();
            return 1;
        }

        return 0;
    }

    Iterator erase(Iterator it)
    {
        S pos = it.pos_;
        assert (pos != table_.count());

        count_--;
        table_[pos] = RIP;
        keys_[pos].~K();
        values_[pos].~V();
        it++;
        return it;
    }

    void clear()
    {
        for (S pos = 0; pos != table_.count(); ++pos)
        {
            if (table_[pos] == UNUSED || table_[pos] == RIP)
                continue;

            keys_[pos].~K();
            values_[pos].~V();
        }

        if (table_.count())
        {
            deallocate(reinterpret_cast<char*>(keys_));
            deallocate(reinterpret_cast<char*>(values_));
            table_.clear();
        }

        count_ = 0;
    }

private:
    S findImpl(const K& key) const
    {
        if (table_.count() == 0)
            return 0;

        H hash = hashWrapper(key);
        S pos = hash & (table_.count() - 1);
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
                    return table_.count();
            }

            // Quadratic probing following p(K,i)=(i^2+i)/2. If the hash table
            // size is a power of two, this will visit every slot
            i++;
            pos += i;
            pos = pos & (table_.count() - 1);
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
    TableContainer table_;
    K* keys_;
    V* values_;
    S count_;
};

}
