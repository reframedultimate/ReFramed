#pragma once

#include "uh/config.hpp"
#include "uh/Plugin.hpp"
#include "uh/DataSetProcessor.hpp"

namespace uh {

class AnalysisResult;
class DataSet;

class UH_PUBLIC_API AnalyzerPlugin : public Plugin
                                   , public DataSetProcessor
{
public:
    virtual void setPointOfView(const String& playerName) = 0;
};

}
