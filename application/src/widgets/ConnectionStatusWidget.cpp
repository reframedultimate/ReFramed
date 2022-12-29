#include "rfcommon/Profiler.hpp"
#include "application/widgets/ConnectionStatusWidget.hpp"
#include "application/models/Protocol.hpp"

#include <QLabel>
#include <QHBoxLayout>

namespace rfapp {

// ----------------------------------------------------------------------------
ConnectionStatusWidget::ConnectionStatusWidget(Protocol* protocol, QWidget* parent)
    : QWidget(parent)
    , protocol_(protocol)
    , text_(new QLabel("Disconnected"))
{
    QHBoxLayout* l = new QHBoxLayout;
    l->addWidget(text_);
    setLayout(l);

    protocol_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
ConnectionStatusWidget::~ConnectionStatusWidget()
{
    protocol_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void ConnectionStatusWidget::onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) {}
void ConnectionStatusWidget::onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) {}

// ----------------------------------------------------------------------------
void ConnectionStatusWidget::onProtocolConnectedToServer(const char* ipAddress, uint16_t port)
{
    PROFILE(ConnectionStatusWidget, onProtocolConnectedToServer);

    text_->setText(QString("Connected to ") + ipAddress + ":" + QString::number(port));
}

// ----------------------------------------------------------------------------
void ConnectionStatusWidget::onProtocolDisconnectedFromServer()
{
    PROFILE(ConnectionStatusWidget, onProtocolDisconnectedFromServer);

    text_->setText("Disconnected");
}

// ----------------------------------------------------------------------------
void ConnectionStatusWidget::onProtocolTrainingStarted(rfcommon::Session* training) {}
void ConnectionStatusWidget::onProtocolTrainingResumed(rfcommon::Session* training) {}
void ConnectionStatusWidget::onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining) {}
void ConnectionStatusWidget::onProtocolTrainingEnded(rfcommon::Session* training) {}
void ConnectionStatusWidget::onProtocolGameStarted(rfcommon::Session* game) {}
void ConnectionStatusWidget::onProtocolGameResumed(rfcommon::Session* game) {}
void ConnectionStatusWidget::onProtocolGameEnded(rfcommon::Session* game) {}

}
