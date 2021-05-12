#pragma once

#include "application/listeners/RecordingManagerListener.hpp"
#include "application/listeners/RecordingGroupListener.hpp"
#include <QWidget>

class QListWidgetItem;
class QStringListModel;

namespace Ui {
    class RecordingGroupView;
}

namespace uhapp {

class RecordingManager;
class RecordingGroup;
class RecordingView;
class RecordingNameCompleter;
class RecordingListWidget;

class RecordingGroupView : public QWidget
                         , public RecordingManagerListener
                         , public RecordingGroupListener
{
    Q_OBJECT

public:
    explicit RecordingGroupView(RecordingManager* recordingManager, QWidget* parent=nullptr);
    ~RecordingGroupView();

public slots:
    /*!
     * \brief Updates the view with data from the specified group. If the group
     * changes (files added/removed) the view will automatically update. If
     * the group is deleted the view will clear itself.
     */
    void setRecordingGroup(RecordingGroup* group);
    void clear();

private slots:
    void onCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* previous);
    void onFiltersTextChanged(const QString& text);

private:
    // In case a group gets deleted, we need remove ourselves as a listener
    void onRecordingManagerGroupRemoved(RecordingGroup* group) override;
    void onRecordingManagerGroupNameChanged(RecordingGroup* group, const QString& oldName, const QString& newName) override;

    void onRecordingManagerGroupAdded(RecordingGroup* group) override { (void)group; }
    void onRecordingManagerDefaultRecordingLocationChanged(const QDir& path) override { (void)path; }
    void onRecordingManagerRecordingSourceAdded(const QString& name, const QDir& path) override { (void)name; (void)path; }
    void onRecordingManagerRecordingSourceNameChanged(const QString& oldName, const QString& newName) override { (void)oldName; (void)newName; }
    void onRecordingManagerRecordingSourceRemoved(const QString& name) override { (void)name; }
    void onRecordingManagerVideoSourceAdded(const QString& name, const QDir& path) override { (void)name; (void)path; }
    void onRecordingManagerVideoSourceNameChanged(const QString& oldName, const QString& newName) override { (void)oldName; (void)newName; }
    void onRecordingManagerVideoSourceRemoved(const QString& name) override { (void)name; }

private:
    void onRecordingGroupFileAdded(RecordingGroup* group, const QFileInfo& absPathToFile) override;
    void onRecordingGroupFileRemoved(RecordingGroup* group, const QFileInfo& absPathToFile) override;

private:
    Ui::RecordingGroupView* ui_;
    RecordingManager* recordingManager_;
    RecordingGroup* currentGroup_ = nullptr;
    RecordingListWidget* recordingListWidget_;
    RecordingView* recordingView_;
    RecordingNameCompleter* filterCompleter_;
};

}
