#pragma once

#include "rfcommon/ProtocolListener.hpp"
#include "rfcommon/Reference.hpp"
#include <QMainWindow>
#include <QDir>
#include <memory>

class QTreeWidgetItem;

namespace Ui {
    class MainWindow;
}

namespace rfcommon {
    class Hash40Strings;
    class MotionLabels;
}

namespace rfapp {

class CategoryTabsView;
class Config;
class PlayerDetails;
class PluginManager;
class Protocol;
class ActiveSessionManager;
class ReplayManager;
class UserMotionLabelsManager;
class UserMotionLabelsEditor;

class MainWindow
        : public QMainWindow
        , public rfcommon::ProtocolListener
{
    Q_OBJECT

public:
    explicit MainWindow(std::unique_ptr<Config>&& config, rfcommon::Hash40Strings* hash40Strings, rfcommon::MotionLabels* motionLabels, QWidget* parent=nullptr);
    ~MainWindow();

private slots:
    void negotiateDefaultRecordingLocation();
    void populateCategories();

    void onConnectActionTriggered();
    void onAttachToN64EmuTriggered();
    void onDisconnectActionTriggered();
    void onImportReplayPackTriggered();
    void onDefaultThemeTriggered();
    void onDarkThemeTriggered();
    void onUserLabelsEditorActionTriggered();
    void onPathManagerActionTriggered();

    void onAboutActionTriggered();
    void onViewLogActionTriggered();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    // Protocol callbacks
    void onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) override;
    void onProtocolFailedToConnectToServer(const char* errorMsg, const char* ipAddress, uint16_t port) override;
    void onProtocolConnectedToServer(const char* ipAddress, uint16_t port) override;
    void onProtocolDisconnectedFromServer() override;

    // All unused
    void onProtocolTrainingStarted(rfcommon::Session* training) override { (void)training; };
    void onProtocolTrainingResumed(rfcommon::Session* training) override { (void)training; }
    void onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining) override { (void)oldTraining; (void)newTraining; }
    void onProtocolTrainingEnded(rfcommon::Session* training) override { (void)training; }
    void onProtocolGameStarted(rfcommon::Session* game) override { (void)game; }
    void onProtocolGameResumed(rfcommon::Session* game) override { (void)game; }
    void onProtocolGameEnded(rfcommon::Session* game) override { (void)game; }

private:
    // Non-modal editors in separate windows will call the main window
    // to notify when they close, so menu items can be updated
    friend class UserMotionLabelsEditor;
    void onUserMotionLabelsEditorClosed();

private:
    rfcommon::Reference<rfcommon::Hash40Strings> hash40Strings_;
    std::unique_ptr<Config> config_;
    std::unique_ptr<PlayerDetails> playerDetails_;
    std::unique_ptr<Protocol> protocol_;
    std::unique_ptr<UserMotionLabelsManager> userMotionLabelsManager_;
    std::unique_ptr<PluginManager> pluginManager_;
    std::unique_ptr<ReplayManager> replayManager_;
    std::unique_ptr<ActiveSessionManager> activeSessionManager_;
    CategoryTabsView* categoryTabsView_;
    Ui::MainWindow* ui_;

    // Non-modal views that appear in a separate window
    UserMotionLabelsEditor* userMotionLabelsEditor_ = nullptr;
};

}
