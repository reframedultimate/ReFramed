#include "rfcommon/dynlib.h"
#include <stdlib.h>
#include <dlfcn.h>
#include <stdio.h>

/* ------------------------------------------------------------------------- */
struct rfcommon_dynlib* rfcommon_dynlib_open(const char* fileName)
{
    void* handle = dlopen(fileName, RTLD_NOW);
    if (handle == NULL)
    {
        printf("dlopen() failed: %s\n", dlerror());
        return NULL;
    }
    return (struct rfcommon_dynlib*)handle;
}

/* ------------------------------------------------------------------------- */
void rfcommon_dynlib_close(struct rfcommon_dynlib* dynlib)
{
    void* handle = (void*)dynlib;
    dlclose(handle);
}

/* ------------------------------------------------------------------------- */
void* rfcommon_dynlib_lookup_symbol_address(struct rfcommon_dynlib* dynlib, const char* name)
{
    void* handle = (void*)dynlib;
    return dlsym(handle, name);
}
