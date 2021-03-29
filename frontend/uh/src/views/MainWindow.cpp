#include "uh/models/Protocol.hpp"
#include "uh/models/Settings.hpp"
#include "uh/ui_MainWindow.h"
#include "uh/views/ActiveRecordingView.hpp"
#include "uh/views/CategoryView.hpp"
#include "uh/views/ConnectView.hpp"
#include "uh/views/MainWindow.hpp"
#include "uh/views/RecordingView.hpp"

#include <QStackedWidget>
#include <QDockWidget>
#include <QStackedWidget>

namespace uh {

// ----------------------------------------------------------------------------
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui_(new Ui::MainWindow)
    , settings_(new Settings)
{
    ui_->setupUi(this);

    categoryView_ = new CategoryView;
    activeRecordingView_ = new ActiveRecordingView;
    RecordingView* recordingView = new RecordingView;

    mainView_ = new QStackedWidget;
    mainView_->addWidget(recordingView);
    mainView_->addWidget(activeRecordingView_);
    setCentralWidget(mainView_);

    QDockWidget* categoryDock = new QDockWidget(this);
    categoryDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    categoryDock->setWidget(categoryView_);
    categoryDock->setFeatures(QDockWidget::DockWidgetMovable);
    addDockWidget(Qt::LeftDockWidgetArea, categoryDock);

    connect(categoryView_, SIGNAL(categoryChanged(CategoryType)), this, SLOT(onCategoryChanged(CategoryType)));

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
    mainView_->setCurrentIndex(1);

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
    connect(protocol_.get(), SIGNAL(matchEnded()), this, SLOT(onProtocolFinishedARecording()));

    setStateConnected();

    protocol_->start();
}

// ----------------------------------------------------------------------------
void MainWindow::onProtocolFinishedARecording()
{
    if (protocol_.isNull())
        return;

    QSharedDataPointer<Recording> recording = protocol_->takeRecording();
    recording->save(QDir("bin"));
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

// ----------------------------------------------------------------------------
void MainWindow::onCategoryChanged(CategoryType category)
{
    switch (category)
    {
        case CategoryType::ANALYSIS          : mainView_->setCurrentIndex(0); break;
        case CategoryType::RECORDING_GROUPS  : break;
        case CategoryType::RECORDING_SOURCES : break;
        case CategoryType::VIDEO_SOURCES     : break;
        case CategoryType::ACTIVE_RECORDING  : mainView_->setCurrentIndex(1); break;
        case CategoryType::TOP_LEVEL : /* should never happen */ break;
    }
}

}
