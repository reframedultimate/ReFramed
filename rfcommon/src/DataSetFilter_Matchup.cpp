#include "rfcommon/DataSetFilter_Matchup.hpp"
#include "rfcommon/DataSet.hpp"
#include "rfcommon/PlayerState.hpp"
#include "rfcommon/SavedGameSession.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
DataSet* DataSetFilter_Matchup::apply(const DataSet* dataSet)
{
    DataSet* out = new DataSet;

    return out;
}

// ----------------------------------------------------------------------------
DataSet* DataSetFilter_Matchup::applyInverse(const DataSet* dataSet)
{
    DataSet* out = new DataSet;
    return out;
}

}
