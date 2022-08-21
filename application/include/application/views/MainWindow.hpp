#pragma once

#include "application/models/CategoryType.hpp"
#include "application/listeners/CategoryListener.hpp"
#include "rfcommon/ProtocolListener.hpp"
#include "rfcommon/Reference.hpp"
#include <QMainWindow>
#include <QDir>
#include <memory>

class QStackedWidget;
class QTreeWidgetItem;

namespace Ui {
    class MainWindow;
}

namespace rfcommon {
    class Hash40Strings;
}

namespace rfapp {

class CategoryView;
class CategoryModel;
class Config;
class PluginManager;
class Protocol;
class ActiveSessionManager;
class ActiveSessionView;
class ReplayGroupView;
class ReplayManager;
class UserMotionLabelsManager;
class UserMotionLabelsEditor;

class MainWindow 
        : public QMainWindow
        , public CategoryListener
        , public rfcommon::ProtocolListener
{
    Q_OBJECT

public:
    explicit MainWindow(rfcommon::Hash40Strings* hash40Strings, QWidget* parent=nullptr);
    ~MainWindow();

private slots:
    void negotiateDefaultRecordingLocation();
    void populateCategories();

    void onConnectActionTriggered();
    void onDisconnectActionTriggered();
    void onUserLabelsEditorActionTriggered();

    void onAboutActionTriggered();
    void onViewLogActionTriggered();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    // Category callbacks
    void onCategorySelected(CategoryType category) override;
    void onCategoryItemSelected(CategoryType category, const QString& name) override { (void)category; (void)name; }

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
    std::unique_ptr<Protocol> protocol_;
    std::unique_ptr<UserMotionLabelsManager> userMotionLabelsManager_;
    std::unique_ptr<PluginManager> pluginManager_;
    std::unique_ptr<ReplayManager> replayManager_;
    std::unique_ptr<ActiveSessionManager> activeSessionManager_;
    std::unique_ptr<CategoryModel> categoryModel_;
    CategoryView* categoryView_;
    ReplayGroupView* replayGroupView_;
    ActiveSessionView* activeSessionView_;
    QStackedWidget* mainView_;
    Ui::MainWindow* ui_;

    // Non-modal views that appear in a separate window
    UserMotionLabelsEditor* userMotionLabelsEditor_ = nullptr;
};

}
