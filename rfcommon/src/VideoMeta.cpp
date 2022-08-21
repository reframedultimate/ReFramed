#include "rfcommon/VideoMeta.hpp"
#include "rfcommon/Profiler.hpp"

namespace rfcommon {

VideoMeta::VideoMeta()
{}

VideoMeta::~VideoMeta()
{}

VideoMeta* VideoMeta::load(const void* data, uint32_t size)
{
    PROFILE(VideoMeta, load);

    return nullptr;
}

uint32_t VideoMeta::save(FILE* fp)
{
    PROFILE(VideoMeta, save);

    return 0;
}

}
