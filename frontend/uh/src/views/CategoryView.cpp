#include "uh/views/CategoryView.hpp"

namespace uh {

// ----------------------------------------------------------------------------
CategoryView::CategoryView(QWidget* parent)
    : QTreeWidget(parent)
{
    setHeaderHidden(true);

    analysisCategoryItem_ = new QTreeWidgetItem({"Analysis"}, static_cast<int>(CategoryType::TOP_LEVEL));
    recordingGroupsCategoryItem_ = new QTreeWidgetItem({"Recording Groups"}, static_cast<int>(CategoryType::TOP_LEVEL));
    recordingSourcesCategoryItem_ = new QTreeWidgetItem({"Recording Sources"}, static_cast<int>(CategoryType::TOP_LEVEL));
    videoSourcesCategoryItem_ = new QTreeWidgetItem({"Video Replay Sources"}, static_cast<int>(CategoryType::TOP_LEVEL));
    activeRecordingCategoryItem_ = new QTreeWidgetItem({"Active Recording"}, static_cast<int>(CategoryType::TOP_LEVEL));

    addTopLevelItem(analysisCategoryItem_);
    addTopLevelItem(recordingGroupsCategoryItem_);
    addTopLevelItem(recordingSourcesCategoryItem_);
    addTopLevelItem(videoSourcesCategoryItem_);
    addTopLevelItem(activeRecordingCategoryItem_);
    //activeRecordingCategoryItem_->setDisabled(true);

    connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            this, SLOT(onTreeWidgetCategoriesCurrentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
}

// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
void CategoryView::onTreeWidgetCategoriesCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
    if (current == analysisCategoryItem_)
        emit categoryChanged(CategoryType::ANALYSIS);
    else if (current == recordingGroupsCategoryItem_)
        emit categoryChanged(CategoryType::RECORDING_GROUPS);
    else if (current == recordingSourcesCategoryItem_)
        emit categoryChanged(CategoryType::RECORDING_SOURCES);
    else if (current == videoSourcesCategoryItem_)
        emit categoryChanged(CategoryType::VIDEO_SOURCES);
    else if (current == activeRecordingCategoryItem_)
        emit categoryChanged(CategoryType::ACTIVE_RECORDING);
}

}
