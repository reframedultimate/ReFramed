#include "application/models/Protocol.hpp"
#include "application/models/ProtocolCommunicateTask.hpp"
#include "application/models/ProtocolConnectTask.hpp"
#include "rfcommon/RunningGameSession.hpp"
#include "rfcommon/RunningTrainingSession.hpp"
#include "rfcommon/ProtocolListener.hpp"

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
void Protocol::onConnectionSuccess(tcp_socket socket, const QString& ipAddress, quint16 port)
{
    connectTask_.reset();
    QByteArray ba = ipAddress.toLocal8Bit();
    const char* ipCstr = ba.data();
    dispatcher.dispatch(&rfcommon::ProtocolListener::onProtocolConnectedToServer, ipCstr, port);

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
    communicateTask_.reset();
    dispatcher.dispatch(&rfcommon::ProtocolListener::onProtocolDisconnectedFromServer);
}

// ----------------------------------------------------------------------------
void Protocol::onTrainingStarted(rfcommon::RunningTrainingSession* training)
{
    // Handle case where match end is not sent (should never happen but you never know)
    endSessionIfNecessary();

    session_ = training;
    dispatcher.dispatch(&rfcommon::ProtocolListener::onProtocolTrainingStarted, training);
}

// ----------------------------------------------------------------------------
void Protocol::onTrainingEnded()
{
    endSessionIfNecessary();
}

// ----------------------------------------------------------------------------
void Protocol::onTrainingReset()
{
    rfcommon::RunningTrainingSession* training = dynamic_cast<rfcommon::RunningTrainingSession*>(session_.get());
    if (training)
    {
        training->resetTraining();
    }
}

// ----------------------------------------------------------------------------
void Protocol::onMatchStarted(rfcommon::RunningGameSession* match)
{
    // Handle case where match end is not sent (should never happen but you never know)
    endSessionIfNecessary();

    session_ = match;
    dispatcher.dispatch(&rfcommon::ProtocolListener::onProtocolMatchStarted, match);
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
        session_->addPlayerState(playerID, rfcommon::PlayerState(frameTimeStamp, frame, posx, posy, damage, hitstun, shield, status, motion, hit_status, stocks, attack_connected, facing_direction));
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

    session_.reset();
}

}
