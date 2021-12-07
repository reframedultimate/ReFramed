#pragma once

#include "uh/DataSetFilter.hpp"
#include <cstdint>

namespace uh {

class UH_PUBLIC_API DataSetFilter_PlayerCount : public DataSetFilter
{
public:
    DataSet* apply(const DataSet* dataSet) override;
    DataSet* applyInverse(const DataSet* dataSet) override;

private:
};

}
