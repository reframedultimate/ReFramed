#pragma once

namespace rfcommon {
    class FrameData;
    class MappingInfo;
    class MetaData;
}

class DataViewerListener
{
public:
    virtual void onNewData(rfcommon::MappingInfo* map, rfcommon::MetaData* meta, rfcommon::FrameData* frames) = 0;
    virtual void onClear() = 0;
    virtual void onNewFrame() = 0;
};
