#include "application/ui_SessionDataView.h"
#include "application/Util.hpp"
#include "application/views/SessionDataView.hpp"
#include "rfcommon/GameSession.hpp"
#include "rfcommon/PlayerState.hpp"
#include "rfcommon/SavedGameSession.hpp"
#include <QDateTime>

#include <QDebug>

namespace rfapp {

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
SessionDataView::SessionDataView(QWidget* parent)
    : QWidget(parent)
    , ui_(new Ui::SessionDataView)
{
    ui_->setupUi(this);
    ui_->splitter->setStretchFactor(0, 0);
    ui_->splitter->setStretchFactor(1, 1);

    connect(ui_->treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            this, SLOT(onCurrentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
}

// ----------------------------------------------------------------------------
SessionDataView::~SessionDataView()
{
    if (session_)
        session_->dispatcher.removeListener(this);
    delete ui_;
}

// ----------------------------------------------------------------------------
void SessionDataView::setSavedGameSession(rfcommon::SavedGameSession* session)
{
    assert(session_.isNull());
    session_ = session;

    int storeCurrentPageIndex = ui_->stackedWidget->currentIndex();

    repopulateTree();
    repopulateGameInfoTable();
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

    session_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
void SessionDataView::clearSavedGameSession(rfcommon::SavedGameSession* session)
{
    assert(session_ == session && session_.notNull());

    session_->dispatcher.removeListener(this);
    session_.drop();

    storeCurrentPageIndex_ = ui_->stackedWidget_playerData->currentIndex();

    ui_->treeWidget->clear();
    ui_->tableWidget_gameInfo->clearContents();
    ui_->tableWidget_gameInfo->setRowCount(0);
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
void SessionDataView::onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
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
                updatePlayerDataTableRowsIfDirty();
                break;
            }
            i++;
        }
    }
}

// ----------------------------------------------------------------------------
void SessionDataView::repopulateTree()
{
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
    for (int i = 0; i != session_->playerCount(); ++i)
    {
        QTreeWidgetItem* player = new QTreeWidgetItem({session_->playerName(i).cStr()});
        playerDataItems_.push(player);
        playerStates->addChild(player);
        player->setExpanded(true);
    }

    mappings->setExpanded(true);
    statusMappings->setExpanded(true);
    playerStates->setExpanded(true);
}

// ----------------------------------------------------------------------------
void SessionDataView::repopulateGameInfoTable()
{
    rfcommon::GameSession* game = dynamic_cast<rfcommon::GameSession*>(session_.get());

    // Fill in data
    ui_->tableWidget_gameInfo->setRowCount(6 + session_->playerCount());
    ui_->tableWidget_gameInfo->setItem(0, 0, new QTableWidgetItem("Time Started"));
    ui_->tableWidget_gameInfo->setItem(0, 1, new QTableWidgetItem(QDateTime::fromMSecsSinceEpoch(session_->timeStampStartedMs()).toString()));
    ui_->tableWidget_gameInfo->setItem(1, 0, new QTableWidgetItem("Format"));
    ui_->tableWidget_gameInfo->setItem(1, 1, new QTableWidgetItem(game ? game ->format().description().cStr() : "--"));
    ui_->tableWidget_gameInfo->setItem(2, 0, new QTableWidgetItem("Set Number"));
    ui_->tableWidget_gameInfo->setItem(2, 1, new QTableWidgetItem(game ? QString::number(game->setNumber()) : "--"));
    ui_->tableWidget_gameInfo->setItem(3, 0, new QTableWidgetItem("Game Number"));
    ui_->tableWidget_gameInfo->setItem(3, 1, new QTableWidgetItem(game ? QString::number(game->gameNumber()) : "--"));
    const rfcommon::String* stageName = session_->mappingInfo().stageID.map(session_->stageID());
    ui_->tableWidget_gameInfo->setItem(4, 0, new QTableWidgetItem("Stage ID"));
    ui_->tableWidget_gameInfo->setItem(4, 1, new QTableWidgetItem(QString::number(session_->stageID()) + " (" + (stageName ? stageName->cStr() : "unknown stage") + ")"));
    ui_->tableWidget_gameInfo->setItem(5, 0, new QTableWidgetItem("Winner"));
    ui_->tableWidget_gameInfo->setItem(5, 1, new QTableWidgetItem(session_->playerName(session_->winner()).cStr()));

    for (int i = 0; i != session_->playerCount(); ++i)
    {
        const rfcommon::String* fighterName = session_->mappingInfo().fighterID.map(session_->playerFighterID(i));
        ui_->tableWidget_gameInfo->setItem(6+i, 0, new QTableWidgetItem(session_->playerName(i).cStr()));
        ui_->tableWidget_gameInfo->setItem(6+i, 1, new QTableWidgetItem(fighterName ? fighterName->cStr() : "(Unknown fighter)"));
    }

    ui_->tableWidget_gameInfo->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

// ----------------------------------------------------------------------------
void SessionDataView::repopulateStageMappingTable()
{
    // Fill in data
    int i = 0;
    const auto& stageMapping = session_->mappingInfo().stageID.get();
    ui_->tableWidget_stageIDs->setRowCount(stageMapping.count());
    for (const auto& it : stageMapping)
    {
        ui_->tableWidget_stageIDs->setItem(i, 0, new IntegerTableWidgetItem(it->key()));
        ui_->tableWidget_stageIDs->setItem(i, 1, new QTableWidgetItem(it->value().cStr()));
        i++;
    }
    ui_->tableWidget_stageIDs->sortByColumn(0, Qt::AscendingOrder);
    ui_->tableWidget_stageIDs->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

// ----------------------------------------------------------------------------
void SessionDataView::repopulateFighterMappingTable()
{
    // Fill in data
    int i = 0;
    const auto& fighterMapping = session_->mappingInfo().fighterID.get();
    ui_->tableWidget_fighterIDs->setRowCount(fighterMapping.count());
    for (const auto& it : fighterMapping)
    {
        ui_->tableWidget_fighterIDs->setItem(i, 0, new IntegerTableWidgetItem(it->key()));
        ui_->tableWidget_fighterIDs->setItem(i, 1, new QTableWidgetItem(it->value().cStr()));
        ++i;
    }
    ui_->tableWidget_fighterIDs->sortByColumn(0, Qt::AscendingOrder);
    ui_->tableWidget_fighterIDs->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

// ----------------------------------------------------------------------------
void SessionDataView::repopulateStatusMappingTable()
{
    // Fill in base status mapping info
    int i = 0;
    const auto& baseStatusMapping = session_->mappingInfo().fighterStatus.baseEnumNames();
    const auto& specificStatusMappings = session_->mappingInfo().fighterStatus.fighterSpecificEnumNames();
    ui_->tableWidget_baseStatusIDs->setRowCount(baseStatusMapping.count());
    for (const auto& it : baseStatusMapping)
    {
        ui_->tableWidget_baseStatusIDs->setItem(i, 0, new IntegerTableWidgetItem(it.key()));
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
            ui_->tableWidget_specificStatusIDs->setItem(i, 0, new DoubleIntegerTableWidgetItem(fighter.key(), it.key()));
            ui_->tableWidget_specificStatusIDs->setItem(i, 1, new IntegerTableWidgetItem(it.key()));
            ui_->tableWidget_specificStatusIDs->setItem(i, 2, new QTableWidgetItem(it.value().cStr()));
            ++i;
        }
    }

    ui_->tableWidget_baseStatusIDs->sortByColumn(0, Qt::AscendingOrder);
    ui_->tableWidget_specificStatusIDs->sortByColumn(0, Qt::AscendingOrder);

    ui_->tableWidget_baseStatusIDs->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    ui_->tableWidget_specificStatusIDs->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

// ----------------------------------------------------------------------------
void SessionDataView::repopulateHitStatusMappingTable()
{
    // Fill in data
    int i = 0;
    const auto& hitStatusMapping = session_->mappingInfo().hitStatus.get();
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
void SessionDataView::repopulatePlayerDataTables()
{
    // Fill in data
    for (int player = 0; player != session_->playerCount(); ++player)
    {
        QTableWidget* table = new QTableWidget(0, 11);
        playerDataTables_.push(table);
        ui_->stackedWidget_playerData->addWidget(table);
        table->setHorizontalHeaderLabels({"Frame", "Position", "Facing", "Damage", "Hitstun", "Shield", "Status", "Motion", "Hit Status", "Stocks", "Attack Connected"});
    }

    // Since we cleared the stacked widget, it won't remember which one was
    // selected. storeCurrentPageIndex_ is saved in clear() and we try to
    // select the same player index again
    if (storeCurrentPageIndex_ >= session_->playerCount())
        ui_->stackedWidget_playerData->setCurrentIndex(0);
    else
        ui_->stackedWidget_playerData->setCurrentIndex(storeCurrentPageIndex_);

    // Defer populating tables because there's a lot of data and it takes half
    // a second or so
    playerDataTableRowsDirty_ = true;
}

// ----------------------------------------------------------------------------
void SessionDataView::updatePlayerDataTableRowsIfDirty()
{
    if (playerDataTableRowsDirty_ == false)
        return;

    qDebug() << "Updating player data tables";

    for (int player = 0; player != session_->playerCount(); ++player)
    {
        QTableWidget* table = playerDataTables_[player];

        // This method can be called multiple times if the active recording is
        // adding new player states. On Windows, growing the table is really
        // slow, so it gets called once every few seconds. Figure out how many
        // rows are already populated and how many need to be added using new
        // data
        int row = table->rowCount();
        if (table->rowCount() < session_->playerStateCount(player))
            table->setRowCount(session_->playerStateCount(player));
        int endRow = table->rowCount();

        while (row < endRow)
        {
            const rfcommon::PlayerState& state = session_->playerStateAt(player, row);
            setPlayerDataTableRow(player, row, state);
            row++;
        }

        playerDataTables_[player]->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
        table->scrollToBottom();
    }

    playerDataTableRowsDirty_ = false;
}

// ----------------------------------------------------------------------------
void SessionDataView::setPlayerDataTableRow(int player, int row, const rfcommon::PlayerState& state)
{
    const auto& statusMapping = session_->mappingInfo().fighterStatus;
    const auto& hitStatusMapping = session_->mappingInfo().hitStatus;

    const rfcommon::String* baseEnum = statusMapping.statusToBaseEnumName(state.status());
    const rfcommon::String* specificEnum = statusMapping.statusToFighterSpecificEnumName(state.status(), session_->playerFighterID(player));
    const rfcommon::String statusStr = baseEnum ? *baseEnum : (specificEnum ? *specificEnum : "");
    const rfcommon::String* hitStatusStr = hitStatusMapping.map(state.hitStatus());

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
void SessionDataView::onRunningGameSessionPlayerNameChanged(int player, const rfcommon::SmallString<15>& name)
{
    if (player >= playerDataItems_.count())
        return;
    playerDataItems_[player]->setText(0, name.cStr());

    for (int i = 0; i != session_->playerCount(); ++i)
        ui_->tableWidget_gameInfo->item(6+i, 0)->setText(session_->playerName(i).cStr());
}

// ----------------------------------------------------------------------------
void SessionDataView::onRunningGameSessionSetNumberChanged(rfcommon::SetNumber number)
{
    ui_->tableWidget_gameInfo->item(2, 1)->setText(QString::number(number));
}

// ----------------------------------------------------------------------------
void SessionDataView::onRunningGameSessionGameNumberChanged(rfcommon::GameNumber number)
{
    ui_->tableWidget_gameInfo->item(3, 1)->setText(QString::number(number));
}

// ----------------------------------------------------------------------------
void SessionDataView::onRunningGameSessionFormatChanged(const rfcommon::SetFormat& format)
{
    ui_->tableWidget_gameInfo->item(1, 1)->setText(format.description().cStr());
}

// ----------------------------------------------------------------------------
void SessionDataView::onRunningSessionNewUniquePlayerState(int player, const rfcommon::PlayerState& state)
{
    if (player >= playerDataTables_.count())
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
     || ui_->stackedWidget_playerData->currentIndex() != player
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

    QTableWidget* table = playerDataTables_[player];
    int row = table->rowCount();
    table->setRowCount(row + 1);
    setPlayerDataTableRow(player, row, state);
    table->scrollToBottom();
}

// ----------------------------------------------------------------------------
void SessionDataView::onRunningSessionNewPlayerState(int player, const rfcommon::PlayerState& state)
{
    (void)player;
    (void)state;
}

// ----------------------------------------------------------------------------
void SessionDataView::onRunningGameSessionWinnerChanged(int winner)
{
    ui_->tableWidget_gameInfo->item(5, 1)->setText(session_->playerName(winner).cStr());
}

}
