#include "application/ui_MainWindow.h"
#include "application/config.hpp"
#include "application/models/ActiveSessionManager.hpp"
#include "application/models/Config.hpp"
#include "application/models/PluginManager.hpp"
#include "application/models/Protocol.hpp"
#include "application/models/ReplayManager.hpp"
#include "application/models/UserMotionLabelsManager.hpp"
#include "application/views/ActiveSessionView.hpp"
#include "application/views/CategoryTabsView.hpp"
#include "application/views/ConnectView.hpp"
#include "application/views/ImportReplayPackDialog.hpp"
#include "application/views/MainWindow.hpp"
#include "application/views/ReplayGroupView.hpp"
#include "application/views/UserMotionLabelsEditor.hpp"
#include "application/Util.hpp"

#include "rfcommon/BuildInfo.hpp"
#include "rfcommon/Hash40Strings.hpp"
#include "rfcommon/Log.hpp"
#include "rfcommon/MappedFile.hpp"
#include "rfcommon/Profiler.hpp"

#include <QStackedWidget>
#include <QDockWidget>
#include <QStackedWidget>
#include <QFileInfo>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDesktopServices>

namespace rfapp {

// ----------------------------------------------------------------------------
MainWindow::MainWindow(rfcommon::Hash40Strings* hash40Strings, QWidget* parent)
    : QMainWindow(parent)
    , hash40Strings_(hash40Strings)
    , config_(new Config)
    , protocol_(new Protocol)
    , userMotionLabelsManager_(new UserMotionLabelsManager(protocol_.get()))
    , pluginManager_(new PluginManager(userMotionLabelsManager_->userMotionLabels(), hash40Strings_))
    , replayManager_(new ReplayManager(config_.get()))
    , activeSessionManager_(new ActiveSessionManager(protocol_.get(), replayManager_.get()))
    , categoryTabsView_(new CategoryTabsView)
    , replayGroupView_(new ReplayGroupView(replayManager_.get(), pluginManager_.get(), userMotionLabelsManager_.get(), hash40Strings_.get()))
    , activeSessionView_(new ActiveSessionView(activeSessionManager_.get(), pluginManager_.get()))
    , ui_(new Ui::MainWindow)
{
    ui_->setupUi(this);

    // Window icon and title
    setWindowTitle("ReFramed - " APP_VERSION_STR);
    setWindowIcon(QIcon(":/icons/reframed-icon.ico"));

    categoryTabsView_->addTab(activeSessionView_, "Session");
    categoryTabsView_->addTab(replayGroupView_, "Replays");
    categoryTabsView_->addTab(new QWidget, "Compare");
    categoryTabsView_->addTab(new QWidget, "Marketplace");
    categoryTabsView_->setCurrentIndex(1);
    setCentralWidget(categoryTabsView_);

    /*
    QDockWidget* categoryDock = new QDockWidget(this);
    categoryDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    categoryDock->setWidget(categoryView_);
    categoryDock->setFeatures(QDockWidget::DockWidgetMovable);
    addDockWidget(Qt::LeftDockWidgetArea, categoryDock);*/

    protocol_->dispatcher.addListener(this);

    connect(ui_->action_connect, &QAction::triggered,
            this, &MainWindow::onConnectActionTriggered);
    connect(ui_->action_disconnect, &QAction::triggered,
            this, &MainWindow::onDisconnectActionTriggered);
    connect(ui_->action_importReplayPack, &QAction::triggered,
            this, &MainWindow::onImportReplayPackTriggered);
    connect(ui_->action_userLabelsEditor, &QAction::triggered,
            this, &MainWindow::onUserLabelsEditorActionTriggered);

    // Execute this later so the main window is visible when the popup opens
    // A single popup without the main window feels weird
    QMetaObject::invokeMethod(this, "negotiateDefaultRecordingLocation", Qt::QueuedConnection);
}

// ----------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    protocol_->dispatcher.removeListener(this);

