#pragma once

#include "application/listeners/SavedGameSessionManagerListener.hpp"
#include "application/models/CategoryType.hpp"
#include "application/Util.hpp"
#include <QHash>
#include <QTreeWidget>

namespace uhapp {

class RecordingManager;

class CategoryView : public QTreeWidget
                   , public SavedGameSessionManagerListener
{
    Q_OBJECT
public:
    explicit CategoryView(RecordingManager* recordingManager, QWidget* parent=nullptr);
    ~CategoryView();

    void setRunningGameSessionViewDisabled(bool enable);

signals:
    /*!
     * \brief When the user clicks on a top-level item or one of their child
     * items. The main window will want to switch the current widget in response
     * to a category change.
     */
    void categoryChanged(CategoryType category);

    /*!
     * \brief When the user clicks on a recording group entry
     */
    void recordingGroupSelected(RecordingGroup* group);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private slots:
    void onCustomContextMenuRequested(const QPoint& pos);
    void onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
    void onItemChanged(QTreeWidgetItem* item, int column);

private:
    CategoryType categoryOf(const QTreeWidgetItem* item) const;

    // These get triggered by the context menu and drag/drop events
    RecordingGroup* newGroup();
    RecordingGroup* duplicateGroup(RecordingGroup* otherGroup);
    void deleteGroup(RecordingGroup* group);

private:
    void onRecordingManagerDefaultRecordingLocationChanged(const QDir& path) override;

    void onRecordingManagerGroupAdded(RecordingGroup* group) override;
    void onRecordingManagerGroupNameChanged(RecordingGroup* group, const QString& oldName, const QString& newName) override;
    void onRecordingManagerGroupRemoved(RecordingGroup* group) override;

    void onRecordingManagerRecordingSourceAdded(const QString& name, const QDir& path) override;
    void onRecordingManagerRecordingSourceNameChanged(const QString& oldName, const QString& newName) override;
    void onRecordingManagerRecordingSourceRemoved(const QString& name) override;

    void onRecordingManagerVideoSourceAdded(const QString& name, const QDir& path) override;
    void onRecordingManagerVideoSourceNameChanged(const QString& oldName, const QString& newName) override;
    void onRecordingManagerVideoSourceRemoved(const QString& name) override;

private:
    RecordingManager* recordingManager_;
    QTreeWidgetItem* dataSetsItem_;
    QTreeWidgetItem* analysisCategoryItem_;
    QTreeWidgetItem* recordingGroupsItem_;
    QTreeWidgetItem* recordingSourcesItem_;
    QTreeWidgetItem* videoSourcesItem_;
    QTreeWidgetItem* activeRecordingItem_;
    QTreeWidgetItem* trainingModeItem_;

    QHash<QTreeWidgetItem*, QString> oldGroupNames_;
};

}
