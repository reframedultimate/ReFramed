#pragma once

#include "application/models/CategoryType.hpp"
#include "application/listeners/CategoryListener.hpp"
#include "application/listeners/ActiveGameSessionManagerListener.hpp"
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
class ActiveGameSessionManager;
class RunningGameSessionView;
class ReplayGroupView;
class ReplayManager;
class TrainingModeModel;

class MainWindow : public QMainWindow
                 , public CategoryListener
                 , public ActiveGameSessionManagerListener
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
    void onActiveGameSessionManagerAttemptConnectToServer(const char* ipAddress, uint16_t port) override;
    void onActiveGameSessionManagerFailedToConnectToServer(const char* ipAddress, uint16_t port) override;
    void onActiveGameSessionManagerConnectedToServer(const char* ipAddress, uint16_t port) override;
    void onActiveGameSessionManagerDisconnectedFromServer() override;

    // All unused
    void onActiveGameSessionManagerMatchStarted(rfcommon::RunningGameSession* recording) override { (void)recording; }
    void onActiveGameSessionManagerMatchEnded(rfcommon::RunningGameSession* recording) override { (void)recording; }
    void onActiveGameSessionManagerNewFrame(int frameIdx, const rfcommon::Frame& frame) override {}
    void onActiveGameSessionManagerPlayerNameChanged(int player, const rfcommon::SmallString<15>& name) override { (void)player; (void)name; }
    void onActiveGameSessionManagerSetNumberChanged(rfcommon::SetNumber number) override { (void)number; }
    void onActiveGameSessionManagerGameNumberChanged(rfcommon::GameNumber number) override { (void)number; }
    void onRunningGameSessionManagerFormatChanged(const rfcommon::SetFormat& format) override { (void)format; }
    void onActiveGameSessionManagerWinnerChanged(int winner) { (void)winner; }

private:
    std::unique_ptr<Config> config_;
    std::unique_ptr<Protocol> protocol_;
    std::unique_ptr<PluginManager> pluginManager_;
    std::unique_ptr<ReplayManager> replayManager_;
    std::unique_ptr<ActiveGameSessionManager> runningGameSessionManager_;
    std::unique_ptr<TrainingModeModel> trainingModeModel_;
    std::unique_ptr<CategoryModel> categoryModel_;
    CategoryView* categoryView_;
    ReplayGroupView* replayGroupView_;
    RunningGameSessionView* runningGameSessionView_;
    QStackedWidget* mainView_;
    Ui::MainWindow* ui_;
};

}
