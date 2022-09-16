#pragma once

#include "application/listeners/ReplayManagerListener.hpp"
#include "application/listeners/ReplayGroupListener.hpp"
#include "rfcommon/Reference.hpp"
#include "rfcommon/Vector.hpp"
#include <QWidget>

class QListWidgetItem;
class QStringListModel;

namespace Ui {
    class ReplayGroupView;
}

namespace rfcommon {
    class Hash40Strings;
    class Session;
}

namespace rfapp {

class PluginDockView;
class PluginManager;
class ReplayManager;
class ReplayGroup;
class ReplayNameCompleter;
class ReplayListWidget;
class UserMotionLabelsManager;

class ReplayGroupView
    : public QWidget
    , public ReplayManagerListener
    , public ReplayGroupListener
{
    Q_OBJECT

public:
    explicit ReplayGroupView(
            ReplayManager* manager,
            PluginManager* pluginManager,
            UserMotionLabelsManager* userMotionLabelsManager,
            rfcommon::Hash40Strings* hash40Strings,
            QWidget* parent=nullptr);
    ~ReplayGroupView();

public slots:
    /*!
     * \brief Updates the view with data from the specified group. If the group
     * changes (files added/removed) the view will automatically update. If
     * the group is deleted the view will clear itself.
     */
    void setReplayGroup(ReplayGroup* group);
    void clearReplayGroup(ReplayGroup* group);

private slots:
    void onItemRightClicked(const QPoint& pos);
    void onItemSelectionChanged();
    void onFiltersTextChanged(const QString& text);
    void onDeleteKeyPressed();

private:
    // In case a group gets deleted, we need to remove ourselves as a listener
    void onReplayManagerGroupRemoved(ReplayGroup* group) override;
    void onReplayManagerGroupNameChanged(ReplayGroup* group, const QString& oldName, const QString& newName) override;

    void onReplayManagerGroupAdded(ReplayGroup* group) override { (void)group; }
    void onReplayManagerDefaultGamePathChanged(const QDir& path) override { (void)path; }
    void onReplayManagerGamePathAdded(const QDir& path) override { (void)path; }
    void onReplayManagerGamePathRemoved(const QDir& path) override { (void)path; }
    void onReplayManagerVideoPathAdded(const QDir& path) override { (void)path; }
    void onReplayManagerVideoPathRemoved(const QDir& path) override { (void)path; }

private:
    void onReplayGroupFileAdded(ReplayGroup* group, const QString& fileName) override;
    void onReplayGroupFileRemoved(ReplayGroup* group, const QString& fileName) override;

private:
    Ui::ReplayGroupView* ui_;
    PluginManager* pluginManager_;
    ReplayManager* replayManager_;
    UserMotionLabelsManager* userMotionLabelsManager_;
    rfcommon::Hash40Strings* hash40Strings_;
    ReplayGroup* currentGroup_ = nullptr;
    ReplayListWidget* replayListWidget_;
    ReplayNameCompleter* filterCompleter_;
    PluginDockView* pluginDockView_;
};

}
