#include "application/ui_RecordingDataView.h"
#include "application/Util.hpp"
#include "application/views/RecordingDataView.hpp"
#include "uh/Recording.hpp"
#include "uh/PlayerState.hpp"
#include <QDateTime>

namespace uhapp {

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

// ----------------------------------------------------------------------------
RecordingDataView::RecordingDataView(QWidget* parent)
    : QWidget(parent)
    , ui_(new Ui::RecordingDataView)
{
    ui_->setupUi(this);
    ui_->splitter->setStretchFactor(0, 0);
    ui_->splitter->setStretchFactor(1, 1);

    connect(ui_->treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            this, SLOT(onCurrentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
}

// ----------------------------------------------------------------------------
RecordingDataView::~RecordingDataView()
{
    if (recording_)
        recording_->dispatcher.removeListener(this);
    delete ui_;
}

// ----------------------------------------------------------------------------
void RecordingDataView::setRecording(uh::Recording* recording)
{
    if (recording_)
        recording_->dispatcher.removeListener(this);
    recording_ = recording;

    int storeCurrentPageIndex = ui_->stackedWidget->currentIndex();

    repopulateTree();
    repopulateGameInfoTable();
    repopulateStageMappingTable();
    repopulateFighterMappingTable();
    repopulateStatusMappingTable();
    repopulateHitStatusMappingTable();
    repopulatePlayerDataTables();

    ui_->stackedWidget->setCurrentIndex(storeCurrentPageIndex);

    if (ui_->stackedWidget->currentWidget() == ui_->page_playerData)
        ensurePlayerDataTablesPopulated();

    recording_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
void RecordingDataView::onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
    (void)previous;

    if (current == gameInfoItem_)
        ui_->stackedWidget->setCurrentWidget(ui_->page_gameInfo);
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
                ensurePlayerDataTablesPopulated();
                break;
            }
            i++;
        }
    }
}

// ----------------------------------------------------------------------------
void RecordingDataView::repopulateTree()
{
    ui_->treeWidget->clear();
    playerDataItems_.clear();

    // Game info
    gameInfoItem_ = new QTreeWidgetItem({"Game Info"});
    ui_->treeWidget->addTopLevelItem(gameInfoItem_);

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

    // Player states
    QTreeWidgetItem* playerStates = new QTreeWidgetItem({"Player States"});
    ui_->treeWidget->addTopLevelItem(playerStates);
    for (int i = 0; i != recording_->playerCount(); ++i)
    {
        QTreeWidgetItem* player = new QTreeWidgetItem({recording_->playerName(i).cStr()});
        playerDataItems_.push(player);
        playerStates->addChild(player);
        player->setExpanded(true);
    }

    mappings->setExpanded(true);
    statusMappings->setExpanded(true);
    playerStates->setExpanded(true);
}

