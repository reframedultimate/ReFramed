#include "damage-time-plot/DamageTimePlotPlugin.hpp"
#include "damage-time-plot/models/DamageTimePlotModel.hpp"
#include "damage-time-plot/views/DamageTimePlotView.hpp"
#include "rfcommon/RunningGameSession.hpp"
#include "rfcommon/SavedGameSession.hpp"

// ----------------------------------------------------------------------------
DamageTimePlotPlugin::DamageTimePlotPlugin(RFPluginFactory* factory)
    : rfcommon::RealtimePlugin(factory)
    , model_(new DamageTimePlotModel)
{
}

// ----------------------------------------------------------------------------
DamageTimePlotPlugin::~DamageTimePlotPlugin()
{
}

// ----------------------------------------------------------------------------
QWidget* DamageTimePlotPlugin::createView()
{
    return new DamageTimePlotView(model_.get());
}

// ----------------------------------------------------------------------------
void DamageTimePlotPlugin::destroyView(QWidget* view)
{
    delete view;
}

// ----------------------------------------------------------------------------
void DamageTimePlotPlugin::onProtocolMatchStarted(rfcommon::RunningGameSession* match)
{
    model_->setSession(match);
}

// ----------------------------------------------------------------------------
void DamageTimePlotPlugin::onProtocolMatchResumed(rfcommon::RunningGameSession* match)
{
    model_->setSession(match);
}

// ----------------------------------------------------------------------------
void DamageTimePlotPlugin::onProtocolMatchEnded(rfcommon::RunningGameSession* match)
{
    model_->clearSession(match);
}

// ----------------------------------------------------------------------------
void DamageTimePlotPlugin::setSavedGameSession(rfcommon::SavedGameSession* session)
{
    model_->setSession(session);
}

// ----------------------------------------------------------------------------
void DamageTimePlotPlugin::clearSavedGameSession(rfcommon::SavedGameSession* session)
{
    model_->clearSession(session);
}
