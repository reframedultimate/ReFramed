#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/RefCounted.hpp"

namespace rfcommon {

class RFCOMMON_PUBLIC_API VideoEmbed : public RefCounted
{
public:
    VideoEmbed();
    virtual ~VideoEmbed();
};

}
