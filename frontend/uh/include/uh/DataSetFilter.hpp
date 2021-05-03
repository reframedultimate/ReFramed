#pragma once

#include "uh/config.hpp"
#include "uh/ListenerDispatcher.hpp"
#include "uh/RefCounted.hpp"

namespace uh {

class DataSet;
class DataSetFilterListener;

template class UH_PUBLIC_API ListenerDispatcher<DataSetFilterListener>::container_type;
template class UH_PUBLIC_API ListenerDispatcher<DataSetFilterListener>;

class UH_PUBLIC_API DataSetFilter : public RefCounted
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
