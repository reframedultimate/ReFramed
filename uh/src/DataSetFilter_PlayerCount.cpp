#include "uh/DataSetFilter_PlayerCount.hpp"
#include "uh/DataSet.hpp"
#include "uh/PlayerState.hpp"
#include "uh/SavedGameSession.hpp"

namespace uh {

// ----------------------------------------------------------------------------
DataSet* DataSetFilter_PlayerCount::apply(const DataSet* dataSet)
{
    DataSet* out = new DataSet;
    return out;
}

// ----------------------------------------------------------------------------
DataSet* DataSetFilter_PlayerCount::applyInverse(const DataSet* dataSet)
{
    DataSet* out = new DataSet;
    return out;
}

}
