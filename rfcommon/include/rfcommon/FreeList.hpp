#pragma once

namespace rfcommon {

template <typename Entry>
class FreeList
{
public:
    FreeList()
        : first_(nullptr)
    {}

    FreeList(const FreeList&) = delete;
    FreeList(FreeList&&) = delete;
    FreeList& operator=(const FreeList&) = delete;
    FreeList& operator=(FreeList&&) = delete;

    void put(Entry* entry)
    {
        entry->next = first_;
        first_ = entry;
    }

    Entry* take()
    {
        Entry* entry = first_;
        if (first_)
            first_ = entry->next;
        return entry;
    }

    int count() const
    {
        const Entry* e = first_;
        int i = 0;
        while (e)
        {
            i++;
            e = e->next;
        }

        return i;
    }

private:
    Entry* first_;
};

template <typename Entry, int N>
class FlatFreeList : public FreeList<Entry>
{
public:
    FlatFreeList()
    {
        for (int i = N - 1; i >= 0; --i)
            FreeList<Entry>::put(&entries_[i]);
    }

    Entry* entries() const { return entries_; }
    int capacity() const { return N; }

private:
    Entry entries_[N];
};

}
