#pragma once

#include "application/listeners/CategoryListener.hpp"
#include "application/listeners/ActiveGameSessionManagerListener.hpp"
#include "application/listeners/ReplayManagerListener.hpp"
#include "application/listeners/TrainingModeListener.hpp"
#include "application/Util.hpp"
#include <QHash>
#include <QTreeWidget>

namespace rfapp {

class CategoryModel;
class ReplayManager;
class ActiveGameSessionManager;
class TrainingModeModel;

class CategoryView : public QTreeWidget
                   , public CategoryListener
                   , public ReplayManagerListener
                   , public ActiveGameSessionManagerListener
                   , public TrainingModeListener
{
    Q_OBJECT
public:
    explicit CategoryView(
            CategoryModel* categoryModel,
            ReplayManager* replayManager,
            ActiveGameSessionManager* activeGameSessionManager,
            TrainingModeModel* trainingModeModel,
            QWidget* parent=nullptr
        );
    ~CategoryView();

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private slots:
    void onCustomContextMenuRequested(const QPoint& pos);
    void onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
    void onItemChanged(QTreeWidgetItem* item, int column);

private:
    void onReplayManagerDefaultReplaySaveLocationChanged(const QDir& path) override;

    void onReplayManagerGroupAdded(ReplayGroup* group) override;
    void onReplayManagerGroupNameChanged(ReplayGroup* group, const QString& oldName, const QString& newName) override;
    void onReplayManagerGroupRemoved(ReplayGroup* group) override;

    void onReplayManagerReplaySourceAdded(const QString& name, const QDir& path) override;
    void onReplayManagerReplaySourceNameChanged(const QString& oldName, const QString& newName) override;
    void onReplayManagerReplaySourcePathChanged(const QString& name, const QDir& oldPath, const QDir& newPath) override;
    void onReplayManagerReplaySourceRemoved(const QString& name) override;

    void onReplayManagerVideoSourceAdded(const QString& name, const QDir& path) override;
    void onReplayManagerVideoSourceNameChanged(const QString& oldName, const QString& newName) override;
    void onReplayManagerVideoSourcePathChanged(const QString& name, const QDir& oldPath, const QDir& newPath) override;
    void onReplayManagerVideoSourceRemoved(const QString& name) override;

private:
    void onActiveGameSessionManagerAttemptConnectToServer(const char* ipAddress, uint16_t port) override { (void)ipAddress; (void)port; }
    void onActiveGameSessionManagerFailedToConnectToServer(const char* ipAddress, uint16_t port) override { (void)ipAddress; (void)port; };
    void onActiveGameSessionManagerConnectedToServer(const char* ipAddress, uint16_t port) override;
    void onActiveGameSessionManagerDisconnectedFromServer() override;

    void onActiveGameSessionManagerMatchStarted(rfcommon::RunningGameSession* session) override;
    void onActiveGameSessionManagerMatchEnded(rfcommon::RunningGameSession* session) override;

    void onActiveGameSessionManagerNewFrame(int frameIdx, const rfcommon::Frame& frame) override {}
    void onRunningGameSessionManagerPlayerNameChanged(int player, const rfcommon::SmallString<15>& name) override { (void)name; }
    void onActiveGameSessionManagerSetNumberChanged(rfcommon::SetNumber number) override { (void)number; }
    void onActiveGameSessionManagerGameNumberChanged(rfcommon::GameNumber number) override { (void)number; }
    void onRunningGameSessionManagerFormatChanged(const rfcommon::SetFormat& format) override { (void)format; }
    void onActiveGameSessionManagerWinnerChanged(int winner) override { (void)winner; }

private:
    void onTrainingModePluginLaunched(const QString& name, rfcommon::Plugin* plugin) override;
    void onTrainingModePluginStopped(const QString& name, rfcommon::Plugin* plugin) override;

private:
    void onCategorySelected(CategoryType category) override;
    void onCategoryItemSelected(CategoryType category, const QString& name) override;

private:
    CategoryModel* categoryModel_;
    ReplayManager* replayManager_;
    ActiveGameSessionManager* activeGameSessionManager_;
    TrainingModeModel* trainingModeModel_;
    QTreeWidgetItem* dataSetsItem_;
    QTreeWidgetItem* analysisCategoryItem_;
    QTreeWidgetItem* replayGroupsItem_;
    QTreeWidgetItem* replaySourcesItem_;
    QTreeWidgetItem* videoSourcesItem_;
    QTreeWidgetItem* sessionItem_;
    QTreeWidgetItem* trainingModeItem_;

    QHash<QTreeWidgetItem*, QString> oldGroupNames_;
};

}
