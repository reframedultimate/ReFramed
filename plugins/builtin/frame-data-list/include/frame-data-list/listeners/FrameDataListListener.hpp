#pragma once

namespace rfcommon {
    class FrameData;
    class MappingInfo;
    class SessionMetaData;
}

class FrameDataListListener
{
public:
    virtual void onNewData(rfcommon::MappingInfo* map, rfcommon::SessionMetaData* meta, rfcommon::FrameData* frames) = 0;
    virtual void onDataFinalized(rfcommon::MappingInfo* map, rfcommon::SessionMetaData* meta, rfcommon::FrameData* frames) = 0;
};
