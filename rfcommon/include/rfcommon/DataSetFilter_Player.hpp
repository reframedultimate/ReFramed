#pragma once

#include "rfcommon/DataSetFilter.hpp"
#include <cstdint>

namespace rfcommon {

class RFCOMMON_PUBLIC_API DataSetFilter_Player : public DataSetFilter
{
public:
    DataSet* apply(const DataSet* dataSet) override;
    DataSet* applyInverse(const DataSet* dataSet) override;

private:
};

}
