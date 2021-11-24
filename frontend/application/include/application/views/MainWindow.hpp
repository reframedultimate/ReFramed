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

class CategoryView;
class Config;
class ConnectView;
class PluginManager;
class Protocol;
class RunningGameSessionManager;
class RunningGameSessionView;
class SavedGameSessionGroupView;
class SavedGameSessionManager;
class TrainingModeModel;

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
    std::unique_ptr<SavedGameSessionManager> savedGameSessionManager_;
    std::unique_ptr<RunningGameSessionManager> runningGameSessionManager_;
    std::unique_ptr<PluginManager> pluginManager_;
    std::unique_ptr<TrainingModeModel> trainingModeModel_;
    CategoryView* categoryView_;
    SavedGameSessionGroupView* savedGameSessionGroupView_;
    RunningGameSessionView* runningGameSessionView_;
    QStackedWidget* mainView_;
    Ui::MainWindow* ui_;
};

}
