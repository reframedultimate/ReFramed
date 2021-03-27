#include "uh/models/Protocol.hpp"
#include "uh/models/Settings.hpp"
#include "uh/ui_MainWindow.h"
#include "uh/views/ConnectView.hpp"
#include "uh/views/MainWindow.hpp"
#include "uh/views/ActiveRecordingView.hpp"

#include <QStackedWidget>
#include <QVBoxLayout>

namespace uh {

// ----------------------------------------------------------------------------
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui_(new Ui::MainWindow)
    , settings_(new Settings)
{
    ui_->setupUi(this);

    activeRecordingView_ = new ActiveRecordingView;
    QVBoxLayout* activeRecordingLayout = new QVBoxLayout;
    activeRecordingLayout->addWidget(activeRecordingView_);
    ui_->page_activeRecording->setLayout(activeRecordingLayout);

    connect(ui_->action_connect, SIGNAL(triggered(bool)), this, SLOT(onConnectActionTriggered()));
    connect(ui_->action_disconnect, SIGNAL(triggered(bool)), this, SLOT(onDisconnectActionTriggered()));
}

// ----------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    delete ui_;
}

// ----------------------------------------------------------------------------
void MainWindow::setStateConnected()
{
    // Be (hopefully) helpful and switch to the active recording view
    ui_->stackedWidget->setCurrentWidget(ui_->page_activeRecording);

    // Replace the "connect" action in the dropdown menu with "disconnect"
    ui_->action_connect->setVisible(false);
    ui_->action_disconnect->setVisible(true);
}

// ----------------------------------------------------------------------------
void MainWindow::setStateDisconnected()
{
    // Replace the "disconnect" action in the dropdown menu with "connect"
    ui_->action_connect->setVisible(true);
    ui_->action_disconnect->setVisible(false);
}

// ----------------------------------------------------------------------------
void MainWindow::transferSocketOwnership(tcp_socket socket)
{
    protocol_.reset(new Protocol(socket));

    // Connect all protocol signals to active recording view so it updates as
    // data comes in
    activeRecordingView_->setWaitingForGame();
    connect(protocol_.get(), SIGNAL(dateChanged(const QDateTime&)), activeRecordingView_, SLOT(setTimeStarted(const QDateTime&)));
    connect(protocol_.get(), SIGNAL(stageChanged(const QString&)), activeRecordingView_, SLOT(setStageName(const QString&)));
    connect(protocol_.get(), SIGNAL(playerCountChanged(int)), activeRecordingView_, SLOT(setPlayerCount(int)));
    connect(protocol_.get(), SIGNAL(playerTagChanged(int, const QString&)), activeRecordingView_, SLOT(setPlayerTag(int, const QString&)));
    connect(protocol_.get(), SIGNAL(playerFighterChanged(int, const QString&)), activeRecordingView_, SLOT(setPlayerFighterName(int, const QString&)));
    connect(protocol_.get(), SIGNAL(matchStarted()), activeRecordingView_, SLOT(setActive()));
    connect(protocol_.get(), SIGNAL(playerStatusChanged(unsigned int, int, unsigned int)), activeRecordingView_, SLOT(setPlayerStatus(unsigned int, int, unsigned int)));
    connect(protocol_.get(), SIGNAL(playerDamageChanged(unsigned int, int, float)), activeRecordingView_, SLOT(setPlayerDamage(unsigned int, int, float)));
    connect(protocol_.get(), SIGNAL(playerStockCountChanged(unsigned int, int, unsigned char)), activeRecordingView_, SLOT(setPlayerStockCount(unsigned int, int, unsigned char)));
    connect(protocol_.get(), SIGNAL(connectionClosed()), activeRecordingView_, SLOT(setDisconnected()));

    // If the protocol loses connection for any reason, we have to delete it and
    // reset the UI
    connect(protocol_.get(), SIGNAL(connectionClosed()), this, SLOT(onConnectionLost()));

    // Protocol will emit a Recording instance of all of the data from a single match
    // when each match ends
    connect(protocol_.get(), SIGNAL(matchEnded()), this, SLOT(onNewRecordingAvailable()));

    setStateConnected();

    protocol_->start();
}

// ----------------------------------------------------------------------------
void MainWindow::onProtocolFinishedARecording()
{
    if (protocol_.isNull())
        return;

    QSharedDataPointer<Recording> recording = protocol_->takeRecording();
}

// ----------------------------------------------------------------------------
void MainWindow::onConnectionLost()
{
    // Let the listening thread join and close the underlying socket
    setStateDisconnected();
    protocol_.reset();
}

// ----------------------------------------------------------------------------
void MainWindow::onDisconnectActionTriggered()
{
    // Closes the socket and joins the listening thread
    protocol_.reset();
}

// ----------------------------------------------------------------------------
static QRect calculatePopupGeometry(const QWidget* main, const QWidget* popup)
{
    QRect mainRect = main->geometry();
    QRect popupRect = popup->geometry();

    return QRect(
        mainRect.left() + mainRect.width() / 2 - popupRect.width() / 2,
        mainRect.top() + mainRect.height() / 2 - popupRect.height() / 2,
        popupRect.width(),
        popupRect.height()
    );
}

// ----------------------------------------------------------------------------
void MainWindow::onConnectActionTriggered()
{
    ConnectView* c = new ConnectView(settings_, this, Qt::Popup | Qt::Dialog);
    c->setWindowModality(Qt::WindowModal);
    c->setAttribute(Qt::WA_DeleteOnClose);
    c->show();
    c->setGeometry(calculatePopupGeometry(this, c));
}

}
