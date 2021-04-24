#pragma once

#include "uh/Plugin.hpp"
#include <string>

namespace uh {

class AnalysisResult;
class DataSet;

class AnalyzerPlugin : public Plugin
{
public:
    virtual void processDataSet(const DataSet* dataSet) = 0;

    void notifyAnalysisProgress(float percent, const std::string& info);
    void giveAnalysisResult(AnalysisResult* result);
};

}
