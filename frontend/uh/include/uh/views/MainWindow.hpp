#pragma once

#include "uh/models/CategoryType.hpp"
#include <QMainWindow>

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
class Recording;
class Settings;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent=nullptr);
    ~MainWindow();

private slots:
    void onCategoryChanged(CategoryType category);

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
    Ui::MainWindow* ui_;
    CategoryView* categoryView_;
    QStackedWidget* mainView_;
    ActiveRecordingManager* activeRecordingManager_;
    QScopedPointer<Settings> settings_;
    QScopedPointer<Recording> previousRecording_;
};

}
