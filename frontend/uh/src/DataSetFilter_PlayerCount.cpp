#include "uh/DataSetFilter_PlayerCount.hpp"
#include "uh/DataSet.hpp"
#include "uh/Recording.hpp"
#include "uh/PlayerState.hpp"

namespace uh {

// ----------------------------------------------------------------------------
DataSet* DataSetFilter_PlayerCount::apply(const DataSet* dataSet)
{
    DataSet* out = new DataSet;
    for (const auto& playerName : dataSet->playerNames())
        for (const auto& dp : dataSet->playerDataSet(playerName)->dataPoints())
        {
        }

    return out;
}

// ----------------------------------------------------------------------------
DataSet* DataSetFilter_PlayerCount::applyInverse(const DataSet* dataSet)
{
    DataSet* out = new DataSet;
    return out;
}

}
