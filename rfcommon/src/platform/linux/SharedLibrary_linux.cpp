#include "rfcommon/Log.hpp"
#include "rfcommon/SharedLibrary.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <linux/limits.h>  // PATH_MAX

namespace rfcommon {

/* ------------------------------------------------------------------------- */
SharedLibrary::SharedLibrary(const char* fileName)
{
    void* handle = dlopen(fileName, RTLD_NOW);
    if (handle == nullptr)
        Log::root()->error("Failed to load shared library: %s: %s", fileName, dlerror());

    handle_ = handle;
}

/* ------------------------------------------------------------------------- */
SharedLibrary::SharedLibrary(SharedLibrary&& other)
    : handle_(other.handle_)
{
    other.handle_ = nullptr;
}

/* ------------------------------------------------------------------------- */
SharedLibrary::~SharedLibrary()
{
    if (handle_ != nullptr)
    {
        char path[PATH_MAX+1];
        if (dlinfo(handle_, RTLD_DI_ORIGIN, static_cast<void*>(path)) == 0)  // GNU extensions
            Log::root()->info("Closing shared library: %s", path);
        else
            Log::root()->info("Closing shared library");
        dlclose(handle_);
    }
}

/* ------------------------------------------------------------------------- */
bool SharedLibrary::isOpen() const
{
    return handle_ != nullptr;
}

/* ------------------------------------------------------------------------- */
void* SharedLibrary::lookupSymbolAddress(const char* name)
{
    return dlsym(handle_, name);
}

}
