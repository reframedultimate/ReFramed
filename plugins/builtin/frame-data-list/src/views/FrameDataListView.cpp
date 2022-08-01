#include "ui_FrameDataListView.h"
#include "frame-data-list/views/FrameDataListView.hpp"
#include "frame-data-list/models/FrameDataListModel.hpp"
#include "rfcommon/SessionMetaData.hpp"
#include "rfcommon/FrameData.hpp"
#include <QDateTime>

namespace {

class IntegerTableWidgetItem : public QTableWidgetItem
{
public:
    IntegerTableWidgetItem(int value)
        : QTableWidgetItem(QString::number(value))
    {}

    bool operator<(const QTableWidgetItem& other) const
    {
        return text().toInt() < other.text().toInt();
    }
};

class DoubleIntegerTableWidgetItem : public QTableWidgetItem
{
public:
    DoubleIntegerTableWidgetItem(int value1, int value2)
        : QTableWidgetItem(QString::number(value1))
        , value2(value2)
    {}

    bool operator<(const QTableWidgetItem& other) const
    {
        const DoubleIntegerTableWidgetItem* otherDouble = dynamic_cast<const DoubleIntegerTableWidgetItem*>(&other);
        if (otherDouble)
        {
            int value1 = text().toInt();
            int otherValue1 = other.text().toInt();
            int otherValue2 = otherDouble->value2;

            if (value1 != otherValue1) return value1 < otherValue1;
            if (value2 != otherValue2) return value2 < otherValue2;
            return false;  // they are equal
        }

        return text().toInt() < other.text().toInt();
    }

private:
    int value2;
};

}

static void clearStackedWidget(QStackedWidget* sw)
{
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

    ui_->tableView_metaData->setModel(model_->metaDataModel());

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
void FrameDataListView::onNewData(rfcommon::MappingInfo* map, rfcommon::SessionMetaData* meta, rfcommon::FrameData* frames)
{
    clearUI();

    int storeCurrentPageIndex = ui_->stackedWidget->currentIndex();

    repopulateTree(meta, frames);
    repopulateStageMappingTable();
    repopulateFighterMappingTable();
    repopulateStatusMappingTable();
    repopulateHitStatusMappingTable();
    repopulatePlayerDataTables();

    ui_->stackedWidget->setCurrentIndex(storeCurrentPageIndex);

    playerDataTableRowsDirty_ = true;
    lastTimePlayerDataTablesUpdated_ = QDateTime::currentMSecsSinceEpoch();
    playerDataTablesUpdateTime_ = 0;

    if (ui_->stackedWidget->currentWidget() == ui_->page_playerData)
        updatePlayerDataTableRowsIfDirty();

    ui_->tableView_metaData->resizeColumnsToContents();
}

// ----------------------------------------------------------------------------
void FrameDataListView::onDataFinalized(rfcommon::MappingInfo* map, rfcommon::SessionMetaData* meta, rfcommon::FrameData* frames)
{
    // Have to update all tables now because the session object is about to
    // be de-ref'd and we won't have a chance to do it later
    updatePlayerDataTableRowsIfDirty();
}

// ----------------------------------------------------------------------------
void FrameDataListView::onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
    (void)previous;

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
        int i = 0;
        for (const auto& item : playerDataItems_)
        {
            if (current == item)
            {
                ui_->stackedWidget->setCurrentWidget(ui_->page_playerData);
                ui_->stackedWidget_playerData->setCurrentIndex(i);
                updatePlayerDataTableRowsIfDirty();
                break;
            }
            i++;
        }
    }
}

// ----------------------------------------------------------------------------
void FrameDataListView::clearUI()
{
    storeCurrentPageIndex_ = ui_->stackedWidget_playerData->currentIndex();

    ui_->treeWidget->clear();
    ui_->tableWidget_stageIDs->clearContents();
    ui_->tableWidget_stageIDs->setRowCount(0);
    ui_->tableWidget_fighterIDs->clearContents();
    ui_->tableWidget_fighterIDs->setRowCount(0);
    ui_->tableWidget_baseStatusIDs->clearContents();
    ui_->tableWidget_baseStatusIDs->setRowCount(0);
    ui_->tableWidget_specificStatusIDs->clearContents();
    ui_->tableWidget_specificStatusIDs->setRowCount(0);
    ui_->tableWidget_hitStatusIDs->clearContents();
    ui_->tableWidget_hitStatusIDs->setRowCount(0);

    clearStackedWidget(ui_->stackedWidget_playerData);

    playerDataItems_.clear();
    playerDataTables_.clear();
}

