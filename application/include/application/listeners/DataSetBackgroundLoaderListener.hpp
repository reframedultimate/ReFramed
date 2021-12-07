#pragma once

namespace uh {
    class DataSet;
}

namespace uhapp {

class ReplayGroup;

class DataSetBackgroundLoaderListener
{
public:
    virtual void onDataSetBackgroundLoaderDataSetLoaded(ReplayGroup* group, uh::DataSet* dataSet) = 0;
};

}
