#include "uh/dynlib.h"
#include <stdlib.h>

#if defined(_WIN32)
#   define WIN32_LEAN_AND_MEAN
#   include <Windows.h>
#elif defined(__APPLE__)
#elif defined(__linux__)
#   include <dlfcn.h>
#endif

/* ------------------------------------------------------------------------- */
struct uh_dynlib* uh_dynlib_open(const char* fileName)
{
#if defined(_WIN32)
    HMODULE handle = LoadLibraryA(fileName);
    if (handle == NULL)
        return NULL;
    return (struct uh_dynlib*)handle;
#elif defined(__APPLE__)
#elif defined(__linux__)
    void* handle = dlopen(fileName, RTLD_NOW);
    if (handle == NULL)
        return NULL;
    return (struct uh_dynlib*)handle;
#endif
}

/* ------------------------------------------------------------------------- */
void uh_dynlib_close(struct uh_dynlib* dynlib)
{
#if defined(_WIN32)
    HMODULE handle = (HMODULE)dynlib;
    FreeLibrary(handle);
#elif defined(__APPLE__)
#elif defined(__linux__)
    void* handle = (void*)dynlib;
    dlclose(handle);
#endif
}

/* ------------------------------------------------------------------------- */
void* uh_dynlib_lookup_symbol_address(struct uh_dynlib* dynlib, const char* name)
{
#if defined(_WIN32)
    HMODULE handle = (HMODULE)dynlib;
    return (void*)GetProcAddress(handle, name);
#elif defined(__APPLE__)
#elif defined(__linux__)
    void* handle = (void*)dynlib;
    return dlsym(handle, name);
#endif
}
