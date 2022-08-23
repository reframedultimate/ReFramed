#include "rfcommon/LastWindowsError.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace rfcommon {

// ----------------------------------------------------------------------------
LastWindowsError::LastWindowsError()
    : str_(nullptr)
{
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&str_,
        0,
        NULL);
}

// ----------------------------------------------------------------------------
LastWindowsError::~LastWindowsError()
{
    if (str_)
        LocalFree(str_);
}

}
