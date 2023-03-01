#include "rfcommon/LastError.hpp"
#include <cerrno>
#include <cstring>

namespace rfcommon {

// ----------------------------------------------------------------------------
LastError::LastError()
{
}

// ----------------------------------------------------------------------------
LastError::~LastError()
{
}

// ----------------------------------------------------------------------------
const char* LastError::cStr() const
{
    return strerror(errno);
}

}
