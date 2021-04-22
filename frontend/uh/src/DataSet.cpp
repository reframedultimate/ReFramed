#include "uh/DataSet.hpp"
#include "uh/DataPoint.hpp"
#include "uh/Recording.hpp"
#include "uh/PlayerState.hpp"
#include <cassert>
#include <cstdlib>
#include <new>
#include <algorithm>

namespace uh {

// ----------------------------------------------------------------------------
DataSet::DataSet()
    : mem_(nullptr)
    , count_(0)
    , capacity_(0)
{
}

// ----------------------------------------------------------------------------
DataSet::~DataSet()
{
    if (mem_)
        delete[] mem_;
}

// ----------------------------------------------------------------------------
DataSet::DataSet(const DataSet& other)
{
    clear();

    if (other.count_ > 0)
    {
        mem_ = new char[sizeof(DataPoint) * other.capacity_];
        count_ = other.count_;
        capacity_ = other.capacity_;

        char* dst = mem_;
        const DataPoint* src = other.begin();
        int count = other.count_;
        while (count--)
        {
            new (dst) DataPoint(*src++);
            dst += sizeof(DataPoint);
        }
    }
}

// ----------------------------------------------------------------------------
DataSet::DataSet(DataSet&& other)
    : DataSet()
{
    swap(*this, other);
}

// ----------------------------------------------------------------------------
DataSet& DataSet::operator=(DataSet rhs)
{
    swap(*this, rhs);
    return *this;
}

// ----------------------------------------------------------------------------
void swap(DataSet& first, DataSet& second) noexcept
{
    using std::swap;
    swap(first.mem_, second.mem_);
    swap(first.count_, second.count_);
    swap(first.capacity_, second.capacity_);
}

// ----------------------------------------------------------------------------
void DataSet::appendPlayerStatesFromRecording(int player, Recording* recording)
{
    const int playerStates = recording->playerStateCount(player);
    if (playerStates == 0)
        return;

    const int offset = std::lower_bound(begin(), end(),
        DataPoint(recording->playerState(player, 0), recording),
        [](const DataPoint& lhs, const DataPoint& rhs) -> bool {
            return lhs.state().timeStampMs() < rhs.state().timeStampMs();
        }
    ) - begin();

    if (capacity_ < count_ + playerStates)
    {
        capacity_ = capacity_ ? capacity_ : 32;
        while (capacity_ < count_ + playerStates)
           capacity_ *= 2;

        char* mem = new char[capacity_ * sizeof(DataPoint)];
        char* dst = mem;
        for (const DataPoint* src = begin(); src != begin() + offset; ++src)
        {
            new (dst) DataPoint(std::move(*src));
            src->~DataPoint();
            dst += sizeof(DataPoint);
        }
        dst += playerStates;
        for (const DataPoint* src = begin() + offset; src != end(); ++src)
        {
            new (dst) DataPoint(std::move(*src));
            src->~DataPoint();
            dst += sizeof(DataPoint);
        }

        if (mem_)
            delete[] mem_;
        mem_ = mem;
    }

    char* dst = mem_ + offset * sizeof(DataPoint);
    for (int i = 0; i != playerStates; ++i)
    {
        const PlayerState& state = recording->playerState(player, i);
        new (dst) DataPoint(state, recording);
        dst += sizeof(DataPoint);
    }

    count_ += playerStates;
}

// ----------------------------------------------------------------------------
void DataSet::appendPlayerState(Recording* recording, const PlayerState& state)
{
    const int offset = std::lower_bound(begin(), end(), DataPoint(state, recording), [](const DataPoint& lhs, const DataPoint& rhs) -> bool {
        return lhs.state().timeStampMs() < rhs.state().timeStampMs();
    }) - begin();

    if (count_ == capacity_)
    {
        capacity_ = capacity_ ? capacity_ * 2 : 32;
        char* mem = new char[capacity_ * sizeof(DataPoint)];
        char* dst = mem;
        for (const DataPoint* src = begin(); src != begin() + offset; ++src)
        {
            new (dst) DataPoint(std::move(*src));
            src->~DataPoint();
            dst += sizeof(DataPoint);
        }
        dst++;
        for (const DataPoint* src = begin() + offset; src != end(); ++src)
        {
            new (dst) DataPoint(std::move(*src));
            src->~DataPoint();
            dst += sizeof(DataPoint);
        }
        if (mem_)
            delete[] mem_;
        mem_ = mem;

        dst = mem_ + offset * sizeof(DataPoint);
        new (dst) DataPoint(state, recording);
    }
    else
    {
        new (end()) DataPoint(state, recording);
        for (DataPoint* item = end(); item != begin() + offset; --item)
            swap(*item, *(item-1));
    }

    ++count_;
}

// ----------------------------------------------------------------------------
void DataSet::erase(const DataPoint* dp)
{
    assert(begin() <= dp && dp <= end() && dp != nullptr);
    erase(dp - begin());
}

// ----------------------------------------------------------------------------
void DataSet::erase(int idx)
{
    assert(idx < count_ && idx >= 0);
    DataPoint* item = begin() + idx + 1;
    for (; item != end(); ++item)
        swap(*(item-1), *item);
    item->~DataPoint();
    --count_;
}

// ----------------------------------------------------------------------------
void DataSet::clear()
{
    for (auto& dp : *this)
        dp.~DataPoint();
    if (mem_)
        delete[] mem_;
    mem_ = nullptr;
    count_ = 0;
    capacity_ = 0;
}

// ----------------------------------------------------------------------------
int DataSet::count() const
{
    return count_;
}

// ----------------------------------------------------------------------------
const DataPoint* DataSet::begin() const
{
    return static_cast<const DataPoint*>(static_cast<const void*>(mem_));
}

// ----------------------------------------------------------------------------
const DataPoint* DataSet::end() const
{
    return begin() + count_;
}

// ----------------------------------------------------------------------------
DataPoint* DataSet::begin()
{
    return static_cast<DataPoint*>(static_cast<void*>(mem_));
}

// ----------------------------------------------------------------------------
DataPoint* DataSet::end()
{
    return begin() + count_;
}

}
