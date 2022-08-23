#pragma once

#include "rfcommon/config.hpp"

namespace rfcommon {

class RFCOMMON_PUBLIC_API SharedLibrary
{
public:
    SharedLibrary(const char* fileName);
    ~SharedLibrary();
    SharedLibrary(const SharedLibrary& other) = delete;
    SharedLibrary(SharedLibrary&& other);

    bool isOpen() const;

    void* lookupSymbolAddress(const char* name);

private:
    void* handle_;
};

}
