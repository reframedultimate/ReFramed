#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/RefCounted.hpp"
#include "rfcommon/Reference.hpp"
#include "rfcommon/String.hpp"
#include "rfcommon/HashMap.hpp"

namespace rfcommon {

class DataSet;

class RFCOMMON_PUBLIC_API AnalysisResult : public RefCounted
{
public:
    void addDataSet(const String& name, DataSet* dataSet);

private:
    //HashMap<String, Reference<DataSet>> dataSets_;
};

}
