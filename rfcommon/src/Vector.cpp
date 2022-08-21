#include "rfcommon/Vector.hpp"
#include "rfcommon/Profiler.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
char* VectorAlloc::allocate(size_t count, size_t size)
{
    NOPROFILE();

    return new char[count * size];
}

// ----------------------------------------------------------------------------
void VectorAlloc::deallocate(char *p)
{
    NOPROFILE();

    delete[] p;
}

}
