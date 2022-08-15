#include "application/ui_MainWindow.h"
#include "application/config.hpp"
#include "application/models/Config.hpp"
#include "application/models/CategoryModel.hpp"
#include "application/models/PluginManager.hpp"
#include "application/models/Protocol.hpp"
#include "application/models/ReplayManager.hpp"
#include "application/models/ActiveSessionManager.hpp"
#include "application/views/ActiveSessionView.hpp"
#include "application/views/AnalysisView.hpp"
#include "application/views/CategoryView.hpp"
#include "application/views/ConnectView.hpp"
#include "application/views/DataSetFilterView.hpp"
#include "application/views/MainWindow.hpp"
#include "application/views/ReplayGroupView.hpp"
#include "application/views/UserLabelsEditor.hpp"
#include "application/views/VisualizerView.hpp"

#include <QStackedWidget>
#include <QDockWidget>
#include <QStackedWidget>
#include <QFileInfo>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>

namespace rfapp {

// ----------------------------------------------------------------------------
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , config_(new Config)
    , protocol_(new Protocol)
    , pluginManager_(new PluginManager)
    , replayManager_(new ReplayManager(config_.get()))
    , activeSessionManager_(new ActiveSessionManager(protocol_.get(), replayManager_.get()))
    , categoryModel_(new CategoryModel)
    , categoryView_(new CategoryView(categoryModel_.get(), replayManager_.get()))
    , replayGroupView_(new ReplayGroupView(replayManager_.get(), pluginManager_.get()))
    , activeSessionView_(new ActiveSessionView(activeSessionManager_.get(), pluginManager_.get()))
    , mainView_(new QStackedWidget)
    , ui_(new Ui::MainWindow)
{
    ui_->setupUi(this);

    // Window icon and title
    setWindowTitle("ReFramed - " APP_VERSION_STR);
    setWindowIcon(QIcon(":/icons/reframed-icon.ico"));

    mainView_->addWidget(replayGroupView_);
    mainView_->addWidget(new DataSetFilterView(replayManager_.get()));
    mainView_->addWidget(new VisualizerView(pluginManager_.get()));
    mainView_->addWidget(new QWidget);
    mainView_->addWidget(new QWidget);
    mainView_->addWidget(activeSessionView_);
    setCentralWidget(mainView_);

    QDockWidget* categoryDock = new QDockWidget(this);
    categoryDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    categoryDock->setWidget(categoryView_);
    categoryDock->setFeatures(QDockWidget::DockWidgetMovable);
    addDockWidget(Qt::LeftDockWidgetArea, categoryDock);

    protocol_->dispatcher.addListener(this);
    categoryModel_->dispatcher.addListener(this);

    connect(ui_->action_connect, &QAction::triggered,
            this, &MainWindow::onConnectActionTriggered);
    connect(ui_->action_disconnect, &QAction::triggered,
            this, &MainWindow::onDisconnectActionTriggered);
    connect(ui_->action_userLabelsEditor, &QAction::triggered,
            this, &MainWindow::onUserLabelsEditorActionTriggered);

    categoryModel_->selectReplayGroupsCategory();

    // Execute this later so the main window is visible when the popup opens
    // A single popup without the main window feels weird
    QMetaObject::invokeMethod(this, "negotiateDefaultRecordingLocation", Qt::QueuedConnection);
}

// ----------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    categoryModel_->dispatcher.removeListener(this);
    protocol_->dispatcher.removeListener(this);

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
void MainWindow::onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port)
{
}

// ----------------------------------------------------------------------------
void MainWindow::onProtocolFailedToConnectToServer(const char* errorMsg, const char* ipAddress, uint16_t port)
{
}

// ----------------------------------------------------------------------------
void MainWindow::onProtocolConnectedToServer(const char* ipAddress, uint16_t port)
{
    setStateConnected();
}

// ----------------------------------------------------------------------------
void MainWindow::onProtocolDisconnectedFromServer()
{
    setStateDisconnected();
}

// ----------------------------------------------------------------------------
static QRect calculatePopupGeometry(const QWidget* main, const QWidget* popup, QRect popupRect)
{
    QRect mainRect = main->geometry();

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
    ConnectView c(config_.get(), protocol_.get(), Qt::Popup | Qt::Dialog);
    c.setGeometry(calculatePopupGeometry(this, &c, c.geometry()));
    c.exec();
}

