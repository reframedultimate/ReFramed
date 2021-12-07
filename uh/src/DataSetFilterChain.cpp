#include "uh/DataSetFilterChain.hpp"
#include "uh/DataSetFilter.hpp"
#include "uh/DataSet.hpp"
#include "uh/SavedGameSession.hpp"
#include <algorithm>

namespace uh {

// ----------------------------------------------------------------------------
int DataSetFilterChain::add(DataSetFilter* filter)
{
    filters_.emplace(filter);
    return filterCount() - 1;
}

// ----------------------------------------------------------------------------
int DataSetFilterChain::remove(DataSetFilter* filter)
{
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
    return static_cast<int>(filters_.count());
}

// ----------------------------------------------------------------------------
DataSetFilter* DataSetFilterChain::filter(int idx) const
{
    return filters_[idx];
}

// ----------------------------------------------------------------------------
DataSet* DataSetFilterChain::apply(const DataSet* ds)
{
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
            uh::Reference<uh::DataSet> ref(out);  // Make sure the previous instance gets deleted
            out = (filter->*applyFunc)(out);
        }
    }

    if (out == nullptr)
    {
        out = new DataSet;
        out->mergeDataFrom(ds);
    }

    return out;
}

}
