#include "rfcommon/VideoMeta.hpp"

namespace rfcommon {

VideoMeta::VideoMeta()
{}

VideoMeta::~VideoMeta()
{}

VideoMeta* VideoMeta::load(const void* data, uint32_t size)
{
    return nullptr;
}

uint32_t VideoMeta::save(FILE* fp)
{
    return 0;
}

}
