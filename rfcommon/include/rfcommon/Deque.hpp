#pragma once

namespace rfcommon {

template <typename Entry>
class Deque
{
public:
    /*!
     * \brief Inserts an item into the back of the queue.
     */
    void putBack(Entry* entry)
    {
        entry->prev = nullptr;
        entry->next = back_;

        if (back_)
            back_->prev = entry;
        else
            front_ = entry;

        count_++;
        back_ = entry;
    }

    void putFront(Entry* entry)
    {
        entry->prev = front_;
        entry->next = nullptr;

        if (front_)
            front_->next = entry;
        else
            back_ = entry;

        count_++;
        front_ = entry;
    }

    Entry* takeBack()
    {
        Entry* entry = back_;

        if (back_)
            back_ = back_->next;
        if (back_ == nullptr)
            front_ = nullptr;

        count_--;
        return entry;
    }

    /*!
     * \brief Pops an item from the front of the queue.
     */
    Entry* takeFront()
    {
        Entry* entry = front_;

        if (front_)
            front_ = front_->prev;
        if (front_ == nullptr)
            back_ = nullptr;

        count_--;
        return entry;
    }

    Entry* peekFront() const
    {
        return front_;
    }

    Entry* peekback() const
    {
        return back_;
    }

    int count() const
    {
        return count_;
    }

private:
    Entry* front_ = nullptr;
    Entry* back_ = nullptr;
    int count_ = 0;
};

}

