#include "rfcommon/Profiler.hpp"
#include "application/listeners/CategoryListener.hpp"
#include "application/models/CategoryModel.hpp"
#include <QString>

namespace rfapp {

// ----------------------------------------------------------------------------
void CategoryModel::selectSessionCategory()
{
    PROFILE(CategoryModel, selectSessionCategory);

    currentCategory_ = CategoryType::TOP_LEVEL_SESSION;
    currentItemName_ = "";
    dispatcher.dispatch(&CategoryListener::onCategorySelected, currentCategory_);
}

// ----------------------------------------------------------------------------
void CategoryModel::selectReplayGroupsCategory()
{
    PROFILE(CategoryModel, selectReplayGroupsCategory);

    currentCategory_ = CategoryType::TOP_LEVEL_REPLAY_GROUPS;
    currentItemName_ = "";
    dispatcher.dispatch(&CategoryListener::onCategorySelected, currentCategory_);
}

// ----------------------------------------------------------------------------
void CategoryModel::selectReplayGroup(const QString& name)
{
    PROFILE(CategoryModel, selectReplayGroup);

    if (currentCategory_ != CategoryType::TOP_LEVEL_REPLAY_GROUPS)
        selectReplayGroupsCategory();

    if (currentItemName_ != name)
    {
        currentItemName_ = name;
        dispatcher.dispatch(&CategoryListener::onCategoryItemSelected, currentCategory_, currentItemName_);
    }
}

}
