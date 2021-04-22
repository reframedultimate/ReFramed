#pragma once

#include "uh/config.hpp"
#include "uh/ListenerDispatcher.hpp"
#include "uh/RefCounted.hpp"

namespace uh {

class DataSet;
class DataSetFilterListener;

class UH_PUBLIC_API DataSetFilter : public RefCounted
{
public:
    virtual void apply(DataSet* ds) = 0;

    DataSetFilter& setEnabled(bool enable);
    DataSetFilter& setInverted(bool invert);

    bool enabled() const { return enabled_; }
    bool isInverted() const { return inverted_; }

    ListenerDispatcher<DataSetFilterListener> dispatcher;

private:
    bool enabled_;
    bool inverted_;
};

}
