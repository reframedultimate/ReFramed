#pragma once

#include <cassert>

namespace rfcommon {

template <typename Entry>
class Deque
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

    /*!
     * \brief Inserts an item into the back of the queue.
     */
    void putBack(Entry* entry)
    {
        assert(entry != nullptr);

        entry->prev = nullptr;
        entry->next = back_;

        if (back_)
            back_->prev = entry;
        else
            front_ = entry;

        count_++;
        back_ = entry;

        assert(front_ != nullptr);
        assert(back_ != nullptr);
    }

    void putFront(Entry* entry)
    {
        assert(entry != nullptr);

        entry->prev = front_;
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
        else
            back_->prev = nullptr;

        count_--;
        return entry;
    }

    /*!
     * \brief Pops an item from the front of the queue.
     */
    Entry* takeFront()
    {
        assert(count_ > 0);

        Entry* entry = front_;

        front_ = front_->prev;
        if (front_ == nullptr)
            back_ = nullptr;
        else
            front_->next = nullptr;

        count_--;
        return entry;
    }

    Entry* peekFront() const
    {
        return front_;
    }

    Entry* peekBack() const
    {
        return back_;
    }

    int count() const
    {
        return count_;
    }

    ConstIterator begin() const { return ConstIterator(back_); }
    ConstIterator end() const { return ConstIterator(nullptr); }

private:
    Entry* front_ = nullptr;
    Entry* back_ = nullptr;
    int count_ = 0;
};

}

