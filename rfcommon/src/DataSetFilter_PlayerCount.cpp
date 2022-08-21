#include "rfcommon/DataSetFilter_PlayerCount.hpp"
#include "rfcommon/DataSet.hpp"
#include "rfcommon/Profiler.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
DataSet* DataSetFilter_PlayerCount::apply(const DataSet* dataSet)
{
    PROFILE(DataSetFilter_PlayerCount, apply);

    DataSet* out = new DataSet;
    return out;
}

// ----------------------------------------------------------------------------
DataSet* DataSetFilter_PlayerCount::applyInverse(const DataSet* dataSet)
{
    PROFILE(DataSetFilter_PlayerCount, applyInverse);

    DataSet* out = new DataSet;
    return out;
}

}
