#include "stats/StatsPlugin.hpp"
#include "stats/exporters/OBSExporter.hpp"
#include "stats/models/PlayerMeta.hpp"
#include "stats/models/StatsCalculator.hpp"
#include "stats/models/SettingsModel.hpp"
#include "stats/util/Paths.hpp"
#include "stats/views/MainView.hpp"

#include "rfcommon/Frame.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/MetaData.hpp"
#include "rfcommon/Session.hpp"

// ----------------------------------------------------------------------------
StatsPlugin::StatsPlugin(RFPluginFactory* factory, rfcommon::UserMotionLabels* userLabels, rfcommon::Hash40Strings* hash40Strings)
    : Plugin(factory)
    , playerMeta_(new PlayerMeta(userLabels, hash40Strings))
    , statsCalculator_(new StatsCalculator)
    , settingsModel_(new SettingsModel(dataDir().absoluteFilePath("settings.json")))
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
    // Statistics reset logic
    switch (settingsModel_->resetBehavior())
    {
        case SettingsModel::RESET_EACH_GAME: {
            statsCalculator_->resetStatistics();
        } break;

        case SettingsModel::RESET_EACH_SET: {
            // Reset statistics if this is game 1 in a set
            auto mdata = session->tryGetMetaData();
            if (mdata && mdata->type() == rfcommon::MetaData::GAME)
            {
                if (static_cast<rfcommon::GameMetaData*>(mdata)->gameNumber().value() == 1)
                    statsCalculator_->resetStatistics();
            }
        } break;
    }
}

// ----------------------------------------------------------------------------
void StatsPlugin::clearSession()
{
    // Unregister from current session
    if (frameData_)
    {
        frameData_->dispatcher.removeListener(this);
        frameData_.drop();
    }

    playerMeta_->clearMetaData();
}

// ----------------------------------------------------------------------------
bool StatsPlugin::addSession(rfcommon::Session* session)
{
    // We need mapping info, metadata and frame data in order to process
    // statistics
    auto map = session->tryGetMappingInfo();
    auto mdata = session->tryGetMetaData();
    auto fdata = session->tryGetFrameData();
    if (map == nullptr || mdata == nullptr || fdata == nullptr)
        return false;

    playerMeta_->setMetaData(map, mdata);

    // If the session already has frames, process them so we are caught up
    statsCalculator_->udpateStatisticsBulk(fdata);

    // Register as listeners so we are informed when data changes
    frameData_ = fdata;
    frameData_->dispatcher.addListener(this);

    return true;
}

