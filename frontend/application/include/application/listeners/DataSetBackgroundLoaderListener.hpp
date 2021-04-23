#pragma once

namespace uh {
    class DataSet;
}

namespace uhapp {

class RecordingGroup;

class DataSetBackgroundLoaderListener
{
public:
    virtual void onDataSetBackgroundLoaderDataSetLoaded(RecordingGroup* group, uh::DataSet* dataSet) = 0;
};

}
