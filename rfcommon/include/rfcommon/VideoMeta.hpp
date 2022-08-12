#pragma once

#include "rfcommon/config.hpp"
#include <cstdio>

namespace rfcommon {

class RFCOMMON_PUBLIC_API VideoMeta : public RefCounted
{
public:
    virtual ~VideoMeta();

    static VideoMeta* load(FILE* fp, uint32_t size);
    uint32_t save(FILE* fp);
};

}
