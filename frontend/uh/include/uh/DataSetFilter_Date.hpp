#pragma once

#include "uh/DataSetFilter.hpp"
#include <cstdint>

namespace uh {

class UH_PUBLIC_API DataSetFilter_Date : public DataSetFilter
{
public:
    DataSet* apply(const DataSet* dataSet) override;
    DataSet* applyInverse(const DataSet* dataSet) override;

    void setStartTimeMs(uint64_t startMs);
    void setEndTimeMs(uint64_t endMs);

private:
    uint64_t startTime_ = 0;
    uint64_t endTime_ = 0;
};

}
