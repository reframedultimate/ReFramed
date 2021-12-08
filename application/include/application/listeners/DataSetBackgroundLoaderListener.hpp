#pragma once

namespace rfcommon {
    class DataSet;
}

namespace rfapp {

class ReplayGroup;

class DataSetBackgroundLoaderListener
{
public:
    virtual void onDataSetBackgroundLoaderDataSetLoaded(ReplayGroup* group, rfcommon::DataSet* dataSet) = 0;
};

}
