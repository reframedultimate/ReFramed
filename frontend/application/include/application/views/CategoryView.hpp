#pragma once

#include "application/listeners/RecordingManagerListener.hpp"
#include "application/models/CategoryType.hpp"
#include <QTreeWidget>

namespace uh {

class RecordingManager;

class CategoryView : public QTreeWidget
                   , public RecordingManagerListener
{
    Q_OBJECT
public:
    explicit CategoryView(RecordingManager* recordingManager, QWidget* parent=nullptr);

    void setActiveRecordingViewDisabled(bool enable);

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

private slots:
    void onCustomContextMenuRequested(const QPoint& pos);
    void onTreeWidgetCategoriesCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);

private:
    CategoryType categoryOf(const QTreeWidgetItem* item) const;

private:
    void onRecordingManagerDefaultRecordingLocationChanged(const QDir& path) override;

    void onRecordingManagerGroupAdded(RecordingGroup* group) override;
    void onRecordingManagerGroupRemoved(RecordingGroup* group) override;

    void onRecordingManagerRecordingSourceAdded(const QString& name, const QDir& path) override;
    void onRecordingManagerRecordingSourceNameChanged(const QString& oldName, const QString& newName) override;
    void onRecordingManagerRecordingSourceRemoved(const QString& name) override;

    void onRecordingManagerVideoSourceAdded(const QString& name, const QDir& path) override;
    void onRecordingManagerVideoSourceNameChanged(const QString& oldName, const QString& newName) override;
    void onRecordingManagerVideoSourceRemoved(const QString& name) override;

private:
    RecordingManager* recordingManager_;
    QTreeWidgetItem* analysisCategoryItem_;
    QTreeWidgetItem* recordingGroupsItem_;
    QTreeWidgetItem* recordingSourcesItem_;
    QTreeWidgetItem* videoSourcesItem_;
    QTreeWidgetItem* activeRecordingItem_;
};

}
