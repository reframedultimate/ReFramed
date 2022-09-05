#pragma once

#include <cassert>

namespace rfcommon {

template <typename Entry>
class Queue
{
public:
    class ConstIterator
    {
    public:
        ConstIterator(Entry* first) : current_(first) {}

        ConstIterator& operator++()
        {
            current_ = current_->next;
            return *this;
        }

        ConstIterator operator++(int)
        {
            ConstIterator tmp(*this);
            operator++();
            return tmp;
        }

        const Entry* operator*() const { return current_; }
        const Entry* operator->() const { return current_; }

        inline bool operator==(const ConstIterator& rhs) const { return current_ == rhs.current_; }
        inline bool operator!=(const ConstIterator& rhs) const { return current_ != rhs.current_; }

    private:
        Entry* current_;
    };

    void putFront(Entry* entry)
    {
        assert(entry != nullptr);

        entry->next = nullptr;

        if (front_)
            front_->next = entry;
        else
            back_ = entry;

        count_++;
        front_ = entry;

        assert(front_ != nullptr);
        assert(back_ != nullptr);
    }

    Entry* takeBack()
    {
        assert(count_ > 0);

        Entry* entry = back_;

        back_ = back_->next;
        if (back_ == nullptr)
            front_ = nullptr;

        count_--;
        return entry;
    }

    ConstIterator begin() const { return ConstIterator(back_); }
    ConstIterator end() const { return ConstIterator(nullptr); }

    Entry* peekFront() const { return front_; }
    Entry* peekback() const { return back_; }
    int count() const { return count_; }

private:
    Entry* front_ = nullptr;
    Entry* back_ = nullptr;
    int count_ = 0;
};

}
