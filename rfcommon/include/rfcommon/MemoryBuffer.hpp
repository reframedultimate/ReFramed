#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Vector.hpp"
#include <cstdint>

namespace rfcommon {

class RFCOMMON_PUBLIC_API MemoryBuffer
{
public:
    MemoryBuffer(int bytes);

    void* address() { return buffer_.data(); }
    int size() const { return buffer_.count(); }

private:
    Vector<unsigned char> buffer_;
};

}
