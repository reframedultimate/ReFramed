#pragma once

#include "application/listeners/ReplayManagerListener.hpp"
#include "application/listeners/ReplayGroupListener.hpp"
#include "rfcommon/Reference.hpp"
#include <QWidget>

class QListWidgetItem;
class QStringListModel;

namespace Ui {
    class ReplayGroupView;
}

namespace rfcommon {
    class SavedSession;
}

namespace rfapp {

class PluginManager;
class ReplayManager;
class ReplayGroup;
class SavedGameSessionNameCompleter;
class SavedGameSessionListWidget;
class SessionView;

class ReplayGroupView : public QWidget
                      , public ReplayManagerListener
                      , public ReplayGroupListener
{
    Q_OBJECT

public:
    explicit ReplayGroupView(
            ReplayManager* manager,
            PluginManager* pluginManager,
            QWidget* parent=nullptr);
    ~ReplayGroupView();

public slots:
    /*!
     * \brief Updates the view with data from the specified group. If the group
     * changes (files added/removed) the view will automatically update. If
     * the group is deleted the view will clear itself.
     */
    void setSavedGameSessionGroup(ReplayGroup* group);
    void clearSavedGameSessionGroup(ReplayGroup* group);

private slots:
    void onCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* previous);
    void onFiltersTextChanged(const QString& text);
    void onDeleteKeyPressed();

private:
    // In case a group gets deleted, we need remove ourselves as a listener
    void onReplayManagerGroupRemoved(ReplayGroup* group) override;
    void onReplayManagerGroupNameChanged(ReplayGroup* group, const QString& oldName, const QString& newName) override;

    void onReplayManagerGroupAdded(ReplayGroup* group) override { (void)group; }
    void onReplayManagerDefaultReplaySaveLocationChanged(const QDir& path) override { (void)path; }
    void onReplayManagerReplaySourceAdded(const QString& name, const QDir& path) override { (void)name; (void)path; }
    void onReplayManagerReplaySourceNameChanged(const QString& oldName, const QString& newName) override { (void)oldName; (void)newName; }
    void onReplayManagerReplaySourcePathChanged(const QString& name, const QDir& oldPath, const QDir& newPath) override { (void)name; (void)oldPath; (void)newPath; }
    void onReplayManagerReplaySourceRemoved(const QString& name) override { (void)name; }
    void onReplayManagerVideoSourceAdded(const QString& name, const QDir& path) override { (void)name; (void)path; }
    void onReplayManagerVideoSourceNameChanged(const QString& oldName, const QString& newName) override { (void)oldName; (void)newName; }
    void onReplayManagerVideoSourcePathChanged(const QString& name, const QDir& oldPath, const QDir& newPath) override { (void)name; (void)oldPath; (void)newPath; }
    void onReplayManagerVideoSourceRemoved(const QString& name) override { (void)name; }

private:
    void onReplayGroupFileAdded(ReplayGroup* group, const QFileInfo& absPathToFile) override;
    void onReplayGroupFileRemoved(ReplayGroup* group, const QFileInfo& absPathToFile) override;

private:
    Ui::ReplayGroupView* ui_;
    ReplayManager* replayManager_;
    ReplayGroup* currentGroup_ = nullptr;
    rfcommon::Reference<rfcommon::SavedSession> currentSession_;
    SavedGameSessionListWidget* savedGameSessionListWidget_;
    SavedGameSessionNameCompleter* filterCompleter_;
    SessionView* sessionView_;
};

}