    // This is to fix an issue with listeners. The RunningGameSessionView (child of central widget)
    // will try to unregister as a listener of RunningGameSessionManager. The central widget
    // would ordinarily be deleted in the base class of this class, after we've deleted the
    // RunningGameSessionManager which gets deleted in this destructor.
    delete takeCentralWidget();

    delete ui_;
}

// ----------------------------------------------------------------------------
void MainWindow::negotiateDefaultRecordingLocation()
{
    PROFILE(MainWindow, negotiateDefaultRecordingLocation);

    auto askForDir = [this](const QString& path) -> QString {
        return QFileDialog::getExistingDirectory(
            this,
            "Choose directory to save recordings to",
            path,
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );
    };

    QDir gamesDir = replayManager_->defaultGamePath();
    if (gamesDir.exists() == false)
    {
        int option = QMessageBox::warning(
            this,
            "Default directory for replays not found",
            QString("The directory for saving replays was not found or has not been configured yet.\n"
                "Would you like to create the directory now?\n\n") +
            gamesDir.path(),
            QMessageBox::Yes | QMessageBox::Open
        );

        if (option == QMessageBox::Open)
        {
            QFileInfo dirInfo = askForDir(gamesDir.path());
            if (dirInfo.exists() && dirInfo.isDir() && dirInfo.isWritable())
            {
                replayManager_->setDefaultGamePath(dirInfo.filePath());
                gamesDir = replayManager_->defaultGamePath();
                if (gamesDir.exists() == false)
                    gamesDir.mkpath(".");
            }
            else
                close();
        }
        else
        {
            if (gamesDir.mkpath("."))
            {
                replayManager_->setDefaultGamePath(gamesDir);
                if (gamesDir.exists() == false)
                    gamesDir.mkpath(".");
            }
            else
            {
                int option = QMessageBox::warning(
                    this,
                    "Failed to create directory",
                    QString("Could not create the directory:\n") +
                    gamesDir.path() +
                    "\n\nWould you like to choose a directory?",
                    QMessageBox::Yes | QMessageBox::No
                );

                if (option == QMessageBox::Yes)
                {
                    QFileInfo dirInfo = askForDir(gamesDir.path());
                    if (dirInfo.exists() && dirInfo.isDir() && dirInfo.isWritable())
                    {
                        replayManager_->setDefaultGamePath(dirInfo.filePath());
                        QDir gamesDir = replayManager_->defaultGamePath();
                        if (gamesDir.exists() == false)
                            gamesDir.mkpath(".");
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

    QDir trainingDir = replayManager_->defaultTrainingPath();
    if (trainingDir.exists() == false)
        trainingDir.mkpath(".");
}

// ----------------------------------------------------------------------------
void MainWindow::populateCategories()
{
    PROFILE(MainWindow, populateCategories);


}

// ----------------------------------------------------------------------------
void MainWindow::onConnectActionTriggered()
{
    PROFILE(MainWindow, onConnectActionTriggered);

    ConnectView c(config_.get(), protocol_.get());
    c.setGeometry(calculatePopupGeometryKeepSize(this, &c, c.geometry()));
    c.exec();
}

// ----------------------------------------------------------------------------
void MainWindow::onDisconnectActionTriggered()
{
    PROFILE(MainWindow, onDisconnectActionTriggered);

    // Should also trigger onRunningGameSessionManagerDisconnectedFromServer()
    protocol_->disconnectFromServer();
}

// ----------------------------------------------------------------------------
void MainWindow::onImportReplayPackTriggered()
{
    ImportReplayPackDialog dialog(replayManager_.get(), this);
    dialog.exec();
}

// ----------------------------------------------------------------------------
void MainWindow::onUserLabelsEditorActionTriggered()
{
    PROFILE(MainWindow, onUserLabelsEditorActionTriggered);

    if (userMotionLabelsEditor_)
        return;

    rfcommon::MappingInfo* map = protocol_->globalMappingInfo();
    if (map == nullptr)
    {
        QDir configPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
        QString mappingFileName = configPath.absoluteFilePath("mappingInfo.json");
        QMessageBox::critical(this, "No Mapping Info",
            "Global mapping information file is missing: " + mappingFileName + "\n\n"
            "Connecting to your Nintendo Switch will re-download this file.");
        return;
    }

    QRect popupGeometry = geometry();
    popupGeometry.setWidth(popupGeometry.width() * 3 / 4);
    popupGeometry.setHeight(popupGeometry.height() * 3 / 4);

    // NOTE: This editor manages its own deletion when it closes via deleteLater()
    userMotionLabelsEditor_ = new UserMotionLabelsEditor(this, userMotionLabelsManager_.get(), hash40Strings_, map);
    userMotionLabelsEditor_->setGeometry(calculatePopupGeometryActiveScreen());
    userMotionLabelsEditor_->show();
    userMotionLabelsEditor_->raise();

    // Disable action in dropdown so user can't open this more than once
    ui_->action_userLabelsEditor->setEnabled(false);
}

// ----------------------------------------------------------------------------
void MainWindow::onAboutActionTriggered()
{
    PROFILE(MainWindow, onAboutActionTriggered);

    QString text = QString(
        "ReFramed\n\n"
        "Created by TheComet\n"
        "  https://github.com/TheComet\n"
        "  https://github.com/reframedultimate/ReFramed\n"
        "  TheComet#5387, @TheComet93\n") +
        "Build information:\n" +
        "  Commit info: " + rfcommon::BuildInfo::commit() + "\n" +
        "  Build host: " + rfcommon::BuildInfo::buildHost() + "\n" +
        "  Compiler: " + rfcommon::BuildInfo::compiler();
    QMessageBox::about(this, "About", text);
}

// ----------------------------------------------------------------------------
void MainWindow::onViewLogActionTriggered()
{
    PROFILE(MainWindow, onViewLogActionTriggered);

    QUrl fileURL = QUrl::fromLocalFile(rfcommon::Log::root()->fileName());
    QDesktopServices::openUrl(fileURL);
}

// ----------------------------------------------------------------------------
void MainWindow::closeEvent(QCloseEvent* event)
{
    PROFILE(MainWindow, closeEvent);

    if (userMotionLabelsEditor_)
        userMotionLabelsEditor_->close();
}

// ----------------------------------------------------------------------------
void MainWindow::onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port)
{
    PROFILE(MainWindow, onProtocolAttemptConnectToServer);

}

// ----------------------------------------------------------------------------
void MainWindow::onProtocolFailedToConnectToServer(const char* errorMsg, const char* ipAddress, uint16_t port)
{
    PROFILE(MainWindow, onProtocolFailedToConnectToServer);

}

// ----------------------------------------------------------------------------
void MainWindow::onProtocolConnectedToServer(const char* ipAddress, uint16_t port)
{
    PROFILE(MainWindow, onProtocolConnectedToServer);

    // Replace the "connect" action in the dropdown menu with "disconnect"
    ui_->action_connect->setVisible(false);
    ui_->action_disconnect->setVisible(true);
}

// ----------------------------------------------------------------------------
void MainWindow::onProtocolDisconnectedFromServer()
{
    PROFILE(MainWindow, onProtocolDisconnectedFromServer);

    // Replace the "disconnect" action in the dropdown menu with "connect"
    ui_->action_connect->setVisible(true);
    ui_->action_disconnect->setVisible(false);
}

// ----------------------------------------------------------------------------
void MainWindow::onUserMotionLabelsEditorClosed()
{
    PROFILE(MainWindow, onUserMotionLabelsEditorClosed);

    ui_->action_userLabelsEditor->setEnabled(true);
    userMotionLabelsEditor_ = nullptr;
}

}
