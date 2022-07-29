#include "rfcommon/DataSetFilter_Date.hpp"
#include "rfcommon/DataSet.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
DataSet* DataSetFilter_Date::apply(const DataSet* dataSet)
{
    DataSet* out = new DataSet;
    return out;
}

// ----------------------------------------------------------------------------
DataSet* DataSetFilter_Date::applyInverse(const DataSet* dataSet)
{
    DataSet* out = new DataSet;
    return out;
}

// ----------------------------------------------------------------------------
void DataSetFilter_Date::setStartTimeMs(uint64_t startMs)
{
    startTime_ = startMs;
    notifyDirty();
}

// ----------------------------------------------------------------------------
void DataSetFilter_Date::setEndTimeMs(uint64_t endMs)
{
    endTime_ = endMs;
    notifyDirty();
}

}
