#include "rfcommon/HashMap.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
char* HashMapAlloc::allocate(size_t count, size_t size)
{
    return new char[count * size];
}

// ----------------------------------------------------------------------------
void HashMapAlloc::deallocate(char *p)
{
    delete[] p;
}

}
