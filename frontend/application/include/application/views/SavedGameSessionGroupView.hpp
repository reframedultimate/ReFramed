#pragma once

#include "application/listeners/SavedGameSessionManagerListener.hpp"
#include "application/listeners/SavedGameSessionGroupListener.hpp"
#include <QWidget>

class QListWidgetItem;
class QStringListModel;

namespace Ui {
    class SavedGameSessionGroupView;
}

namespace uhapp {

class SavedGameSessionManager;
class SavedGameSessionGroup;
class SavedGameSessionNameCompleter;
class SavedGameSessionListWidget;
class SessionView;

class SavedGameSessionGroupView : public QWidget
                                , public SavedGameSessionManagerListener
                                , public SavedGameSessionGroupListener
{
    Q_OBJECT

public:
    explicit SavedGameSessionGroupView(SavedGameSessionManager* manager, QWidget* parent=nullptr);
    ~SavedGameSessionGroupView();

public slots:
    /*!
     * \brief Updates the view with data from the specified group. If the group
     * changes (files added/removed) the view will automatically update. If
     * the group is deleted the view will clear itself.
     */
    void setSavedGameSessionGroup(SavedGameSessionGroup* group);
    void clear();

private slots:
    void onCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* previous);
    void onFiltersTextChanged(const QString& text);
    void onDeleteKeyPressed();

private:
    // In case a group gets deleted, we need remove ourselves as a listener
    void onSavedGameSessionManagerGroupRemoved(SavedGameSessionGroup* group) override;
    void onSavedGameSessionManagerGroupNameChanged(SavedGameSessionGroup* group, const QString& oldName, const QString& newName) override;

    void onSavedGameSessionManagerGroupAdded(SavedGameSessionGroup* group) override { (void)group; }
    void onSavedGameSessionManagerDefaultGameSessionSaveLocationChanged(const QDir& path) override { (void)path; }
    void onSavedGameSessionManagerGameSessionSourceAdded(const QString& name, const QDir& path) override { (void)name; (void)path; }
    void onSavedGameSessionManagerGameSessionSourceNameChanged(const QString& oldName, const QString& newName) override { (void)oldName; (void)newName; }
    void onSavedGameSessionManagerGameSessionSourceRemoved(const QString& name) override { (void)name; }
    void onSavedGameSessionManagerVideoSourceAdded(const QString& name, const QDir& path) override { (void)name; (void)path; }
    void onSavedGameSessionManagerVideoSourceNameChanged(const QString& oldName, const QString& newName) override { (void)oldName; (void)newName; }
    void onSavedGameSessionManagerVideoSourceRemoved(const QString& name) override { (void)name; }

private:
    void onSavedGameSessionGroupFileAdded(SavedGameSessionGroup* group, const QFileInfo& absPathToFile) override;
    void onSavedGameSessionGroupFileRemoved(SavedGameSessionGroup* group, const QFileInfo& absPathToFile) override;

private:
    Ui::SavedGameSessionGroupView* ui_;
    SavedGameSessionManager* savedGameSessionManager_;
    SavedGameSessionGroup* currentGroup_ = nullptr;
    SavedGameSessionListWidget* savedGameSessionListWidget_;
    SavedGameSessionNameCompleter* filterCompleter_;
    SessionView* sessionView_;
};

}
