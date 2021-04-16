#pragma once

#include "uh/models/CategoryType.hpp"
#include "uh/listeners/ActiveRecordingManagerListener.hpp"
#include <QMainWindow>
#include <QDir>

class QStackedWidget;
class QTreeWidgetItem;

namespace Ui {
    class MainWindow;
}

namespace uh {

class ActiveRecordingManager;
class CategoryView;
class ConnectView;
class Protocol;
class RecordingGroupView;
class RecordingManager;
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
    void onActiveRecordingManagerRecordingStarted(ActiveRecording* recording) override { (void)recording; }
    void onActiveRecordingManagerRecordingEnded(ActiveRecording* recording) override { (void)recording; }
    void onActiveRecordingManagerP1NameChanged(const QString& name) override { (void)name; }
    void onActiveRecordingManagerP2NameChanged(const QString& name) override { (void)name; }
    void onActiveRecordingManagerSetNumberChanged(int number) override { (void)number; }
    void onActiveRecordingManagerGameNumberChanged(int number) override { (void)number; }
    void onActiveRecordingManagerFormatChanged(const SetFormat& format) override { (void)format; }
    void onActiveRecordingManagerPlayerStateAdded(int player, const PlayerState& state) override { (void)player; (void)state; }
    void onActiveRecordingManagerWinnerChanged(int winner) { (void)winner; }

private:
    QScopedPointer<Config> config_;
    QScopedPointer<RecordingManager> recordingManager_;
    QScopedPointer<ActiveRecordingManager> activeRecordingManager_;
    CategoryView* categoryView_;
    RecordingGroupView* recordingGroupView_;
    QStackedWidget* mainView_;
    Ui::MainWindow* ui_;
};

}