// ----------------------------------------------------------------------------
void RecordingDataView::repopulateGameInfoTable()
{
    // Clear
    ui_->tableWidget_gameInfo->clearContents();

    // Fill in data
    ui_->tableWidget_gameInfo->setRowCount(6 + recording_->playerCount());
    ui_->tableWidget_gameInfo->setItem(0, 0, new QTableWidgetItem("Time Started"));
    ui_->tableWidget_gameInfo->setItem(0, 1, new QTableWidgetItem(QDateTime::fromMSecsSinceEpoch(recording_->timeStampStartedMs()).toString()));
    ui_->tableWidget_gameInfo->setItem(1, 0, new QTableWidgetItem("Format"));
    ui_->tableWidget_gameInfo->setItem(1, 1, new QTableWidgetItem(QString::fromStdString(recording_->format().description())));
    ui_->tableWidget_gameInfo->setItem(2, 0, new QTableWidgetItem("Set Number"));
    ui_->tableWidget_gameInfo->setItem(2, 1, new QTableWidgetItem(QString::number(recording_->setNumber())));
    ui_->tableWidget_gameInfo->setItem(3, 0, new QTableWidgetItem("Game Number"));
    ui_->tableWidget_gameInfo->setItem(3, 1, new QTableWidgetItem(QString::number(recording_->gameNumber())));
    const uh::String* stageName = recording_->mappingInfo().stageID.map(recording_->stageID());
    ui_->tableWidget_gameInfo->setItem(4, 0, new QTableWidgetItem("Stage ID"));
    ui_->tableWidget_gameInfo->setItem(4, 1, new QTableWidgetItem(QString::number(recording_->stageID()) + " (" + (stageName ? stageName->cStr() : "unknown stage") + ")"));
    ui_->tableWidget_gameInfo->setItem(5, 0, new QTableWidgetItem("Winner"));
    ui_->tableWidget_gameInfo->setItem(5, 1, new QTableWidgetItem(recording_->playerName(recording_->winner()).cStr()));

    for (int i = 0; i != recording_->playerCount(); ++i)
    {
        const uh::String* fighterName = recording_->mappingInfo().fighterID.map(recording_->playerFighterID(i));
        ui_->tableWidget_gameInfo->setItem(6+i, 0, new QTableWidgetItem(recording_->playerName(i).cStr()));
        ui_->tableWidget_gameInfo->setItem(6+i, 1, new QTableWidgetItem(fighterName ? fighterName->cStr() : "(Unknown fighter)"));
    }

    ui_->tableWidget_gameInfo->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

// ----------------------------------------------------------------------------
void RecordingDataView::repopulateStageMappingTable()
{
    // Clear
    const auto& stageMapping = recording_->mappingInfo().stageID.get();
    ui_->tableWidget_stageIDs->clearContents();

    // Fill in data
    int i = 0;
    ui_->tableWidget_stageIDs->setRowCount(stageMapping.size());
    for (auto it = stageMapping.begin(); it != stageMapping.end(); ++it, ++i)
    {
        ui_->tableWidget_stageIDs->setItem(i, 0, new IntegerTableWidgetItem(it->first));
        ui_->tableWidget_stageIDs->setItem(i, 1, new QTableWidgetItem(it->second.cStr()));
    }
    ui_->tableWidget_stageIDs->sortByColumn(0, Qt::AscendingOrder);
    ui_->tableWidget_stageIDs->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

// ----------------------------------------------------------------------------
void RecordingDataView::repopulateFighterMappingTable()
{
    // Clear
    const auto& fighterMapping = recording_->mappingInfo().fighterID.get();
    ui_->tableWidget_fighterIDs->clearContents();

    // Fill in data
    int i = 0;
    ui_->tableWidget_fighterIDs->setRowCount(fighterMapping.size());
    for (const auto& it : fighterMapping)
    {
        ui_->tableWidget_fighterIDs->setItem(i, 0, new IntegerTableWidgetItem(it.first));
        ui_->tableWidget_fighterIDs->setItem(i, 1, new QTableWidgetItem(it.second.cStr()));
        ++i;
    }
    ui_->tableWidget_fighterIDs->sortByColumn(0, Qt::AscendingOrder);
    ui_->tableWidget_fighterIDs->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

// ----------------------------------------------------------------------------
void RecordingDataView::repopulateStatusMappingTable()
{
    // Clear
    const auto& baseStatusMapping = recording_->mappingInfo().fighterStatus.baseEnumNames();
    const auto& specificStatusMappings = recording_->mappingInfo().fighterStatus.fighterSpecificEnumNames();
    ui_->tableWidget_baseStatusIDs->clearContents();
    ui_->tableWidget_specificStatusIDs->clearContents();

    // Fill in base status mapping info
    int i = 0;
    ui_->tableWidget_baseStatusIDs->setRowCount(baseStatusMapping.size());
    for (const auto& it : baseStatusMapping)
    {
        ui_->tableWidget_baseStatusIDs->setItem(i, 0, new IntegerTableWidgetItem(it.first));
        ui_->tableWidget_baseStatusIDs->setItem(i, 1, new QTableWidgetItem(it.second.cStr()));
        ++i;
    }

    // Fill in fighter specific status mapping info
    i = 0;
    for (const auto& fighter : specificStatusMappings)
    {
        ui_->tableWidget_specificStatusIDs->setRowCount(
                    ui_->tableWidget_specificStatusIDs->rowCount() + fighter.second.size());
        for (const auto& it : fighter.second)
        {
            ui_->tableWidget_specificStatusIDs->setItem(i, 0, new DoubleIntegerTableWidgetItem(fighter.first, it.first));
            ui_->tableWidget_specificStatusIDs->setItem(i, 1, new IntegerTableWidgetItem(it.first));
            ui_->tableWidget_specificStatusIDs->setItem(i, 2, new QTableWidgetItem(it.second.cStr()));
            ++i;
        }
    }

    ui_->tableWidget_baseStatusIDs->sortByColumn(0, Qt::AscendingOrder);
    ui_->tableWidget_specificStatusIDs->sortByColumn(0, Qt::AscendingOrder);

    ui_->tableWidget_baseStatusIDs->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    ui_->tableWidget_specificStatusIDs->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

// ----------------------------------------------------------------------------
void RecordingDataView::repopulateHitStatusMappingTable()
{
    // Clear
    const auto& hitStatusMapping = recording_->mappingInfo().hitStatus.get();
    ui_->tableWidget_hitStatusIDs->clearContents();

    // Fill in data
    int i = 0;
    ui_->tableWidget_hitStatusIDs->setRowCount(hitStatusMapping.count());
    for (const auto& it : hitStatusMapping)
    {
        ui_->tableWidget_hitStatusIDs->setItem(i, 0, new IntegerTableWidgetItem(it.key()));
        ui_->tableWidget_hitStatusIDs->setItem(i, 1, new QTableWidgetItem(it.value().cStr()));
        ++i;
    }
    ui_->tableWidget_hitStatusIDs->sortByColumn(0, Qt::AscendingOrder);
    ui_->tableWidget_hitStatusIDs->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

// ----------------------------------------------------------------------------
void RecordingDataView::repopulatePlayerDataTables()
{
    int storeCurrentPageIndex = ui_->stackedWidget_playerData->currentIndex();

    // Clear
    clearStackedWidget(ui_->stackedWidget_playerData);
    playerDataTables_.clear();

    // Fill in data
    for (int player = 0; player != recording_->playerCount(); ++player)
    {
        QTableWidget* table = new QTableWidget(recording_->playerStateCount(player), 11);
        playerDataTables_.push(table);
        ui_->stackedWidget_playerData->addWidget(table);
        table->setHorizontalHeaderLabels({"Frame", "Position", "Facing", "Damage", "Hitstun", "Shield", "Status", "Motion", "Hit Status", "Stocks", "Attack Connected"});
    }

    if (storeCurrentPageIndex >= recording_->playerCount())
        ui_->stackedWidget_playerData->setCurrentIndex(0);
    else
        ui_->stackedWidget_playerData->setCurrentIndex(storeCurrentPageIndex);

    // Defer populating tables because there's a lot of data and it takes half
    // a second or so
    playerDataTableRowsLoaded_ = false;
}

// ----------------------------------------------------------------------------
void RecordingDataView::ensurePlayerDataTablesPopulated()
{
    if (playerDataTableRowsLoaded_ == true)
        return;

    for (int player = 0; player != recording_->playerCount(); ++player)
    {
        int row = 0;
        for (int i = 0; i < recording_->playerStateCount(player); ++i)
        {
            setPlayerDataTableRow(player, row, recording_->playerStateAt(player, i));
            row++;
        }
        playerDataTables_[player]->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    }

    playerDataTableRowsLoaded_ = true;
}

// ----------------------------------------------------------------------------
void RecordingDataView::setPlayerDataTableRow(int player, int row, const uh::PlayerState& state)
{
    const auto& statusMapping = recording_->mappingInfo().fighterStatus;
    const auto& hitStatusMapping = recording_->mappingInfo().hitStatus;

    const uh::String* baseEnum = statusMapping.statusToBaseEnumName(state.status());
    const uh::String* specificEnum = statusMapping.statusToFighterSpecificEnumName(state.status(), recording_->playerFighterID(player));
    const uh::String statusStr = baseEnum ? *baseEnum : (specificEnum ? *specificEnum : "");
    const uh::String* hitStatusStr = hitStatusMapping.map(state.hitStatus());

    // "Frame", "Position", "Facing", "Damage", "Hitstun", "Shield", "Status", "Motion", "Hit Status", "Stocks", "Attack Connected"
    QTableWidget* table = playerDataTables_[player];
    table->setItem(row, 0, new QTableWidgetItem(QString::number(state.frame())));
    table->setItem(row, 1, new QTableWidgetItem(QString::number(state.posx()) + ", " + QString::number(state.posy())));
    table->setItem(row, 2, new QTableWidgetItem(state.facingDirection() ? "Left" : "Right"));
    table->setItem(row, 3, new QTableWidgetItem(QString::number(state.damage())));
    table->setItem(row, 4, new QTableWidgetItem(QString::number(state.hitstun())));
    table->setItem(row, 5, new QTableWidgetItem(QString::number(state.shield())));
    table->setItem(row, 6, new QTableWidgetItem("(" + QString::number(state.status()) + ") " + statusStr.cStr()));
    table->setItem(row, 7, new QTableWidgetItem("(" + QString::number(state.motion()) + ") "));
    table->setItem(row, 8, new QTableWidgetItem("(" + QString::number(state.hitStatus()) + ") " + (hitStatusStr ? hitStatusStr->cStr() : "")));
    table->setItem(row, 9, new QTableWidgetItem(QString::number(state.stocks())));
    table->setItem(row, 10, new QTableWidgetItem(state.attackConnected() ? "True" : "False"));
}

// ----------------------------------------------------------------------------
void RecordingDataView::onActiveRecordingPlayerNameChanged(int player, const uh::SmallString<15>& name)
{
    if (player >= playerDataItems_.count())
        return;
    playerDataItems_[player]->setText(0, name.cStr());

    for (int i = 0; i != recording_->playerCount(); ++i)
        ui_->tableWidget_gameInfo->item(6+i, 0)->setText(recording_->playerName(i).cStr());
}

// ----------------------------------------------------------------------------
void RecordingDataView::onActiveRecordingSetNumberChanged(uh::SetNumber number)
{
    ui_->tableWidget_gameInfo->item(2, 1)->setText(QString::number(number));
}

// ----------------------------------------------------------------------------
void RecordingDataView::onActiveRecordingGameNumberChanged(uh::GameNumber number)
{
    ui_->tableWidget_gameInfo->item(3, 1)->setText(QString::number(number));
}

// ----------------------------------------------------------------------------
void RecordingDataView::onActiveRecordingFormatChanged(const uh::SetFormat& format)
{
    ui_->tableWidget_gameInfo->item(1, 1)->setText(QString::fromStdString(format.description()));
}

// ----------------------------------------------------------------------------
void RecordingDataView::onActiveRecordingNewUniquePlayerState(int player, const uh::PlayerState& state)
{
    if (player >= playerDataTables_.count())
        return;

    ensurePlayerDataTablesPopulated();

    QTableWidget* table = playerDataTables_[player];
    int row = table->rowCount();
    table->setRowCount(row + 1);
    setPlayerDataTableRow(player, row, state);
    table->scrollToBottom();
}

// ----------------------------------------------------------------------------
void RecordingDataView::onActiveRecordingNewPlayerState(int player, const uh::PlayerState& state)
{
    (void)player;
    (void)state;
}

// ----------------------------------------------------------------------------
void RecordingDataView::onRecordingWinnerChanged(int winner)
{
    ui_->tableWidget_gameInfo->item(5, 1)->setText(recording_->playerName(winner).cStr());
}

}
