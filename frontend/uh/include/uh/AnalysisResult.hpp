#pragma once

#include "uh/config.hpp"
#include "uh/RefCounted.hpp"
#include "uh/Reference.hpp"
#include "uh/String.hpp"
#include "uh/HashMap.hpp"

namespace uh {

class DataSet;

class UH_PUBLIC_API AnalysisResult : public RefCounted
{
public:
    void addDataSet(const String& name, DataSet* dataSet);

private:
    HashMap<String, Reference<DataSet>> dataSets_;
};

}
