#include "ui_DataViewerView.h"
#include "data-viewer/views/DataViewerView.hpp"
#include "data-viewer/models/DataViewerModel.hpp"
#include "rfcommon/MetaData.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/Profiler.hpp"
#include <QDateTime>

static void clearStackedWidget(QStackedWidget* sw)
{
    PROFILE(DataViewerViewGlobal, clearStackedWidget);

    while (sw->count())
    {
        QWidget* widget = sw->widget(0);
        sw->removeWidget(widget);
        widget->deleteLater();
    }
}

// ----------------------------------------------------------------------------
DataViewerView::DataViewerView(DataViewerModel* model, QWidget* parent)
    : QWidget(parent)
    , model_(model)
    , ui_(new Ui::DataViewerView)
{
    ui_->setupUi(this);
    ui_->splitter->setStretchFactor(0, 0);
    ui_->splitter->setStretchFactor(1, 1);

    ui_->tableView_baseStatusIDs->setModel(model_->baseStatusIDModel());
    ui_->tableView_metaData->setModel(model_->metaDataModel());
    ui_->tableView_hitStatusIDs->setModel(model_->hitStatusIDModel());
    ui_->tableView_stageIDs->setModel(model_->stageIDModel());
    ui_->tableView_specificStatusIDs->setModel(model_->specificStatusIDModel());
    ui_->tableView_fighterIDs->setModel(model_->fighterIDModel());

    DataViewerView::onNewData(model_->mappingInfo(), model_->metaData(), model_->frameData());

    connect(ui_->treeWidget, &QTreeWidget::currentItemChanged,
            this, &DataViewerView::onCurrentItemChanged);

    model_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
DataViewerView::~DataViewerView()
{
    model_->dispatcher.removeListener(this);

    delete ui_;
}

// ----------------------------------------------------------------------------
void DataViewerView::onNewData(rfcommon::MappingInfo* map, rfcommon::MetaData* meta, rfcommon::FrameData* frames)
{
    PROFILE(DataViewerView, onNewData);

    populatePlayerDataTables();
    populateTree(map, meta, frames);

    ui_->tableView_baseStatusIDs->resizeColumnsToContents();
    ui_->tableView_metaData->resizeColumnsToContents();
    ui_->tableView_hitStatusIDs->resizeColumnsToContents();
    ui_->tableView_stageIDs->resizeColumnsToContents();
    ui_->tableView_specificStatusIDs->resizeColumnsToContents();
    ui_->tableView_fighterIDs->resizeColumnsToContents();
}

// ----------------------------------------------------------------------------
void DataViewerView::onClear()
{
    PROFILE(DataViewerView, onClear);

    playerDataTableIdxOnClear_ = ui_->stackedWidget_playerData->currentIndex();
    selectedTreeItemOnClear_ = [this]() -> int {
        const auto item = ui_->treeWidget->currentItem();
        if (item == nullptr)
            return -1;
        if (item == metaDataItem_) return 0;
        else if (item == stageIDMappingsItem_) return 1;
        else if (item == fighterIDMappingsItem_) return 2;
        else if (item == baseStatusIDMappingsItem_) return 3;
        else if (item == specificStatusIDMappingsItem_) return 4;
        else if (item == hitStatusIDMappingsItem_) return 5;
        for (int i = 0; i != playerDataItems_.count(); ++i)
            if (item == playerDataItems_[i])
                return 6 + i;
        return -1;
    }();

    clearStackedWidget(ui_->stackedWidget_playerData);
    playerDataTables_.clearCompact();

    ui_->treeWidget->clear();
    playerDataItems_.clear();

    metaDataItem_ = nullptr;
    stageIDMappingsItem_ = nullptr;
    fighterIDMappingsItem_ = nullptr;
    baseStatusIDMappingsItem_ = nullptr;
    specificStatusIDMappingsItem_ = nullptr;
    hitStatusIDMappingsItem_ = nullptr;
}

// ----------------------------------------------------------------------------
void DataViewerView::onNewFrame()
{
    PROFILE(DataViewerView, onNewFrame);

    for (auto table : playerDataTables_)
        table->scrollToBottom();
}

// ----------------------------------------------------------------------------
void DataViewerView::onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
    PROFILE(DataViewerView, onCurrentItemChanged);

    (void)previous;

    if (current == nullptr)
        return;

    // Saves a bit of time when player is selecting different replays, because
    // the UI doesn't have to build the tables if they're not visible
    if (previous == baseStatusIDMappingsItem_)
        ui_->tableView_baseStatusIDs->setModel(nullptr);
    else if (previous == specificStatusIDMappingsItem_)
        ui_->tableView_specificStatusIDs->setModel(nullptr);
    for (int i = 0; i != model_->fighterStatesModelCount(); ++i)
        if (previous == playerDataItems_[i])
        {
            playerDataTables_[i]->setModel(nullptr);
            break;
        }

    if (current == metaDataItem_)
        ui_->stackedWidget->setCurrentWidget(ui_->page_metaData);
    else if (current == stageIDMappingsItem_)
        ui_->stackedWidget->setCurrentWidget(ui_->page_stageIDs);
    else if (current == fighterIDMappingsItem_)
        ui_->stackedWidget->setCurrentWidget(ui_->page_fighterIDs);
    else if (current == baseStatusIDMappingsItem_)
    {
        ui_->stackedWidget->setCurrentWidget(ui_->page_baseStatusIDs);
        ui_->tableView_baseStatusIDs->setModel(model_->baseStatusIDModel());
    }
    else if (current == specificStatusIDMappingsItem_)
    {
        ui_->stackedWidget->setCurrentWidget(ui_->page_specificStatusIDs);
        ui_->tableView_specificStatusIDs->setModel(model_->specificStatusIDModel());
    }
    else if (current == hitStatusIDMappingsItem_)
        ui_->stackedWidget->setCurrentWidget(ui_->page_hitStatusIDs);
    else
    {
        for (int i = 0; i != model_->fighterStatesModelCount(); ++i)
            if (current == playerDataItems_[i])
            {
                ui_->stackedWidget->setCurrentWidget(ui_->page_playerData);
                ui_->stackedWidget_playerData->setCurrentIndex(i);

                playerDataTables_[i]->setModel(model_->fighterStatesModel(i));
                playerDataTables_[i]->resizeColumnsToContents();
                playerDataTables_[i]->scrollToBottom();
                break;
            }
    }
}

