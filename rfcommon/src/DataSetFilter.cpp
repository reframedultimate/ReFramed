#include "rfcommon/DataSetFilter.hpp"
#include "rfcommon/DataSetFilterListener.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
DataSetFilter& DataSetFilter::setEnabled(bool enable)
{
    enabled_ = enable;
    notifyDirty();
    return *this;
}

// ----------------------------------------------------------------------------
DataSetFilter& DataSetFilter::setInverted(bool invert)
{
    inverted_ = invert;
    notifyDirty();
    return *this;
}

// ----------------------------------------------------------------------------
void DataSetFilter::notifyDirty()
{
    dispatcher.dispatch(&DataSetFilterListener::onDataSetFilterDirtied, this);
}

}
