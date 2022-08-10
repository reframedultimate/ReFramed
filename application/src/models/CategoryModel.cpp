#include "application/listeners/CategoryListener.hpp"
#include "application/models/CategoryModel.hpp"
#include <QString>

namespace rfapp {

// ----------------------------------------------------------------------------
void CategoryModel::selectDataSetsCategory()
{
    currentCategory_ = CategoryType::TOP_LEVEL_DATA_SETS;
    currentItemName_ = "";
    dispatcher.dispatch(&CategoryListener::onCategorySelected, currentCategory_);
}

// ----------------------------------------------------------------------------
void CategoryModel::selectAnalysisCategory()
{
    currentCategory_ = CategoryType::TOP_LEVEL_ANALYSIS;
    currentItemName_ = "";
    dispatcher.dispatch(&CategoryListener::onCategorySelected, currentCategory_);
}

// ----------------------------------------------------------------------------
void CategoryModel::selectReplayGroupsCategory()
{
    currentCategory_ = CategoryType::TOP_LEVEL_REPLAY_GROUPS;
    currentItemName_ = "";
    dispatcher.dispatch(&CategoryListener::onCategorySelected, currentCategory_);
}

// ----------------------------------------------------------------------------
void CategoryModel::selectReplaySourcesCategory()
{
    currentCategory_ = CategoryType::TOP_LEVEL_REPLAY_SOURCES;
    currentItemName_ = "";
    dispatcher.dispatch(&CategoryListener::onCategorySelected, currentCategory_);
}

// ----------------------------------------------------------------------------
void CategoryModel::selectVideoSourcesCategory()
{
    currentCategory_ = CategoryType::TOP_LEVEL_VIDEO_SOURCES;
    currentItemName_ = "";
    dispatcher.dispatch(&CategoryListener::onCategorySelected, currentCategory_);
}

// ----------------------------------------------------------------------------
void CategoryModel::selectSessionCategory()
{
    currentCategory_ = CategoryType::TOP_LEVEL_SESSION;
    currentItemName_ = "";
    dispatcher.dispatch(&CategoryListener::onCategorySelected, currentCategory_);
}

// ----------------------------------------------------------------------------
void CategoryModel::selectReplayGroup(const QString& name)
{
    if (currentCategory_ != CategoryType::TOP_LEVEL_REPLAY_GROUPS)
        selectReplayGroupsCategory();

    if (currentItemName_ != name)
    {
        currentItemName_ = name;
        dispatcher.dispatch(&CategoryListener::onCategoryItemSelected, currentCategory_, currentItemName_);
    }
}

// ----------------------------------------------------------------------------
void CategoryModel::selectReplaySource(const QString& name)
{
    if (currentCategory_ != CategoryType::TOP_LEVEL_REPLAY_SOURCES)
        selectReplayGroupsCategory();

    if (currentItemName_ != name)
    {
        currentItemName_ = name;
        dispatcher.dispatch(&CategoryListener::onCategoryItemSelected, currentCategory_, currentItemName_);
    }
}

// ----------------------------------------------------------------------------
void CategoryModel::selectVideoSource(const QString& name)
{
    if (currentCategory_ != CategoryType::TOP_LEVEL_VIDEO_SOURCES)
        selectReplayGroupsCategory();

    if (currentItemName_ != name)
    {
        currentItemName_ = name;
        dispatcher.dispatch(&CategoryListener::onCategoryItemSelected, currentCategory_, currentItemName_);
    }
}

}
