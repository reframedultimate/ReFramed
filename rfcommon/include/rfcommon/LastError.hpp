#pragma once

#include "rfcommon/config.hpp"

namespace rfcommon {

#if defined(RFCOMMON_PLATFORM_WINDOWS)
class RFCOMMON_PUBLIC_API LastError
{
public:
    LastError();
    ~LastError();

    const char* cStr() const { return str_; }

private:
    char* str_;
};
#else
class RFCOMMON_PUBLIC_API LastError
{
public:
    LastError();
    ~LastError();

    const char* cStr() const;
};
#endif

}
