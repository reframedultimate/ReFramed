#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/RefCounted.hpp"

namespace rfcommon {

class DataSet;
class DataSetFilterListener;

extern template class RFCOMMON_TEMPLATE_API SmallVector<DataSetFilterListener*, 4>;
extern template class RFCOMMON_TEMPLATE_API ListenerDispatcher<DataSetFilterListener>;

class RFCOMMON_PUBLIC_API DataSetFilter : public RefCounted
{
public:
    virtual DataSet* apply(const DataSet* dataSet) = 0;
    virtual DataSet* applyInverse(const DataSet* dataSet) = 0;

    DataSetFilter& setEnabled(bool enable);
    DataSetFilter& setInverted(bool invert);

    bool enabled() const { return enabled_; }
    bool inverted() const { return inverted_; }

    ListenerDispatcher<DataSetFilterListener> dispatcher;

protected:
    void notifyDirty();

private:
    bool enabled_ = true;
    bool inverted_ = false;
};

}
