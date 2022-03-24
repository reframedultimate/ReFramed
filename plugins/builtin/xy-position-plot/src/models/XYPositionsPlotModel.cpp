#include "xy-positions-plot/models/XYPositionsPlotModel.hpp"
#include "xy-positions-plot/views/XYPositionsPlotView.hpp"
#include "xy-positions-plot/listeners/XYPositionsPlotListener.hpp"
#include "rfcommon/Frame.hpp"
#include "rfcommon/RunningGameSession.hpp"
#include "rfcommon/RunningTrainingSession.hpp"
#include "rfcommon/SavedGameSession.hpp"

// ----------------------------------------------------------------------------
XYPositionsPlotModel::XYPositionsPlotModel(RFPluginFactory* factory)
    : RealtimePlugin(factory)
{
}

// ----------------------------------------------------------------------------
XYPositionsPlotModel::~XYPositionsPlotModel()
{
}

// ----------------------------------------------------------------------------
void XYPositionsPlotModel::setSession(rfcommon::Session* session)
{
    session_ = session;
    session_->dispatcher.addListener(this);
    dispatcher.dispatch(&XYPositionsPlotListener::onXYPositionsPlotSessionSet, session);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotModel::clearSession(rfcommon::Session* session)
{
    dispatcher.dispatch(&XYPositionsPlotListener::onXYPositionsPlotSessionCleared, session);
    session->dispatcher.removeListener(this);
    session_.drop();
}

// ----------------------------------------------------------------------------
QWidget* XYPositionsPlotModel::createView()
{
    return new XYPositionsPlotView(this);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotModel::destroyView(QWidget* view)
{
    delete view;
}

// ----------------------------------------------------------------------------
void XYPositionsPlotModel::setSavedGameSession(rfcommon::SavedGameSession* session)
{
    setSession(session);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotModel::clearSavedGameSession(rfcommon::SavedGameSession* session)
{
    clearSession(session);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotModel::onProtocolTrainingStarted(rfcommon::RunningTrainingSession* training)
{
    setSession(training);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotModel::onProtocolTrainingResumed(rfcommon::RunningTrainingSession* training)
{
    setSession(training);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotModel::onProtocolTrainingReset(rfcommon::RunningTrainingSession* oldTraining, rfcommon::RunningTrainingSession* newTraining)
{
    // We probably want to reset everything in this case
    clearSession(oldTraining);
    setSession(newTraining);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotModel::onProtocolTrainingEnded(rfcommon::RunningTrainingSession* training)
{
    clearSession(training);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotModel::onProtocolMatchStarted(rfcommon::RunningGameSession* match)
{
    setSession(match);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotModel::onProtocolMatchResumed(rfcommon::RunningGameSession* match)
{
    setSession(match);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotModel::onProtocolMatchEnded(rfcommon::RunningGameSession* match)
{
    clearSession(match);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotModel::onRunningGameSessionPlayerNameChanged(int player, const rfcommon::SmallString<15>& name)
{
    dispatcher.dispatch(&XYPositionsPlotListener::onXYPositionsPlotNameChanged, player, name);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotModel::onRunningSessionNewUniqueFrame(int frameIdx, const rfcommon::Frame& frame)
{
    rfcommon::SmallVector<float, 8> posx;
    rfcommon::SmallVector<float, 8> posy;
    for (const auto& state : frame)
    {
        posx.push(state.posx());
        posy.push(state.posy());
    }
    dispatcher.dispatch(&XYPositionsPlotListener::onXYPositionsPlotNewValue, posx, posy);
}
