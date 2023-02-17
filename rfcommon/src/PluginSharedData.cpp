#include "rfcommon/PluginSharedData.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
PluginSharedData::TimeInterval::TimeInterval(const char* name, FrameIndex start, FrameIndex end)
    : name(name)
    , start(start)
    , end(end)
{}

// ----------------------------------------------------------------------------
PluginSharedData::TimeInterval::~TimeInterval()
{}

}
