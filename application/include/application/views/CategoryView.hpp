#pragma once

#include "application/listeners/CategoryListener.hpp"
#include "application/listeners/ReplayManagerListener.hpp"
#include "application/Util.hpp"
#include <QHash>
#include <QTreeWidget>

namespace rfapp {

class CategoryModel;
class ReplayManager;

class CategoryView 
        : public QTreeWidget
        , public CategoryListener
        , public ReplayManagerListener
{
    Q_OBJECT
public:
    explicit CategoryView(
            CategoryModel* categoryModel,
            ReplayManager* replayManager,
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
    void onCategorySelected(CategoryType category) override;
    void onCategoryItemSelected(CategoryType category, const QString& name) override;

private:
    CategoryModel* categoryModel_;
    QTreeWidgetItem* sessionItem_;
    ReplayManager* replayManager_;
    QTreeWidgetItem* replayGroupsItem_;

    QHash<QTreeWidgetItem*, QString> oldGroupNames_;
};

}
