#include "stage-stats/StageStatsPlugin.hpp"
#include "stage-stats/models/StageStatsModel.hpp"
#include "stage-stats/views/StageStatsView.hpp"
#include "rfcommon/MetaData.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Session.hpp"

// ----------------------------------------------------------------------------
StageStatsPlugin::StageStatsPlugin(RFPluginFactory* factory)
    : Plugin(factory)
    , model_(new StageStatsModel)
{
}

// ----------------------------------------------------------------------------
StageStatsPlugin::~StageStatsPlugin()
{
}

// ----------------------------------------------------------------------------
rfcommon::Plugin::UIInterface* StageStatsPlugin::uiInterface() { return this; }
rfcommon::Plugin::RealtimeInterface* StageStatsPlugin::realtimeInterface() { return nullptr; }
rfcommon::Plugin::ReplayInterface* StageStatsPlugin::replayInterface() { return this; }
rfcommon::Plugin::VisualizerInterface* StageStatsPlugin::visualizerInterface() { return nullptr; }
rfcommon::Plugin::VideoPlayerInterface* StageStatsPlugin::videoPlayerInterface() { return nullptr; }

// ----------------------------------------------------------------------------
QWidget* StageStatsPlugin::createView()
{
    PROFILE(StageStatsPlugin, createView);

    return new StageStatsView(model_.get());
}

// ----------------------------------------------------------------------------
void StageStatsPlugin::destroyView(QWidget* view)
{
    PROFILE(StageStatsPlugin, destroyView);

    delete view;
}

// ----------------------------------------------------------------------------
void StageStatsPlugin::onGameSessionLoaded(rfcommon::Session* session) {}
void StageStatsPlugin::onGameSessionUnloaded(rfcommon::Session* session) {}
void StageStatsPlugin::onTrainingSessionLoaded(rfcommon::Session* training) {}
void StageStatsPlugin::onTrainingSessionUnloaded(rfcommon::Session* training) {}

// ----------------------------------------------------------------------------
void StageStatsPlugin::onGameSessionSetLoaded(rfcommon::Session** games, int numGames)
{
    model_->clearStats();
    for (int i = 0; i != numGames; ++i)
        if (auto mdata = games[i]->tryGetMetaData())
            if (auto map = games[i]->tryGetMappingInfo())
                if (mdata->type() == rfcommon::MetaData::GAME)
                    model_->addSessionData(map, static_cast<rfcommon::GameMetaData*>(mdata));
    model_->notifyUpdated();
}

// ----------------------------------------------------------------------------
void StageStatsPlugin::onGameSessionSetUnloaded(rfcommon::Session** games, int numGames)
{
    model_->clearStats();
    model_->notifyUpdated();
}
