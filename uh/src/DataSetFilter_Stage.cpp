#include "uh/DataSetFilter_Stage.hpp"
#include "uh/DataSet.hpp"
#include "uh/PlayerState.hpp"
#include "uh/SavedGameSession.hpp"

namespace uh {

// ----------------------------------------------------------------------------
DataSet* DataSetFilter_Stage::apply(const DataSet* dataSet)
{
    DataSet* out = new DataSet;
    return out;
}

// ----------------------------------------------------------------------------
DataSet* DataSetFilter_Stage::applyInverse(const DataSet* dataSet)
{
    DataSet* out = new DataSet;
    return out;
}

}
