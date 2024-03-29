#include "stats/StatsPlugin.hpp"
#include "stats/exporters/OBSExporter.hpp"
#include "stats/exporters/WSExporter.hpp"
#include "stats/models/PlayerMeta.hpp"
#include "stats/models/StatsCalculator.hpp"
#include "stats/models/SettingsModel.hpp"
#include "stats/models/WebSocketServer.hpp"
#include "stats/util/Paths.hpp"
#include "stats/views/MainView.hpp"

#include "rfcommon/Frame.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/GameMetadata.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/PluginContext.hpp"
#include "rfcommon/PluginSharedData.hpp"

// ----------------------------------------------------------------------------
StatsPlugin::StatsPlugin(rfcommon::PluginContext* pluginCtx, RFPluginFactory* factory, rfcommon::MotionLabels* labels)
    : Plugin(factory)
    , Plugin::SharedDataInterface(pluginCtx, factory)
    , playerMeta_(new PlayerMeta(labels))
    , statsCalculator_(new StatsCalculator)
    , settingsModel_(new SettingsModel(dataDir().absoluteFilePath("settings.json")))
    , wsServer_(new WebSocketServer)
{
    settingsModel_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
StatsPlugin::~StatsPlugin()
{
    clearSession();
    settingsModel_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void StatsPlugin::resetStatsIfAppropriate(rfcommon::Session* session)
{
    PROFILE(StatsPlugin, resetStatsIfAppropriate);

    // Statistics reset logic
    switch (settingsModel_->resetBehavior())
    {
        case SettingsModel::RESET_EACH_GAME: {
            statsCalculator_->resetStatistics();
        } break;

        case SettingsModel::RESET_EACH_SET: {
            // Reset statistics if this is game 1 in a set
            auto mdata = session->tryGetMetadata();
            if (mdata && mdata->type() == rfcommon::Metadata::GAME)
            {
                if (mdata->asGame()->score().gameNumber().value() == 1)
                    statsCalculator_->resetStatistics();
            }
        } break;
    }
}

// ----------------------------------------------------------------------------
void StatsPlugin::clearSession()
{
    PROFILE(StatsPlugin, clearSession);

    // Unregister from current session
    if (frameData_)
    {
        frameData_->dispatcher.removeListener(this);
        frameData_.drop();
    }

    playerMeta_->clearMetadata();
}

// ----------------------------------------------------------------------------
bool StatsPlugin::addSession(rfcommon::Session* session)
{
    PROFILE(StatsPlugin, addSession);

    // We need mapping info, metadata and frame data in order to process
    // statistics
    auto map = session->tryGetMappingInfo();
    auto mdata = session->tryGetMetadata();
    auto fdata = session->tryGetFrameData();
    if (map == nullptr || mdata == nullptr || fdata == nullptr)
        return false;

    playerMeta_->setMetadata(map, mdata);

    // If the session already has frames, process them so we are caught up
    statsCalculator_->udpateStatisticsBulk(fdata);

    // Register as listeners so we are informed when data changes
    frameData_ = fdata;
    frameData_->dispatcher.addListener(this);

    return true;
}

// ----------------------------------------------------------------------------
void StatsPlugin::exportOBSEmptyStats() const
{
    PROFILE(StatsPlugin, exportOBSEmptyStats);

    if (settingsModel_->obsEnabled())
    {
        OBSExporter exporter(playerMeta_.get(), statsCalculator_.get(), settingsModel_.get());
        exporter.setPlayerTag(0, playerMeta_->name(0));
        exporter.setPlayerTag(1, playerMeta_->name(1));
        exporter.setPlayerCharacter(0, playerMeta_->character(0));
        exporter.setPlayerCharacter(1, playerMeta_->character(1));
        exporter.exportEmptyValues();
    }
}

// ----------------------------------------------------------------------------
void StatsPlugin::exportOBSStats() const
{
    PROFILE(StatsPlugin, exportOBSStats);

    if (settingsModel_->obsEnabled())
    {
        OBSExporter exporter(playerMeta_.get(), statsCalculator_.get(), settingsModel_.get());
        exporter.setPlayerTag(0, playerMeta_->name(0));
        exporter.setPlayerTag(1, playerMeta_->name(1));
        exporter.setPlayerCharacter(0, playerMeta_->character(0));
        exporter.setPlayerCharacter(1, playerMeta_->character(1));
        exporter.exportStatistics();
    }
}

// ----------------------------------------------------------------------------
void StatsPlugin::exportToOtherPlugins()
{
    PROFILE(StatsPlugin, exportToOtherPlugins);

    rfcommon::PluginSharedData data;
    for (int p = 0; p != playerMeta_->playerCount(); ++p)
    {
        rfcommon::Vector<rfcommon::PluginSharedData::TimeInterval> timeIntervals;
        for (const auto& interval : statsCalculator_->advantageStateTimeIntervals(p))
            timeIntervals.emplace("String", interval.start, interval.end);
        auto name = playerMeta_->name(p) + " Strings";
        data.timeIntervalSets.insertAlways(name.toUtf8().constData(), std::move(timeIntervals));
    }
    setSharedData(std::move(data));
}

// ----------------------------------------------------------------------------
void StatsPlugin::sendWebSocketStats(bool gameStarted, bool gameEnded) const
{
    PROFILE(StatsPlugin, sendWebSocketStats);

    if (settingsModel_->wsEnabled())
    {
        WSExporter exporter(playerMeta_.get(), statsCalculator_.get(), settingsModel_.get(), wsServer_.get());
        exporter.writeJSON(gameStarted, gameEnded);
    }
}

// ----------------------------------------------------------------------------
rfcommon::Plugin::UIInterface* StatsPlugin::uiInterface() { return this; }
rfcommon::Plugin::RealtimeInterface* StatsPlugin::realtimeInterface() { return this; }
rfcommon::Plugin::ReplayInterface* StatsPlugin::replayInterface() { return this; }
rfcommon::Plugin::SharedDataInterface* StatsPlugin::sharedInterface() { return this; }
rfcommon::Plugin::VideoPlayerInterface* StatsPlugin::videoPlayerInterface() { return nullptr; }

// ----------------------------------------------------------------------------
QWidget* StatsPlugin::createView()
{
    PROFILE(StatsPlugin, createView);

    // Create new instance of view. The view registers as a listener to this model
    //return new StatsView(model_.get());
    return new MainView(playerMeta_.get(), statsCalculator_.get(), settingsModel_.get(), wsServer_.get());
}

// ----------------------------------------------------------------------------
void StatsPlugin::destroyView(QWidget* view)
{
    PROFILE(StatsPlugin, destroyView);

    // ReFramed no longer needs the view, delete it
    delete view;
}

// ----------------------------------------------------------------------------
void StatsPlugin::onProtocolGameStarted(rfcommon::Session* game)
{
    PROFILE(StatsPlugin, onProtocolGameStarted);

    resetStatsIfAppropriate(game);
    clearSession();
    if (addSession(game))
    {
        // OBS should display empty statistics, since a new game has started now
        exportOBSEmptyStats();
        sendWebSocketStats(true, false);
    }

    weAreLive_ = true;
}

// ----------------------------------------------------------------------------
void StatsPlugin::onProtocolGameResumed(rfcommon::Session* game)
{
    PROFILE(StatsPlugin, onProtocolGameResumed);

    resetStatsIfAppropriate(game);
    clearSession();
    if (addSession(game))
    {
        // OBS should display empty statistics, since a new game has started now
        exportOBSEmptyStats();
        sendWebSocketStats(true, false);
    }

    weAreLive_ = true;
}

// ----------------------------------------------------------------------------
void StatsPlugin::onProtocolGameEnded(rfcommon::Session* game)
{
    PROFILE(StatsPlugin, onProtocolGameEnded);

    weAreLive_ = false;

    exportOBSStats();
    sendWebSocketStats(false, true);

    // We hold on to our reference to the data until a new session is started,
    // so that if settings change, the exporters still have data to export
}

// ----------------------------------------------------------------------------
void StatsPlugin::onProtocolTrainingStarted(rfcommon::Session* training)
{
    PROFILE(StatsPlugin, onProtocolTrainingStarted);

    statsCalculator_->resetStatistics();
    clearSession();
}

// ----------------------------------------------------------------------------
void StatsPlugin::onProtocolTrainingResumed(rfcommon::Session* training)
{
    PROFILE(StatsPlugin, onProtocolTrainingResumed);

    statsCalculator_->resetStatistics();
    clearSession();
}

// ----------------------------------------------------------------------------
void StatsPlugin::onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining) {}
void StatsPlugin::onProtocolTrainingEnded(rfcommon::Session* training) {}

// ----------------------------------------------------------------------------
void StatsPlugin::onGameSessionLoaded(rfcommon::Session* game)
{
    PROFILE(StatsPlugin, onGameSessionLoaded);

    statsCalculator_->resetStatistics();
    clearSession();
    if (addSession(game))
    {
        exportOBSStats();
        exportToOtherPlugins();
        sendWebSocketStats(false, false);
    }
}

// ----------------------------------------------------------------------------
void StatsPlugin::onGameSessionUnloaded(rfcommon::Session* game)
{
    PROFILE(StatsPlugin, onGameSessionUnloaded);

    statsCalculator_->resetStatistics();
    clearSession();
}

// ----------------------------------------------------------------------------
void StatsPlugin::onTrainingSessionLoaded(rfcommon::Session* training)
{
    PROFILE(StatsPlugin, onTrainingSessionLoaded);

    statsCalculator_->resetStatistics();
    clearSession();
}

// ----------------------------------------------------------------------------
void StatsPlugin::onTrainingSessionUnloaded(rfcommon::Session* training) {}

// ----------------------------------------------------------------------------
void StatsPlugin::onGameSessionSetLoaded(rfcommon::Session** games, int numGames)
{
    PROFILE(StatsPlugin, onGameSessionSetLoaded);

    statsCalculator_->resetStatistics();

    for (int s = 0; s != numGames; ++s)
    {
        clearSession();
        addSession(games[s]);
    }

    exportOBSStats();
    sendWebSocketStats(false, false);
}

// ----------------------------------------------------------------------------
void StatsPlugin::onGameSessionSetUnloaded(rfcommon::Session** games, int numGames)
{
    PROFILE(StatsPlugin, onGameSessionSetUnloaded);

    statsCalculator_->resetStatistics();
    clearSession();
}

// ----------------------------------------------------------------------------
void StatsPlugin::onFrameDataNewFrame(int frameIdx, const rfcommon::Frame<4>& frame)
{
    PROFILE(StatsPlugin, onFrameDataNewFrame);

    statsCalculator_->updateStatistics(frame);

    if (weAreLive_ && settingsModel_->obsExportInterval() > 0)
    {
        const int intervalFrames = settingsModel_->obsExportInterval() * 60;
        if (frameIdx % intervalFrames == 0)
            exportOBSStats();
    }

    if (weAreLive_ && (frameIdx % 60) == 0)
    {
        sendWebSocketStats(false, false);
    }
}

// ----------------------------------------------------------------------------
void StatsPlugin::onSettingsStatsChanged()
{
    PROFILE(StatsPlugin, onSettingsStatsChanged);

    if (frameData_)
    {
        exportOBSStats();
        sendWebSocketStats(false, false);
        exportToOtherPlugins();
    }
    else
        exportOBSEmptyStats();
}

// ----------------------------------------------------------------------------
void StatsPlugin::onSettingsOBSChanged()
{
    PROFILE(StatsPlugin, onSettingsOBSChanged);

    if (frameData_)
    {
        exportOBSStats();
        sendWebSocketStats(false, false);
    }
    else
        exportOBSEmptyStats();
}

// ----------------------------------------------------------------------------
void StatsPlugin::onSettingsWSChanged()
{
    PROFILE(StatsPlugin, onSettingsWSChanged);
}

// ----------------------------------------------------------------------------
// Unused callbacks
void StatsPlugin::onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) {}
void StatsPlugin::onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) {}
void StatsPlugin::onProtocolConnectedToServer(const char* ipAddress, uint16_t port) {}
void StatsPlugin::onProtocolDisconnectedFromServer() {}

void StatsPlugin::onFrameDataNewUniqueFrame(int frameIdx, const rfcommon::Frame<4>& frame) {}
