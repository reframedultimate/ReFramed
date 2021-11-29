#include "application/models/Protocol.hpp"
#include "application/models/ProtocolCommunicateTask.hpp"
#include "application/models/ProtocolConnectTask.hpp"
#include "uh/RunningGameSession.hpp"
#include "uh/RunningTrainingSession.hpp"
#include "uh/ProtocolListener.hpp"

namespace uhapp {

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
    dispatcher.dispatch(&uh::ProtocolListener::onProtocolAttemptConnectToServer, ipCstr, port);

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
void Protocol::onConnectionSuccess(tcp_socket socket, const QString& ipAddress, uint16_t port)
{
    connectTask_.reset();
    QByteArray ba = ipAddress.toLocal8Bit();
    const char* ipCstr = ba.data();
    dispatcher.dispatch(&uh::ProtocolListener::onProtocolConnectedToServer, ipCstr, port);

    communicateTask_.reset(new ProtocolCommunicateTask(socket));

    connect(communicateTask_.get(), &ProtocolCommunicateTask::connectionClosed,
            this, &Protocol::onProtocolDisconnected);
    connect(communicateTask_.get(), &ProtocolCommunicateTask::trainingStarted,
            this, &Protocol::onTrainingStarted);
    connect(communicateTask_.get(), &ProtocolCommunicateTask::trainingReset,
            this, &Protocol::onTrainingReset);
    connect(communicateTask_.get(), &ProtocolCommunicateTask::trainingEnded,
            this, &Protocol::onTrainingEnded);
    connect(communicateTask_.get(), &ProtocolCommunicateTask::matchStarted,
            this, &Protocol::onMatchStarted);
    connect(communicateTask_.get(), &ProtocolCommunicateTask::matchEnded,
            this, &Protocol::onMatchEnded);
    connect(communicateTask_.get(), &ProtocolCommunicateTask::playerState,
            this, &Protocol::onPlayerState);

    communicateTask_->start();
}

// ----------------------------------------------------------------------------
void Protocol::onConnectionFailure(const QString& ipAddress, uint16_t port)
{
    connectTask_.reset();
    QByteArray ba = ipAddress.toLocal8Bit();
    const char* ipCstr = ba.data();
    dispatcher.dispatch(&uh::ProtocolListener::onProtocolFailedToConnectToServer, ipCstr, port);
}

// ----------------------------------------------------------------------------
void Protocol::onProtocolDisconnected()
{
    communicateTask_.reset();
    dispatcher.dispatch(&uh::ProtocolListener::onProtocolDisconnectedFromServer);
}

// ----------------------------------------------------------------------------
void Protocol::onTrainingStarted(uh::RunningTrainingSession* training)
{
    // Handle case where match end is not sent (should never happen but you never know)
    endSessionIfNecessary();

    session_ = training;
    dispatcher.dispatch(&uh::ProtocolListener::onProtocolTrainingStarted, training);
}

// ----------------------------------------------------------------------------
void Protocol::onTrainingEnded()
{
    endSessionIfNecessary();
}

// ----------------------------------------------------------------------------
void Protocol::onTrainingReset()
{
    uh::RunningTrainingSession* training = dynamic_cast<uh::RunningTrainingSession*>(session_.get());
    if (training)
    {
        training->resetTraining();
    }
}

// ----------------------------------------------------------------------------
void Protocol::onMatchStarted(uh::RunningGameSession* match)
{
    // Handle case where match end is not sent (should never happen but you never know)
    endSessionIfNecessary();

    session_ = match;
    dispatcher.dispatch(&uh::ProtocolListener::onProtocolMatchStarted, match);
}

// ----------------------------------------------------------------------------
void Protocol::onMatchEnded()
{
    endSessionIfNecessary();
}

// ----------------------------------------------------------------------------
void Protocol::onPlayerState(
        quint64 frameTimeStamp,
        quint32 frame,
        quint8 playerID,
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
        bool facing_direction)
{
    if (session_.notNull())
        session_->addPlayerState(playerID, uh::PlayerState(frameTimeStamp, frame, posx, posy, damage, hitstun, shield, status, motion, hit_status, stocks, attack_connected, facing_direction));
}

// ----------------------------------------------------------------------------
void Protocol::endSessionIfNecessary()
{
    if (session_.isNull())
        return;

    if (uh::RunningGameSession* match = dynamic_cast<uh::RunningGameSession*>(session_.get()))
    {
        dispatcher.dispatch(&uh::ProtocolListener::onProtocolMatchEnded, match);
    }
    else if (uh::RunningTrainingSession* training = dynamic_cast<uh::RunningTrainingSession*>(session_.get()))
    {
        dispatcher.dispatch(&uh::ProtocolListener::onProtocolTrainingEnded, training);
    }

    session_.reset();
}

}
