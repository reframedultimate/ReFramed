#pragma once

#include "application/listeners/ReplayManagerListener.hpp"
#include "application/models/CategoryType.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include <QVector>
#include <QString>

namespace rfapp {

class CategoryListener;
class ReplayGroup;
class ReplayManager;

class CategoryModel
{
public:
    // Functions to select the top level categories
    void selectDataSetsCategory();
    void selectAnalysisCategory();
    void selectReplayGroupsCategory();
    void selectReplaySourcesCategory();
    void selectVideoSourcesCategory();
    void selectSessionCategory();

    CategoryType currentCategorySelected() const
        { return currentCategory_; }

    // Functions to select items of each category (implicitly also selects
    // the top level category if it's not really selected)
    void selectReplayGroup(const QString& name);
    void selectReplaySource(const QString& name);
    void selectVideoSource(const QString& name);
    void selectTrainingModePlugin(const QString& name);

    const QString& currentItemSelected() const
        { return currentItemName_; }

    rfcommon::ListenerDispatcher<CategoryListener> dispatcher;

private:
    ReplayManager* savedGameSessionManager_;
    CategoryType currentCategory_ = CategoryType::TOP_LEVEL_REPLAY_GROUPS;
    QString currentItemName_ = "";
};

}