// ----------------------------------------------------------------------------
void MainWindow::onDisconnectActionTriggered()
{
    // Should also trigger onRunningGameSessionManagerDisconnectedFromServer()
    protocol_->disconnectFromServer();
}

// ----------------------------------------------------------------------------
void MainWindow::onUserLabelsEditorActionTriggered()
{
    rfcommon::MappingInfo* map = protocol_->globalMappingInfo();
    if (map == nullptr)
    {
        QDir configPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
        QString mappingFileName = configPath.absoluteFilePath("mappingInfo.json");
        QMessageBox::critical(this, "Error",
                "Global mapping information file is missing: " + mappingFileName + "\n\n"
                "Connecting to your Nintendo Switch will re-download this file.");
        return;
    }

    QRect popupGeometry = geometry();
    popupGeometry.setWidth(popupGeometry.width() * 3 / 4);
    popupGeometry.setHeight(popupGeometry.height() * 3 / 4);

    UserLabelsEditor editor(userLabels_.get());
    editor.setGeometry(calculatePopupGeometry(this, &editor, popupGeometry));
    editor.exec();
}

// ----------------------------------------------------------------------------
void MainWindow::onCategorySelected(CategoryType category)
{
    switch (category)
    {
        case CategoryType::TOP_LEVEL_REPLAY_GROUPS:
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

    QDir replayDir = replayManager_->defaultReplaySourceDirectory();
    QDir gamesDir = replayManager_->defaultGameSessionSourceDirectory();
    QDir trainingDir = replayManager_->defaultTrainingSessionSourceDirectory();
    if (!gamesDir.exists() || !trainingDir.exists())
    {
        int option = QMessageBox::warning(
            this,
            "Default directory for replays not found",
            QString("The directory for saving replays was not found or has not been configured yet.\n"
                    "Would you like to create the directory now?\n\n") +
                    replayDir.path(),
            QMessageBox::Yes | QMessageBox::Open
        );

        if (option == QMessageBox::Open)
        {
            QFileInfo dirInfo = askForDir(replayDir.path());
            if (dirInfo.exists() && dirInfo.isDir() && dirInfo.isWritable())
            {
                replayManager_->setDefaultReplaySourceDirectory(dirInfo.filePath());
                QDir gamesDir = replayManager_->defaultGameSessionSourceDirectory();
                QDir trainingDir = replayManager_->defaultTrainingSessionSourceDirectory();

                if (gamesDir.exists() == false)
                    gamesDir.mkpath(".");
                if (trainingDir.exists() == false)
                    trainingDir.mkpath(".");
            }
            else
                close();
        }
        else
        {
            if (replayDir.mkpath("."))
            {
                replayManager_->setDefaultReplaySourceDirectory(replayDir);
                QDir gamesDir = replayManager_->defaultGameSessionSourceDirectory();
                QDir trainingDir = replayManager_->defaultTrainingSessionSourceDirectory();

                if (gamesDir.exists() == false)
                    gamesDir.mkpath(".");
                if (trainingDir.exists() == false)
                    trainingDir.mkpath(".");
            }
            else
            {
                int option = QMessageBox::warning(
                    this,
                    "Failed to create directory",
                    QString("Could not create the directory:\n") +
                            replayDir.path() +
                            "\n\nWould you like to choose a directory?",
                    QMessageBox::Yes | QMessageBox::No
                );

                if (option == QMessageBox::Yes)
                {
                    QFileInfo dirInfo = askForDir(replayDir.path());
                    if (dirInfo.exists() && dirInfo.isDir() && dirInfo.isWritable())
                    {
                        replayManager_->setDefaultReplaySourceDirectory(dirInfo.filePath());
                        QDir gamesDir = replayManager_->defaultGameSessionSourceDirectory();
                        QDir trainingDir = replayManager_->defaultTrainingSessionSourceDirectory();

                        if (gamesDir.exists() == false)
                            gamesDir.mkpath(".");
                        if (trainingDir.exists() == false)
                            trainingDir.mkpath(".");
                    }
                    else
                        close();
                }
                else
                {
                    close();
                }
            }
        }
    }
}

// ----------------------------------------------------------------------------
void MainWindow::populateCategories()
{

}

}
