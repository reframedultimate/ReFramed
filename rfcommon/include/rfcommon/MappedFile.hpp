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

    bool open(const char* fileName);
    void close();
    static bool setDeleteOnClose(const char* fileName);

    const void* address() const { return address_; }
    const void* addressAtOffset(uint32_t offset) const { return static_cast<const unsigned char*>(address_) + offset; }
    uint64_t size() const { return size_; }

private:
    void* fileHandle_;
    void* address_;
    uint64_t size_;
};

}
