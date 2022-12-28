#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Hashers.hpp"
#include <limits>
#include <cassert>

namespace rfcommon {

class RFCOMMON_PUBLIC_API HashMapAlloc
{
public:
    template <typename T>
    char* allocate(size_t count) { return allocate(count, sizeof(T)); }
    static char* allocate(size_t count, size_t size);
    static void deallocate(char* p);
};

template <typename K>
struct CompareEqual
{
    bool operator()(const K& a, const K& b) const { return a == b; }
};

template <typename K, typename V, typename Hasher=Hasher<K>, typename CompareEqual=CompareEqual<K>, typename S=int32_t>
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
        const KeyValueRef* operator->() const { return this; }

    private:
        K& key_;
        V& value_;
    };

    class Iterator
    {
    public:
        Iterator(TableContainer& table, K* keys, V* values, S offset)
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
            if (pos_ < table_.count())
            {
                ++pos_;
                while (pos_ < table_.count())
                {
                    if (table_[pos_] != UNUSED && table_[pos_] != RIP)
                        break;
                    ++pos_;
                }
            }

            return *this;
        }
        Iterator operator++(int)
        {
            Iterator tmp(*this);
            operator++();
            return tmp;
        }

        Iterator& operator+=(int rhs)
        {
            while (rhs--)
                operator++();
            return *this;
        }

        Iterator operator+(int rhs)
        {
            Iterator tmp(*this);
            tmp += rhs;
            return tmp;
        }

        inline bool operator==(const Iterator& rhs) const { return pos_ == rhs.pos_; }
        inline bool operator!=(const Iterator& rhs) const { return !operator==(rhs); }

    private:
        friend class HashMap;
        TableContainer& table_;
        K* keys_;
        V* values_;
        S pos_;
    };

    class ConstIterator
    {
    public:
        ConstIterator(const TableContainer& table, const K* keys, const V* values, S offset)
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
            if (pos_ < table_.count())
            {
                ++pos_;
                while (pos_ < table_.count())
                {
                    if (table_[pos_] != UNUSED && table_[pos_] != RIP)
                        break;
                    ++pos_;
                }
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
        const TableContainer& table_;
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
        : table_(TableContainer::makeResized(nextPowerOf2(initialTableSize)))
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
        K* newKeys = reinterpret_cast<K*>(allocate<K>(table_.count()));
        V* newValues = reinterpret_cast<V*>(allocate<V>(table_.count()));
        auto newTable = Vector<H, S>::makeResized(table_.count());

        for (S oldPos = 0; oldPos != table_.count(); ++oldPos)
        {
            if (table_[oldPos] == UNUSED || table_[oldPos] == RIP)
                continue;

            H hash = table_[oldPos];
            S newPos = hash & (newTable.count() - 1);
            S i = 0;
            while (newTable[newPos] != UNUSED)
            {
                i++;
                newPos += i;
                newPos = newPos & (newTable.count() - 1);
            }

            newTable[newPos] = hash;
            new (newKeys + newPos) K(std::move(keys_[oldPos]));
            new (newValues + newPos) V(std::move(values_[oldPos]));
            keys_[oldPos].~K();
            values_[oldPos].~V();
        }

        deallocate(reinterpret_cast<char*>(keys_));
        deallocate(reinterpret_cast<char*>(values_));

        table_ = std::move(newTable);
        keys_ = newKeys;
        values_ = newValues;
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
                CompareEqual c;
                if (c(keys_[pos], key))
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
     * an interator to the new insertion. If the key exists, returns end()
     * and nothing is inserted.
     */
    template <typename KK, typename VV>
    Iterator insertIfNew(KK&& key, VV&& value)
    {
        if (table_.count() == 0)
            resize(128);
        else if (count_ * 100 / table_.count() >= 70)
            resize(table_.count() * 2);

        H hash;
        S pos;
        if (findFreeInsertSlot(key, hash, pos) == false)
            return end();

        table_[pos] = hash;
        construct(keys_ + pos, std::forward<KK&&>(key));
        construct(values_ + pos, std::forward<VV&&>(value));
        count_++;
        return Iterator(table_, keys_, values_, pos);
    }

    /*!
     * \brief Inserts the key-value pair if the key exists, overwriting
     * the existing key and value. Returns end() if the key did not
     * exist, returns an iterator to the updated pair if it did exist.
     */
    template <typename KK, typename VV>
    Iterator insertIfExists(KK&& key, VV&& value)
    {
        H hash;
        S pos;
        if (findFreeInsertSlot(key, hash, pos))
            return end();

        keys_[pos].~K();
        values_[pos].~V();
        construct(keys_ + pos, std::forward<KK&&>(key));
        construct(values_ + pos, std::forward<VV&&>(value));
        return Iterator(table_, keys_, values_, pos);
    }

    /*!
     * \brief Inserts the key-value pair, regardless of whether it already
     * existed or not. Returns an iterator to the updated (or newly inserted)
     * key-value pair.
     */
    template <typename KK, typename VV>
    Iterator insertAlways(KK&& key, VV&& value)
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
        else
        {
            keys_[pos].~K();
            values_[pos].~V();
            construct(keys_ + pos, std::forward<KK&&>(key));
            construct(values_ + pos, std::forward<VV&&>(value));
        }

        return Iterator(table_, keys_, values_, pos);
    }

    /*!
     * \brief Inserts the key-value pair, but only if it didn't exist yet.
     * Returns
     */
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
    Iterator insertDefaultOrGet(KK&& key)
    {
        return insertOrGet(std::move(key), VV());
    }

    template <typename K1, typename K2>
    Iterator reinsert(K1&& oldKey, K2&& newKey)
    {
        if (table_.count() == 0)
            return end();

        S oldPos = findImpl(oldKey);
        if (oldPos == table_.count())
            return end();

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

    bool erase(const K& key)
    {
        S pos = findImpl(key);
        if (pos != table_.count())
        {
            count_--;
            table_[pos] = RIP;
            keys_[pos].~K();
            values_[pos].~V();
            return true;
        }

        return false;
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

    V& find(const K& key, V& defaultValue)
    {
        S pos = findImpl(key);
        if (pos == table_.count())
            return defaultValue;
        return values_[pos];
    }

    const V& find(const K& key, const V& defaultValue) const
    {
        S pos = findImpl(key);
        if (pos == table_.count())
            return defaultValue;
        return values_[pos];
    }

    bool exists(const K& key) const
    {
        return find(key) != end();
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
