#include "rfcommon/HashMap.hpp"
#include "rfcommon/Profiler.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
char* HashMapAlloc::allocate(size_t count, size_t size)
{
    NOPROFILE();

    return new char[count * size];
}

// ----------------------------------------------------------------------------
void HashMapAlloc::deallocate(char *p)
{
    NOPROFILE();

    delete[] p;
}

}
