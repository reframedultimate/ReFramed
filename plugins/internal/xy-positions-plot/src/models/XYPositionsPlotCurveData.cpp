#include "xy-positions-plot/models/XYPositionsPlotModel.hpp"
#include "xy-positions-plot/models/XYPositionsPlotCurveData.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/Metadata.hpp"
#include "rfcommon/FighterState.hpp"
#include "rfcommon/Frame.hpp"
#include "rfcommon/Profiler.hpp"

// ----------------------------------------------------------------------------
XYPositionsPlotCurveData::XYPositionsPlotCurveData(XYPositionsPlotModel* model, rfcommon::FrameData* frameData, int fighterIdx)
    : model_(model)
    , frameData_(frameData)
    , fighterIdx_(fighterIdx)
{
    frameData_->dispatcher.addListener(this);

    for (int frameIdx = 0; frameIdx != frameData->frameCount(); ++frameIdx)
    {
        const auto& state = frameData->stateAt(fighterIdx, frameIdx);
        appendDataPoint(state.pos());
    }
    updateBoundingRect();
}

// ----------------------------------------------------------------------------
XYPositionsPlotCurveData::~XYPositionsPlotCurveData()
{
    frameData_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
bool XYPositionsPlotCurveData::hasThisFrameData(const rfcommon::FrameData* frameData) const
{
    PROFILE(XYPositionsPlotCurveData, hasThisFrameData);

    return frameData_ == frameData;
}

// ----------------------------------------------------------------------------
void XYPositionsPlotCurveData::appendDataPoint(const rfcommon::Vec2& pos)
{
    PROFILE(XYPositionsPlotCurveData, appendDataPoint);

    // Only add if the position is different from the last
    if (points_.count() == 0
        || rfcommon::Vec2::fromValues(points_.back().x(), points_.back().y()) != pos)
    {
        points_.emplace(pos.x(), pos.y());
    }
}

// ----------------------------------------------------------------------------
void XYPositionsPlotCurveData::onFrameDataNewUniqueFrame(int frameIdx, const rfcommon::Frame<4>& frame)
{
    PROFILE(XYPositionsPlotCurveData, onFrameDataNewUniqueFrame);

    appendDataPoint(frame[fighterIdx_].pos());
    updateBoundingRect();
    model_->onCurveDataChanged();
}


// ----------------------------------------------------------------------------
void XYPositionsPlotCurveData::onFrameDataNewFrame(int frameIdx, const rfcommon::Frame<4> & frame) 
{
    PROFILE(XYPositionsPlotCurveData, onFrameDataNewFrame);
 
    (void)frameIdx; 
    (void)frame; 
}
