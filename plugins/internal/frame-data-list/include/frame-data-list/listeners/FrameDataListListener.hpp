#pragma once

namespace rfcommon {
    class FrameData;
    class MappingInfo;
    class MetaData;
}

class FrameDataListListener
{
public:
    virtual void onNewData(rfcommon::MappingInfo* map, rfcommon::MetaData* meta, rfcommon::FrameData* frames) = 0;
    virtual void onDataFinalized(rfcommon::MappingInfo* map, rfcommon::MetaData* meta, rfcommon::FrameData* frames) = 0;
    virtual void onNewFrame() = 0;
};
