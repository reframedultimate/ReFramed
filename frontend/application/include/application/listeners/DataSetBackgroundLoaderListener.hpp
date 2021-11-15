#pragma once

namespace uh {
    class DataSet;
}

namespace uhapp {

class SavedGameSessionGroup;

class DataSetBackgroundLoaderListener
{
public:
    virtual void onDataSetBackgroundLoaderDataSetLoaded(SavedGameSessionGroup* group, uh::DataSet* dataSet) = 0;
};

}
