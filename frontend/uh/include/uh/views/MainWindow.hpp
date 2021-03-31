#pragma once

#include "uh/models/CategoryType.hpp"
#include "uh/listeners/ConnectedListener.hpp"
#include "uh/listeners/RecordingListener.hpp"
#include <QMainWindow>

class QStackedWidget;
class QTreeWidgetItem;

namespace Ui {
    class MainWindow;
}

namespace uh {

class ActiveRecordingView;
class CategoryView;
class Protocol;
class Recording;
class Settings;

class MainWindow : public QMainWindow
                 , public ConnectedListener
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent=nullptr);
    ~MainWindow();

private:
    void transferSocketOwnership(tcp_socket socket) override;

    // Changes the UI to reflect connected/disconnected state
    void setStateConnected();
    void setStateDisconnected();

private slots:
    void onConnectActionTriggered();
    void onDisconnectActionTriggered();
    void onServerConnectionLost();
    void onProtocolRecordingEnded(Recording* recording);
    void onCategoryChanged(CategoryType category);

private:
    Ui::MainWindow* ui_;
    CategoryView* categoryView_;
    QStackedWidget* mainView_;
    ActiveRecordingView* activeRecordingView_;
    QSharedDataPointer<Settings> settings_;
    QScopedPointer<Protocol> protocol_;
};

}