// ----------------------------------------------------------------------------
void FrameDataListView::repopulateTree(rfcommon::SessionMetaData* meta, rfcommon::FrameData* frameData)
{
    // Meta Data
    metaDataItem_ = new QTreeWidgetItem({"Meta Data"});
    ui_->treeWidget->addTopLevelItem(metaDataItem_);

    // Mapping info
    baseStatusIDMappingsItem_ = new QTreeWidgetItem({"Base IDs"});
    specificStatusIDMappingsItem_ = new QTreeWidgetItem({"Fighter Specific IDs"});
    QTreeWidgetItem* statusMappings = new QTreeWidgetItem({"Status IDs"});
    statusMappings->addChildren({baseStatusIDMappingsItem_, specificStatusIDMappingsItem_});

    QTreeWidgetItem* mappings = new QTreeWidgetItem({"Mapping Info"});
    stageIDMappingsItem_ = new QTreeWidgetItem({"Stage IDs"});
    fighterIDMappingsItem_ = new QTreeWidgetItem({"Fighter IDs"});
    hitStatusIDMappingsItem_ = new QTreeWidgetItem({"Hit Status IDs"});
    mappings->addChildren({stageIDMappingsItem_, fighterIDMappingsItem_, statusMappings, hitStatusIDMappingsItem_});
    ui_->treeWidget->addTopLevelItem(mappings);

    // meta and frame data might be null
    const int fighterCount =
            meta ? meta->fighterCount() :
            frameData ? frameData->fighterCount() :
            0;

    // Player states
    QTreeWidgetItem* playerStates = new QTreeWidgetItem({"Player States"});
    ui_->treeWidget->addTopLevelItem(playerStates);
    for (int i = 0; i != fighterCount; ++i)
    {
        QString playerName = meta ?
                    QString(meta->name(i).cStr()) :
                    QString("Player ") + QString::number(i+1);
        QTreeWidgetItem* player = new QTreeWidgetItem({playerName});
        playerDataItems_.push(player);
        playerStates->addChild(player);
        player->setExpanded(true);
    }

    mappings->setExpanded(true);
    statusMappings->setExpanded(true);
    playerStates->setExpanded(true);
}

// ----------------------------------------------------------------------------
void FrameDataListView::repopulateStageMappingTable()
{
    /*
    // Fill in data
    int i = 0;
    rfcommon::Session* session = model_->session();
    const auto& stageMapping = session->mappingInfo().stageID.get();
    ui_->tableWidget_stageIDs->setRowCount(stageMapping.count());
    for (const auto& it : stageMapping)
    {
        ui_->tableWidget_stageIDs->setItem(i, 0, new IntegerTableWidgetItem(it->key().value()));
        ui_->tableWidget_stageIDs->setItem(i, 1, new QTableWidgetItem(it->value().cStr()));
        i++;
    }
    ui_->tableWidget_stageIDs->sortByColumn(0, Qt::AscendingOrder);
    ui_->tableWidget_stageIDs->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);*/
}

// ----------------------------------------------------------------------------
void FrameDataListView::repopulateFighterMappingTable()
{
    /*
    // Fill in data
    int i = 0;
    rfcommon::Session* session = model_->session();
    const auto& fighterMapping = session->mappingInfo().fighterID.get();
    ui_->tableWidget_fighterIDs->setRowCount(fighterMapping.count());
    for (const auto& it : fighterMapping)
    {
        ui_->tableWidget_fighterIDs->setItem(i, 0, new IntegerTableWidgetItem(it->key().value()));
        ui_->tableWidget_fighterIDs->setItem(i, 1, new QTableWidgetItem(it->value().cStr()));
        ++i;
    }
    ui_->tableWidget_fighterIDs->sortByColumn(0, Qt::AscendingOrder);
    ui_->tableWidget_fighterIDs->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);*/
}

