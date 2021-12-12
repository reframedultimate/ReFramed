#pragma once

#include "rfcommon/String.hpp"
#include "rfcommon/Types.hpp"

namespace rfcommon {
    class Session;
}

class DamageTimePlotListener
{
public:
    virtual void onDamageTimePlotSessionSet(rfcommon::Session* session) = 0;
    virtual void onDamageTimePlotSessionCleared(rfcommon::Session* session) = 0;
    virtual void onDamageTimePlotNameChanged(int playerIdx, const rfcommon::SmallString<15>& name) = 0;
    virtual void onDamageTimePlotNewValue(int playerIdx, rfcommon::Frame frame, float damage) = 0;
};
