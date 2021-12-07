#include "uh/Vector.hpp"

namespace uh {

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