// ----------------------------------------------------------------------------
void DataViewerView::populateTree(rfcommon::MappingInfo* map, rfcommon::MetaData* meta, rfcommon::FrameData* frames)
{
    PROFILE(DataViewerView, populateTree);

    // Meta Data
    if (meta)
    {
        metaDataItem_ = new QTreeWidgetItem({"Meta Data"});
        ui_->treeWidget->addTopLevelItem(metaDataItem_);
    }

    // Mapping info
    if (map)
    {
        baseStatusIDMappingsItem_ = new QTreeWidgetItem({"Base IDs"});
        specificStatusIDMappingsItem_ = new QTreeWidgetItem({"Fighter Specific IDs"});
        QTreeWidgetItem* statusMappings = new QTreeWidgetItem({"Status IDs"});
        statusMappings->addChildren({baseStatusIDMappingsItem_, specificStatusIDMappingsItem_});
        statusMappings->setFlags(Qt::ItemIsEnabled);

        QTreeWidgetItem* mappings = new QTreeWidgetItem({"Mapping Info"});
        stageIDMappingsItem_ = new QTreeWidgetItem({"Stage IDs"});
        fighterIDMappingsItem_ = new QTreeWidgetItem({"Fighter IDs"});
        hitStatusIDMappingsItem_ = new QTreeWidgetItem({"Hit Status IDs"});
        mappings->addChildren({stageIDMappingsItem_, fighterIDMappingsItem_, statusMappings, hitStatusIDMappingsItem_});
        mappings->setFlags(Qt::ItemIsEnabled);

        ui_->treeWidget->addTopLevelItem(mappings);
        mappings->setExpanded(true);
        statusMappings->setExpanded(true);
    }

    // Player states
    if (model_->fighterStatesModelCount() > 0)
    {
        QTreeWidgetItem* playerStates = new QTreeWidgetItem({"Player States"});
        for (int i = 0; i != model_->fighterStatesModelCount(); ++i)
        {
            QString playerName = meta ?
                        QString(meta->name(i).cStr()) :
                        QString("Player ") + QString::number(i+1);
            QTreeWidgetItem* player = new QTreeWidgetItem({playerName});
            playerDataItems_.push(player);
            playerStates->addChild(player);
            player->setExpanded(true);
        }

        ui_->treeWidget->addTopLevelItem(playerStates);
        playerStates->setExpanded(true);
    }

    switch (selectedTreeItemOnClear_)
    {
#define SELECT(item) { ui_->treeWidget->setCurrentItem(item); }
        case 0: SELECT(metaDataItem_) break;
        case 1: SELECT(stageIDMappingsItem_) break;
        case 2: SELECT(fighterIDMappingsItem_) break;
        case 3: SELECT(baseStatusIDMappingsItem_) break;
        case 4: SELECT(specificStatusIDMappingsItem_) break;
        case 5: SELECT(hitStatusIDMappingsItem_) break;
        default:
            if (selectedTreeItemOnClear_ >= 6 && selectedTreeItemOnClear_ < 6 + playerDataItems_.count())
                SELECT(playerDataItems_[selectedTreeItemOnClear_ - 6])
            break;
#undef SELECT
    }
}

// ----------------------------------------------------------------------------
void DataViewerView::populatePlayerDataTables()
{
    PROFILE(DataViewerView, populatePlayerDataTables);

    for (int i = 0; i != model_->fighterStatesModelCount(); ++i)
    {
        QTableView* view = new QTableView;
        view->verticalHeader()->hide();
        ui_->stackedWidget_playerData->addWidget(view);
        playerDataTables_.push(view);
    }

    // Since we cleared the stacked widget, it won't remember which one was
    // selected. Try to select the same player index again
    if (playerDataTableIdxOnClear_ >= 0 && playerDataTableIdxOnClear_ < model_->fighterStatesModelCount())
        ui_->stackedWidget_playerData->setCurrentIndex(playerDataTableIdxOnClear_);
    else
        ui_->stackedWidget_playerData->setCurrentIndex(0);
}
