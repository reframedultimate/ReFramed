#include "damage-time-plot/models/DamageTimeCurveData.hpp"
#include "damage-time-plot/models/DamageTimePlotModel.hpp"
#include "damage-time-plot/views/DamageTimePlotView.hpp"
#include "damage-time-plot/listeners/DamageTimePlotListener.hpp"
#include "rfcommon/Frame.hpp"
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
    sessions_.clearCompact();
    dispatcher.dispatch(&DamageTimePlotListener::onDataSetChanged);
}

// ----------------------------------------------------------------------------
int DamageTimePlotModel::sessionCount() const
{
    return sessions_.count();
}

// ----------------------------------------------------------------------------
int DamageTimePlotModel::fighterCount(int sessionIdx)
{
    return sessions_[sessionIdx]->tryGetFrameData()->fighterCount();
}

// ----------------------------------------------------------------------------
DamageTimeCurveData* DamageTimePlotModel::newCurveData(int sessionIdx, int fighterIdx)
{
    return new DamageTimeCurveData(this, sessions_[sessionIdx]->tryGetMetaData(), sessions_[sessionIdx]->tryGetFrameData(), fighterIdx);
}

// ----------------------------------------------------------------------------
void DamageTimePlotModel::onCurveDataChanged()
{
    dispatcher.dispatch(&DamageTimePlotListener::onDataChanged);
}
