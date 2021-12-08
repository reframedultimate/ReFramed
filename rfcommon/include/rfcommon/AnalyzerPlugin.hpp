#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Plugin.hpp"
#include "rfcommon/DataSetProcessor.hpp"

namespace rfcommon {

class AnalysisResult;
class DataSet;

class RFCOMMON_PUBLIC_API AnalyzerPlugin : public Plugin
                                   , public DataSetProcessor
{
public:
    // MSVC requires classes to have constructors/destructors defined in exported classes
    AnalyzerPlugin(RFPluginFactory* factory);
    ~AnalyzerPlugin();

    virtual void setPointOfView(const String& playerName) = 0;
};

}
