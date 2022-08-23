#include "rfcommon/LastWindowsError.hpp"
#include "rfcommon/Log.hpp"
#include "rfcommon/SharedLibrary.hpp"
#include <cstdlib>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace rfcommon {

/* ------------------------------------------------------------------------- */
SharedLibrary::SharedLibrary(const char* fileName)
{
    HMODULE hModule = LoadLibraryA(fileName);
    if (hModule == NULL)
        Log::root()->error("Failed to load DLL: %s: %s", fileName, LastWindowsError().cStr());

    handle_ = static_cast<void*>(hModule);
}

/* ------------------------------------------------------------------------- */
SharedLibrary::~SharedLibrary()
{
    HMODULE hModule = static_cast<HMODULE>(handle_);
    if (hModule != NULL)
    {
        CHAR buf[MAX_PATH+1];
        if (GetModuleFileNameA(hModule, buf, MAX_PATH) != 0)
            Log::root()->info("Closing DLL: %s", buf);
        else
            Log::root()->info("Closing DLL");
        FreeLibrary(hModule);
    }
}

/* ------------------------------------------------------------------------- */
SharedLibrary::SharedLibrary(SharedLibrary&& other)
    : handle_(other.handle_)
{
    other.handle_ = static_cast<void*>(NULL);
}

/* ------------------------------------------------------------------------- */
bool SharedLibrary::isOpen() const
{
    HMODULE hModule = static_cast<HMODULE>(handle_);
    return hModule != NULL;
}

/* ------------------------------------------------------------------------- */
void* SharedLibrary::lookupSymbolAddress(const char* name)
{
    HMODULE hModule = static_cast<HMODULE>(handle_);
    return (void*)GetProcAddress(hModule, name);
}

}
