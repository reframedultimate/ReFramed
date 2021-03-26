#include "uh/models/Protocol.hpp"
#include "uh/models/Settings.hpp"
#include "uh/ui_MainWindow.h"
#include "uh/views/ConnectView.hpp"
#include "uh/views/MainWindow.hpp"
#include "uh/views/ActiveRecordingView.hpp"

#include <QStackedWidget>
#include <QTcpSocket>

namespace uh {

// ----------------------------------------------------------------------------
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui_(new Ui::MainWindow)
    , settings_(new Settings)
{
    ui_->setupUi(this);

    QVBoxLayout* activeRecordingLayout = new QVBoxLayout;
    ui_->page_activeRecording->setLayout(activeRecordingLayout);
    activeRecordingView_ = new ActiveRecordingView;
    activeRecordingLayout->addWidget(activeRecordingView_);

    connect(ui_->action_Connect, SIGNAL(triggered(bool)), this, SLOT(onConnectActionTriggered()));
}

// ----------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    delete ui_;
}

// ----------------------------------------------------------------------------
void MainWindow::transferSocketOwnership(tcp_socket socket)
{
    protocol_.reset(new Protocol(socket));
    ui_->stackedWidget->setCurrentWidget(ui_->page_activeRecording);
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
    connect(protocol_.get(), SIGNAL(matchEnded(Recording*)), activeRecordingView_, SLOT(setWaitingForGame()));
    connect(protocol_.get(), SIGNAL(connectionClosed()), activeRecordingView_, SLOT(setDisconnected()));

    protocol_->start();
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
