#include "rfcommon/dynlib.h"
#include <stdlib.h>

#if defined(_WIN32)
#   define WIN32_LEAN_AND_MEAN
#   include <Windows.h>
#elif defined(__APPLE__)
#elif defined(__linux__)
#   include <dlfcn.h>
#   include <stdio.h>
#endif

/* ------------------------------------------------------------------------- */
struct rfcommon_dynlib* rfcommon_dynlib_open(const char* fileName)
{
#if defined(_WIN32)
    HMODULE handle = LoadLibraryA(fileName);
    if (handle == NULL)
        return NULL;
    return (struct rfcommon_dynlib*)handle;
#elif defined(__APPLE__)
#elif defined(__linux__)
    void* handle = dlopen(fileName, RTLD_NOW);
    if (handle == NULL)
    {
        printf("dlopen() failed: %s\n", dlerror());
        return NULL;
    }
    return (struct rfcommon_dynlib*)handle;
#endif
}

/* ------------------------------------------------------------------------- */
void rfcommon_dynlib_close(struct rfcommon_dynlib* dynlib)
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
void* rfcommon_dynlib_lookup_symbol_address(struct rfcommon_dynlib* dynlib, const char* name)
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
