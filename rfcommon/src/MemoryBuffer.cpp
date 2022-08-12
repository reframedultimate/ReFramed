#include "rfcommon/MemoryBuffer.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
MemoryBuffer::MemoryBuffer(int bytes)
    : buffer_(Vector<unsigned char>::makeResized(bytes))
{
}

}
