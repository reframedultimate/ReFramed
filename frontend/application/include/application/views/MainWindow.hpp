#pragma once

#include "application/models/CategoryType.hpp"
#include "application/listeners/RunningGameSessionManagerListener.hpp"
#include <QMainWindow>
#include <QDir>
#include <memory>

class QStackedWidget;
class QTreeWidgetItem;

namespace Ui {
    class MainWindow;
}

namespace uhapp {

class RunningGameSessionManager;
class RunningGameSessionView;
class CategoryView;
class ConnectView;
class PluginManager;
class Protocol;
class RecordingGroupView;
class RecordingManager;
class TrainingMode;
class Config;

class MainWindow : public QMainWindow
                 , public RunningGameSessionManagerListener
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent=nullptr);
    ~MainWindow();

private slots:
    void onCategoryChanged(CategoryType category);
    void negotiateDefaultRecordingLocation();
    void populateCategories();

private:
    // Changes the UI to reflect connected/disconnected state
    void setStateConnected();
    void setStateDisconnected();

private slots:
    void onRunningGameSessionManagerConnectedToServer();
    void onRunningGameSessionManagerDisconnectedFromServer();
    void onConnectActionTriggered();
    void onDisconnectActionTriggered();

private:
    // All unused
    void onRunningGameSessionManagerRecordingStarted(uh::RunningGameSession* recording) override { (void)recording; }
    void onRunningGameSessionManagerRecordingEnded(uh::RunningGameSession* recording) override { (void)recording; }
    void onRunningGameSessionManagerP1NameChanged(const QString& name) override { (void)name; }
    void onRunningGameSessionManagerP2NameChanged(const QString& name) override { (void)name; }
    void onRunningGameSessionManagerSetNumberChanged(uh::SetNumber number) override { (void)number; }
    void onRunningGameSessionManagerGameNumberChanged(uh::GameNumber number) override { (void)number; }
    void onRunningGameSessionManagerFormatChanged(const uh::SetFormat& format) override { (void)format; }
    void onRunningGameSessionManagerPlayerStateAdded(int player, const uh::PlayerState& state) override { (void)player; (void)state; }
    void onRunningGameSessionManagerWinnerChanged(int winner) { (void)winner; }

private:
    std::unique_ptr<Config> config_;
    std::unique_ptr<RecordingManager> recordingManager_;
    std::unique_ptr<RunningGameSessionManager> activeRecordingManager_;
    std::unique_ptr<TrainingMode> trainingMode_;
    std::unique_ptr<PluginManager> pluginManager_;
    CategoryView* categoryView_;
    RecordingGroupView* recordingGroupView_;
    RunningGameSessionView* activeRecordingView_;
    QStackedWidget* mainView_;
    Ui::MainWindow* ui_;
};

}
