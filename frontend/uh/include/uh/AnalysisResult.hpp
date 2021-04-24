#pragma once

#include "uh/config.hpp"
#include "uh/RefCounted.hpp"
#include <string>
#include <unordered_map>

namespace uh {

class DataSet;

class UH_PUBLIC_API AnalysisResult : public RefCounted
{
public:
    DataSet& getOrCreateDataSet(const std::string& name);
};

}
