#pragma once

#include "uh/config.hpp"
#include "uh/String.hpp"

namespace uh {

class DataSet;

class UH_PUBLIC_API DataSetProcessorListener
{
public:
    virtual void dataSetPreparing(float progress, const String& info) = 0;
    virtual void dataSetCancelled() = 0;
    virtual void processDataSet(const DataSet* dataSet) = 0;
};

}
