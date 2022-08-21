#include "damage-time-plot/models/DamageTimePlotModel.hpp"
#include "damage-time-plot/models/DamageTimeCurveData.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/MetaData.hpp"
#include "rfcommon/FighterState.hpp"
#include "rfcommon/Frame.hpp"
#include "rfcommon/Profiler.hpp"

// ----------------------------------------------------------------------------
DamageTimeCurveData::DamageTimeCurveData(DamageTimePlotModel* model, rfcommon::MetaData* metaData, rfcommon::FrameData* frameData, int fighterIdx)
    : model_(model)
    , metaData_(metaData)
    , frameData_(frameData)
    , fighterIdx_(fighterIdx)
{
    frameData_->dispatcher.addListener(this);
    if (metaData_)
        metaData_->dispatcher.addListener(this);

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
    if (metaData_)
        metaData_->dispatcher.removeListener(this);
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
void DamageTimeCurveData::onMetaDataTimeStartedChanged(rfcommon::TimeStamp timeStarted) { (void)timeStarted; }
void DamageTimeCurveData::onMetaDataTimeEndedChanged(rfcommon::TimeStamp timeEnded) { (void)timeEnded; }

// ----------------------------------------------------------------------------
void DamageTimeCurveData::onMetaDataPlayerNameChanged(int fighterIdx, const rfcommon::String& name)
{
    PROFILE(DamageTimeCurveData, onMetaDataPlayerNameChanged);

}

// ----------------------------------------------------------------------------
void DamageTimeCurveData::onMetaDataSetNumberChanged(rfcommon::SetNumber number) { (void)number; }
void DamageTimeCurveData::onMetaDataGameNumberChanged(rfcommon::GameNumber number) { (void)number; }
void DamageTimeCurveData::onMetaDataSetFormatChanged(const rfcommon::SetFormat& format) { (void)format; }
void DamageTimeCurveData::onMetaDataWinnerChanged(int winnerPlayerIdx) { (void)winnerPlayerIdx; }
void DamageTimeCurveData::onMetaDataTrainingSessionNumberChanged(rfcommon::GameNumber number) { (void)number; }
void DamageTimeCurveData::onFrameDataNewUniqueFrame(int frameIdx, const rfcommon::Frame<4>& frame) { (void)frameIdx; (void)frame; }

// ----------------------------------------------------------------------------
void DamageTimeCurveData::onFrameDataNewFrame(int frameIdx, const rfcommon::Frame<4>& frame)
{
    PROFILE(DamageTimeCurveData, onFrameDataNewFrame);

    appendDataPoint(frame[fighterIdx_].framesLeft(), frame[fighterIdx_].damage());
    updateBoundingRect();
    model_->onCurveDataChanged();
}
