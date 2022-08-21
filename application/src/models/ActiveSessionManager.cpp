#include "application/Util.hpp"
#include "application/listeners/ActiveSessionManagerListener.hpp"
#include "application/models/ActiveSessionManager.hpp"
#include "application/models/ReplayManager.hpp"
#include "rfcommon/Frame.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/MetaData.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/tcp_socket.h"
#include <QDateTime>

namespace rfapp {

// ----------------------------------------------------------------------------
ActiveSessionManager::ActiveSessionManager(Protocol* protocol, ReplayManager* manager, QObject* parent)
    : QObject(parent)
    , protocol_(protocol)
    , replayManager_(manager)
    , format_(rfcommon::SetFormat::fromType(rfcommon::SetFormat::FRIENDLIES))
    , gameNumber_(rfcommon::GameNumber::fromValue(1))
    , setNumber_(rfcommon::SetNumber::fromValue(1))
{
    protocol_->dispatcher.addListener(this);
    replayManager_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
ActiveSessionManager::~ActiveSessionManager()
{
    replayManager_->dispatcher.removeListener(this);
    protocol_->dispatcher.removeListener(this);

    if (activeMetaData_)
        activeMetaData_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void ActiveSessionManager::setSetFormat(const rfcommon::SetFormat& format)
{
    PROFILE(ActiveSessionManager, setSetFormat);

    if (activeMetaData_ && activeMetaData_->type() == rfcommon::MetaData::GAME)
    {
        auto meta = static_cast<rfcommon::GameMetaData*>(activeMetaData_.get());
        meta->setSetFormat(format);
    }
    else
        format_ = format;

    dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerSetFormatChanged, format);
}

// ----------------------------------------------------------------------------
void ActiveSessionManager::setP1Name(const QString& name)
{
    PROFILE(ActiveSessionManager, setP1Name);

    if (activeMetaData_
        && activeMetaData_->fighterCount() == 2
        && activeMetaData_->type() == rfcommon::MetaData::GAME)
    {
        auto meta = static_cast<rfcommon::GameMetaData*>(activeMetaData_.get());
        if (name == "")
            meta->setName(0, meta->tag(0));
        else
            meta->setName(0, name.toStdString().c_str());

        dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerPlayerNameChanged, 0, meta->name(0));
    }
    else
    {
        p1Name_ = name;
        dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerPlayerNameChanged, 0, rfcommon::String(name.toStdString().c_str()));
    }
}

// ----------------------------------------------------------------------------
void ActiveSessionManager::setP2Name(const QString& name)
{
    PROFILE(ActiveSessionManager, setP2Name);

    if (activeMetaData_
        && activeMetaData_->fighterCount() == 2
        && activeMetaData_->type() == rfcommon::MetaData::GAME)
    {
        auto meta = static_cast<rfcommon::GameMetaData*>(activeMetaData_.get());
        if (name == "")
            meta->setName(1, meta->tag(1));
        else
            meta->setName(1, name.toStdString().c_str());

        dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerPlayerNameChanged, 1, meta->name(1));
    }
    else
    {
        p2Name_ = name;
        dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerPlayerNameChanged, 1, rfcommon::String(name.toStdString().c_str()));
    }
}

// ----------------------------------------------------------------------------
void ActiveSessionManager::setGameNumber(rfcommon::GameNumber number)
{
    PROFILE(ActiveSessionManager, setGameNumber);

    if (activeMetaData_ && activeMetaData_->type() == rfcommon::MetaData::GAME)
    {
        auto meta = static_cast<rfcommon::GameMetaData*>(activeMetaData_.get());
        meta->setGameNumber(number);

        replayManager_->findFreeSetAndGameNumbers(activeMappingInfo_, meta);

        dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerSetNumberChanged, meta->setNumber());
        dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerGameNumberChanged, meta->gameNumber());
    }
    else
    {
        gameNumber_ = number;
        dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerGameNumberChanged, number);
    }
}

