#include "rfcommon/DataSetFilterChain.hpp"
#include "rfcommon/DataSetFilter.hpp"
#include "rfcommon/DataSet.hpp"
#include "rfcommon/Profiler.hpp"
#include <algorithm>

namespace rfcommon {

// ----------------------------------------------------------------------------
int DataSetFilterChain::add(DataSetFilter* filter)
{
    PROFILE(DataSetFilterChain, add);

    filters_.emplace(filter);
    return filterCount() - 1;
}

// ----------------------------------------------------------------------------
int DataSetFilterChain::remove(DataSetFilter* filter)
{
    PROFILE(DataSetFilterChain, remove);

    const auto it = std::find(filters_.begin(), filters_.end(), filter);
    if (it == filters_.end())
        return -1;
    int pos = static_cast<int>(it - filters_.begin());
    filters_.erase(it);
    return pos;
}

// ----------------------------------------------------------------------------
int DataSetFilterChain::moveLater(DataSetFilter* filter)
{
    PROFILE(DataSetFilterChain, moveLater);

    const auto it = std::find(filters_.begin(), filters_.end(), filter);
    if (it == filters_.end())
        return -1;

    // Already at end
    if (it == filters_.end() - 1)
        return filterCount() - 1;

    std::iter_swap(it, it + 1);
    return static_cast<int>(it - filters_.begin() + 1);
}

// ----------------------------------------------------------------------------
int DataSetFilterChain::moveEarlier(DataSetFilter* filter)
{
    PROFILE(DataSetFilterChain, moveEarlier);

    const auto it = std::find(filters_.begin(), filters_.end(), filter);
    if (it == filters_.end())
        return -1;

    // Already at beginning
    if (it == filters_.begin())
        return 0;

    std::iter_swap(it, it - 1);
    return static_cast<int>(it - filters_.begin() - 1);
}

// ----------------------------------------------------------------------------
int DataSetFilterChain::filterCount() const
{
    PROFILE(DataSetFilterChain, filterCount);

    return static_cast<int>(filters_.count());
}

// ----------------------------------------------------------------------------
DataSetFilter* DataSetFilterChain::filter(int idx) const
{
    PROFILE(DataSetFilterChain, filter);

    return filters_[idx];
}

// ----------------------------------------------------------------------------
DataSet* DataSetFilterChain::apply(const DataSet* ds)
{
    PROFILE(DataSetFilterChain, apply);

    DataSet* out = nullptr;
    for (const auto& filter : filters_)
    {
        if (filter->enabled() == false)
            continue;

        DataSet* (DataSetFilter::*applyFunc)(const DataSet*) = filter->inverted() ?
                    &DataSetFilter::applyInverse : &DataSetFilter::apply;

        if (out == nullptr)
        {
            out = (filter->*applyFunc)(ds);
        }
        else
        {
            rfcommon::Reference<rfcommon::DataSet> ref(out);  // Make sure the previous instance gets deleted
            out = (filter->*applyFunc)(out);
        }
    }

    if (out == nullptr)
    {
        out = new DataSet;
    }

    return out;
}

}
