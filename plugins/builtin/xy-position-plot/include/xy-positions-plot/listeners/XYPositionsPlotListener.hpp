#pragma once

#include "rfcommon/String.hpp"
#include "rfcommon/Types.hpp"

namespace rfcommon {
    class Session;
}

class XYPositionsPlotListener
{
public:
    virtual void onXYPositionsPlotSessionSet(rfcommon::Session* session) = 0;
    virtual void onXYPositionsPlotSessionCleared(rfcommon::Session* session) = 0;
    virtual void onXYPositionsPlotNameChanged(int playerIdx, const rfcommon::SmallString<15>& name) = 0;
    virtual void onXYPositionsPlotNewValue(int playerIdx, float posx, float posy) = 0;
};