// ----------------------------------------------------------------------------
void ActiveSessionManager::setTrainingSessionNumber(rfcommon::GameNumber number)
{
    PROFILE(ActiveSessionManager, setTrainingSessionNumber);

    if (activeMetaData_ && activeMetaData_->type() == rfcommon::MetaData::TRAINING)
    {
        auto meta = static_cast<rfcommon::TrainingMetaData*>(activeMetaData_.get());
        meta->setSessionNumber(number);

        replayManager_->findFreeSetAndGameNumbers(activeMappingInfo_, meta);

        dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerTrainingSessionNumberChanged, meta->sessionNumber());
    }
    else
    {
        gameNumber_ = number;
        dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerGameNumberChanged, number);
    }
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
    if (pastGameMetaData_.size() == 0)
        return true;

    const auto& prev = pastGameMetaData_.back();

    // Player tags might have changed
    if (prev->tag(0) != meta->tag(0) ||
        prev->tag(1) != meta->tag(1))
    {
        return true;
    }

    // Player names might have changed
    if (prev->name(0) != meta->name(0) ||
        prev->name(1) != meta->name(1))
    {
        return true;
    }

    // Format might have changed
    if (prev->setFormat().type() != meta->setFormat().type())
        return true;

    // tally up wins for each player
    int win[2] = {0, 0};
    for (const auto& pastMeta : pastGameMetaData_)
        if (pastMeta->winner() >= 0)  // Could be -1. Shouldn't be, but who knows
            win[pastMeta->winner()]++;

    switch (meta->setFormat().type())
    {
        case rfcommon::SetFormat::BO3:
        case rfcommon::SetFormat::BO3MM: {
            if (win[0] >= 2 || win[1] >= 2)
                return true;
        } break;

        case rfcommon::SetFormat::BO5:
        case rfcommon::SetFormat::BO5MM: {
            if (win[0] >= 3 || win[1] >= 3)
                return true;
        } break;

        case rfcommon::SetFormat::BO7:
        case rfcommon::SetFormat::BO7MM: {
            if (win[0] >= 4 || win[1] >= 4)
                return true;
        } break;

        case rfcommon::SetFormat::FT5:
        case rfcommon::SetFormat::FT5MM: {
            if (win[0] >= 5 || win[1] >= 5)
                return true;
        } break;

        case rfcommon::SetFormat::FT10:
        case rfcommon::SetFormat::FT10MM: {
            if (win[0] >= 10 || win[1] >= 10)
                return true;
        } break;

        case rfcommon::SetFormat::FRIENDLIES:
        case rfcommon::SetFormat::PRACTICE:
        case rfcommon::SetFormat::TRAINING:
        case rfcommon::SetFormat::COACHING:
        case rfcommon::SetFormat::OTHER:
            break;
    }

    return false;
}

// ----------------------------------------------------------------------------
void ActiveSessionManager::onProtocolConnectedToServer(const char* ipAddress, uint16_t port)
{
    PROFILE(ActiveSessionManager, onProtocolConnectedToServer);

    dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerConnected, ipAddress, port);
}

// ----------------------------------------------------------------------------
void ActiveSessionManager::onProtocolDisconnectedFromServer()
{
    PROFILE(ActiveSessionManager, onProtocolDisconnectedFromServer);

    dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerDisconnected);
}

// ----------------------------------------------------------------------------
void ActiveSessionManager::onProtocolTrainingStarted(rfcommon::Session* training)
{
    PROFILE(ActiveSessionManager, onProtocolTrainingStarted);

    dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerTrainingStarted, training);
}

// ----------------------------------------------------------------------------
void ActiveSessionManager::onProtocolTrainingResumed(rfcommon::Session* training)
{
    PROFILE(ActiveSessionManager, onProtocolTrainingResumed);

    dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerTrainingStarted, training);
}

// ----------------------------------------------------------------------------
void ActiveSessionManager::onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining)
{
    PROFILE(ActiveSessionManager, onProtocolTrainingReset);

}

// ----------------------------------------------------------------------------
void ActiveSessionManager::onProtocolTrainingEnded(rfcommon::Session* training)
{
    PROFILE(ActiveSessionManager, onProtocolTrainingEnded);

    dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerTrainingEnded, training);
}

// ----------------------------------------------------------------------------
void ActiveSessionManager::onProtocolGameStarted(rfcommon::Session* game)
{
    PROFILE(ActiveSessionManager, onProtocolGameStarted);

    activeMappingInfo_ = game->tryGetMappingInfo();
    activeMetaData_ = game->tryGetMetaData();
    activeFrameData_ = game->tryGetFrameData();
    assert(activeMappingInfo_.notNull());
    assert(activeMetaData_.notNull());
    assert(activeFrameData_.notNull());
    assert(activeMetaData_->type() == rfcommon::MetaData::GAME);

    auto meta = static_cast<rfcommon::GameMetaData*>(activeMetaData_.get());

    // first off, copy the data we've stored from the UI into the new sessions
    // so comparing previous sessions is consistent
    meta->setSetFormat(format_);
    meta->setGameNumber(gameNumber_);
    meta->setSetNumber(setNumber_);
    if (meta->fighterCount() == 2)
    {
        if (p1Name_.length() > 0)
            meta->setName(0, p1Name_.toStdString().c_str());
        if (p2Name_.length() > 0)
            meta->setName(1, p2Name_.toStdString().c_str());
    }

    if (shouldStartNewSet(meta))
    {
        meta->setGameNumber(rfcommon::GameNumber::fromValue(1));
        meta->setSetNumber(rfcommon::SetNumber::fromValue(1));
        pastGameMetaData_.clear();
    }
    else
    {
        // Go to the next game in the set
        meta->setGameNumber(meta->gameNumber() + 1);
    }

    // Modify game/set numbers until we have a unique filename
    replayManager_->findFreeSetAndGameNumbers(activeMappingInfo_, meta);

    activeMetaData_->dispatcher.addListener(this);
    activeFrameData_->dispatcher.addListener(this);

    dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerSetNumberChanged, meta->setNumber());
    dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerGameNumberChanged, meta->gameNumber());
    dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerSetFormatChanged, meta->setFormat());
    if (meta->fighterCount() == 2)
    {
        dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerPlayerNameChanged,
                0, meta->name(0));
        dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerPlayerNameChanged,
                1, meta->name(1));
    }

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

    dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerGameEnded, game);
}

