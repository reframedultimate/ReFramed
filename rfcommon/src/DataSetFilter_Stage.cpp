#include "rfcommon/DataSetFilter_Stage.hpp"
#include "rfcommon/DataSet.hpp"
#include "rfcommon/Profiler.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
DataSet* DataSetFilter_Stage::apply(const DataSet* dataSet)
{
    PROFILE(DataSetFilter_Stage, apply);

    DataSet* out = new DataSet;
    return out;
}

// ----------------------------------------------------------------------------
DataSet* DataSetFilter_Stage::applyInverse(const DataSet* dataSet)
{
    PROFILE(DataSetFilter_Stage, applyInverse);

    DataSet* out = new DataSet;
    return out;
}

}
