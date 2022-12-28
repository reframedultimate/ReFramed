#pragma once

namespace rfcommon {
    class FrameData;
    class MappingInfo;
    class Metadata;
    class VideoMeta;
}

class DataViewerListener
{
public:
    virtual void onNewData(rfcommon::MappingInfo* map, rfcommon::Metadata* meta, rfcommon::VideoMeta* vmeta, rfcommon::FrameData* frames) = 0;
    virtual void onClear() = 0;
    virtual void onNewFrame() = 0;
};
