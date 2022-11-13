#include "application/Util.hpp"
#include "application/listeners/ActiveSessionManagerListener.hpp"
#include "application/models/ActiveSessionManager.hpp"
#include "application/models/ReplayManager.hpp"
#include "rfcommon/Frame.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/GameMetaData.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/TrainingMetaData.hpp"
#include <QDateTime>

namespace rfapp {

// ----------------------------------------------------------------------------
ActiveSessionManager::ActiveSessionManager(Protocol* protocol, ReplayManager* manager, QObject* parent)
    : QObject(parent)
    , protocol_(protocol)
    , replayManager_(manager)
{
    protocol_->dispatcher.addListener(this);
    replayManager_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
ActiveSessionManager::~ActiveSessionManager()
{
    replayManager_->dispatcher.removeListener(this);
    protocol_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
Protocol* ActiveSessionManager::protocol() const
{
    PROFILE(ActiveSessionManager, protocol);

    return protocol_;
}

// ----------------------------------------------------------------------------
bool ActiveSessionManager::shouldStartNewSet(const rfcommon::GameMetaData* meta)
{
    PROFILE(ActiveSessionManager, shouldStartNewSet);

    // For any game that doesn't have exactly 2 players we don't care about sets
    if (meta->fighterCount() != 2)
        return true;

    // No past sessions? -> new set
    if (pastMetaData_.size() == 0)
        return true;

    const auto& prev = pastMetaData_.back();

    // Player tags might have changed
    if (prev->playerTag(0) != meta->playerTag(0) ||
        prev->playerTag(1) != meta->playerTag(1))
    {
        return true;
    }

    // Player names might have changed
    if (prev->playerName(0) != meta->playerName(0) ||
        prev->playerName(1) != meta->playerName(1))
    {
        return true;
    }

    // Format might have changed
    if (prev->setFormat().type() != meta->setFormat().type())
        return true;

    // tally up wins for each player
    int win[2] = {0, 0};
    for (const auto& pastMeta : pastMetaData_)
        if (pastMeta->winner() >= 0)  // Could be -1. Shouldn't be, but who knows
            win[pastMeta->winner()]++;

    switch (meta->setFormat().type())
    {
        case rfcommon::SetFormat::BO3: {
            if (win[0] >= 2 || win[1] >= 2)
                return true;
        } break;

        case rfcommon::SetFormat::BO5: {
            if (win[0] >= 3 || win[1] >= 3)
                return true;
        } break;

        case rfcommon::SetFormat::BO7: {
            if (win[0] >= 4 || win[1] >= 4)
                return true;
        } break;

        case rfcommon::SetFormat::FT5: {
            if (win[0] >= 5 || win[1] >= 5)
                return true;
        } break;

        case rfcommon::SetFormat::FT10: {
            if (win[0] >= 10 || win[1] >= 10)
                return true;
        } break;

        case rfcommon::SetFormat::FREE:
            break;
    }

    return false;
}

// ----------------------------------------------------------------------------
bool ActiveSessionManager::findFreeRoundAndGameNumbers(rfcommon::MappingInfo* map, rfcommon::MetaData* mdata)
{
    const QDir& dir = replayManager_->defaultGamePath();
    while (true)
    {
        QString fileName = rfcommon::ReplayFileParts::fromMetaData(map, mdata).toFileName().cStr();
        if (fileName == "")
            return false;
        if (QFileInfo::exists(dir.absoluteFilePath(fileName)) == false)
            return true;

        switch (mdata->type())
        {
            case rfcommon::MetaData::GAME: {
                auto gameMeta = mdata->asGame();
                const auto sessionNumber = gameMeta->round().number();
                gameMeta->setRound(rfcommon::Round::fromType(gameMeta->round().type(), sessionNumber + 1));
            } break;

            case rfcommon::MetaData::TRAINING: {
                auto trainingMeta = mdata->asTraining();
                trainingMeta->setSessionNumber(trainingMeta->sessionNumber() + 1);
            } break;
        }
    }
}

// ----------------------------------------------------------------------------
void ActiveSessionManager::onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) {}
void ActiveSessionManager::onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) {}
void ActiveSessionManager::onProtocolConnectedToServer(const char* ipAddress, uint16_t port) {}
void ActiveSessionManager::onProtocolDisconnectedFromServer() {}
void ActiveSessionManager::onProtocolTrainingStarted(rfcommon::Session* training) {}
void ActiveSessionManager::onProtocolTrainingResumed(rfcommon::Session* training) {}
void ActiveSessionManager::onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining) {}
void ActiveSessionManager::onProtocolTrainingEnded(rfcommon::Session* training) {}

