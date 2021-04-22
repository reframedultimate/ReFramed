#pragma once

namespace uh {

class DataSetFilterListener
{
public:
    virtual void onDataSetFilterEnabledChanged(bool enabled) = 0;
    virtual void onDataSetFilterInvertedChanged(bool inverted) = 0;
};

}
