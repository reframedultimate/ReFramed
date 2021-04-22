#include "uh/DataSetFilter.hpp"
#include "uh/DataSetFilterListener.hpp"

namespace uh {

// ----------------------------------------------------------------------------
DataSetFilter& DataSetFilter::setEnabled(bool enable)
{
    enabled_ = enable;
    dispatcher.dispatch(&DataSetFilterListener::onDataSetFilterEnabledChanged, enable);
    return *this;
}

// ----------------------------------------------------------------------------
DataSetFilter& DataSetFilter::setInverted(bool invert)
{
    inverted_ = invert;
    dispatcher.dispatch(&DataSetFilterListener::onDataSetFilterInvertedChanged, invert);
    return *this;
}

}
