#include "application/models/Protocol.hpp"
#include "application/models/ProtocolCommunicateTask.hpp"
#include "application/models/ProtocolConnectTask.hpp"
#include "rfcommon/Frame.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/ProtocolListener.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/SessionMetaData.hpp"
#include <QTimer>

namespace rfapp {

// ----------------------------------------------------------------------------
Protocol::Protocol(QObject* parent)
    : QObject(parent)
{
}

// ----------------------------------------------------------------------------
Protocol::~Protocol()
{
}

// ----------------------------------------------------------------------------
void Protocol::connectToServer(const QString& ipAddress, uint16_t port)
{
    communicateTask_.reset();
    connectTask_.reset(new ProtocolConnectTask(ipAddress, port));

    QByteArray ba = ipAddress.toLocal8Bit();
    const char* ipCstr = ba.data();
    dispatcher.dispatch(&rfcommon::ProtocolListener::onProtocolAttemptConnectToServer, ipCstr, port);

    connect(connectTask_.get(), &ProtocolConnectTask::connectionSuccess,
            this, &Protocol::onConnectionSuccess);
    connect(connectTask_.get(), &ProtocolConnectTask::connectionFailure,
            this, &Protocol::onConnectionFailure);

    connectTask_->start();
}

// ----------------------------------------------------------------------------
void Protocol::disconnectFromServer()
{
    connectTask_.reset();
    communicateTask_.reset();
}

// ----------------------------------------------------------------------------
bool Protocol::isTryingToConnect() const
{
    return connectTask_ != nullptr;
}

// ----------------------------------------------------------------------------
bool Protocol::isConnected() const
{
    return communicateTask_ != nullptr;
}

// ----------------------------------------------------------------------------
void Protocol::onConnectionSuccess(void* socket_handle, const QString& ipAddress, quint16 port)
{
    connectTask_.reset();
    QByteArray ba = ipAddress.toLocal8Bit();
    const char* ipCstr = ba.data();
    dispatcher.dispatch(&rfcommon::ProtocolListener::onProtocolConnectedToServer, ipCstr, port);

    communicateTask_.reset(new ProtocolCommunicateTask(
                               tcp_socket_from_handle(socket_handle)));

    connect(communicateTask_.get(), &ProtocolCommunicateTask::connectionClosed,
            this, &Protocol::onProtocolDisconnected);
    connect(communicateTask_.get(), &ProtocolCommunicateTask::trainingStarted,
            this, &Protocol::onTrainingStartedProxy);
    connect(communicateTask_.get(), &ProtocolCommunicateTask::trainingResumed,
            this, &Protocol::onTrainingResumed);
    connect(communicateTask_.get(), &ProtocolCommunicateTask::trainingEnded,
            this, &Protocol::onTrainingEndedProxy);
    connect(communicateTask_.get(), &ProtocolCommunicateTask::gameStarted,
            this, &Protocol::onGameStarted);
    connect(communicateTask_.get(), &ProtocolCommunicateTask::gameResumed,
            this, &Protocol::onGameResumed);
    connect(communicateTask_.get(), &ProtocolCommunicateTask::gameEnded,
            this, &Protocol::onGameEnded);
    connect(communicateTask_.get(), &ProtocolCommunicateTask::fighterState,
            this, &Protocol::onFighterState);

    communicateTask_->start();
}

// ----------------------------------------------------------------------------
void Protocol::onConnectionFailure(const QString& errormsg, const QString& ipAddress, quint16 port)
{
    connectTask_.reset();
    QByteArray ipba = ipAddress.toLocal8Bit();
    const char* ipCstr = ipba.data();
    QByteArray errorba = errormsg.toLocal8Bit();
    const char* errorCstr = errorba.data();
    dispatcher.dispatch(&rfcommon::ProtocolListener::onProtocolFailedToConnectToServer, errorCstr, ipCstr, port);
}

// ----------------------------------------------------------------------------
void Protocol::onProtocolDisconnected()
{
    endSessionIfNecessary();

    communicateTask_.reset();
    dispatcher.dispatch(&rfcommon::ProtocolListener::onProtocolDisconnectedFromServer);
}

// ----------------------------------------------------------------------------
void Protocol::onTrainingStartedProxy(rfcommon::Session* training)
{
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
            rfcommon::Reference<rfcommon::Session> oldSession = activeSession_;
            activeSession_ = training;
            dispatcher.dispatch(&rfcommon::ProtocolListener::onProtocolTrainingReset, oldSession, activeSession_);
        }
        else
        {
            // Somehow the flag was set and there was no active session? Whatever,
            // just act like the training started instead of reset
            onTrainingStartedActually(training);
        }
    }
    else
    {
        // This was a normal start event
        onTrainingStartedActually(training);
    }
}

// ----------------------------------------------------------------------------
void Protocol::onTrainingStartedActually(rfcommon::Session* training)
{
    // Handle case where game end is not sent (should never happen but you never know)
    endSessionIfNecessary();

    // Reset state buffer
    stateBuffer_.clearCompact();
    stateBuffer_.resize(training->metaData()->fighterCount());

    activeSession_ = training;
    dispatcher.dispatch(&rfcommon::ProtocolListener::onProtocolTrainingStarted, training);
}

// ----------------------------------------------------------------------------
void Protocol::onTrainingResumed(rfcommon::Session* training)
{
    // Handle case where game end is not sent (should never happen but you never know)
    endSessionIfNecessary();

    // Reset state buffer
    stateBuffer_.clearCompact();
    stateBuffer_.resize(training->metaData()->fighterCount());

    activeSession_ = training;
    dispatcher.dispatch(&rfcommon::ProtocolListener::onProtocolTrainingResumed, training);
}

