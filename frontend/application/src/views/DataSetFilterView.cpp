#include "application/ui_DataSetFilterView.h"
#include "application/Util.hpp"
#include "application/views/DataSetFilterView.hpp"
#include "application/views/DataSetFilterWidget_Date.hpp"
#include "application/views/DataSetFilterWidget_Game.hpp"
#include "application/views/DataSetFilterWidget_Matchup.hpp"
#include "application/views/DataSetFilterWidget_Player.hpp"
#include "application/views/DataSetFilterWidget_PlayerCount.hpp"
#include "application/views/DataSetFilterWidget_Stage.hpp"
#include "application/models/RecordingManager.hpp"
#include "application/models/RecordingGroup.hpp"
#include "application/models/DataSetBackgroundLoader.hpp"
#include "uh/DataSetFilterChain.hpp"
#include "uh/DataSet.hpp"
#include "uh/DataSetFilter.hpp"
#include "uh/PlayerState.hpp"
#include "uh/Reference.hpp"
#include "uh/SavedRecording.hpp"

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

namespace uhapp {

// ----------------------------------------------------------------------------
DataSetFilterView::DataSetFilterView(RecordingManager* recordingManager, QWidget* parent)
    : QWidget(parent)
    , ui_(new Ui::DataSetFilterView)
    , filterWidgetsLayout_(new QVBoxLayout)
    , recordingManager_(recordingManager)
    , dataSetBackgroundLoader_(new DataSetBackgroundLoader(this))
    , dataSetFilterChain_(new uh::DataSetFilterChain)
    , inputDataSetMerged_(new uh::DataSet)
    , outputDataSet_(new uh::DataSet)
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
    for (const auto& it : recordingManager_->recordingGroups())
        onRecordingManagerGroupAdded(it.second.get());

    connect(ui_->toolButton_addFilter, &QToolButton::triggered,
            this, &DataSetFilterView::onToolButtonAddFilterTriggered);
    connect(ui_->listWidget_inputGroups, &QListWidget::itemChanged,
            this, &DataSetFilterView::onInputGroupItemChanged);

