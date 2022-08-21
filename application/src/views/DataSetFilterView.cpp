#include "application/ui_DataSetFilterView.h"
#include "application/Util.hpp"
#include "application/views/DataSetFilterView.hpp"
#include "application/views/DataSetFilterWidget_Date.hpp"
#include "application/views/DataSetFilterWidget_Game.hpp"
#include "application/views/DataSetFilterWidget_Matchup.hpp"
#include "application/views/DataSetFilterWidget_Player.hpp"
#include "application/views/DataSetFilterWidget_PlayerCount.hpp"
#include "application/views/DataSetFilterWidget_Stage.hpp"
#include "application/models/ReplayManager.hpp"
#include "application/models/ReplayGroup.hpp"
#include "application/models/DataSetBackgroundLoader.hpp"
#include "rfcommon/DataSetFilterChain.hpp"
#include "rfcommon/DataSet.hpp"
#include "rfcommon/DataSetFilter.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Reference.hpp"

#include <QMenu>
#include <QListWidgetItem>

#include <unordered_set>

#define FILTER_LIST \
    X(Date) \
    X(Game) \
    X(Matchup) \
    X(Player) \
    X(PlayerCount) \
    X(Stage)

namespace rfapp {

// ----------------------------------------------------------------------------
DataSetFilterView::DataSetFilterView(ReplayManager* manager, QWidget* parent)
    : QWidget(parent)
    , ui_(new Ui::DataSetFilterView)
    , filterWidgetsLayout_(new QVBoxLayout)
    , savedGameSessionManager_(manager)
    , dataSetBackgroundLoader_(new DataSetBackgroundLoader(this))
    , dataSetFilterChain_(new rfcommon::DataSetFilterChain)
    /*, inputDataSetMerged_(new rfcommon::DataSet)
    , outputDataSet_(new rfcommon::DataSet)*/
{
    ui_->setupUi(this);

    /*
     * The QAction objects responsible for adding a new filters are created in
     * QtDesigner and are available through the ui. Create a menu and add each
     * action to the menu so the user can add modifiers.
     */
    QMenu* menu = new QMenu(this);
#define X(filter) menu->addAction(ui_->action##filter);
    FILTER_LIST
#undef X
    ui_->toolButton_addFilter->setMenu(menu);

    // Filter widgets should align to the top, not in the middle
    ui_->scrollAreaWidgetContents_filters->setLayout(filterWidgetsLayout_);
    filterWidgetsLayout_->setAlignment(Qt::AlignTop);

    // Set up progress bar and info text
    ui_->progressBar->setVisible(false);
    ui_->label_outputInfo->setText("");

    // Fill in input groups list
    for (int i = 0; i != savedGameSessionManager_->replayGroupsCount(); ++i)
        onReplayManagerGroupAdded(savedGameSessionManager_->replayGroup(i));

    connect(ui_->toolButton_addFilter, &QToolButton::triggered,
            this, &DataSetFilterView::onToolButtonAddFilterTriggered);
    connect(ui_->listWidget_inputGroups, &QListWidget::itemChanged,
            this, &DataSetFilterView::onInputGroupItemChanged);

    savedGameSessionManager_->dispatcher.addListener(this);
    dataSetBackgroundLoader_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
DataSetFilterView::~DataSetFilterView()
{
    // Have to do this so we unregister as listeners to every recording group
    for (int i = 0; i != savedGameSessionManager_->replayGroupsCount(); ++i)
        onReplayManagerGroupRemoved(savedGameSessionManager_->replayGroup(i));

    dataSetBackgroundLoader_->dispatcher.removeListener(this);
    savedGameSessionManager_->dispatcher.removeListener(this);
    delete ui_;
}

// ----------------------------------------------------------------------------
bool DataSetFilterView::eventFilter(QObject* target, QEvent* e)
{
    PROFILE(DataSetFilterView, eventFilter);

    // Stops the scroll wheel from interfering with the elements in each
    // modifier widget
    if(e->type() == QEvent::Wheel)
        return true;
    return false;
}


// ----------------------------------------------------------------------------
void DataSetFilterView::onToolButtonAddFilterTriggered(QAction* action)
{
    PROFILE(DataSetFilterView, onToolButtonAddFilterTriggered);

    if (0) {}
#define X(filter) \
    else if (action == ui_->action##filter) \
        addNewFilterWidget(new DataSetFilterWidget_##filter);
    FILTER_LIST
#undef X

    dirtyDataSetFilters();
}

// ----------------------------------------------------------------------------
void DataSetFilterView::onInputGroupItemChanged(QListWidgetItem* item)
{
    PROFILE(DataSetFilterView, onInputGroupItemChanged);

    ReplayGroup* group = savedGameSessionManager_->replayGroup(item->text());
    if (group == nullptr)
        return;

    if (item->checkState() == Qt::Checked)
    {
        dataSetBackgroundLoader_->loadGroup(group);
        addGroupToInputRecordingsList(group);
    }
    else
    {
        dataSetBackgroundLoader_->cancelGroup(group);
        removeGroupFromInputDataSet(group);
        removeGroupFromInputRecordingsList(group);
    }
}

// ----------------------------------------------------------------------------
void DataSetFilterView::onFilterEnabled(DataSetFilterWidget* widget, bool enable)
{
    PROFILE(DataSetFilterView, onFilterEnabled);

    widget->filter()->setEnabled(enable);
    widget->contentWidget()->setEnabled(enable);
}

// ----------------------------------------------------------------------------
void DataSetFilterView::onFilterInverted(DataSetFilterWidget* widget, bool inverted)
{
    PROFILE(DataSetFilterView, onFilterInverted);

    widget->filter()->setInverted(inverted);
}

// ----------------------------------------------------------------------------
void DataSetFilterView::onFilterMoveUp(DataSetFilterWidget* widget)
{
    PROFILE(DataSetFilterView, onFilterMoveUp);

    int newIndex = dataSetFilterChain_->moveEarlier(widget->filter());
    moveFilterWidgetInLayout(widget, newIndex);

    dirtyDataSetFilters();
}

// ----------------------------------------------------------------------------
void DataSetFilterView::onFilterMoveDown(DataSetFilterWidget* widget)
{
    PROFILE(DataSetFilterView, onFilterMoveDown);

    int newIndex = dataSetFilterChain_->moveLater(widget->filter());
    moveFilterWidgetInLayout(widget, newIndex);

    dirtyDataSetFilters();
}

// ----------------------------------------------------------------------------
void DataSetFilterView::onRemoveFilterRequested(DataSetFilterWidget* widget)
{
    PROFILE(DataSetFilterView, onRemoveFilterRequested);

    widget->filter()->dispatcher.removeListener(this);
    int wasIndex = dataSetFilterChain_->remove(widget->filter());
    QWidget* at = filterWidgetsLayout_->itemAt(wasIndex)->widget();
    assert(at == widget);
    (void)at;
    widget->deleteLater();

    dirtyDataSetFilters();
}

// ----------------------------------------------------------------------------
void DataSetFilterView::moveFilterWidgetInLayout(DataSetFilterWidget* widget, int layoutIndex)
{
    PROFILE(DataSetFilterView, moveFilterWidgetInLayout);

    for (int i = 0; i != filterWidgetsLayout_->count(); ++i)
    {
        if (filterWidgetsLayout_->itemAt(i)->widget() == widget)
        {
            QLayoutItem* item = filterWidgetsLayout_->takeAt(i);
            filterWidgetsLayout_->insertItem(layoutIndex, item);
            return;
        }
    }

    assert(false);
}

// ----------------------------------------------------------------------------
void DataSetFilterView::reprocessInputDataSet()
{
    PROFILE(DataSetFilterView, reprocessInputDataSet);

    if (dataSetFiltersDirty_ == false)
        return;

    /*outputDataSet_ = dataSetFilterChain_->apply(inputDataSetMerged_.get());

    std::unordered_set<rfcommon::SavedGameSession*> uniqueSessions;
    for (const rfcommon::DataPoint* p = outputDataSet_->dataPointsBegin(); p != outputDataSet_->dataPointsEnd(); ++p)
        uniqueSessions.emplace(p->session());

    ui_->listWidget_outputGroup->clear();
    for (const auto& session : uniqueSessions)
        ui_->listWidget_outputGroup->addItem(composeFileName(session));
    ui_->listWidget_outputGroup->sortItems(Qt::DescendingOrder);

    ui_->label_outputInfo->setText(QString("Found %1 data points in %2 recordings")
                                       .arg(outputDataSet_->dataPointCount())
                                       .arg(uniqueSessions.size()));*/

    dataSetFiltersDirty_ = false;
}

// ----------------------------------------------------------------------------
void DataSetFilterView::dirtyDataSetFilters()
{
    PROFILE(DataSetFilterView, dirtyDataSetFilters);

    dataSetFiltersDirty_ = true;
    QMetaObject::invokeMethod(this, "reprocessInputDataSet", Qt::QueuedConnection);
}

// ----------------------------------------------------------------------------
void DataSetFilterView::addNewFilterWidget(DataSetFilterWidget* widget)
{
    PROFILE(DataSetFilterView, addNewFilterWidget);

    filterWidgetsLayout_->addWidget(widget);
    //recursivelyInstallEventFilter(widget);

    dataSetFilterChain_->add(widget->filter());
    widget->filter()->dispatcher.addListener(this);  // Listen to dirtying events

    connect(widget, &DataSetFilterWidget::enableFilter, this, &DataSetFilterView::onFilterEnabled);
    connect(widget, &DataSetFilterWidget::invertFilter, this, &DataSetFilterView::onFilterInverted);
    connect(widget, &DataSetFilterWidget::moveFilterUp, this, &DataSetFilterView::onFilterMoveUp);
    connect(widget, &DataSetFilterWidget::moveFilterDown, this, &DataSetFilterView::onFilterMoveDown);
    connect(widget, &DataSetFilterWidget::removeFilterRequested, this, &DataSetFilterView::onRemoveFilterRequested);
}

// ----------------------------------------------------------------------------
void DataSetFilterView::recursivelyInstallEventFilter(QObject* obj)
{
    PROFILE(DataSetFilterView, recursivelyInstallEventFilter);

    const QObjectList& children = obj->children();
    for(QObjectList::const_iterator child = children.begin(); child != children.end(); ++child)
        recursivelyInstallEventFilter(*child);

    if (obj->isWidgetType())
        obj->installEventFilter(this);
}

// ----------------------------------------------------------------------------
void DataSetFilterView::addGroupToInputRecordingsList(ReplayGroup* group)
{
    PROFILE(DataSetFilterView, addGroupToInputRecordingsList);

    for (const auto& fileInfo : group->absFilePathList())
        onReplayGroupFileAdded(group, fileInfo);
}

// ----------------------------------------------------------------------------
void DataSetFilterView::removeGroupFromInputRecordingsList(ReplayGroup* group)
{
    PROFILE(DataSetFilterView, removeGroupFromInputRecordingsList);

    for (const auto& fileInfo : group->absFilePathList())
        onReplayGroupFileRemoved(group, fileInfo);
}

// ----------------------------------------------------------------------------
void DataSetFilterView::removeGroupFromInputDataSet(ReplayGroup* group)
{
    PROFILE(DataSetFilterView, removeGroupFromInputDataSet);

    inputDataSets_.erase(group);

    /*inputDataSetMerged_->clear();
    for (const auto& [group, dataSet] : inputDataSets_)
        inputDataSetMerged_->mergeDataFrom(dataSet);*/

    dirtyDataSetFilters();
}

// ----------------------------------------------------------------------------
void DataSetFilterView::onReplayGroupFileAdded(ReplayGroup* group, const QFileInfo& absPathToFile)
{
    PROFILE(DataSetFilterView, onReplayGroupFileAdded);

    (void)group;
    ui_->listWidget_inputRecordings->addItem(absPathToFile.completeBaseName());
    ui_->listWidget_inputRecordings->sortItems(Qt::DescendingOrder);
}

// ----------------------------------------------------------------------------
void DataSetFilterView::onReplayGroupFileRemoved(ReplayGroup* group, const QFileInfo& absPathToFile)
{
    PROFILE(DataSetFilterView, onReplayGroupFileRemoved);

    (void)group;
    for (const auto& item : ui_->listWidget_inputRecordings->findItems(absPathToFile.completeBaseName(), Qt::MatchExactly))
        delete item;
}

// ----------------------------------------------------------------------------
void DataSetFilterView::onReplayManagerGroupAdded(ReplayGroup* group)
{
    PROFILE(DataSetFilterView, onReplayManagerGroupAdded);

    QListWidgetItem* item = new QListWidgetItem(group->name());
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Unchecked);
    ui_->listWidget_inputGroups->addItem(item);

    group->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
void DataSetFilterView::onReplayManagerGroupNameChanged(ReplayGroup* group, const QString& oldName, const QString& newName)
{
    PROFILE(DataSetFilterView, onReplayManagerGroupNameChanged);

    (void)group;
    for (int i = 0; i != ui_->listWidget_inputGroups->count(); ++i)
    {
        QListWidgetItem* item = ui_->listWidget_inputGroups->item(i);
        if (item->text() == oldName)
        {
            item->setText(newName);
            break;
        }
    }
}

// ----------------------------------------------------------------------------
void DataSetFilterView::onReplayManagerGroupRemoved(ReplayGroup* group)
{
    PROFILE(DataSetFilterView, onReplayManagerGroupRemoved);

    group->dispatcher.removeListener(this);
    for (const auto& item : ui_->listWidget_inputGroups->findItems(group->name(), Qt::MatchExactly))
        delete item;
}

// ----------------------------------------------------------------------------
void DataSetFilterView::onDataSetBackgroundLoaderDataSetLoaded(ReplayGroup* group, rfcommon::DataSet* dataSet)
{
    PROFILE(DataSetFilterView, onDataSetBackgroundLoaderDataSetLoaded);

    /*inputDataSets_.emplace(group, dataSet);
    inputDataSetMerged_->mergeDataFrom(dataSet);*/

    dirtyDataSetFilters();
}

// ----------------------------------------------------------------------------
void DataSetFilterView::onDataSetFilterDirtied(rfcommon::DataSetFilter* filter)
{
    PROFILE(DataSetFilterView, onDataSetFilterDirtied);

    (void)filter;
    dirtyDataSetFilters();
}

}
