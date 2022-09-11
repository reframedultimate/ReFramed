#include "rfcommon/VisualizerData.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
VisualizerData::TimeInterval::TimeInterval(const char* name, FrameIndex start, FrameIndex end)
    : name(name)
    , start(start)
    , end(end)
{}

// ----------------------------------------------------------------------------
VisualizerData::TimeInterval::~TimeInterval()
{}

}
