#pragma once

#include "rfcommon/config.hpp"

namespace rfcommon {

#if defined(RFCOMMON_PLATFORM_WINDOWS)
class RFCOMMON_PUBLIC_API LastWindowsError
{
public:
    LastWindowsError();
    ~LastWindowsError();

    const char* cStr() const { return str_; }

private:
    char* str_;
};
#endif

}
