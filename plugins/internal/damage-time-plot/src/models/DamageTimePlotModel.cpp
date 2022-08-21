#include "damage-time-plot/models/DamageTimeCurveData.hpp"
#include "damage-time-plot/models/DamageTimePlotModel.hpp"
#include "damage-time-plot/views/DamageTimePlotView.hpp"
#include "damage-time-plot/listeners/DamageTimePlotListener.hpp"
#include "rfcommon/Frame.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/MetaData.hpp"
#include "rfcommon/FrameData.hpp"
#include "qwt_series_data.h"

// ----------------------------------------------------------------------------
DamageTimePlotModel::DamageTimePlotModel()
{}

// ----------------------------------------------------------------------------
DamageTimePlotModel::~DamageTimePlotModel()
{
    clearAll();
}

// ----------------------------------------------------------------------------
void DamageTimePlotModel::addSession(rfcommon::Session* session)
{
    PROFILE(DamageTimePlotModel, addSession);

    rfcommon::FrameData* frameData = session->tryGetFrameData();
    rfcommon::MetaData* metaData = session->tryGetMetaData();
    if (frameData == nullptr)
        return;  // No frame data, no point

    sessions_.emplace(session);

    dispatcher.dispatch(&DamageTimePlotListener::onDataSetChanged);
}

// ----------------------------------------------------------------------------
void DamageTimePlotModel::clearAll()
{
    PROFILE(DamageTimePlotModel, clearAll);

    sessions_.clearCompact();
    dispatcher.dispatch(&DamageTimePlotListener::onDataSetChanged);
}

// ----------------------------------------------------------------------------
int DamageTimePlotModel::sessionCount() const
{
    PROFILE(DamageTimePlotModel, sessionCount);

    return sessions_.count();
}

// ----------------------------------------------------------------------------
int DamageTimePlotModel::fighterCount(int sessionIdx)
{
    PROFILE(DamageTimePlotModel, fighterCount);

    return sessions_[sessionIdx]->tryGetFrameData()->fighterCount();
}

// ----------------------------------------------------------------------------
DamageTimeCurveData* DamageTimePlotModel::newCurveData(int sessionIdx, int fighterIdx)
{
    PROFILE(DamageTimePlotModel, newCurveData);

    return new DamageTimeCurveData(this, sessions_[sessionIdx]->tryGetMetaData(), sessions_[sessionIdx]->tryGetFrameData(), fighterIdx);
}

// ----------------------------------------------------------------------------
void DamageTimePlotModel::onCurveDataChanged()
{
    PROFILE(DamageTimePlotModel, onCurveDataChanged);

    dispatcher.dispatch(&DamageTimePlotListener::onDataChanged);
}
