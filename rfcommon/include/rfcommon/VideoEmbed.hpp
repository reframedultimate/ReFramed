#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/RefCounted.hpp"
#include "rfcommon/Reference.hpp"
#include <cstdio>
#include <cstdint>

namespace rfcommon {

class MappedFile;

class RFCOMMON_PUBLIC_API VideoEmbed : public RefCounted
{
public:
    VideoEmbed(MappedFile* file, const void* data, uint32_t size);
    virtual ~VideoEmbed();

    const void* address() const { return address_; }
    uint32_t size() const { return size_; }

private:
    Reference<MappedFile> file_;
    const void* address_;
    uint32_t size_;
};

}