// ----------------------------------------------------------------------------
void Protocol::onTrainingEndedProxy()
{
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
    endSessionIfNecessary();
}

// ----------------------------------------------------------------------------
void Protocol::onGameStarted(rfcommon::Session* game)
{
    // Handle case where game end is not sent (should never happen but you never know)
    endSessionIfNecessary();

    // Reset state buffer
    stateBuffer_.clearCompact();
    stateBuffer_.resize(game->metaData()->fighterCount());

    activeSession_ = game;
    dispatcher.dispatch(&rfcommon::ProtocolListener::onProtocolGameStarted, game);
}

// ----------------------------------------------------------------------------
void Protocol::onGameResumed(rfcommon::Session* game)
{
    // Handle case where game end is not sent (should never happen but you never know)
    endSessionIfNecessary();

    // Reset state buffer
    stateBuffer_.clearCompact();
    stateBuffer_.resize(game->metaData()->fighterCount());

    activeSession_ = game;
    dispatcher.dispatch(&rfcommon::ProtocolListener::onProtocolGameResumed, game);
}

// ----------------------------------------------------------------------------
void Protocol::onGameEnded()
{
    endSessionIfNecessary();
}

// ----------------------------------------------------------------------------
void Protocol::onFighterState(
        quint64 frameTimeStamp,
        quint32 frame,
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
    if (activeSession_.isNull())
        return;

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
            rfcommon::FrameNumber::fromValue(0),  // We change the frame number later
            rfcommon::FramesLeft::fromValue(frame),
            posx, posy,
            damage,
            hitstun,
            shield,
            rfcommon::FighterStatus::fromValue(status),
            rfcommon::FighterMotion::fromValue(motion),
            rfcommon::FighterHitStatus::fromValue(hitStatus),
            rfcommon::FighterStocks::fromValue(stocks),
            rfcommon::FighterFlags::fromFlags(attackConnected, facingDirection, opponentInHitlag));

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

    if (haveAtLeast(1) == false)
        return;

    // If we've collected enough states to assemble a frame, and they all have
    // the same "frames left" value (and we're also not in training mode, where
    // "frames left" will equal 0), then add the frame to the session.
    if (framesLeftMatch())
    {
        if (stateBuffer_[0][0].framesLeft().value() != 0)
        {
            rfcommon::Frame frame;
            const int frameCount = activeSession_->frameData()->frameCount();
            const auto frameNumber = rfcommon::FrameNumber::fromValue(frameCount);
            for (auto& states : stateBuffer_)
                frame.push(states.take(0).withNewFrameNumber(frameNumber));
            activeSession_->frameData()->addFrame(std::move(frame));

            return;
        }

        // The above check failed so it must mean this is training mode. Only
        // add frames once we know we're synced. This will be the case when the
        // session object has more than 0 frames.
        if (activeSession_->frameData()->frameCount() > 0)
        {
            rfcommon::Frame frame;
            const int frameCount = activeSession_->frameData()->frameCount();
            const auto frameNumber = rfcommon::FrameNumber::fromValue(frameCount);
            for (auto& states : stateBuffer_)
                frame.push(states.take(0).withNewFrameNumber(frameNumber));
            activeSession_->frameData()->addFrame(std::move(frame));

            return;
        }
    }

    // Try to figure out which state needs to be removed in order to sync up
    // based on the "frames left" value of each state. This only works if we're
    // not in training mode. We need at least 2 frames worth of states to
    // make this decision.
    if (haveAtLeast(2) && stateBuffer_[0][0].framesLeft().value() != 0)
    {
        const auto findHighestFramesLeftValue = [this]() -> rfcommon::FramesLeft::Type
        {
            rfcommon::FramesLeft::Type framesLeft = stateBuffer_[0][0].framesLeft().value();
            for (int i = 1; i < stateBuffer_.count(); ++i)
                if (framesLeft < stateBuffer_[i][0].framesLeft().value())
                    framesLeft = stateBuffer_[i][0].framesLeft().value();
            return framesLeft;
        };

        const auto everyFighterHasThisValue = [this](rfcommon::FramesLeft::Type value) -> bool
        {
            for (const auto& states : stateBuffer_)
            {
                if ([&states, &value]() -> bool {
                    for (const auto& state : states)
                        if (state.framesLeft().value() == value)
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
                    if (state.framesLeft().value() == value)
                        return true;
            return false;
        };

        rfcommon::FramesLeft::Type firstValidFramesLeft = findHighestFramesLeftValue();
        while (everyFighterHasThisValue(firstValidFramesLeft) == false && anyFighterHasThisValue(firstValidFramesLeft) == true)
            firstValidFramesLeft--;

        for (auto& states : stateBuffer_)
        {
            while (states.count() > 0 && states[0].framesLeft().value() != firstValidFramesLeft)
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
        rfcommon::Frame frame;
        const int frameCount = activeSession_->frameData()->frameCount();
        const auto frameNumber = rfcommon::FrameNumber::fromValue(frameCount);
        for (auto& states : stateBuffer_)
            frame.push(states.take(0).withNewFrameNumber(frameNumber));
        activeSession_->frameData()->addFrame(std::move(frame));

        return;
    }
}

// ----------------------------------------------------------------------------
void Protocol::endSessionIfNecessary()
{
    if (activeSession_.isNull())
        return;

    if (activeSession_->metaData()->type() == rfcommon::SessionMetaData::GAME)
    {
        dispatcher.dispatch(&rfcommon::ProtocolListener::onProtocolGameEnded, activeSession_);
    }
    else
    {
        assert(activeSession_->metaData()->type() == rfcommon::SessionMetaData::TRAINING);
        dispatcher.dispatch(&rfcommon::ProtocolListener::onProtocolTrainingEnded, activeSession_);
    }

    activeSession_.drop();
}

}
