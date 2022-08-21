#include "rfcommon/DataSetFilter.hpp"
#include "rfcommon/DataSetFilterListener.hpp"
#include "rfcommon/Profiler.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
DataSetFilter& DataSetFilter::setEnabled(bool enable)
{
    PROFILE(DataSetFilter, setEnabled);

    enabled_ = enable;
    notifyDirty();
    return *this;
}

// ----------------------------------------------------------------------------
DataSetFilter& DataSetFilter::setInverted(bool invert)
{
    PROFILE(DataSetFilter, setInverted);

    inverted_ = invert;
    notifyDirty();
    return *this;
}

// ----------------------------------------------------------------------------
void DataSetFilter::notifyDirty()
{
    PROFILE(DataSetFilter, notifyDirty);

    dispatcher.dispatch(&DataSetFilterListener::onDataSetFilterDirtied, this);
}

}
