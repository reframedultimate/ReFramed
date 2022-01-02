#pragma once

#include "application/models/CategoryType.hpp"
#include "application/listeners/CategoryListener.hpp"
#include "application/listeners/RunningGameSessionManagerListener.hpp"
#include <QMainWindow>
#include <QDir>
#include <memory>

class QStackedWidget;
class QTreeWidgetItem;

namespace Ui {
    class MainWindow;
}

namespace rfapp {

class CategoryView;
class CategoryModel;
class Config;
class PluginManager;
class Protocol;
class RunningGameSessionManager;
class RunningGameSessionView;
class ReplayGroupView;
class ReplayManager;
class TrainingModeModel;

class MainWindow : public QMainWindow
                 , public CategoryListener
                 , public RunningGameSessionManagerListener
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent=nullptr);
    ~MainWindow();

private slots:
    void negotiateDefaultRecordingLocation();
    void populateCategories();

private:
    // Changes the UI to reflect connected/disconnected state
    void setStateConnected();
    void setStateDisconnected();

private slots:
    void onConnectActionTriggered();
    void onDisconnectActionTriggered();

private:
    void onCategorySelected(CategoryType category) override;
    void onCategoryItemSelected(CategoryType category, const QString& name) override { (void)category; (void)name; }

private:
    void onRunningGameSessionManagerAttemptConnectToServer(const char* ipAddress, uint16_t port) override;
    void onRunningGameSessionManagerFailedToConnectToServer(const char* ipAddress, uint16_t port) override;
    void onRunningGameSessionManagerConnectedToServer(const char* ipAddress, uint16_t port) override;
    void onRunningGameSessionManagerDisconnectedFromServer() override;

    // All unused
    void onRunningGameSessionManagerMatchStarted(rfcommon::RunningGameSession* recording) override { (void)recording; }
    void onRunningGameSessionManagerMatchEnded(rfcommon::RunningGameSession* recording) override { (void)recording; }
    void onRunningGameSessionManagerPlayerNameChanged(int player, const rfcommon::SmallString<15>& name) override { (void)player; (void)name; }
    void onRunningGameSessionManagerSetNumberChanged(rfcommon::SetNumber number) override { (void)number; }
    void onRunningGameSessionManagerGameNumberChanged(rfcommon::GameNumber number) override { (void)number; }
    void onRunningGameSessionManagerFormatChanged(const rfcommon::SetFormat& format) override { (void)format; }
    void onRunningGameSessionManagerNewPlayerState(int player, const rfcommon::FighterFrame& state) override { (void)player; (void)state; }
    void onRunningGameSessionManagerWinnerChanged(int winner) { (void)winner; }

private:
    std::unique_ptr<Config> config_;
    std::unique_ptr<Protocol> protocol_;
    std::unique_ptr<PluginManager> pluginManager_;
    std::unique_ptr<ReplayManager> replayManager_;
    std::unique_ptr<RunningGameSessionManager> runningGameSessionManager_;
    std::unique_ptr<TrainingModeModel> trainingModeModel_;
    std::unique_ptr<CategoryModel> categoryModel_;
    CategoryView* categoryView_;
    ReplayGroupView* replayGroupView_;
    RunningGameSessionView* runningGameSessionView_;
    QStackedWidget* mainView_;
    Ui::MainWindow* ui_;
};

}
