#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/RefCounted.hpp"
#include <cstdint>

namespace rfcommon {

class RFCOMMON_PUBLIC_API MappedFile : public RefCounted
{
public:
    MappedFile();
    ~MappedFile();

    bool open(const char* utf8_filename);
    void close();
    static bool setDeleteOnClose(const char* utf8_filename);

    const void* address() const { return address_; }
    const void* addressAtOffset(uint32_t offset) const { return static_cast<const unsigned char*>(address_) + offset; }
    uint64_t size() const { return size_; }

private:
    void* address_;
    uint64_t size_;
};

}
