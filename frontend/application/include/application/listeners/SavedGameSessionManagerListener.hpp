#pragma once

#include <QString>
#include <QDir>

namespace uhapp {

class SavedGameSessionGroup;
class SavedGameSessionManagerListener
{
public:
    virtual void onSavedGameSessionManagerDefaultGameSessionSaveLocationChanged(const QDir& path) = 0;

    virtual void onSavedGameSessionManagerGroupAdded(SavedGameSessionGroup* group) = 0;
    virtual void onSavedGameSessionManagerGroupNameChanged(SavedGameSessionGroup* group, const QString& oldName, const QString& newName) = 0;
    virtual void onSavedGameSessionManagerGroupRemoved(SavedGameSessionGroup* group) = 0;

    virtual void onSavedGameSessionManagerGameSessionSourceAdded(const QString& name, const QDir& path) = 0;
    virtual void onSavedGameSessionManagerGameSessionSourceNameChanged(const QString& oldName, const QString& newName) = 0;
    virtual void onSavedGameSessionManagerGameSessionSourceRemoved(const QString& name) = 0;

    virtual void onSavedGameSessionManagerVideoSourceAdded(const QString& name, const QDir& path) = 0;
    virtual void onSavedGameSessionManagerVideoSourceNameChanged(const QString& oldName, const QString& newName) = 0;
    virtual void onSavedGameSessionManagerVideoSourceRemoved(const QString& name) = 0;
};

}
