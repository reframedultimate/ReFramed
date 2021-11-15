#pragma once

#include "application/listeners/SavedGameSessionManagerListener.hpp"
#include "application/models/CategoryType.hpp"
#include "application/Util.hpp"
#include <QHash>
#include <QTreeWidget>

namespace uhapp {

class SavedGameSessionManager;

class CategoryView : public QTreeWidget
                   , public SavedGameSessionManagerListener
{
    Q_OBJECT
public:
    explicit CategoryView(SavedGameSessionManager* recordingManager, QWidget* parent=nullptr);
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
     * \brief When the user clicks on a game group entry
     */
    void savedGameSessionGroupSelected(SavedGameSessionGroup* group);

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
    SavedGameSessionGroup* newGroup();
    SavedGameSessionGroup* duplicateGroup(SavedGameSessionGroup* otherGroup);
    void deleteGroup(SavedGameSessionGroup* group);

private:
    void onSavedGameSessionManagerDefaultGameSessionSaveLocationChanged(const QDir& path) override;

    void onSavedGameSessionManagerGroupAdded(SavedGameSessionGroup* group) override;
    void onSavedGameSessionManagerGroupNameChanged(SavedGameSessionGroup* group, const QString& oldName, const QString& newName) override;
    void onSavedGameSessionManagerGroupRemoved(SavedGameSessionGroup* group) override;

    void onSavedGameSessionManagerGameSessionSourceAdded(const QString& name, const QDir& path) override;
    void onSavedGameSessionManagerGameSessionSourceNameChanged(const QString& oldName, const QString& newName) override;
    void onSavedGameSessionManagerGameSessionSourceRemoved(const QString& name) override;

    void onSavedGameSessionManagerVideoSourceAdded(const QString& name, const QDir& path) override;
    void onSavedGameSessionManagerVideoSourceNameChanged(const QString& oldName, const QString& newName) override;
    void onSavedGameSessionManagerVideoSourceRemoved(const QString& name) override;

private:
    SavedGameSessionManager* savedGameSessionManager_;
    QTreeWidgetItem* dataSetsItem_;
    QTreeWidgetItem* analysisCategoryItem_;
    QTreeWidgetItem* savedGameSessionGroupsItem_;
    QTreeWidgetItem* recordingSourcesItem_;
    QTreeWidgetItem* videoSourcesItem_;
    QTreeWidgetItem* activeRecordingItem_;
    QTreeWidgetItem* trainingModeItem_;

    QHash<QTreeWidgetItem*, QString> oldGroupNames_;
};

}
