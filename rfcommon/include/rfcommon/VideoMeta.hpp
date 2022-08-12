#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/RefCounted.hpp"
#include <cstdio>
#include <cstdint>

namespace rfcommon {

class RFCOMMON_PUBLIC_API VideoMeta : public RefCounted
{
public:
    VideoMeta();
    virtual ~VideoMeta();

    static VideoMeta* load(const void* data, uint32_t size);
    uint32_t save(FILE* fp);
};

}
