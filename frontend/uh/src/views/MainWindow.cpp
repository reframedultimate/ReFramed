#include "uh/models/Settings.hpp"
#include "uh/models/ActiveRecordingManager.hpp"
#include "uh/models/RecordingManager.hpp"
#include "uh/ui_MainWindow.h"
#include "uh/views/ActiveRecordingView.hpp"
#include "uh/views/CategoryView.hpp"
#include "uh/views/ConnectView.hpp"
#include "uh/views/MainWindow.hpp"
#include "uh/views/RecordingGroupView.hpp"
#include "uh/views/RecordingView.hpp"

#include <QStackedWidget>
#include <QDockWidget>
#include <QStackedWidget>
#include <QFileInfo>
#include <QMessageBox>
#include <QFileDialog>

namespace uh {

// ----------------------------------------------------------------------------
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , settings_(new Settings)
    , activeRecordingManager_(new ActiveRecordingManager(settings_.get()))
    , recordingManager_(new RecordingManager(settings_.get()))
    , categoryView_(new CategoryView(recordingManager_.get()))
    , recordingGroupView_(new RecordingGroupView)
    , mainView_(new QStackedWidget)
    , ui_(new Ui::MainWindow)
{
    ui_->setupUi(this);

    ActiveRecordingView* activeRecordingView = new ActiveRecordingView(activeRecordingManager_.get());
    RecordingView* recordingView = new RecordingView;

    mainView_->addWidget(recordingView);
    mainView_->addWidget(recordingGroupView_);
    mainView_->addWidget(activeRecordingView);
    setCentralWidget(mainView_);

    QDockWidget* categoryDock = new QDockWidget(this);
    categoryDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    categoryDock->setWidget(categoryView_);
    categoryDock->setFeatures(QDockWidget::DockWidgetMovable);
    addDockWidget(Qt::LeftDockWidgetArea, categoryDock);

    connect(activeRecordingManager_.get(), SIGNAL(connectedToServer()), this, SLOT(onActiveRecordingManagerConnectedToServer()));
    connect(activeRecordingManager_.get(), SIGNAL(disconnectedFromServer()), this, SLOT(onActiveRecordingManagerDisconnectedFromServer()));

    connect(categoryView_, SIGNAL(categoryChanged(CategoryType)), this, SLOT(onCategoryChanged(CategoryType)));
    connect(categoryView_, SIGNAL(recordingGroupSelected(RecordingGroup*)), recordingGroupView_, SLOT(setRecordingGroup(RecordingGroup*)));

    connect(ui_->action_connect, SIGNAL(triggered(bool)), this, SLOT(onConnectActionTriggered()));
    connect(ui_->action_disconnect, SIGNAL(triggered(bool)), this, SLOT(onDisconnectActionTriggered()));

    // Execute this later so the main window is visible when the popup opens
    // A single popup without the main window feels weird
    QMetaObject::invokeMethod(this, "negotiateDefaultRecordingLocation", Qt::QueuedConnection);
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
    mainView_->setCurrentIndex(2);

    // Replace the "connect" action in the dropdown menu with "disconnect"
    ui_->action_connect->setVisible(false);
    ui_->action_disconnect->setVisible(true);

    // Enable active recording view in the category view
    categoryView_->setActiveRecordingViewDisabled(false);
}

// ----------------------------------------------------------------------------
void MainWindow::setStateDisconnected()
{
    // Replace the "disconnect" action in the dropdown menu with "connect"
    ui_->action_connect->setVisible(true);
    ui_->action_disconnect->setVisible(false);

    // Disable active recording view in the category view
    categoryView_->setActiveRecordingViewDisabled(true);
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

    connect(c, SIGNAL(connectRequest(const QString&,uint16_t)), activeRecordingManager_.get(), SLOT(tryConnectToServer(const QString&, uint16_t)));
    connect(activeRecordingManager_.get(), SIGNAL(connectedToServer()), c, SLOT(close()));
    connect(activeRecordingManager_.get(), SIGNAL(failedToConnectToServer()), c, SLOT(setConnectFailed()));

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
void MainWindow::onActiveRecordingManagerRecordingSaved(const QFileInfo& absPathToFile)
{
    recordingManager_->allRecordingGroup()->addFile(absPathToFile);
}

// ----------------------------------------------------------------------------
void MainWindow::onCategoryChanged(CategoryType category)
{
    switch (category)
    {
        case CategoryType::ANALYSIS          : mainView_->setCurrentIndex(0); break;
        case CategoryType::RECORDING_GROUPS  : mainView_->setCurrentIndex(1); break;
        case CategoryType::RECORDING_SOURCES : break;
        case CategoryType::VIDEO_SOURCES     : break;
        case CategoryType::ACTIVE_RECORDING  : mainView_->setCurrentIndex(2); break;
        case CategoryType::TOP_LEVEL : /* should never happen */ break;
    }
}

// ----------------------------------------------------------------------------
void MainWindow::negotiateDefaultRecordingLocation()
{
    QDir path = recordingManager_->defaultRecordingSourceDirectory();

    auto askForDir = [this](const QString& path) -> QString {
        return QFileDialog::getExistingDirectory(
            this,
            "Choose directory to save recordings to",
            path,
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );
    };

    QFileInfo pathInfo(path.absolutePath());
    if (!pathInfo.exists())
    {
        int option = QMessageBox::warning(
            this,
            "Default directory for recordings not found",
            QString("The directory for saving recordings was not found or has not been configured yet.\n"
                    "Would you like to create the directory now?\n\n") +
                    pathInfo.filePath(),
            QMessageBox::Yes | QMessageBox::Open
        );

        if (option == QMessageBox::Open)
        {
            pathInfo = askForDir(pathInfo.filePath());
            if (pathInfo.exists() && pathInfo.isDir() && pathInfo.isWritable())
                recordingManager_->setDefaultRecordingSourceDirectory(pathInfo.filePath());
            else
                close();
        }
        else
        {
            pathInfo = path.absolutePath();
            if (QDir().mkdir(pathInfo.filePath()))
            {
                recordingManager_->setDefaultRecordingSourceDirectory(pathInfo.filePath());
            }
            else
            {
                int option = QMessageBox::warning(
                    this,
                    "Failed to create directory",
                    QString("Could not create the directory:\n") +
                            pathInfo.filePath() +
                            "\n\nWould you like to choose a directory?",
                    QMessageBox::Yes | QMessageBox::No
                );

                if (option == QMessageBox::Yes)
                {
                    pathInfo = askForDir(pathInfo.filePath());
                    if (!pathInfo.exists() || !pathInfo.isDir() || !pathInfo.isWritable())
                        close();
                    else
                        recordingManager_->setDefaultRecordingSourceDirectory(pathInfo.filePath());
                }
                else
                {
                    close();
                }
            }
        }
    }

    if (!pathInfo.isDir() || !pathInfo.isWritable())
    {
        int option = QMessageBox::warning(
            this,
            "Default directory for recordings not writable",
            QString("The directory for saving recordings is either not a directory or not writable.\n"
                    "Would you like to choose another directory?"),
            QMessageBox::Yes | QMessageBox::No
        );

        if (option == QMessageBox::Yes)
        {
            pathInfo = askForDir(pathInfo.filePath());
            if (pathInfo.exists() && pathInfo.isDir() && pathInfo.isWritable())
                recordingManager_->setDefaultRecordingSourceDirectory(pathInfo.filePath());
            else
                close();
        }
        else
        {
            close();
        }
    }
}

// ----------------------------------------------------------------------------
void MainWindow::populateCategories()
{

}

}
