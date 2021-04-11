#pragma once

#include "uh/listeners/RecordingManagerListener.hpp"
#include "uh/models/CategoryType.hpp"
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
     * \brief When the user clicks on a top-level item. The main window will
     * want to switch the current widget in response to a category change.
     */
    void categoryChanged(CategoryType category);

    /*!
     * \brief When the user clicks on a recording group entry
     */
    void recordingGroupSelected(RecordingGroup* group);

private slots:
    void onTreeWidgetCategoriesCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);

private:
    void onRecordingManagerDefaultRecordingLocationChanged(const QDir& path) override;
    void onRecordingManagerGroupAdded(RecordingGroup* group) override;
    void onRecordingManagerGroupRemoved(RecordingGroup* group) override;

private:
    RecordingManager* recordingManager_;
    QTreeWidgetItem* analysisCategoryItem_;
    QTreeWidgetItem* recordingGroupsCategoryItem_;
    QTreeWidgetItem* recordingSourcesCategoryItem_;
    QTreeWidgetItem* videoSourcesCategoryItem_;
    QTreeWidgetItem* activeRecordingCategoryItem_;
};

}
