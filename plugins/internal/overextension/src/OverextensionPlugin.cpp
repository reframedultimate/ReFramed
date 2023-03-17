#include "overextension/OverextensionPlugin.hpp"
#include "overextension/models/OverextensionModel.hpp"
#include "overextension/views/OverextensionView.hpp"

#include "rfcommon/Profiler.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/Metadata.hpp"

#include <QWidget>

// ----------------------------------------------------------------------------
OverextensionPlugin::OverextensionPlugin(RFPluginFactory* factory, rfcommon::PluginContext* pluginCtx, rfcommon::MotionLabels* labels)
    : Plugin(factory)
    , Plugin::SharedDataInterface(pluginCtx, factory)
    , model_(new OverextensionModel)
    , labels_(labels)
{
    model_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
OverextensionPlugin::~OverextensionPlugin()
{
    model_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
rfcommon::Plugin::UIInterface* OverextensionPlugin::uiInterface() { return this; }
rfcommon::Plugin::RealtimeInterface* OverextensionPlugin::realtimeInterface() { return this; }
rfcommon::Plugin::ReplayInterface* OverextensionPlugin::replayInterface() { return this; }
rfcommon::Plugin::SharedDataInterface* OverextensionPlugin::sharedInterface() { return this; }
rfcommon::Plugin::VideoPlayerInterface* OverextensionPlugin::videoPlayerInterface() { return nullptr; }

// ----------------------------------------------------------------------------
QWidget* OverextensionPlugin::createView()
{
    PROFILE(OverextensionPlugin, createView);

    return new OverextensionView(model_.get(), labels_);
}

// ----------------------------------------------------------------------------
void OverextensionPlugin::destroyView(QWidget* view)
{
    PROFILE(OverextensionPlugin, destroyView);

    delete view;
}

// ----------------------------------------------------------------------------
void OverextensionPlugin::onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) {}
void OverextensionPlugin::onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) {}
void OverextensionPlugin::onProtocolConnectedToServer(const char* ipAddress, uint16_t port) {}
void OverextensionPlugin::onProtocolDisconnectedFromServer() {}

// ----------------------------------------------------------------------------
void OverextensionPlugin::onProtocolTrainingStarted(rfcommon::Session* training) {}
void OverextensionPlugin::onProtocolTrainingResumed(rfcommon::Session* training) {}
void OverextensionPlugin::onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining) {}
void OverextensionPlugin::onProtocolTrainingEnded(rfcommon::Session* training) {}
void OverextensionPlugin::onProtocolGameStarted(rfcommon::Session* match) {}
void OverextensionPlugin::onProtocolGameResumed(rfcommon::Session* match) {}
void OverextensionPlugin::onProtocolGameEnded(rfcommon::Session* match) {}

// ----------------------------------------------------------------------------
void OverextensionPlugin::onGameSessionLoaded(rfcommon::Session* session)
{
    model_->clearAll();

    auto map = session->tryGetMappingInfo();
    auto mdata = session->tryGetMetadata();
    if (mdata->fighterCount() != 2)
        return;
    auto fdata = session->tryGetFrameData();

    model_->startNewSession(map, mdata->asGame());
    model_->addAllFrames(fdata);
}
void OverextensionPlugin::onGameSessionUnloaded(rfcommon::Session* session)
{
    model_->clearAll();
}
void OverextensionPlugin::onTrainingSessionLoaded(rfcommon::Session* training) {}
void OverextensionPlugin::onTrainingSessionUnloaded(rfcommon::Session* training) {}

// ----------------------------------------------------------------------------
void OverextensionPlugin::onGameSessionSetLoaded(rfcommon::Session** games, int numGames)
{
    model_->clearAll();

    for (int i = 0; i != numGames; ++i)
    {
        auto map = games[i]->tryGetMappingInfo();
        auto mdata = games[i]->tryGetMetadata();
        if (mdata->fighterCount() != 2)
            continue;
        auto fdata = games[i]->tryGetFrameData();

        model_->startNewSession(map, mdata->asGame());
        model_->addAllFrames(fdata);
    }
}
void OverextensionPlugin::onGameSessionSetUnloaded(rfcommon::Session** games, int numGames) {}

// ----------------------------------------------------------------------------
void OverextensionPlugin::onSharedDataChanged() {}

// ----------------------------------------------------------------------------
void OverextensionPlugin::onPlayersChanged() {}
void OverextensionPlugin::onDataChanged() { exportTimeIntervals(); }
void OverextensionPlugin::onCurrentFighterChanged(int fighterIdx) { exportTimeIntervals(); }
void OverextensionPlugin::exportTimeIntervals()
{
    rfcommon::PluginSharedData data;
    if (model_->fighterCount() > 0)
    {
        int fighterIdx = model_->currentFighter();
        for (auto cat : {
             OverextensionModel::TRUE_COMBO,
             OverextensionModel::COMBO,
             OverextensionModel::WINNING_OVEREXTENSION,
             OverextensionModel::LOSING_OVEREXTENSION,
             OverextensionModel::TRADING_OVEREXTENSION
        }){
            rfcommon::Vector<rfcommon::PluginSharedData::TimeInterval> intervals;
            for (int moveIdx = 0; moveIdx != model_->moveCount(fighterIdx, cat); ++moveIdx)
                intervals.emplace("", model_->moveStart(fighterIdx, moveIdx, cat), model_->moveEnd(fighterIdx, moveIdx, cat));
            data.timeIntervalSets.insertAlways(OverextensionModel::categoryName(cat), intervals);
        }
    }
    setSharedData(std::move(data));
}
