#include "damage-time-plot/models/DamageTimePlotModel.hpp"
#include "damage-time-plot/models/DamageTimeCurveData.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/Metadata.hpp"
#include "rfcommon/FighterState.hpp"
#include "rfcommon/Frame.hpp"
#include "rfcommon/Profiler.hpp"

// ----------------------------------------------------------------------------
DamageTimeCurveData::DamageTimeCurveData(DamageTimePlotModel* model, rfcommon::Metadata* metadata, rfcommon::FrameData* frameData, int fighterIdx)
    : model_(model)
    , metadata_(metadata)
    , frameData_(frameData)
    , fighterIdx_(fighterIdx)
{
    frameData_->dispatcher.addListener(this);

    for (int frameIdx = 0; frameIdx != frameData->frameCount(); ++frameIdx)
    {
        const auto& state = frameData->stateAt(fighterIdx, frameIdx);
        appendDataPoint(state.framesLeft(), state.damage());
    }
    updateBoundingRect();
}

// ----------------------------------------------------------------------------
DamageTimeCurveData::~DamageTimeCurveData()
{
    frameData_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
bool DamageTimeCurveData::hasThisFrameData(const rfcommon::FrameData* frameData) const
{
    PROFILE(DamageTimeCurveData, hasThisFrameData);

    return frameData_ == frameData;
}

// ----------------------------------------------------------------------------
void DamageTimeCurveData::appendDataPoint(rfcommon::FramesLeft framesLeft, float damage)
{
    PROFILE(DamageTimeCurveData, appendDataPoint);

    // If the last point has the same damage value, then we don't add a new data
    // point, instead, the X value is simply adjusted so a straight line is drawn
    // from the last value to the current value
    if (points_.count() >= 2
        && points_.back(1).y() == points_.back(2).y()
        && points_.back(1).y() == damage)
    {
        points_.back(1).setX(framesLeft.secondsLeft());
    }
    else
    {
        points_.emplace(framesLeft.secondsLeft(), damage);
    }
}

// ----------------------------------------------------------------------------
void DamageTimeCurveData::onFrameDataNewUniqueFrame(int frameIdx, const rfcommon::Frame<4>& frame) { (void)frameIdx; (void)frame; }
void DamageTimeCurveData::onFrameDataNewFrame(int frameIdx, const rfcommon::Frame<4>& frame)
{
    PROFILE(DamageTimeCurveData, onFrameDataNewFrame);

    appendDataPoint(frame[fighterIdx_].framesLeft(), frame[fighterIdx_].damage());
    updateBoundingRect();
    model_->onCurveDataChanged();
}
