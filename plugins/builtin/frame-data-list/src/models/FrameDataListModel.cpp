#include "frame-data-list/models/FrameDataListModel.hpp"
#include "frame-data-list/views/FrameDataListView.hpp"
#include "frame-data-list/listeners/FrameDataListListener.hpp"
#include "rfcommon/FighterFrame.hpp"
#include "rfcommon/RunningGameSession.hpp"
#include "rfcommon/RunningTrainingSession.hpp"
#include "rfcommon/SavedGameSession.hpp"

// ----------------------------------------------------------------------------
FrameDataListModel::FrameDataListModel(RFPluginFactory* factory)
    : RealtimePlugin(factory)
{
}

// ----------------------------------------------------------------------------
FrameDataListModel::~FrameDataListModel()
{
}

// ----------------------------------------------------------------------------
void FrameDataListModel::setSession(rfcommon::Session* session)
{
    session_ = session;
    dispatcher.dispatch(&FrameDataListListener::onFrameDataListSessionSet, session);
}

// ----------------------------------------------------------------------------
void FrameDataListModel::clearSession(rfcommon::Session* session)
{
    dispatcher.dispatch(&FrameDataListListener::onFrameDataListSessionCleared, session);
    session_.drop();
}

// ----------------------------------------------------------------------------
QWidget* FrameDataListModel::createView()
{
    return new FrameDataListView(this);
}

// ----------------------------------------------------------------------------
void FrameDataListModel::destroyView(QWidget* view)
{
    delete view;
}

// ----------------------------------------------------------------------------
void FrameDataListModel::setSavedGameSession(rfcommon::SavedGameSession* session)
{
    setSession(session);
}

// ----------------------------------------------------------------------------
void FrameDataListModel::clearSavedGameSession(rfcommon::SavedGameSession* session)
{
    clearSession(session);
}

// ----------------------------------------------------------------------------
void FrameDataListModel::onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) { (void)ipAddress; (void)port; }
void FrameDataListModel::onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) { (void)errormsg; (void)ipAddress; (void)port; }
void FrameDataListModel::onProtocolConnectedToServer(const char* ipAddress, uint16_t port) { (void)ipAddress; (void)port; }
void FrameDataListModel::onProtocolDisconnectedFromServer() {}

// ----------------------------------------------------------------------------
void FrameDataListModel::onProtocolTrainingStarted(rfcommon::RunningTrainingSession* training)
{
    setSession(training);
}

// ----------------------------------------------------------------------------
void FrameDataListModel::onProtocolTrainingResumed(rfcommon::RunningTrainingSession* training)
{
    setSession(training);
}

// ----------------------------------------------------------------------------
void FrameDataListModel::onProtocolTrainingReset(rfcommon::RunningTrainingSession* oldTraining, rfcommon::RunningTrainingSession* newTraining)
{
    // We probably want to clear the existing data in this case
    clearSession(oldTraining);
    setSession(newTraining);
}

// ----------------------------------------------------------------------------
void FrameDataListModel::onProtocolTrainingEnded(rfcommon::RunningTrainingSession* training)
{
    clearSession(training);
}

// ----------------------------------------------------------------------------
void FrameDataListModel::onProtocolMatchStarted(rfcommon::RunningGameSession* match)
{
    setSession(match);
}

// ----------------------------------------------------------------------------
void FrameDataListModel::onProtocolMatchResumed(rfcommon::RunningGameSession* match)
{
    setSession(match);
}

// ----------------------------------------------------------------------------
void FrameDataListModel::onProtocolMatchEnded(rfcommon::RunningGameSession* match)
{
    clearSession(match);
}
