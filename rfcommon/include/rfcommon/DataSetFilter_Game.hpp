#pragma once

#include "rfcommon/DataSetFilter.hpp"
#include "rfcommon/SetFormat.hpp"
#include "rfcommon/String.hpp"
#include <cstdint>

namespace rfcommon {

class RFCOMMON_PUBLIC_API DataSetFilter_Game : public DataSetFilter
{
public:
    DataSetFilter_Game();

    DataSet* apply(const DataSet* dataSet) override;
    DataSet* applyInverse(const DataSet* dataSet) override;

    void setSetFormat(const SetFormat& format);
    void setAnySetFormat(bool filter);
    void setWinner(const String& name);
    void setMinLengthMs(uint64_t length);
    void setMaxLengthMs(uint64_t length);

private:
    String winner_;
    uint64_t minLength_ = 0;
    uint64_t maxLength_ = 8 * 60 * 1000;
    SetFormat format_;
    bool anySetFormat_ = true;
};

}
