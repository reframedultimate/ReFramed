#include "uh/DataSetFilter_Date.hpp"
#include "uh/DataSet.hpp"
#include "uh/Recording.hpp"

namespace uh {

// ----------------------------------------------------------------------------
DataSet* DataSetFilter_Date::apply(const DataSet* dataSet)
{
    DataSet* out = new DataSet;
    for (const auto& playerName : dataSet->playerNames())
    {
        for (const auto& dp : dataSet->playerDataSet(playerName)->dataPoints())
        {
            const PlayerState& state = dp.state();
            if (state.timeStampMs() >= startTime_ && state.timeStampMs() <= endTime_)
                out->appendDataPoint(playerName, dp);
        }
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
