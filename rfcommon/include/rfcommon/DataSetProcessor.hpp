#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/DataSetProcessorListener.hpp"
#include "rfcommon/ListenerDispatcher.hpp"

namespace rfcommon {

class AnalysisResult;
class DataSet;

extern template class RFCOMMON_TEMPLATE_API SmallVector<DataSetProcessorListener*, 4>;
extern template class RFCOMMON_TEMPLATE_API ListenerDispatcher<DataSetProcessorListener>;

class RFCOMMON_PUBLIC_API DataSetProcessor : public DataSetProcessorListener
{
public:
    ListenerDispatcher<DataSetProcessorListener> dispatcher;

protected:
    void notifyDataSetProcessing(float progress, const String& info);
    void notifyDataSetCancelled();
    void notifyDataSetComplete(const DataSet* dataSet);
};

}
