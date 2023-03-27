#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/String.hpp"
#include "rfcommon/FrameIndex.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/HashMap.hpp"

namespace rfcommon {

class RFCOMMON_PUBLIC_API PluginSharedData
{
public:
    class RFCOMMON_PUBLIC_API TimeInterval
    {
    public:
        TimeInterval(const rfcommon::String& name, FrameIndex start, FrameIndex end);
        ~TimeInterval();

        const String name;
        const FrameIndex start;
        const FrameIndex end;
    };

    HashMap<String, Vector<TimeInterval>> timeIntervalSets;
};

}
