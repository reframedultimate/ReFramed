#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/FrameIndex.hpp"
#include "rfcommon/RefCounted.hpp"
#include "rfcommon/String.hpp"
#include <cstdio>
#include <cstdint>

namespace rfcommon {

class RFCOMMON_PUBLIC_API VideoMeta : public RefCounted
{
public:
    VideoMeta(const char* fileName, rfcommon::FrameIndex offset, bool embedded);
    virtual ~VideoMeta();

    static VideoMeta* load(const void* data, uint32_t size);
    uint32_t save(FILE* fp);

    const char* fileName() const { return fileName_.cStr(); }
    rfcommon::FrameIndex offset() const { return offset_; }
    bool isEmbedded() const { return isEmbedded_; }

private:
    rfcommon::String fileName_;
    rfcommon::FrameIndex offset_;
    bool isEmbedded_;
};

}
