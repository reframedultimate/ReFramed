#include "uh/DataSetFilter.hpp"
#include "uh/DataSetFilterListener.hpp"

namespace uh {

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