// ----------------------------------------------------------------------------
void ActiveSessionManager::onReplayManagerDefaultReplaySaveLocationChanged(const QDir& path)
{
    PROFILE(ActiveSessionManager, onReplayManagerDefaultReplaySaveLocationChanged);

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
    }
}

// ----------------------------------------------------------------------------
void ActiveSessionManager::onMetaDataTimeStartedChanged(rfcommon::TimeStamp timeStarted)
{
    PROFILE(ActiveSessionManager, onMetaDataTimeStartedChanged);

}

// ----------------------------------------------------------------------------
void ActiveSessionManager::onMetaDataTimeEndedChanged(rfcommon::TimeStamp timeEnded)
{
    PROFILE(ActiveSessionManager, onMetaDataTimeEndedChanged);

}

// ----------------------------------------------------------------------------
void ActiveSessionManager::onMetaDataPlayerNameChanged(int player, const rfcommon::String& name)
{
    PROFILE(ActiveSessionManager, onMetaDataPlayerNameChanged);

    if (activeMetaData_->fighterCount() == 2)
    {
        if (player == 0)
            dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerPlayerNameChanged, 0, name);
        else
            dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerPlayerNameChanged, 1, name);
    }
}

// ----------------------------------------------------------------------------
void ActiveSessionManager::onMetaDataSetNumberChanged(rfcommon::SetNumber number)
{
    PROFILE(ActiveSessionManager, onMetaDataSetNumberChanged);

    dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerSetNumberChanged, number);
}

// ----------------------------------------------------------------------------
void ActiveSessionManager::onMetaDataGameNumberChanged(rfcommon::GameNumber number)
{
    PROFILE(ActiveSessionManager, onMetaDataGameNumberChanged);

    dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerGameNumberChanged, number);
}

// ----------------------------------------------------------------------------
void ActiveSessionManager::onMetaDataSetFormatChanged(const rfcommon::SetFormat& format)
{
    PROFILE(ActiveSessionManager, onMetaDataSetFormatChanged);

    dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerSetFormatChanged, format);
}

// ----------------------------------------------------------------------------
void ActiveSessionManager::onMetaDataWinnerChanged(int winner)
{
    PROFILE(ActiveSessionManager, onMetaDataWinnerChanged);

    dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerWinnerChanged, winner);
}

// ----------------------------------------------------------------------------
void ActiveSessionManager::onMetaDataTrainingSessionNumberChanged(rfcommon::GameNumber number)
{
    PROFILE(ActiveSessionManager, onMetaDataTrainingSessionNumberChanged);

    dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerTrainingSessionNumberChanged, number);
}

// ----------------------------------------------------------------------------
void ActiveSessionManager::onFrameDataNewUniqueFrame(int frameIdx, const rfcommon::Frame<4>& frame)
{
    PROFILE(ActiveSessionManager, onFrameDataNewUniqueFrame);

    for (int fighterIdx = 0; fighterIdx != frame.count(); ++fighterIdx)
    {
        const auto& state = frame[fighterIdx];
        dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerFighterStateChanged,
            fighterIdx, state.damage(), int(state.stocks().count()));
    }

    dispatcher.dispatch(&ActiveSessionManagerListener::onActiveSessionManagerTimeRemainingChanged, frame[0].framesLeft().secondsLeft());
}

// ----------------------------------------------------------------------------
void ActiveSessionManager::onFrameDataNewFrame(int frameIdx, const rfcommon::Frame<4>& frame)
{
    PROFILE(ActiveSessionManager, onFrameDataNewFrame);

}

}