// ----------------------------------------------------------------------------
void FrameDataListView::repopulateStatusMappingTable()
{
    /*
    // Fill in base status mapping info
    int i = 0;
    rfcommon::Session* session = model_->session();
    const auto& baseStatusMapping = session->mappingInfo().fighterStatus.baseEnumNames();
    const auto& specificStatusMappings = session->mappingInfo().fighterStatus.fighterSpecificEnumNames();
    ui_->tableWidget_baseStatusIDs->setRowCount(baseStatusMapping.count());
    for (const auto& it : baseStatusMapping)
    {
        ui_->tableWidget_baseStatusIDs->setItem(i, 0, new IntegerTableWidgetItem(it.key().value()));
        ui_->tableWidget_baseStatusIDs->setItem(i, 1, new QTableWidgetItem(it.value().cStr()));
        ++i;
    }

    // Fill in fighter specific status mapping info
    i = 0;
    ui_->tableWidget_specificStatusIDs->setRowCount(0);
    for (const auto& fighter : specificStatusMappings)
    {
        ui_->tableWidget_specificStatusIDs->setRowCount(
                    ui_->tableWidget_specificStatusIDs->rowCount() + fighter.value().count());
        for (const auto& it : fighter.value())
        {
            ui_->tableWidget_specificStatusIDs->setItem(i, 0, new DoubleIntegerTableWidgetItem(fighter.key().value(), it.key().value()));
            ui_->tableWidget_specificStatusIDs->setItem(i, 1, new IntegerTableWidgetItem(it.key().value()));
            ui_->tableWidget_specificStatusIDs->setItem(i, 2, new QTableWidgetItem(it.value().cStr()));
            ++i;
        }
    }

    ui_->tableWidget_baseStatusIDs->sortByColumn(0, Qt::AscendingOrder);
    ui_->tableWidget_specificStatusIDs->sortByColumn(0, Qt::AscendingOrder);

    ui_->tableWidget_baseStatusIDs->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    ui_->tableWidget_specificStatusIDs->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);*/
}

// ----------------------------------------------------------------------------
void FrameDataListView::repopulateHitStatusMappingTable()
{
    /*
    // Fill in data
    int i = 0;
    rfcommon::Session* session = model_->session();
    const auto& hitStatusMapping = session->mappingInfo().hitStatus.get();
    ui_->tableWidget_hitStatusIDs->setRowCount(hitStatusMapping.count());
    for (const auto& it : hitStatusMapping)
    {
        ui_->tableWidget_hitStatusIDs->setItem(i, 0, new IntegerTableWidgetItem(it.key().value()));
        ui_->tableWidget_hitStatusIDs->setItem(i, 1, new QTableWidgetItem(it.value().cStr()));
        ++i;
    }
    ui_->tableWidget_hitStatusIDs->sortByColumn(0, Qt::AscendingOrder);
    ui_->tableWidget_hitStatusIDs->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);*/
}

// ----------------------------------------------------------------------------
void FrameDataListView::repopulatePlayerDataTables()
{
    /*
    // Fill in data
    rfcommon::Session* session = model_->session();
    for (int player = 0; player != session->fighterCount(); ++player)
    {
        QTableWidget* table = new QTableWidget(0, 11);
        playerDataTables_.push(table);
        ui_->stackedWidget_playerData->addWidget(table);
        table->setHorizontalHeaderLabels({"Frame", "Position", "Facing", "Damage", "Hitstun", "Shield", "Status", "Motion", "Hit Status", "Stocks", "Attack Connected"});
    }

    // Since we cleared the stacked widget, it won't remember which one was
    // selected. storeCurrentPageIndex_ is saved in clear() and we try to
    // select the same player index again
    if (storeCurrentPageIndex_ >= session->fighterCount())
        ui_->stackedWidget_playerData->setCurrentIndex(0);
    else
        ui_->stackedWidget_playerData->setCurrentIndex(storeCurrentPageIndex_);

    // Defer populating tables because there's a lot of data and it takes half
    // a second or so
    playerDataTableRowsDirty_ = true;*/
}

