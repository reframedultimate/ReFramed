#include "ui_FrameDataListView.h"
#include "frame-data-list/views/FrameDataListView.hpp"
#include "frame-data-list/models/FrameDataListModel.hpp"
#include "rfcommon/MetaData.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/Profiler.hpp"
#include <QDateTime>

static void clearStackedWidget(QStackedWidget* sw)
{
    PROFILE(StreametaViewGlobal, clearStackedWidget);

    while (sw->count())
    {
        QWidget* widget = sw->widget(0);
        sw->removeWidget(widget);
        widget->deleteLater();
    }
}

// ----------------------------------------------------------------------------
FrameDataListView::FrameDataListView(FrameDataListModel* model, QWidget* parent)
    : QWidget(parent)
    , model_(model)
    , ui_(new Ui::FrameDataListView)
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

    FrameDataListView::onNewData(model_->mappingInfo(), model_->metaData(), model_->frameData());

    connect(ui_->treeWidget, &QTreeWidget::currentItemChanged,
            this, &FrameDataListView::onCurrentItemChanged);

    model_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
FrameDataListView::~FrameDataListView()
{
    model_->dispatcher.removeListener(this);

    delete ui_;
}

// ----------------------------------------------------------------------------
void FrameDataListView::onNewData(rfcommon::MappingInfo* map, rfcommon::MetaData* meta, rfcommon::FrameData* frames)
{
    PROFILE(FrameDataListView, onNewData);

    repopulatePlayerDataTables();
    repopulateTree(map, meta, frames);

    ui_->tableView_baseStatusIDs->resizeColumnsToContents();
    ui_->tableView_metaData->resizeColumnsToContents();
    ui_->tableView_hitStatusIDs->resizeColumnsToContents();
    ui_->tableView_stageIDs->resizeColumnsToContents();
    ui_->tableView_specificStatusIDs->resizeColumnsToContents();
    ui_->tableView_fighterIDs->resizeColumnsToContents();
}

// ----------------------------------------------------------------------------
void FrameDataListView::onDataFinalized(rfcommon::MappingInfo* map, rfcommon::MetaData* meta, rfcommon::FrameData* frames)
{
    PROFILE(FrameDataListView, onDataFinalized);

}

// ----------------------------------------------------------------------------
void FrameDataListView::onNewFrame()
{
    PROFILE(FrameDataListView, onNewFrame);

    for (auto table : playerDataTables_)
        table->scrollToBottom();
}

// ----------------------------------------------------------------------------
void FrameDataListView::onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
    PROFILE(FrameDataListView, onCurrentItemChanged);

    (void)previous;

    if (current == nullptr)
        return;

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
        ui_->stackedWidget->setCurrentWidget(ui_->page_baseStatusIDs);
    else if (current == specificStatusIDMappingsItem_)
        ui_->stackedWidget->setCurrentWidget(ui_->page_specificStatusIDs);
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
void FrameDataListView::repopulateTree(rfcommon::MappingInfo* map, rfcommon::MetaData* meta, rfcommon::FrameData* frames)
{
    PROFILE(FrameDataListView, repopulateTree);

    const int previouslySelectedItem = [this]() -> int {
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

    ui_->treeWidget->clear();
    playerDataItems_.clear();

    metaDataItem_ = nullptr;
    stageIDMappingsItem_ = nullptr;
    fighterIDMappingsItem_ = nullptr;
    baseStatusIDMappingsItem_ = nullptr;
    specificStatusIDMappingsItem_ = nullptr;
    hitStatusIDMappingsItem_ = nullptr;

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

    switch (previouslySelectedItem)
    {
#define SELECT(item) { ui_->treeWidget->setCurrentItem(item); }
        case 0: SELECT(metaDataItem_) break;
        case 1: SELECT(stageIDMappingsItem_) break;
        case 2: SELECT(fighterIDMappingsItem_) break;
        case 3: SELECT(baseStatusIDMappingsItem_) break;
        case 4: SELECT(specificStatusIDMappingsItem_) break;
        case 5: SELECT(hitStatusIDMappingsItem_) break;
        default:
            if (previouslySelectedItem >= 6 && previouslySelectedItem < 6 + playerDataItems_.count())
                SELECT(playerDataItems_[previouslySelectedItem - 6])
            break;
    }
}

// ----------------------------------------------------------------------------
void FrameDataListView::repopulatePlayerDataTables()
{
    PROFILE(FrameDataListView, repopulatePlayerDataTables);

    const int storeIdx = ui_->stackedWidget_playerData->currentIndex();
    clearStackedWidget(ui_->stackedWidget_playerData);
    playerDataTables_.clearCompact();

    for (int i = 0; i != model_->fighterStatesModelCount(); ++i)
    {
        QTableView* view = new QTableView;
        view->verticalHeader()->hide();
        ui_->stackedWidget_playerData->addWidget(view);
        playerDataTables_.push(view);
    }

    // Since we cleared the stacked widget, it won't remember which one was
    // selected. storeCurrentPageIndex_ is saved in clearUI() and we try to
    // select the same player index again
    if (storeIdx >= model_->fighterStatesModelCount())
        ui_->stackedWidget_playerData->setCurrentIndex(0);
    else
        ui_->stackedWidget_playerData->setCurrentIndex(storeIdx);
}
