#pragma once

#include "uh/config.hpp"
#include "uh/DataSetProcessorListener.hpp"
#include "uh/ListenerDispatcher.hpp"

namespace uh {

class AnalysisResult;
class DataSet;

template class UH_PUBLIC_API ListenerDispatcher<DataSetProcessorListener>::container_type;
template class UH_PUBLIC_API ListenerDispatcher<DataSetProcessorListener>;

class UH_PUBLIC_API DataSetProcessor : public DataSetProcessorListener
{
public:
    ListenerDispatcher<DataSetProcessorListener> dispatcher;

protected:
    void notifyDataSetProcessing(float progress, const String& info);
    void notifyDataSetCancelled();
    void notifyDataSetComplete(const DataSet* dataSet);
};

}
