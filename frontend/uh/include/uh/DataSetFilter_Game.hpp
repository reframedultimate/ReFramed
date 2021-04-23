#pragma once

#include "uh/DataSetFilter.hpp"
#include "uh/SetFormat.hpp"
#include <string>
#include <cstdint>

namespace uh {

class UH_PUBLIC_API DataSetFilter_Game : public DataSetFilter
{
public:
    DataSetFilter_Game();

    DataSet* apply(const DataSet* dataSet) override;

    void setSetFormat(const SetFormat& format);
    void setAnySetFormat(bool filter);
    void setWinner(const std::string& name);
    void setMinLengthMs(uint64_t length);
    void setMaxLengthMs(uint64_t length);

private:
    std::string winner_;
    uint64_t minLength_ = 0;
    uint64_t maxLength_ = 8 * 60 * 1000;
    SetFormat format_;
    bool anySetFormat_ = true;
};

}
