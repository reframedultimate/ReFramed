#include "rfcommon/DataSetFilter_Matchup.hpp"
#include "rfcommon/DataSet.hpp"
#include "rfcommon/Profiler.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
DataSet* DataSetFilter_Matchup::apply(const DataSet* dataSet)
{
    PROFILE(DataSetFilter_Matchup, apply);

    DataSet* out = new DataSet;

    return out;
}

// ----------------------------------------------------------------------------
DataSet* DataSetFilter_Matchup::applyInverse(const DataSet* dataSet)
{
    PROFILE(DataSetFilter_Matchup, applyInverse);

    DataSet* out = new DataSet;
    return out;
}

}
