#include "rfcommon/DataSetFilter_Date.hpp"
#include "rfcommon/DataSet.hpp"
#include "rfcommon/Profiler.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
DataSet* DataSetFilter_Date::apply(const DataSet* dataSet)
{
    PROFILE(DataSetFilter_Date, apply);

    DataSet* out = new DataSet;
    return out;
}

// ----------------------------------------------------------------------------
DataSet* DataSetFilter_Date::applyInverse(const DataSet* dataSet)
{
    PROFILE(DataSetFilter_Date, applyInverse);

    DataSet* out = new DataSet;
    return out;
}

// ----------------------------------------------------------------------------
void DataSetFilter_Date::setStartTimeMs(uint64_t startMs)
{
    PROFILE(DataSetFilter_Date, setStartTimeMs);

    startTime_ = startMs;
    notifyDirty();
}

// ----------------------------------------------------------------------------
void DataSetFilter_Date::setEndTimeMs(uint64_t endMs)
{
    PROFILE(DataSetFilter_Date, setEndTimeMs);

    endTime_ = endMs;
    notifyDirty();
}

}
