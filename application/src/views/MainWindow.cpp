#include "application/ui_MainWindow.h"
#include "application/config.hpp"
#include "application/models/ActiveSessionManager.hpp"
#include "application/models/Config.hpp"
#include "application/models/PlayerDetails.hpp"
#include "application/models/PluginManager.hpp"
#include "application/models/Protocol.hpp"
#include "application/models/ReplayManager.hpp"
#include "application/models/MotionLabelsManager.hpp"
#include "application/views/CategoryTabsView.hpp"
#include "application/views/ConnectView.hpp"
#include "application/views/ImportReplayPackDialog.hpp"
#include "application/views/MainWindow.hpp"
#include "application/views/PathManagerDialog.hpp"
#include "application/views/MotionLabelsEditor.hpp"
#include "application/widgets/ConnectionStatusWidget.hpp"
#include "application/Util.hpp"

#include "rfcommon/BuildInfo.hpp"
#include "rfcommon/Log.hpp"
#include "rfcommon/Profiler.hpp"

#include <QStackedWidget>
#include <QDockWidget>
#include <QStackedWidget>
#include <QFileInfo>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QTextStream>

namespace rfapp {

using namespace nlohmann;

// ----------------------------------------------------------------------------
MainWindow::MainWindow(std::unique_ptr<Config>&& config, rfcommon::MotionLabels* labels, QWidget* parent)
    : QMainWindow(parent)
    , config_(std::move(config))
    , playerDetails_(new PlayerDetails)
    , protocol_(new Protocol)
    , motionLabelsManager_(new MotionLabelsManager(protocol_.get(), labels))
    , pluginManager_(new PluginManager(labels))
    , replayManager_(new ReplayManager(config_.get()))
    , activeSessionManager_(new ActiveSessionManager(config_.get(), protocol_.get(), replayManager_.get(), pluginManager_.get()))
    , categoryTabsView_(new CategoryTabsView(config_.get(), replayManager_.get(), pluginManager_.get(), activeSessionManager_.get(), playerDetails_.get(), motionLabelsManager_.get()))
    , ui_(new Ui::MainWindow)
{
    ui_->setupUi(this);

    json& cfgTheme = config_->root["theme"];
    if (cfgTheme == "darkstyle")
        ui_->action_darkTheme->setChecked(true);
    else
        ui_->action_defaultTheme->setChecked(true);

    // Window icon and title
    setWindowTitle("ReFramed - " APP_VERSION_STR);
    setWindowIcon(QIcon(":/icons/reframed-icon.ico"));

    // Set icons in dropdown menus
    ui_->action_quit->setIcon(QIcon::fromTheme("power"));

    setCentralWidget(categoryTabsView_);

    statusBar()->addPermanentWidget(new ConnectionStatusWidget(protocol_.get()));

    protocol_->dispatcher.addListener(this);

    connect(ui_->action_connect, &QAction::triggered,
            this, &MainWindow::onConnectActionTriggered);
    connect(ui_->action_attachToN64Emu, &QAction::triggered,
            this, &MainWindow::onAttachToN64EmuTriggered);
    connect(ui_->action_disconnect, &QAction::triggered,
            this, &MainWindow::onDisconnectActionTriggered);
    connect(ui_->action_importReplayPack, &QAction::triggered,
            this, &MainWindow::onImportReplayPackTriggered);
    connect(ui_->action_defaultTheme, &QAction::triggered,
            this, &MainWindow::onDefaultThemeTriggered);
    connect(ui_->action_darkTheme, &QAction::triggered,
            this, &MainWindow::onDarkThemeTriggered);
    connect(ui_->action_userLabelsEditor, &QAction::triggered,
        this, &MainWindow::onUserLabelsEditorActionTriggered);
    connect(ui_->action_pathManager, &QAction::triggered,
        this, &MainWindow::onPathManagerActionTriggered);

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
    delete centralWidget();
    delete statusBar();

    delete ui_;
}

// ----------------------------------------------------------------------------
void MainWindow::negotiateDefaultRecordingLocation()
{
    PROFILE(MainWindow, negotiateDefaultRecordingLocation);

    auto askForDir = [this](const QString& path) -> QString {
        return QFileDialog::getExistingDirectory(
            this,
            "Choose directory to save replays to",
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
            QFileInfo dirInfo(askForDir(gamesDir.path()));
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
                    QFileInfo dirInfo(askForDir(gamesDir.path()));
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

    ConnectView c(config_.get(), protocol_.get(), this);
    c.setGeometry(calculatePopupGeometryKeepSize(this, &c, c.geometry()));
    c.exec();
}

// ----------------------------------------------------------------------------
void MainWindow::onAttachToN64EmuTriggered()
{
    PROFILE(MainWindow, onAttachToN64EmuTriggered);

    protocol_->connectToSSB64Process();
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
    PROFILE(MainWindow, onImportReplayPackTriggered);

    ImportReplayPackDialog dialog(replayManager_.get(), this);
    dialog.exec();
}

// ----------------------------------------------------------------------------
void MainWindow::onDefaultThemeTriggered()
{
    PROFILE(MainWindow, onDefaultThemeTriggered);

    // NOTE: Hack, but we change the app's style *after* changing the icon
    // theme so that widgets that listen to QEvent::StyleChange can reload
    // the icons properly
    QIcon::setThemeName("feather-light");
    qApp->setStyleSheet("");

    ui_->action_defaultTheme->setChecked(true);
    ui_->action_darkTheme->setChecked(false);

    config_->root["theme"] = "default";
    config_->save();
}

// ----------------------------------------------------------------------------
void MainWindow::onDarkThemeTriggered()
{
    PROFILE(MainWindow, onDarkThemeTriggered);

    QFile f(":/qdarkstyle/dark/darkstyle.qss");
    f.open(QIODevice::ReadOnly);
    if (!f.isOpen())
        return;

    // NOTE: Hack, but we change the app's style *after* changing the icon
    // theme so that widgets that listen to QEvent::StyleChange can reload
    // the icons properly
    QIcon::setThemeName("feather-dark");
    QTextStream ts(&f);
    qApp->setStyleSheet(ts.readAll());

    ui_->action_defaultTheme->setChecked(false);
    ui_->action_darkTheme->setChecked(true);

    config_->root["theme"] = "darkstyle";
    config_->save();
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
    userMotionLabelsEditor_ = new MotionLabelsEditor(this, motionLabelsManager_.get(), protocol_.get(), map);
    userMotionLabelsEditor_->setGeometry(calculatePopupGeometryActiveScreen());
    userMotionLabelsEditor_->show();
    userMotionLabelsEditor_->raise();

    // Disable action in dropdown so user can't open this more than once
    ui_->action_userLabelsEditor->setEnabled(false);
}

// ----------------------------------------------------------------------------
void MainWindow::onPathManagerActionTriggered()
{
    PathManagerDialog dialog(replayManager_.get(), replayManager_.get(), this);
    dialog.setGeometry(calculatePopupGeometryActiveScreen(800));
    dialog.exec();
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

    // Replace the "connect" action in the dropdown menu with "disconnect"
    ui_->action_connect->setVisible(false);
    ui_->action_attachToN64Emu->setVisible(false);
    ui_->action_disconnect->setVisible(true);
}

// ----------------------------------------------------------------------------
void MainWindow::onProtocolFailedToConnectToServer(const char* errorMsg, const char* ipAddress, uint16_t port)
{
    PROFILE(MainWindow, onProtocolFailedToConnectToServer);

    // Replace the "disconnect" action in the dropdown menu with "connect"
    ui_->action_connect->setVisible(true);
    ui_->action_attachToN64Emu->setVisible(true);
    ui_->action_disconnect->setVisible(false);
}

// ----------------------------------------------------------------------------
void MainWindow::onProtocolConnectedToServer(const char* ipAddress, uint16_t port)
{
    PROFILE(MainWindow, onProtocolConnectedToServer);
}

// ----------------------------------------------------------------------------
void MainWindow::onProtocolDisconnectedFromServer()
{
    PROFILE(MainWindow, onProtocolDisconnectedFromServer);

    // Replace the "disconnect" action in the dropdown menu with "connect"
    ui_->action_connect->setVisible(true);
    ui_->action_attachToN64Emu->setVisible(true);
    ui_->action_disconnect->setVisible(false);
}

// ----------------------------------------------------------------------------
void MainWindow::onMotionLabelsEditorClosed()
{
    PROFILE(MainWindow, onMotionLabelsEditorClosed);

    ui_->action_userLabelsEditor->setEnabled(true);
    userMotionLabelsEditor_ = nullptr;
}

}
