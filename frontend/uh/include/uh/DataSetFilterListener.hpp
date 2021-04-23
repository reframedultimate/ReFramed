#pragma once

namespace uh {

class DataSetFilter;

class DataSetFilterListener
{
public:
    virtual void onDataSetFilterDirtied(DataSetFilter* filter) = 0;
};

}