// ----------------------------------------------------------------------------
void StatsPlugin::exportEmptyStats() const
{
    if (settingsModel_->exportToOBS())
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
void StatsPlugin::exportStats() const
{
    if (settingsModel_->exportToOBS())
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
rfcommon::Plugin::UIInterface* StatsPlugin::uiInterface() { return this; }
rfcommon::Plugin::RealtimeInterface* StatsPlugin::realtimeInterface() { return this; }
rfcommon::Plugin::ReplayInterface* StatsPlugin::replayInterface() { return this; }
rfcommon::Plugin::VisualizerInterface* StatsPlugin::visualizerInterface() { return nullptr; }
rfcommon::Plugin::VideoPlayerInterface* StatsPlugin::videoPlayerInterface() { return nullptr; }

// ----------------------------------------------------------------------------
QWidget* StatsPlugin::createView()
{
    // Create new instance of view. The view registers as a listener to this model
    //return new StatsView(model_.get());
    return new MainView(playerMeta_.get(), statsCalculator_.get(), settingsModel_.get());
}

// ----------------------------------------------------------------------------
void StatsPlugin::destroyView(QWidget* view)
{
    // ReFramed no longer needs the view, delete it
    delete view;
}

// ----------------------------------------------------------------------------
void StatsPlugin::onProtocolGameStarted(rfcommon::Session* game)
{
    resetStatsIfAppropriate(game);
    clearSession();
    if (addSession(game))
    {
        // OBS should display empty statistics, since a new game has started now
        exportEmptyStats();
    }

    weAreLive_ = true;
}

// ----------------------------------------------------------------------------
void StatsPlugin::onProtocolGameResumed(rfcommon::Session* game)
{
    resetStatsIfAppropriate(game);
    clearSession();
    if (addSession(game))
    {
        // OBS should display empty statistics, since a new game has started now
        exportEmptyStats();
    }

    weAreLive_ = true;
}

// ----------------------------------------------------------------------------
void StatsPlugin::onProtocolGameEnded(rfcommon::Session* game)
{
    weAreLive_ = false;

    exportStats();

    // We hold on to our reference to the data until a new session is started,
    // so that if settings change, the exporters still have data to export
}

// ----------------------------------------------------------------------------
void StatsPlugin::onProtocolTrainingStarted(rfcommon::Session* training)
{
    statsCalculator_->resetStatistics();
    clearSession();
}

// ----------------------------------------------------------------------------
void StatsPlugin::onProtocolTrainingResumed(rfcommon::Session* training)
{
    statsCalculator_->resetStatistics();
    clearSession();
}

// ----------------------------------------------------------------------------
void StatsPlugin::onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining) {}
void StatsPlugin::onProtocolTrainingEnded(rfcommon::Session* training) {}

// ----------------------------------------------------------------------------
void StatsPlugin::onGameSessionLoaded(rfcommon::Session* game)
{
    statsCalculator_->resetStatistics();
    clearSession();
    if (addSession(game))
        exportStats();
}

// ----------------------------------------------------------------------------
void StatsPlugin::onGameSessionUnloaded(rfcommon::Session* game)
{
    statsCalculator_->resetStatistics();
    clearSession();
}

// ----------------------------------------------------------------------------
void StatsPlugin::onTrainingSessionLoaded(rfcommon::Session* training)
{
    statsCalculator_->resetStatistics();
    clearSession();
}

// ----------------------------------------------------------------------------
void StatsPlugin::onTrainingSessionUnloaded(rfcommon::Session* training) {}

// ----------------------------------------------------------------------------
void StatsPlugin::onGameSessionSetLoaded(rfcommon::Session** games, int numGames)
{
    statsCalculator_->resetStatistics();

    for (int s = 0; s != numGames; ++s)
    {
        clearSession();
        addSession(games[s]);
    }

    exportStats();
}

// ----------------------------------------------------------------------------
void StatsPlugin::onGameSessionSetUnloaded(rfcommon::Session** games, int numGames)
{
    statsCalculator_->resetStatistics();
    clearSession();
}

// ----------------------------------------------------------------------------
void StatsPlugin::onFrameDataNewFrame(int frameIdx, const rfcommon::Frame<4>& frame)
{
    statsCalculator_->updateStatistics(frame);

    if (weAreLive_ && settingsModel_->exportIntervalOBS() > 0)
    {
        const int interfalFrames = settingsModel_->exportIntervalOBS() * 60;
        if (frameIdx % interfalFrames == 0)
            exportStats();
    }
}

// ----------------------------------------------------------------------------
void StatsPlugin::onSettingsStatsChanged()
{
    if (frameData_)
        exportStats();
    else
        exportEmptyStats();
}

// ----------------------------------------------------------------------------
void StatsPlugin::onSettingsOBSChanged()
{
    if (frameData_)
        exportStats();
    else
        exportEmptyStats();
}

// ----------------------------------------------------------------------------
// Unused callbacks
void StatsPlugin::onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) {}
void StatsPlugin::onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) {}
void StatsPlugin::onProtocolConnectedToServer(const char* ipAddress, uint16_t port) {}
void StatsPlugin::onProtocolDisconnectedFromServer() {}

void StatsPlugin::onFrameDataNewUniqueFrame(int frameIdx, const rfcommon::Frame<4>& frame) {}
