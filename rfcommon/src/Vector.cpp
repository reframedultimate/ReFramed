#include "rfcommon/Vector.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
char* VectorAlloc::allocate(size_t count, size_t size)
{
    return new char[count * size];
}

// ----------------------------------------------------------------------------
void VectorAlloc::deallocate(char *p)
{
    delete[] p;
}

}
