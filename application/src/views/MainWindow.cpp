#include "application/config.hpp"
#include "application/models/Config.hpp"
#include "application/models/CategoryModel.hpp"
#include "application/models/PluginManager.hpp"
#include "application/models/Protocol.hpp"
#include "application/models/RunningGameSessionManager.hpp"
#include "application/models/SavedGameSessionManager.hpp"
#include "application/models/TrainingModeModel.hpp"
#include "application/ui_MainWindow.h"
#include "application/views/RunningGameSessionView.hpp"
#include "application/views/AnalysisView.hpp"
#include "application/views/CategoryView.hpp"
#include "application/views/ConnectView.hpp"
#include "application/views/DataSetFilterView.hpp"
#include "application/views/MainWindow.hpp"
#include "application/views/SavedGameSessionGroupView.hpp"
#include "application/views/SessionView.hpp"
#include "application/views/TrainingModeView.hpp"
#include "application/views/VisualizerView.hpp"

#include "rfcommon/VisualizerPlugin.hpp"

#include <QStackedWidget>
#include <QDockWidget>
#include <QStackedWidget>
#include <QFileInfo>
#include <QMessageBox>
#include <QFileDialog>

namespace rfapp {

// ----------------------------------------------------------------------------
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , config_(new Config)
    , protocol_(new Protocol)
    , pluginManager_(new PluginManager(protocol_.get()))
    , replayManager_(new ReplayManager(config_.get()))
    , runningGameSessionManager_(new RunningGameSessionManager(protocol_.get(), replayManager_.get()))
    , trainingModeModel_(new TrainingModeModel(pluginManager_.get()))
    , categoryModel_(new CategoryModel)
    , categoryView_(new CategoryView(categoryModel_.get(), replayManager_.get(), runningGameSessionManager_.get(), trainingModeModel_.get()))
    , replayGroupView_(new ReplayGroupView(replayManager_.get(), pluginManager_.get()))
    , runningGameSessionView_(new RunningGameSessionView(runningGameSessionManager_.get(), pluginManager_.get()))
    , mainView_(new QStackedWidget)
    , ui_(new Ui::MainWindow)
{
    ui_->setupUi(this);

    setWindowTitle("ReFramed - " APP_VERSION_STR);

    QDir pluginDir("share/reframed/plugins");
    for (const auto& pluginFile : pluginDir.entryList(QStringList() << "*.so" << "*.dll", QDir::Files))
        pluginManager_->loadPlugin(pluginDir.absoluteFilePath(pluginFile));

    mainView_->addWidget(replayGroupView_);
    mainView_->addWidget(new DataSetFilterView(replayManager_.get()));
    mainView_->addWidget(new VisualizerView(pluginManager_.get()));
    mainView_->addWidget(new QWidget);
    mainView_->addWidget(new QWidget);
    mainView_->addWidget(runningGameSessionView_);
    mainView_->addWidget(new TrainingModeView(trainingModeModel_.get(), categoryModel_.get()));
    setCentralWidget(mainView_);

    QDockWidget* categoryDock = new QDockWidget(this);
    categoryDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    categoryDock->setWidget(categoryView_);
    categoryDock->setFeatures(QDockWidget::DockWidgetMovable);
    addDockWidget(Qt::LeftDockWidgetArea, categoryDock);

    categoryModel_->dispatcher.addListener(this);
    runningGameSessionManager_->dispatcher.addListener(this);

    connect(ui_->action_connect, &QAction::triggered,
            this, &MainWindow::onConnectActionTriggered);
    connect(ui_->action_disconnect, &QAction::triggered,
            this, &MainWindow::onDisconnectActionTriggered);

    categoryModel_->selectTrainingModeCategory();

    // Execute this later so the main window is visible when the popup opens
    // A single popup without the main window feels weird
    QMetaObject::invokeMethod(this, "negotiateDefaultRecordingLocation", Qt::QueuedConnection);
}

// ----------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    runningGameSessionManager_->dispatcher.removeListener(this);
    categoryModel_->dispatcher.removeListener(this);

    // This is to fix an issue with listeners. The RunningGameSessionView (child of central widget)
    // will try to unregister as a listener of RunningGameSessionManager. The central widget
    // would ordinarily be deleted in the base class of this class, after we've deleted the
    // RunningGameSessionManager which gets deleted in this destructor.
    delete takeCentralWidget();
    delete categoryView_;

    delete ui_;
}

// ----------------------------------------------------------------------------
void MainWindow::setStateConnected()
{
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
void MainWindow::onRunningGameSessionManagerAttemptConnectToServer(const char* ipAddress, uint16_t port)
{
}

// ----------------------------------------------------------------------------
void MainWindow::onRunningGameSessionManagerFailedToConnectToServer(const char* ipAddress, uint16_t port)
{
}

// ----------------------------------------------------------------------------
void MainWindow::onRunningGameSessionManagerConnectedToServer(const char* ipAddress, uint16_t port)
{
    setStateConnected();
}

// ----------------------------------------------------------------------------
void MainWindow::onRunningGameSessionManagerDisconnectedFromServer()
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
    ConnectView* c = new ConnectView(config_.get(), protocol_.get(), Qt::Popup | Qt::Dialog);
    c->setAttribute(Qt::WA_DeleteOnClose);
    c->show();
    c->setGeometry(calculatePopupGeometry(this, c));
}

// ----------------------------------------------------------------------------
void MainWindow::onDisconnectActionTriggered()
{
    // Should also trigger onRunningGameSessionManagerDisconnectedFromServer()
    protocol_->disconnectFromServer();
}

// ----------------------------------------------------------------------------
void MainWindow::onCategorySelected(CategoryType category)
{
    switch (category)
    {
        case CategoryType::TOP_LEVEL_REPLAY_GROUPS:
            replayGroupView_->clear();  // Kinda ugly, should make this a listener of CategoryModel
            mainView_->setCurrentIndex(0);
            break;

        case CategoryType::TOP_LEVEL_DATA_SETS:
            mainView_->setCurrentIndex(1);
            break;

        case CategoryType::TOP_LEVEL_ANALYSIS:
            mainView_->setCurrentIndex(2);
            break;

        case CategoryType::TOP_LEVEL_REPLAY_SOURCES:
            mainView_->setCurrentIndex(3);
            break;

        case CategoryType::TOP_LEVEL_VIDEO_SOURCES:
            mainView_->setCurrentIndex(4);
            break;

        case CategoryType::TOP_LEVEL_SESSION:
            mainView_->setCurrentIndex(5);
            break;

        case CategoryType::TOP_LEVEL_TRAINING_MODE:
            mainView_->setCurrentIndex(6);
            break;

        default: break;
    }
}

// ----------------------------------------------------------------------------
void MainWindow::negotiateDefaultRecordingLocation()
{
    auto askForDir = [this](const QString& path) -> QString {
        return QFileDialog::getExistingDirectory(
            this,
            "Choose directory to save recordings to",
            path,
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );
    };

    QFileInfo pathInfo(replayManager_->defaultRecordingSourceDirectory().path());
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
                replayManager_->setDefaultRecordingSourceDirectory(pathInfo.filePath());
            else
                close();
        }
        else
        {
            if (QDir().mkdir(pathInfo.filePath()))
            {
                replayManager_->setDefaultRecordingSourceDirectory(pathInfo.filePath());
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
                        replayManager_->setDefaultRecordingSourceDirectory(pathInfo.filePath());
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
                replayManager_->setDefaultRecordingSourceDirectory(pathInfo.filePath());
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