// ----------------------------------------------------------------------------
void FrameDataListView::updatePlayerDataTableRowsIfDirty()
{
    /*
    if (playerDataTableRowsDirty_ == false)
        return;

    rfcommon::Session* session = model_->session();
    for (int player = 0; player != session->fighterCount(); ++player)
    {
        QTableWidget* table = playerDataTables_[player];

        // This method can be called multiple times if the active recording is
        // adding new player states. On Windows, growing the table is really
        // slow, so it gets called once every few seconds. Figure out how many
        // rows are already populated and how many need to be added using new
        // data
        int row = table->rowCount();
        if (table->rowCount() < session->frameCount())
            table->setRowCount(session->frameCount());
        int endRow = table->rowCount();

        while (row < endRow)
        {
            const rfcommon::FighterState& state = session->state(row, player);
            setPlayerDataTableRow(player, row, state);
            row++;
        }

        playerDataTables_[player]->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
        table->scrollToBottom();
    }

    playerDataTableRowsDirty_ = false;*/
}

// ----------------------------------------------------------------------------
void FrameDataListView::setPlayerDataTableRow(int player, int row, const rfcommon::FighterState& state)
{
    /*
    rfcommon::Session* session = model_->session();
    const auto& statusMapping = session->mappingInfo().fighterStatus;
    const auto& hitStatusMapping = session->mappingInfo().hitStatus;

    const rfcommon::String* baseEnum = statusMapping.statusToBaseEnumName(state.status());
    const rfcommon::String* specificEnum = statusMapping.statusToFighterSpecificEnumName(state.status(), session->fighterID(player));
    const rfcommon::String statusStr = baseEnum ? *baseEnum : (specificEnum ? *specificEnum : "");
    const rfcommon::String* hitStatusStr = hitStatusMapping.map(state.hitStatus());

    // "Frame", "Position", "Facing", "Damage", "Hitstun", "Shield", "Status", "Motion", "Hit Status", "Stocks", "Attack Connected"
    QTableWidget* table = playerDataTables_[player];
    table->setItem(row, 0, new QTableWidgetItem(QString::number(state.framesLeft().value())));
    table->setItem(row, 1, new QTableWidgetItem(QString::number(state.posx()) + ", " + QString::number(state.posy())));
    table->setItem(row, 2, new QTableWidgetItem(state.flags().facingDirection() ? "Right" : "Left"));
    table->setItem(row, 3, new QTableWidgetItem(QString::number(state.damage())));
    table->setItem(row, 4, new QTableWidgetItem(QString::number(state.hitstun())));
    table->setItem(row, 5, new QTableWidgetItem(QString::number(state.shield())));
    table->setItem(row, 6, new QTableWidgetItem("(" + QString::number(state.status().value()) + ") " + statusStr.cStr()));
    table->setItem(row, 7, new QTableWidgetItem("(" + QString::number(state.motion().value()) + ") "));
    table->setItem(row, 8, new QTableWidgetItem("(" + QString::number(state.hitStatus().value()) + ") " + (hitStatusStr ? hitStatusStr->cStr() : "")));
    table->setItem(row, 9, new QTableWidgetItem(QString::number(state.stocks().value())));
    table->setItem(row, 10, new QTableWidgetItem(state.flags().attackConnected() ? "True" : "False"));*/
}

/*
// ----------------------------------------------------------------------------
void FrameDataListView::onRunningSessionNewUniqueFrame(int frameIdx, const rfcommon::Frame& frame)
{
    if (frame.fighterCount() > playerDataTables_.count())
        return;

    // The fact this callback was called means there's new data
    playerDataTableRowsDirty_ = true;

    // Updating this table is really slow on Windows, specifically, resizing
    // the table (adding rows) so had to get a bit creative. This is a problem
    // because we receive state updates 60 times per second, so if any listener
    // is slower than that, the entire UI freezes as it tries to update the
    // ever increasing amount of incoming data.
    const uint64_t time = QDateTime::currentMSecsSinceEpoch();
    const uint64_t leeway = playerDataTablesUpdateTime_ * 2;
    if (ui_->stackedWidget->currentWidget() != ui_->page_playerData
         || time - lastTimePlayerDataTablesUpdated_ < playerDataTablesUpdateTime_ + leeway)
    {
        return;
    }

    if (ui_->stackedWidget->currentWidget() == ui_->page_playerData)
    {
        updatePlayerDataTableRowsIfDirty();

        const uint64_t timeAfterUpdate = QDateTime::currentMSecsSinceEpoch();
        playerDataTablesUpdateTime_ = timeAfterUpdate - time;
        lastTimePlayerDataTablesUpdated_ = timeAfterUpdate;
    }
}*/
