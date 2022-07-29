#include "application/models/Protocol.hpp"
#include "application/models/ProtocolCommunicateTask.hpp"
#include "application/models/ProtocolConnectTask.hpp"
#include "rfcommon/ProtocolListener.hpp"
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
    connect(communicateTask_.get(), &ProtocolCommunicateTask::matchStarted,
            this, &Protocol::onMatchStarted);
    connect(communicateTask_.get(), &ProtocolCommunicateTask::matchResumed,
            this, &Protocol::onMatchResumed);
    connect(communicateTask_.get(), &ProtocolCommunicateTask::matchEnded,
            this, &Protocol::onMatchEnded);
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
void Protocol::onTrainingStartedProxy(rfcommon::RunningTrainingSession* training)
{
    // If the timer did not reset this in time, it means that a stop and a
    // start event occurred in quick succession. This is how we detect a reset
    // in training mode.
    if (trainingEndedProxyWasCalled_)
    {
        trainingEndedProxyWasCalled_ = false;

        rfcommon::RunningTrainingSession* oldTraining =
                dynamic_cast<rfcommon::RunningTrainingSession*>(session_.get());

        if (oldTraining)
        {
            rfcommon::Reference<rfcommon::RunningSession> oldSession = session_;
            session_ = training;
            dispatcher.dispatch(&rfcommon::ProtocolListener::onProtocolTrainingReset, oldTraining, training);
        }
        else
        {
            // fallback
            onTrainingStartedActually(training);
        }
    }
    else
    {
        // This was a valid start event
        onTrainingStartedActually(training);
    }
}

// ----------------------------------------------------------------------------
void Protocol::onTrainingStartedActually(rfcommon::RunningTrainingSession* training)
{
    // Handle case where match end is not sent (should never happen but you never know)
    endSessionIfNecessary();

    // Reset state buffer
    stateBuffer_.clearCompact();
    stateBuffer_.resize(training->fighterCount());

    session_ = training;
    dispatcher.dispatch(&rfcommon::ProtocolListener::onProtocolTrainingStarted, training);
}

// ----------------------------------------------------------------------------
void Protocol::onTrainingResumed(rfcommon::RunningTrainingSession* training)
{
    // Handle case where match end is not sent (should never happen but you never know)
    endSessionIfNecessary();

    // Reset state buffer
    stateBuffer_.clearCompact();
    stateBuffer_.resize(training->fighterCount());

    session_ = training;
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
void Protocol::onMatchStarted(rfcommon::RunningGameSession* match)
{
    // Handle case where match end is not sent (should never happen but you never know)
    endSessionIfNecessary();

    // Reset state buffer
    stateBuffer_.clearCompact();
    stateBuffer_.resize(match->fighterCount());

    session_ = match;
    dispatcher.dispatch(&rfcommon::ProtocolListener::onProtocolMatchStarted, match);
}

// ----------------------------------------------------------------------------
void Protocol::onMatchResumed(rfcommon::RunningGameSession* match)
{
    // Handle case where match end is not sent (should never happen but you never know)
    endSessionIfNecessary();

    // Reset state buffer
    stateBuffer_.clearCompact();
    stateBuffer_.resize(match->fighterCount());

    session_ = match;
    dispatcher.dispatch(&rfcommon::ProtocolListener::onProtocolMatchResumed, match);
}

// ----------------------------------------------------------------------------
void Protocol::onMatchEnded()
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
        quint8 hit_status,
        quint8 stocks,
        bool attack_connected,
        bool facing_direction,
        bool opponent_in_hitlag)
{
    if (session_.isNull())
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
    // resized to the correct size when a match/training session is started/resumed
    if (fighterIdx >= stateBuffer_.count())
        return;

    stateBuffer_[fighterIdx].emplace(
            rfcommon::TimeStamp::fromMillisSinceEpoch(frameTimeStamp),
            0,  // We change the frame number later
            rfcommon::FramesLeft(frame),
            posx, posy,
            damage,
            hitstun,
            shield,
            rfcommon::FighterStatus(status),
            rfcommon::FighterMotion(motion),
            rfcommon::FighterHitStatus(hit_status),
            rfcommon::FighterStocks(stocks),
            rfcommon::FighterFlags(attack_connected, facing_direction, opponent_in_hitlag));

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
    if (framesLeftMatch() && stateBuffer_[0][0].framesLeft() != 0)
    {
        rfcommon::SmallVector<rfcommon::FighterState, 2> frame;
        for (auto& states : stateBuffer_)
            frame.push(states.take(0).withNewFrameNumber(session_->frameCount()));
        session_->addFrame(std::move(frame));

        return;
    }

    // If it is training mode, then only add frames once we know we're synced.
    // This will be the case when the session object has more than 0 frames.
    if (framesLeftMatch() && session_->frameCount() > 0)
    {
        rfcommon::SmallVector<rfcommon::FighterState, 2> frame;
        for (auto& states : stateBuffer_)
            frame.push(states.take(0).withNewFrameNumber(session_->frameCount()));
        session_->addFrame(std::move(frame));

        return;
    }

    // Try to figure out which state needs to be removed in order to sync up
    // based on the "frames left" value of each state. This only works if we're
    // not in training mode. We need at least 2 frames worth of states to
    // make this decision.
    if (haveAtLeast(2) && stateBuffer_[0][0].framesLeft() != 0)
    {
        const auto findHighestFramesLeftValue = [this]() -> int
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
                        if (state.framesLeft() == value)
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
                    if (state.framesLeft() == value)
                        return true;
            return false;
        };

        rfcommon::FramesLeft::Type firstValidFramesLeft = findHighestFramesLeftValue();
        while (everyFighterHasThisValue(firstValidFramesLeft) == false && anyFighterHasThisValue(firstValidFramesLeft) == true)
            firstValidFramesLeft--;

        for (auto& states : stateBuffer_)
        {
            while (states.count() > 0 && states[0].framesLeft() != firstValidFramesLeft)
                states.erase(0);
        }

        return;
    }

    // In the case of training mode, we really don't care right now
    if (haveAtLeast(2))
    {
        rfcommon::SmallVector<rfcommon::FighterState, 2> frame;
        for (auto& states : stateBuffer_)
            frame.push(states.take(0).withNewFrameNumber(session_->frameCount()));
        session_->addFrame(std::move(frame));

        return;
    }
}

// ----------------------------------------------------------------------------
void Protocol::endSessionIfNecessary()
{
    if (session_.isNull())
        return;

    if (rfcommon::RunningGameSession* match = dynamic_cast<rfcommon::RunningGameSession*>(session_.get()))
    {
        dispatcher.dispatch(&rfcommon::ProtocolListener::onProtocolMatchEnded, match);
    }
    else if (rfcommon::RunningTrainingSession* training = dynamic_cast<rfcommon::RunningTrainingSession*>(session_.get()))
    {
        dispatcher.dispatch(&rfcommon::ProtocolListener::onProtocolTrainingEnded, training);
    }

    session_.drop();
}

}