// ----------------------------------------------------------------------------
void ActiveSessionManager::onProtocolGameStarted(rfcommon::Session* game)
{
    PROFILE(ActiveSessionManager, onProtocolGameStarted);

    auto map = game->tryGetMappingInfo();
    auto mdata = game->tryGetMetaData();
    assert(map); assert(mdata);

    auto allTagsMatch = [](rfcommon::MetaData* a, rfcommon::MetaData* b) -> bool {
        assert(a->fighterCount() == b->fighterCount());
        for (int i = 0; i != a->fighterCount(); ++i)
            if (a->playerTag(i) != b->playerTag(i))
                return false;
        return true;
    };

    auto allFightersMatch = [](rfcommon::MetaData* a, rfcommon::MetaData* b) -> bool {
        assert(a->fighterCount() == b->fighterCount());
        for (int i = 0; i != a->fighterCount(); ++i)
            if (a->playerFighterID(i) != b->playerFighterID(i))
                return false;
        return true;
    };

    // First off, if we have data from previous sessions, determine if the new
    // session has the same format and players. If yes, then copy the data from
    // the previous session to the current session so we can calculate the next
    // round/game number
    if (pastMetaData_.size() > 0)
    {
        rfcommon::MetaData* past = pastMetaData_.back();
        if (mdata->type() == past->type() &&
                mdata->fighterCount() == past->fighterCount() &&
                allTagsMatch(mdata, past) &&
                allFightersMatch(mdata, past))
        {
            switch (mdata->type())
            {
                case rfcommon::MetaData::GAME: {
                    auto gdata = mdata->asGame();
                    auto gpast = past->asGame();
                    gdata->setEventType(gpast->eventType());
                    gdata->setEventURL(gpast->eventURL().cStr());
                    gdata->setRound(gpast->round());
                    gdata->setScore(gpast->score());
                    gdata->setSetFormat(gpast->setFormat());
                    for (int i = 0; i != mdata->fighterCount(); ++i)
                        gdata->setPlayerName(i, gpast->playerName(i).cStr());
                    // TODO copy over all of the other fields
                } break;

                case rfcommon::MetaData::TRAINING: {
                    mdata->asTraining()->setSessionNumber(past->asTraining()->sessionNumber());
                } break;
            }
        }
    }

    switch (mdata->type())
    {
        case rfcommon::MetaData::GAME:
            if (shouldStartNewSet(mdata->asGame()))
            {
                auto gdata = mdata->asGame();
                gdata->setScore(rfcommon::ScoreCount::fromScore(0, 0));
                pastMetaData_.clear();
            }
            else if (pastMetaData_.size() > 0)
            {
                // Try to to the next game in the set
                if (int winner = pastMetaData_.back()->winner(); winner > -1)
                {
                    auto pastScore = pastMetaData_.back()->score();
                    mdata->asGame()->setScore(rfcommon::ScoreCount::fromScore(
                        pastScore.left() + (winner == 0), pastScore.right() + (winner == 1)
                    ));
                }
            }
            break;

        case rfcommon::MetaData::TRAINING:
            break;
    }

    // Modify game/set numbers until we have a unique filename
    findFreeRoundAndGameNumbers(map, mdata);

    dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerGameStarted, game);
}

// ----------------------------------------------------------------------------
void ActiveSessionManager::onProtocolGameResumed(rfcommon::Session* game)
{
    PROFILE(ActiveSessionManager, onProtocolGameResumed);

    ActiveSessionManager::onProtocolGameStarted(game);
}

// ----------------------------------------------------------------------------
void ActiveSessionManager::onProtocolGameEnded(rfcommon::Session* game)
{
    PROFILE(ActiveSessionManager, onProtocolGameEnded);
/*
    assert(activeMappingInfo_ == game->tryGetMappingInfo());
    assert(activeMetaData_ == game->tryGetMetaData());
    assert(activeMetaData_->type() == rfcommon::MetaData::GAME);

    // Save as replay. This will also add the session to the "All" replay group
    if (replayManager_->saveReplayWithDefaultSettings(game) == false)
    {
        // TODO need to handle this somehow
    }

    // In between sessions (when players are in the menu) there is no active
    // session, but it's still possible to edit the names/format/game number/etc
    // so copy the data out of the session here so it can be edited, and when
    // a new session starts again we copy the data into the session.
    auto meta = static_cast<rfcommon::GameMetaData*>(activeMetaData_.get());
    format_ = meta->setFormat();
    gameNumber_ = meta->gameNumber();
    setNumber_ = meta->setNumber();
    if (meta->fighterCount() == 2)
    {
        p1Name_ = meta->name(0).cStr();
        p2Name_ = meta->name(1).cStr();
    }

    activeFrameData_->dispatcher.removeListener(this);
    activeMetaData_->dispatcher.removeListener(this);
    pastGameMetaData_.push_back(meta);
    activeMappingInfo_.drop();
    activeMetaData_.drop();

    dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerGameEnded, game);*/
}

// ----------------------------------------------------------------------------
void ActiveSessionManager::onReplayManagerDefaultGamePathChanged(const QDir& path)
{
    PROFILE(ActiveSessionManager, onReplayManagerDefaultGamePathChanged);
/*
    (void)path;
    if (activeMetaData_.isNull())
        return;

    switch (activeMetaData_->type())
    {
        case rfcommon::MetaData::GAME: {
            auto meta = static_cast<rfcommon::GameMetaData*>(activeMetaData_.get());
            replayManager_->findFreeSetAndGameNumbers(activeMappingInfo_, meta);

            dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerSetNumberChanged, meta->setNumber());
            dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerGameNumberChanged, meta->gameNumber());
        } break;

        case rfcommon::MetaData::TRAINING: {
            auto meta = static_cast<rfcommon::TrainingMetaData*>(activeMetaData_.get());
            replayManager_->findFreeSetAndGameNumbers(activeMappingInfo_, meta);

            dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerTrainingSessionNumberChanged, meta->sessionNumber());
        } break;
    }*/
}

void ActiveSessionManager::onReplayManagerGroupAdded(ReplayGroup* group) {}
void ActiveSessionManager::onReplayManagerGroupNameChanged(ReplayGroup* group, const QString& oldName, const QString& newName) {}
void ActiveSessionManager::onReplayManagerGroupRemoved(ReplayGroup* group) {}

void ActiveSessionManager::onReplayManagerGamePathAdded(const QDir& path) {}
void ActiveSessionManager::onReplayManagerGamePathRemoved(const QDir& path) {}

void ActiveSessionManager::onReplayManagerVideoPathAdded(const QDir& path) {}
void ActiveSessionManager::onReplayManagerVideoPathRemoved(const QDir& path) {}

}
