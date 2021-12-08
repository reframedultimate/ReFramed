#include "rfcommon/DataSetFilter_Date.hpp"
#include "rfcommon/DataSet.hpp"
#include "rfcommon/SavedGameSession.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
DataSet* DataSetFilter_Date::apply(const DataSet* dataSet)
{
    DataSet* out = new DataSet;
    out->reserve(dataSet->dataPointCount());
    for (const DataPoint* p = dataSet->dataPointsBegin(); p != dataSet->dataPointsEnd(); ++p)
    {
        const PlayerState& state = p->state();
        if (state.timeStampMs() >= startTime_ && state.timeStampMs() <= endTime_)
            out->addDataPointToEnd(*p);
    }

    return out;
}

// ----------------------------------------------------------------------------
DataSet* DataSetFilter_Date::applyInverse(const DataSet* dataSet)
{
    DataSet* out = new DataSet;
    out->reserve(dataSet->dataPointCount());
    for (const DataPoint* p = dataSet->dataPointsBegin(); p != dataSet->dataPointsEnd(); ++p)
    {
        const PlayerState& state = p->state();
        if (state.timeStampMs() < startTime_ || state.timeStampMs() > endTime_)
            out->addDataPointToEnd(*p);
    }

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
