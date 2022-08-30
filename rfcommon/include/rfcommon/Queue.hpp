#pragma once

namespace rfcommon {

template <typename Entry>
class Queue
{
public:
    void putFront(Entry* entry)
    {
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
        {
            back_ = back_->next;
            count_--;
        }
        if (back_ == nullptr)
            front_ = nullptr;

        return entry;
    }

    Entry* peekFront() const { return front_; }
    Entry* peekback() const { return back_; }
    int count() const { return count_; }

private:
    Entry* front_ = nullptr;
    Entry* back_ = nullptr;
    int count_ = 0;
};

}
