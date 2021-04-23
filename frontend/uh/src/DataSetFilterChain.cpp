#include "uh/DataSetFilterChain.hpp"
#include "uh/DataSetFilter.hpp"
#include "uh/DataSet.hpp"
#include "uh/Recording.hpp"
#include "uh/Reference.hpp"
#include <vector>
#include <algorithm>

namespace uh {

class DataSetFilterChainData
{
public:
    std::vector<Reference<DataSetFilter>> filters;
};

// ----------------------------------------------------------------------------
DataSetFilterChain::DataSetFilterChain()
    : d(new DataSetFilterChainData)
{
}

// ----------------------------------------------------------------------------
DataSetFilterChain::~DataSetFilterChain()
{
    delete d;
}

// ----------------------------------------------------------------------------
int DataSetFilterChain::add(DataSetFilter* filter)
{
    d->filters.emplace_back(filter);
    return filterCount() - 1;
}

// ----------------------------------------------------------------------------
int DataSetFilterChain::remove(DataSetFilter* filter)
{
    const auto it = std::find(d->filters.begin(), d->filters.end(), filter);
    if (it == d->filters.end())
        return -1;
    int pos = it - d->filters.begin();
    d->filters.erase(it);
    return pos;
}

// ----------------------------------------------------------------------------
int DataSetFilterChain::moveLater(DataSetFilter* filter)
{
    const auto it = std::find(d->filters.begin(), d->filters.end(), filter);
    if (it == d->filters.end())
        return -1;

    // Already at end
    if (it == d->filters.end() - 1)
        return filterCount() - 1;

    std::iter_swap(it, it + 1);
    return it - d->filters.begin() + 1;
}

// ----------------------------------------------------------------------------
int DataSetFilterChain::moveEarlier(DataSetFilter* filter)
{
    const auto it = std::find(d->filters.begin(), d->filters.end(), filter);
    if (it == d->filters.end())
        return -1;

    // Already at beginning
    if (it == d->filters.begin())
        return 0;

    std::iter_swap(it, it - 1);
    return it - d->filters.begin() - 1;
}

// ----------------------------------------------------------------------------
int DataSetFilterChain::filterCount() const
{
    return static_cast<int>(d->filters.size());
}

// ----------------------------------------------------------------------------
DataSetFilter* DataSetFilterChain::filter(int idx) const
{
    return d->filters[idx];
}

// ----------------------------------------------------------------------------
DataSet* DataSetFilterChain::apply(const DataSet* ds)
{
    DataSet* out = new DataSet;
    for (const auto& filter : d->filters)
        out->mergeDataFrom(filter->apply(ds));
    if (d->filters.size() == 0)
        out->mergeDataFrom(ds);
    return out;
}

}
