#pragma once

#include "application/models/CategoryType.hpp"
#include "application/listeners/ActiveRecordingManagerListener.hpp"
#include <QMainWindow>
#include <QDir>
#include <memory>

class QStackedWidget;
class QTreeWidgetItem;

namespace Ui {
    class MainWindow;
}

namespace uhapp {

class ActiveRecordingManager;
class ActiveRecordingView;
class CategoryView;
class ConnectView;
class PluginManager;
class Protocol;
class RecordingGroupView;
class RecordingManager;
class TrainingMode;
class Config;

class MainWindow : public QMainWindow
                 , public ActiveRecordingManagerListener
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
    void onActiveRecordingManagerConnectedToServer();
    void onActiveRecordingManagerDisconnectedFromServer();
    void onConnectActionTriggered();
    void onDisconnectActionTriggered();

private:
    // All unused
    void onActiveRecordingManagerRecordingStarted(uh::ActiveRecording* recording) override { (void)recording; }
    void onActiveRecordingManagerRecordingEnded(uh::ActiveRecording* recording) override { (void)recording; }
    void onActiveRecordingManagerP1NameChanged(const QString& name) override { (void)name; }
    void onActiveRecordingManagerP2NameChanged(const QString& name) override { (void)name; }
    void onActiveRecordingManagerSetNumberChanged(uh::SetNumber number) override { (void)number; }
    void onActiveRecordingManagerGameNumberChanged(uh::GameNumber number) override { (void)number; }
    void onActiveRecordingManagerFormatChanged(const uh::SetFormat& format) override { (void)format; }
    void onActiveRecordingManagerPlayerStateAdded(int player, const uh::PlayerState& state) override { (void)player; (void)state; }
    void onActiveRecordingManagerWinnerChanged(int winner) { (void)winner; }

private:
    std::unique_ptr<Config> config_;
    std::unique_ptr<RecordingManager> recordingManager_;
    std::unique_ptr<ActiveRecordingManager> activeRecordingManager_;
    std::unique_ptr<TrainingMode> trainingMode_;
    std::unique_ptr<PluginManager> pluginManager_;
    CategoryView* categoryView_;
    RecordingGroupView* recordingGroupView_;
    ActiveRecordingView* activeRecordingView_;
    QStackedWidget* mainView_;
    Ui::MainWindow* ui_;
};

}
