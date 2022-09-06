#include "rfcommon/VideoEmbed.hpp"
#include "rfcommon/MappedFile.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
VideoEmbed::VideoEmbed(MappedFile* file, const void* data, uint64_t size)
    : file_(file)
    , address_(data)
    , size_(size)
{}

// ----------------------------------------------------------------------------
VideoEmbed::~VideoEmbed()
{}

}
