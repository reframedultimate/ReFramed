#include "application/models/Protocol.hpp"
#include "application/models/ProtocolTask.hpp"
#include "application/models/ProtocolTaskSSB64.hpp"

#include "rfcommon/Frame.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/Log.hpp"
#include "rfcommon/MappedFile.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/Metadata.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/ProtocolListener.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/Utf8.hpp"

#include <QTimer>
#include <QStandardPaths>
#include <QDir>

namespace rfapp {

// ----------------------------------------------------------------------------
Protocol::Protocol(QObject* parent)
    : QObject(parent)
    , trainingEndedProxyWasCalled_(false)
{
    tryLoadGlobalMappingInfo();
}

// ----------------------------------------------------------------------------
Protocol::~Protocol()
{
    disconnectFromServer();
}

// ----------------------------------------------------------------------------
void Protocol::connectToServer(const QString& ipAddress, uint16_t port)
{
    PROFILE(Protocol, connectToServer);

    disconnectFromServer();

    uint32_t mappingInfoChecksum = globalMappingInfo_.notNull() ? globalMappingInfo_->checksum() : 0;
    task_.reset(new ProtocolTask(ipAddress, port, mappingInfoChecksum, rfcommon::Log::root()->child("protocol")));

    QByteArray ba = ipAddress.toUtf8();
    const char* ipCstr = ba.constData();
    dispatcher.dispatch(&rfcommon::ProtocolListener::onProtocolAttemptConnectToServer, ipCstr, port);

    connect(task_.get(), &ProtocolTask::connectionSuccess,
            this, &Protocol::onConnectionSuccess);
    connect(task_.get(), &ProtocolTask::connectionFailure,
            this, &Protocol::onConnectionFailure);
    connect(task_.get(), &ProtocolTask::connectionClosed,
            this, &Protocol::onConnectionClosed);

    connect(task_.get(), &ProtocolTask::mappingInfoReceived,
            this, &Protocol::onMappingInfoReceived);
    connect(task_.get(), &ProtocolTask::trainingStarted,
            this, &Protocol::onTrainingStartedProxy);
    connect(task_.get(), &ProtocolTask::trainingResumed,
            this, &Protocol::onTrainingResumed);
    connect(task_.get(), &ProtocolTask::trainingEnded,
            this, &Protocol::onTrainingEndedProxy);
    connect(task_.get(), &ProtocolTask::gameStarted,
            this, &Protocol::onGameStarted);
    connect(task_.get(), &ProtocolTask::gameResumed,
            this, &Protocol::onGameResumed);
    connect(task_.get(), &ProtocolTask::gameEnded,
            this, &Protocol::onGameEnded);
    connect(task_.get(), &ProtocolTask::fighterState,
            this, &Protocol::onFighterState);

    task_->start();
}

// ----------------------------------------------------------------------------
void Protocol::connectToSSB64Process()
{
    PROFILE(Protocol, connectToSSB64Process);

    disconnectFromServer();

    uint32_t mappingInfoChecksum = globalMappingInfo_.notNull() ? globalMappingInfo_->checksum() : 0;
    ssb64Task_.reset(new ProtocolTaskSSB64(rfcommon::Log::root()->child("protocol")));

    dispatcher.dispatch(&rfcommon::ProtocolListener::onProtocolAttemptConnectToServer, "N64 Emulator", 0);

    connect(ssb64Task_.get(), &ProtocolTaskSSB64::connectionSuccess,
        this, &Protocol::onConnectionSuccess);
    connect(ssb64Task_.get(), &ProtocolTaskSSB64::connectionFailure,
        this, &Protocol::onConnectionFailure);
    connect(ssb64Task_.get(), &ProtocolTaskSSB64::connectionClosed,
        this, &Protocol::onConnectionClosed);

    connect(ssb64Task_.get(), &ProtocolTaskSSB64::trainingStarted,
        this, &Protocol::onTrainingStartedProxy);
    connect(ssb64Task_.get(), &ProtocolTaskSSB64::trainingResumed,
        this, &Protocol::onTrainingResumed);
    connect(ssb64Task_.get(), &ProtocolTaskSSB64::trainingEnded,
        this, &Protocol::onTrainingEndedProxy);
    connect(ssb64Task_.get(), &ProtocolTaskSSB64::gameStarted,
        this, &Protocol::onGameStarted);
    connect(ssb64Task_.get(), &ProtocolTaskSSB64::gameResumed,
        this, &Protocol::onGameResumed);
    connect(ssb64Task_.get(), &ProtocolTaskSSB64::gameEnded,
        this, &Protocol::onGameEnded);
    connect(ssb64Task_.get(), &ProtocolTaskSSB64::fighterState,
        this, &Protocol::onFighterState);

    ssb64Task_->start();
}

// ----------------------------------------------------------------------------
void Protocol::disconnectFromServer()
{
    PROFILE(Protocol, disconnectFromServer);

    task_.reset();
    ssb64Task_.reset();
}

// ----------------------------------------------------------------------------
rfcommon::MappingInfo* Protocol::globalMappingInfo() const
{
    PROFILE(Protocol, globalMappingInfo);

    return globalMappingInfo_;
}

// ----------------------------------------------------------------------------
void Protocol::onConnectionSuccess(const QString& ipAddress, quint16 port)
{
    PROFILE(Protocol, onConnectionSuccess);

    QByteArray ba = ipAddress.toUtf8();
    const char* ipCstr = ba.data();
    dispatcher.dispatch(&rfcommon::ProtocolListener::onProtocolConnectedToServer, ipCstr, port);
}

// ----------------------------------------------------------------------------
void Protocol::onConnectionFailure(const QString& errormsg, const QString& ipAddress, quint16 port)
{
    PROFILE(Protocol, onConnectionFailure);

    task_.reset();

    QByteArray ipba = ipAddress.toUtf8();
    const char* ipCstr = ipba.data();
    QByteArray errorba = errormsg.toUtf8();
    const char* errorCstr = errorba.data();
    dispatcher.dispatch(&rfcommon::ProtocolListener::onProtocolFailedToConnectToServer, errorCstr, ipCstr, port);
}

// ----------------------------------------------------------------------------
void Protocol::onConnectionClosed()
{
    PROFILE(Protocol, onConnectionClosed);

    endSessionIfNecessary();

    task_.reset();
    dispatcher.dispatch(&rfcommon::ProtocolListener::onProtocolDisconnectedFromServer);
}

// ----------------------------------------------------------------------------
void Protocol::onMappingInfoReceived(rfcommon::MappingInfo* mappingInfo)
{
    PROFILE(Protocol, onMappingInfoReceived);

    globalMappingInfo_ = mappingInfo;
    saveGlobalMappingInfo();
}

// ----------------------------------------------------------------------------
void Protocol::onTrainingStartedProxy(rfcommon::Metadata* trainingMeta)
{
    PROFILE(Protocol, onTrainingStartedProxy);

    // If the timer did not reset this in time, it means that a stop and a
    // start event occurred in quick succession. This is how we detect a reset
    // in training mode.
    //
    // If there was no training ended event that preceeded this event, then
    // it simply means this is a normal training started event
    if (trainingEndedProxyWasCalled_)
    {
        trainingEndedProxyWasCalled_ = false;
        if (activeSession_.notNull())
        {
            // We create a new session object to hold the data of the data after reset
            rfcommon::Reference<rfcommon::Session> oldSession = activeSession_;
            activeSession_ = rfcommon::Session::newActiveSession(globalMappingInfo_, trainingMeta);

            // Reset state buffer
            stateBuffer_.clearCompact();
            stateBuffer_.resize(trainingMeta->fighterCount());

            dispatcher.dispatch(&rfcommon::ProtocolListener::onProtocolTrainingReset, oldSession, activeSession_);
        }
        else
        {
            // Somehow the flag was set and there was no active session? Whatever,
            // just act like the training started instead of reset
            onTrainingStartedActually(trainingMeta);
        }
    }
    else
    {
        // This was a normal start event
        onTrainingStartedActually(trainingMeta);
    }
}

// ----------------------------------------------------------------------------
void Protocol::onTrainingStartedActually(rfcommon::Metadata* trainingMeta)
{
    PROFILE(Protocol, onTrainingStartedActually);

    // Handle case where game end is not sent (should never happen but you never know)
    endSessionIfNecessary();

    // Reset state buffer
    stateBuffer_.clearCompact();
    stateBuffer_.resize(trainingMeta->fighterCount());

    activeSession_ = rfcommon::Session::newActiveSession(globalMappingInfo_, trainingMeta);
    dispatcher.dispatch(&rfcommon::ProtocolListener::onProtocolTrainingStarted, activeSession_);
}

// ----------------------------------------------------------------------------
void Protocol::onTrainingResumed(rfcommon::Metadata* trainingMeta)
{
    PROFILE(Protocol, onTrainingResumed);

    // Handle case where game end is not sent (should never happen but you never know)
    endSessionIfNecessary();

    // Reset state buffer
    stateBuffer_.clearCompact();
    stateBuffer_.resize(trainingMeta->fighterCount());

    activeSession_ = rfcommon::Session::newActiveSession(globalMappingInfo_, trainingMeta);
    dispatcher.dispatch(&rfcommon::ProtocolListener::onProtocolTrainingResumed, activeSession_);
}

// ----------------------------------------------------------------------------
void Protocol::onTrainingEndedProxy()
{
    PROFILE(Protocol, onTrainingEndedProxy);

    trainingEndedProxyWasCalled_ = true;
    QTimer::singleShot(1000, this, [this](){
        if (trainingEndedProxyWasCalled_)
        {
            onTrainingEndedActually();
            trainingEndedProxyWasCalled_ = false;
        }
    });
}

// ----------------------------------------------------------------------------
void Protocol::onTrainingEndedActually()
{
    PROFILE(Protocol, onTrainingEndedActually);

    endSessionIfNecessary();
}

// ----------------------------------------------------------------------------
void Protocol::onGameStarted(rfcommon::Metadata* gameMeta)
{
    PROFILE(Protocol, onGameStarted);

    // Handle case where game end is not sent (should never happen but you never know)
    endSessionIfNecessary();

    // Reset state buffer
    stateBuffer_.clearCompact();
    stateBuffer_.resize(gameMeta->fighterCount());

    activeSession_ = rfcommon::Session::newActiveSession(globalMappingInfo_, gameMeta);
    dispatcher.dispatch(&rfcommon::ProtocolListener::onProtocolGameStarted, activeSession_);
}

// ----------------------------------------------------------------------------
void Protocol::onGameResumed(rfcommon::Metadata* gameMeta)
{
    PROFILE(Protocol, onGameResumed);

    // Handle case where game end is not sent (should never happen but you never know)
    endSessionIfNecessary();

    // Reset state buffer
    stateBuffer_.clearCompact();
    stateBuffer_.resize(gameMeta->fighterCount());

    activeSession_ = rfcommon::Session::newActiveSession(globalMappingInfo_, gameMeta);
    dispatcher.dispatch(&rfcommon::ProtocolListener::onProtocolGameResumed, activeSession_);
}

// ----------------------------------------------------------------------------
void Protocol::onGameEnded()
{
    PROFILE(Protocol, onGameEnded);

    endSessionIfNecessary();
}

// ----------------------------------------------------------------------------
void Protocol::onFighterState(
        quint64 frameTimeStamp,
        quint32 framesLeft,
        quint8 fighterIdx,
        float posx,
        float posy,
        float damage,
        float hitstun,
        float shield,
        quint16 status,
        quint64 motion,
        quint8 hitStatus,
        quint8 stocks,
        bool attackConnected,
        bool facingDirection,
        bool opponentInHitlag)
{
    PROFILE(Protocol, onFighterState);

    if (activeSession_.isNull())
        return;

    rfcommon::FrameData* frameData = activeSession_->tryGetFrameData();
    assert(frameData != nullptr);

    // For whatever reason, fighter states aren't guaranteed to be received
    // (read: processed by the game) in a defined order. It has been observed
    // that mid game, the processing order switches from 0,1,0,1,... suddenly
    // to 1,0,1,0,... and after a while it sometimes switches back again
    //
    // We have to buffer the fighter states until we have enough information
    // to assemble a frame, and be aware that the order they arrived in could
    // be arbitrary

    // Should never happen but you never know. State buffer should have been
    // resized to the correct size when a game/training session is started/resumed
    if (fighterIdx >= stateBuffer_.count())
        return;

    stateBuffer_[fighterIdx].emplace(
            rfcommon::TimeStamp::fromMillisSinceEpoch(frameTimeStamp),
            rfcommon::FrameIndex::fromValue(0),  // We change the frame number later
            rfcommon::FramesLeft::fromValue(framesLeft),
            rfcommon::Vec2::fromValues(posx, posy),
            damage,
            hitstun,
            shield,
            rfcommon::FighterStatus::fromValue(status),
            rfcommon::FighterMotion::fromValue(motion),
            rfcommon::FighterHitStatus::fromValue(hitStatus),
            rfcommon::FighterStocks::fromValue(stocks),
            rfcommon::FighterFlags::fromFlags(attackConnected, facingDirection, opponentInHitlag));

    // In training mode, when a reset occurs, we recieve a single state from the
    // CPU before receiving the next matching pair of states from player + CPU.
    // It seems that deleting this state is the only way to sync up the states
    // in training mode
    assert(activeSession_->tryGetMetadata() != nullptr);
    if (activeSession_->tryGetMetadata()->type() == rfcommon::Metadata::TRAINING)
    {
        if (stateBuffer_[0].count() == 0)
        {
            for (int i = 1; i < stateBuffer_.count(); ++i)
                stateBuffer_[i].clear();
            return;
        }
    }

    const auto haveAtLeast = [this](int value) -> bool
    {
        for (const auto& states : stateBuffer_)
            if (states.count() < value)
                return false;
        return true;
    };

    const auto framesLeftMatch = [this]() -> bool
    {
        for (int i = 1; i < stateBuffer_.count(); ++i)
            if (stateBuffer_[0][0].framesLeft() != stateBuffer_[i][0].framesLeft())
                return false;
        return true;
    };

    // Wait until we have collected at least 1 state from all players
    if (haveAtLeast(1) == false)
        return;

    // If we've collected enough states to assemble a frame, and they all have
    // the same "frames left" value (and we're also not in training mode, where
    // "frames left" will equal 0), then add the frame to the session.
    if (framesLeftMatch())
    {
        if (stateBuffer_[0][0].framesLeft().count() != 0)
        {
            rfcommon::Frame<4> frame;
            const int frameCount = frameData->frameCount();
            const auto frameNumber = rfcommon::FrameIndex::fromValue(frameCount);
            for (auto& states : stateBuffer_)
                frame.push(states.take(0).withNewFrameIndex(frameNumber));
            frameData->addFrame(std::move(frame));

            return;
        }

        // The above check failed so it must mean this is training mode. Only
        // add frames once we know we're synced. This will be the case when the
        // session object has more than 0 frames.
        if (frameData->frameCount() > 0)
        {
            rfcommon::Frame<4> frame;
            const int frameCount = frameData->frameCount();
            const auto frameNumber = rfcommon::FrameIndex::fromValue(frameCount);
            for (auto& states : stateBuffer_)
                frame.push(states.take(0).withNewFrameIndex(frameNumber));
            frameData->addFrame(std::move(frame));

            return;
        }
    }

    // Try to figure out which state needs to be removed in order to sync up
    // based on the "frames left" value of each state. This only works if we're
    // not in training mode. We need at least 2 frames worth of states to
    // make this decision.
    if (haveAtLeast(2) && stateBuffer_[0][0].framesLeft().count() != 0)
    {
        assert(activeSession_->tryGetMetadata() != nullptr);
        assert(activeSession_->tryGetMetadata()->type() == rfcommon::Metadata::GAME);

        const auto findHighestFramesLeftValue = [this]() -> rfcommon::FramesLeft::Type
        {
            rfcommon::FramesLeft::Type framesLeft = stateBuffer_[0][0].framesLeft().count();
            for (int i = 1; i < stateBuffer_.count(); ++i)
                if (framesLeft < stateBuffer_[i][0].framesLeft().count())
                    framesLeft = stateBuffer_[i][0].framesLeft().count();
            return framesLeft;
        };

        const auto everyFighterHasThisValue = [this](rfcommon::FramesLeft::Type value) -> bool
        {
            for (const auto& states : stateBuffer_)
            {
                if ([&states, &value]() -> bool {
                    for (const auto& state : states)
                        if (state.framesLeft().count() == value)
                            return true;
                    return false;
                }() == false)
                {
                    return false;
                }
            }

            return true;
        };

        const auto anyFighterHasThisValue = [this](rfcommon::FramesLeft::Type value) -> bool
        {
            for (const auto& states : stateBuffer_)
                for (const auto& state : states)
                    if (state.framesLeft().count() == value)
                        return true;
            return false;
        };

        rfcommon::FramesLeft::Type firstValidFramesLeft = findHighestFramesLeftValue();
        while (everyFighterHasThisValue(firstValidFramesLeft) == false && anyFighterHasThisValue(firstValidFramesLeft) == true)
            firstValidFramesLeft--;

        for (auto& states : stateBuffer_)
        {
            while (states.count() > 0 && states[0].framesLeft().count() != firstValidFramesLeft)
                states.erase(0);
        }

        return;
    }

    // In the case of training mode, we really don't care right now. If you can
    // think of a way to sync up frames without access to the "frames left"
    // property, go for it. Keep in mind that the frame counter is not sent
    // from the Nintendo Switch, but is a value we calculate locally, so it
    // can't be used for synchronization.
    if (haveAtLeast(2))
    {
        rfcommon::Frame<4> frame;
        const int frameCount = frameData->frameCount();
        const auto frameNumber = rfcommon::FrameIndex::fromValue(frameCount);
        for (auto& states : stateBuffer_)
            frame.push(states.take(0).withNewFrameIndex(frameNumber));
        frameData->addFrame(std::move(frame));

        return;
    }
}

// ----------------------------------------------------------------------------
void Protocol::endSessionIfNecessary()
{
    PROFILE(Protocol, endSessionIfNecessary);

    if (activeSession_.isNull())
        return;

    rfcommon::Metadata* meta = activeSession_->tryGetMetadata();
    assert(meta != nullptr);

    switch (meta->type())
    {
        case rfcommon::Metadata::GAME:
            dispatcher.dispatch(&rfcommon::ProtocolListener::onProtocolGameEnded, activeSession_);
            break;
        case rfcommon::Metadata::TRAINING:
            dispatcher.dispatch(&rfcommon::ProtocolListener::onProtocolTrainingEnded, activeSession_);
            break;
    }

    activeSession_.drop();
}

// ----------------------------------------------------------------------------
void Protocol::tryLoadGlobalMappingInfo()
{
    PROFILE(Protocol, tryLoadGlobalMappingInfo);

    QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
    QByteArray ba = dir.absoluteFilePath("mappingInfo.json").toUtf8();
    rfcommon::MappedFile file;
    if (file.open(ba.constData()) == false)
        return;

    globalMappingInfo_ = rfcommon::MappingInfo::load(file.address(), file.size());
}

// ----------------------------------------------------------------------------
void Protocol::saveGlobalMappingInfo()
{
    PROFILE(Protocol, saveGlobalMappingInfo);

    QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
    QByteArray ba = dir.absoluteFilePath("mappingInfo.json").toUtf8();
    FILE* fp = rfcommon::utf8_fopen_wb(ba.constData(), ba.size());
    if (fp == nullptr)
        return;

    globalMappingInfo_->save(fp);
    fclose(fp);
}

}