    recordingManager_->dispatcher.addListener(this);
    dataSetBackgroundLoader_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
DataSetFilterView::~DataSetFilterView()
{
    // Have to do this so we unregister as listeners to every recording group
    for (const auto& it : recordingManager_->recordingGroups())
        onRecordingManagerGroupRemoved(it.second.get());

    dataSetBackgroundLoader_->dispatcher.removeListener(this);
    recordingManager_->dispatcher.removeListener(this);
    delete ui_;
}

// ----------------------------------------------------------------------------
bool DataSetFilterView::eventFilter(QObject* target, QEvent* e)
{
    // Stops the scroll wheel from interfering with the elements in each
    // modifier widget
    if(e->type() == QEvent::Wheel)
        return true;
    return false;
}


// ----------------------------------------------------------------------------
void DataSetFilterView::onToolButtonAddFilterTriggered(QAction* action)
{
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
    RecordingGroup* group = recordingManager_->recordingGroup(item->text());
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
    widget->filter()->setEnabled(enable);
    widget->contentWidget()->setEnabled(enable);
}

// ----------------------------------------------------------------------------
void DataSetFilterView::onFilterInverted(DataSetFilterWidget* widget, bool inverted)
{
    widget->filter()->setInverted(inverted);
}

// ----------------------------------------------------------------------------
void DataSetFilterView::onFilterMoveUp(DataSetFilterWidget* widget)
{
    int newIndex = dataSetFilterChain_->moveEarlier(widget->filter());
    moveFilterWidgetInLayout(widget, newIndex);

    dirtyDataSetFilters();
}

// ----------------------------------------------------------------------------
void DataSetFilterView::onFilterMoveDown(DataSetFilterWidget* widget)
{
    int newIndex = dataSetFilterChain_->moveLater(widget->filter());
    moveFilterWidgetInLayout(widget, newIndex);

    dirtyDataSetFilters();
}

// ----------------------------------------------------------------------------
void DataSetFilterView::onRemoveFilterRequested(DataSetFilterWidget* widget)
{
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
    for (int i = 0; i != filterWidgetsLayout_->count(); ++i)
    {
        if (filterWidgetsLayout_->itemAt(i)->widget() == widget)
        {
            filterWidgetsLayout_->takeAt(i);
            filterWidgetsLayout_->insertWidget(layoutIndex, widget);
            return;
        }
    }

    assert(false);
}

// ----------------------------------------------------------------------------
void DataSetFilterView::reprocessInputDataSet()
{
    if (dataSetFiltersDirty_ == false)
        return;

    outputDataSet_ = dataSetFilterChain_->apply(inputDataSetMerged_.get());

    std::unordered_set<uh::GameSession*> uniqueRecordings;
    for (const uh::DataPoint* p = outputDataSet_->dataPointsBegin(); p != outputDataSet_->dataPointsEnd(); ++p)
        uniqueRecordings.emplace(p->session());

    ui_->listWidget_outputGroup->clear();
    for (const auto& recording : uniqueRecordings)
        ui_->listWidget_outputGroup->addItem(composeFileName(recording));
    ui_->listWidget_outputGroup->sortItems(Qt::DescendingOrder);

    ui_->label_outputInfo->setText(QString("Found %1 data points in %2 recordings").arg(outputDataSet_->dataPointCount()).arg(uniqueRecordings.size()));

    dataSetFiltersDirty_ = false;
}

// ----------------------------------------------------------------------------
void DataSetFilterView::dirtyDataSetFilters()
{
    dataSetFiltersDirty_ = true;
    QMetaObject::invokeMethod(this, "reprocessInputDataSet", Qt::QueuedConnection);
}

// ----------------------------------------------------------------------------
void DataSetFilterView::addNewFilterWidget(DataSetFilterWidget* widget)
{
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
    const QObjectList& children = obj->children();
    for(QObjectList::const_iterator child = children.begin(); child != children.end(); ++child)
        recursivelyInstallEventFilter(*child);

    if (obj->isWidgetType())
        obj->installEventFilter(this);
}

// ----------------------------------------------------------------------------
void DataSetFilterView::addGroupToInputRecordingsList(RecordingGroup* group)
{
    for (const auto& fileInfo : group->absFilePathList())
        onRecordingGroupFileAdded(group, fileInfo);
}

// ----------------------------------------------------------------------------
void DataSetFilterView::removeGroupFromInputRecordingsList(RecordingGroup* group)
{
    for (const auto& fileInfo : group->absFilePathList())
        onRecordingGroupFileRemoved(group, fileInfo);
}

// ----------------------------------------------------------------------------
void DataSetFilterView::removeGroupFromInputDataSet(RecordingGroup* group)
{
    inputDataSets_.erase(group);

    inputDataSetMerged_->clear();
    for (const auto& [group, dataSet] : inputDataSets_)
        inputDataSetMerged_->mergeDataFrom(dataSet);

    dirtyDataSetFilters();
}

// ----------------------------------------------------------------------------
void DataSetFilterView::onRecordingGroupFileAdded(RecordingGroup* group, const QFileInfo& absPathToFile)
{
    (void)group;
    ui_->listWidget_inputRecordings->addItem(absPathToFile.completeBaseName());
    ui_->listWidget_inputRecordings->sortItems(Qt::DescendingOrder);
}

// ----------------------------------------------------------------------------
void DataSetFilterView::onRecordingGroupFileRemoved(RecordingGroup* group, const QFileInfo& absPathToFile)
{
    (void)group;
    for (const auto& item : ui_->listWidget_inputRecordings->findItems(absPathToFile.completeBaseName(), Qt::MatchExactly))
        delete item;
}

// ----------------------------------------------------------------------------
void DataSetFilterView::onRecordingManagerGroupAdded(RecordingGroup* group)
{
    QListWidgetItem* item = new QListWidgetItem(group->name());
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Unchecked);
    ui_->listWidget_inputGroups->addItem(item);

    group->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
void DataSetFilterView::onRecordingManagerGroupNameChanged(RecordingGroup* group, const QString& oldName, const QString& newName)
{
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
void DataSetFilterView::onRecordingManagerGroupRemoved(RecordingGroup* group)
{
    group->dispatcher.removeListener(this);
    for (const auto& item : ui_->listWidget_inputGroups->findItems(group->name(), Qt::MatchExactly))
        delete item;
}

// ----------------------------------------------------------------------------
void DataSetFilterView::onDataSetBackgroundLoaderDataSetLoaded(RecordingGroup* group, uh::DataSet* dataSet)
{
    inputDataSets_.emplace(group, dataSet);
    inputDataSetMerged_->mergeDataFrom(dataSet);

    dirtyDataSetFilters();
}

// ----------------------------------------------------------------------------
void DataSetFilterView::onDataSetFilterDirtied(uh::DataSetFilter* filter)
{
    (void)filter;
    dirtyDataSetFilters();
}

}
