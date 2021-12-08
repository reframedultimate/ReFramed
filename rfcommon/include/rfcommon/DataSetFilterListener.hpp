#pragma once

namespace rfcommon {

class DataSetFilter;

class DataSetFilterListener
{
public:
    virtual void onDataSetFilterDirtied(DataSetFilter* filter) = 0;
};

}
