#pragma once

#include "rfcommon/config.hpp"

namespace rfcommon {

class RFCOMMON_PUBLIC_API BuildInfo
{
public:
    static const char* commit();
    static const char* buildHost();
    static const char* compiler();
    static const char* platform();
    static int bits();
};

}
