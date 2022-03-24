#include "damage-time-plot/models/DamageTimePlotModel.hpp"
#include "damage-time-plot/views/DamageTimePlotView.hpp"
#include "damage-time-plot/listeners/DamageTimePlotListener.hpp"
#include "rfcommon/Frame.hpp"
#include "rfcommon/RunningGameSession.hpp"
#include "rfcommon/SavedGameSession.hpp"
#include "qwt_series_data.h"

// ----------------------------------------------------------------------------
void DamageTimePlotModel::setSession(rfcommon::Session* session)
{
    data_.clearCompact();
    data_.resize(session->fighterCount());
    for (int frameIdx = 0; frameIdx != session->frameCount(); ++frameIdx)
        for (int fighterIdx = 0; fighterIdx != session->fighterCount(); ++fighterIdx)
        {
            const auto& state = session->state(frameIdx, fighterIdx);
            appendDataPoint(fighterIdx, state.framesLeft(), state.damage());
        }

    session_ = session;
    session_->dispatcher.addListener(this);

    dispatcher.dispatch(&DamageTimePlotListener::onDamageTimePlotStartNew);
}

// ----------------------------------------------------------------------------
void DamageTimePlotModel::clearSession(rfcommon::Session* session)
{
    session->dispatcher.removeListener(this);
    session_.drop();
}

// ----------------------------------------------------------------------------
int DamageTimePlotModel::fighterCount() const
{
    return session_ ? session_->fighterCount() : 0;
}

// ----------------------------------------------------------------------------
const rfcommon::SmallString<15>& DamageTimePlotModel::name(int fighterIdx) const
{
    return session_->name(fighterIdx);
}

// ----------------------------------------------------------------------------
void DamageTimePlotModel::appendDataPoint(int fighterIdx, rfcommon::FramesLeft framesLeft, float damage)
{
    auto& fdata = data_[fighterIdx];

    // If the last point has the same damage value, then we don't add a new data
    // point, instead, the X value is simply adjusted so a straight line is drawn
    // from the last value to the current value
    if (fdata.count() >= 2
        && fdata.back(1).damage == fdata.back(2).damage
        && fdata.back(1).damage == damage)
    {
        fdata.back(1).secondsLeft = framesLeft.secondsLeft();
    }
    else
    {
        data_[fighterIdx].push({
            (float)framesLeft.secondsLeft(),
            (float)damage
        });
    }
}

// ----------------------------------------------------------------------------
void DamageTimePlotModel::onRunningGameSessionPlayerNameChanged(int playerIdx, const rfcommon::SmallString<15>& name)
{
    dispatcher.dispatch(&DamageTimePlotListener::onDamageTimePlotNameChanged, playerIdx);
}

// ----------------------------------------------------------------------------
void DamageTimePlotModel::onRunningSessionNewFrame(int frameIdx, const rfcommon::Frame& frame)
{
    for (int fighterIdx = 0; fighterIdx != frame.fighterCount(); ++fighterIdx)
    {
        const auto& state = frame.fighter(fighterIdx);
        appendDataPoint(fighterIdx, state.framesLeft(), state.damage());
    }

    dispatcher.dispatch(&DamageTimePlotListener::onDamageTimePlotDataChanged);
}
