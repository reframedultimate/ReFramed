#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/String.hpp"

namespace rfcommon {

class DataSet;

class RFCOMMON_PUBLIC_API DataSetProcessorListener
{
public:
    virtual void dataSetPreparing(float progress, const String& info) = 0;
    virtual void dataSetCancelled() = 0;
    virtual void processDataSet(const DataSet* dataSet) = 0;
};

}
