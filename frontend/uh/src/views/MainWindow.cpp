#include "uh/models/Settings.hpp"
#include "uh/models/ActiveRecordingManager.hpp"
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

    activeRecordingManager_ = new ActiveRecordingManager(settings_.get());
    ActiveRecordingView* activeRecordingView = new ActiveRecordingView(activeRecordingManager_);
    RecordingView* recordingView = new RecordingView;

    mainView_ = new QStackedWidget;
    mainView_->addWidget(recordingView);
    mainView_->addWidget(activeRecordingView);
    setCentralWidget(mainView_);

    QDockWidget* categoryDock = new QDockWidget(this);
    categoryDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    categoryDock->setWidget(categoryView_);
    categoryDock->setFeatures(QDockWidget::DockWidgetMovable);
    addDockWidget(Qt::LeftDockWidgetArea, categoryDock);

    connect(activeRecordingManager_, SIGNAL(connectedToServer()), this, SLOT(onActiveRecordingManagerConnectedToServer()));
    connect(activeRecordingManager_, SIGNAL(disconnectedFromServer()), this, SLOT(onActiveRecordingManagerDisconnectedFromServer()));

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
void MainWindow::onActiveRecordingManagerConnectedToServer()
{
    setStateConnected();
}

// ----------------------------------------------------------------------------
void MainWindow::onActiveRecordingManagerDisconnectedFromServer()
{
    setStateDisconnected();
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
    ConnectView* c = new ConnectView(settings_.get(), Qt::Popup | Qt::Dialog);

    connect(c, SIGNAL(connectRequest(const QString&,uint16_t)), activeRecordingManager_, SLOT(tryConnectToServer(const QString&, uint16_t)));
    connect(activeRecordingManager_, SIGNAL(connectedToServer()), c, SLOT(close()));
    connect(activeRecordingManager_, SIGNAL(failedToConnectToServer()), c, SLOT(setConnectFailed()));

    c->setAttribute(Qt::WA_DeleteOnClose);
    c->show();
    c->setGeometry(calculatePopupGeometry(this, c));
}

// ----------------------------------------------------------------------------
void MainWindow::onDisconnectActionTriggered()
{
    // Should also trigger onActiveRecordingManagerDisconnectedFromServer()
    activeRecordingManager_->disconnectFromServer();
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
