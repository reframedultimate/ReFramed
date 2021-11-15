#pragma once

#include <QString>
#include <QFileInfo>

namespace uhapp {

class SavedGameSessionGroup;

class SavedGameSessionGroupListener
{
public:
    virtual void onSavedGameSessionGroupFileAdded(SavedGameSessionGroup* group, const QFileInfo& absPathToFile) = 0;
    virtual void onSavedGameSessionGroupFileRemoved(SavedGameSessionGroup* group, const QFileInfo& absPathToFile) = 0;
};

}
