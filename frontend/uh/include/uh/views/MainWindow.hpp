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
class RecordingManager;
class Settings;

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
    void onActiveRecordingManagerRecordingSaved(const QString& fileName) override;

private:
    QScopedPointer<Settings> settings_;
    QScopedPointer<ActiveRecordingManager> activeRecordingManager_;
    QScopedPointer<RecordingManager> recordingManager_;
    CategoryView* categoryView_;
    QStackedWidget* mainView_;
    Ui::MainWindow* ui_;
};

}
