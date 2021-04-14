#include "uh/ui_RecordingView.h"
#include "uh/Util.hpp"
#include "uh/views/RecordingView.hpp"
#include "uh/views/DamagePlot.hpp"
#include "uh/models/Recording.hpp"
#include "uh/models/PlayerState.hpp"

#include <QTreeWidgetItem>

namespace uh {

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
RecordingView::RecordingView(QWidget *parent)
    : QWidget(parent)
    , ui_(new Ui::RecordingView)
    , plot_(new DamagePlot)
    , gameInfo_(new QTreeWidgetItem({"Game Info"}))
    , stageIDMappings_(new QTreeWidgetItem({"Stage IDs"}))
    , fighterIDMappings_(new QTreeWidgetItem({"Fighter IDs"}))
    , baseStatusIDMappings_(new QTreeWidgetItem({"Base IDs"}))
    , specificStatusIDMappings_(new QTreeWidgetItem({"Fighter Specific IDs"}))
{
    ui_->setupUi(this);

    ui_->splitter->setStretchFactor(0, 0);
    ui_->splitter->setStretchFactor(1, 1);

    QTreeWidgetItem* statusMappings = new QTreeWidgetItem({"Status IDs"});
    statusMappings->addChildren({baseStatusIDMappings_, specificStatusIDMappings_});
    QTreeWidgetItem* mappings = new QTreeWidgetItem({"Value Mappings"});
    mappings->addChildren({stageIDMappings_, fighterIDMappings_, statusMappings});
    ui_->treeWidget_data->addTopLevelItem(gameInfo_);
    ui_->treeWidget_data->addTopLevelItem(mappings);
    ui_->treeWidget_data->addTopLevelItem(new QTreeWidgetItem({"Player Data"}));

    connect(ui_->treeWidget_data, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(onCurrentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));

    // This is still broken, see
    // https://www.qtcentre.org/threads/66591-QwtPlot-is-broken-(size-constraints-disregarded)
    QMetaObject::invokeMethod(this, "addDamagePlotToUI", Qt::QueuedConnection);
}

// ----------------------------------------------------------------------------
void RecordingView::addDamagePlotToUI()
{
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(plot_);
    ui_->tab_damage->setLayout(layout);
}

// ----------------------------------------------------------------------------
RecordingView::~RecordingView()
{
    if (recording_)
        recording_->dispatcher.removeListener(this);
    delete ui_;
}

// ----------------------------------------------------------------------------
void RecordingView::setRecording(Recording* recording)
{
    if (recording_)
        recording_->dispatcher.removeListener(this);
    recording_ = recording;

    // Plot existing data
    int playerCount = recording_->playerCount();
    plot_->resetPlot(playerCount);
    for (int i = 0; i != playerCount; ++i)
    {
        plot_->setPlayerName(i, recording_->playerName(i));
        for (const auto& state : recording_->playerStates(i))
            plot_->addPlayerDamageValue(i, state.frame(), state.damage());
    }
    plot_->autoScale();

    int storeCurrentPageIndex = ui_->stackedWidget->currentIndex();

    // Reset tree widget
    ui_->treeWidget_data->clear();
    gameInfo_ = new QTreeWidgetItem({"Game Info"});
    stageIDMappings_ = new QTreeWidgetItem({"Stage IDs"});
    fighterIDMappings_ = new QTreeWidgetItem({"Fighter IDs"});
    baseStatusIDMappings_ = new QTreeWidgetItem({"Base IDs"});
    specificStatusIDMappings_ = new QTreeWidgetItem({"Fighter Specific IDs"});
    QTreeWidgetItem* playerData = new QTreeWidgetItem({"Player Data"});
    QTreeWidgetItem* statusMappings = new QTreeWidgetItem({"Status IDs"});
    statusMappings->addChildren({baseStatusIDMappings_, specificStatusIDMappings_});
    QTreeWidgetItem* mappings = new QTreeWidgetItem({"Value Mappings"});
    mappings->addChildren({stageIDMappings_, fighterIDMappings_, statusMappings});
    ui_->treeWidget_data->addTopLevelItem(gameInfo_);
    ui_->treeWidget_data->addTopLevelItem(mappings);
    ui_->treeWidget_data->addTopLevelItem(playerData);
    mappings->setExpanded(true);
    statusMappings->setExpanded(true);
    playerData->setExpanded(true);

    // Fill in game info data
    ui_->tableWidget_gameInfo->clearContents();
    ui_->tableWidget_gameInfo->setRowCount(5);
    ui_->tableWidget_gameInfo->setItem(0, 0, new QTableWidgetItem("Time Started"));
    ui_->tableWidget_gameInfo->setItem(0, 1, new QTableWidgetItem(recording_->timeStarted().toString()));
    ui_->tableWidget_gameInfo->setItem(1, 0, new QTableWidgetItem("Format"));
    ui_->tableWidget_gameInfo->setItem(1, 1, new QTableWidgetItem(recording_->format().description()));
    ui_->tableWidget_gameInfo->setItem(2, 0, new QTableWidgetItem("Set Number"));
    ui_->tableWidget_gameInfo->setItem(2, 1, new QTableWidgetItem(QString::number(recording_->setNumber())));
    ui_->tableWidget_gameInfo->setItem(3, 0, new QTableWidgetItem("Game Number"));
    ui_->tableWidget_gameInfo->setItem(3, 1, new QTableWidgetItem(QString::number(recording_->gameNumber())));
    const QString* stageName = recording_->mappingInfo().stageID.map(recording_->stageID());
    ui_->tableWidget_gameInfo->setItem(4, 0, new QTableWidgetItem("Stage ID"));
    ui_->tableWidget_gameInfo->setItem(4, 1, new QTableWidgetItem(QString::number(recording_->stageID()) + " (" + (stageName ? *stageName : "unknown stage") + ")"));
    ui_->tableWidget_gameInfo->setColumnWidth(1, 400);

    // Fill in stage mapping info
    const auto& stageMapping = recording_->mappingInfo().stageID.get();
    ui_->tableWidget_stageIDs->clearContents();
    ui_->tableWidget_stageIDs->setRowCount(stageMapping.size());
    int i = 0;
    for (auto it = stageMapping.begin(); it != stageMapping.end(); ++it, ++i)
    {
        ui_->tableWidget_stageIDs->setItem(i, 0, new IntegerTableWidgetItem(it.key()));
        ui_->tableWidget_stageIDs->setItem(i, 1, new QTableWidgetItem(it.value()));
    }
    ui_->tableWidget_stageIDs->setColumnWidth(1, 600);
    ui_->tableWidget_stageIDs->sortByColumn(0, Qt::AscendingOrder);

    // Fill in fighter mapping info
    const auto& fighterMapping = recording_->mappingInfo().fighterID.get();
    ui_->tableWidget_fighterIDs->clearContents();
    ui_->tableWidget_fighterIDs->setRowCount(fighterMapping.size());
    i = 0;
    for (auto it = fighterMapping.begin(); it != fighterMapping.end(); ++it, ++i)
    {
        ui_->tableWidget_fighterIDs->setItem(i, 0, new IntegerTableWidgetItem(it.key()));
        ui_->tableWidget_fighterIDs->setItem(i, 1, new QTableWidgetItem(it.value()));
    }
    ui_->tableWidget_fighterIDs->setColumnWidth(1, 600);
    ui_->tableWidget_fighterIDs->sortByColumn(0, Qt::AscendingOrder);

    // Fill in base status mapping info
    const auto& baseStatusMapping = recording_->mappingInfo().fighterStatus.baseEnumNames();
    ui_->tableWidget_baseStatusIDs->clearContents();
    ui_->tableWidget_baseStatusIDs->setRowCount(baseStatusMapping.size());
    i = 0;
    for (auto it = baseStatusMapping.begin(); it != baseStatusMapping.end(); ++it, ++i)
    {
        ui_->tableWidget_baseStatusIDs->setItem(i, 0, new IntegerTableWidgetItem(it.key()));
        ui_->tableWidget_baseStatusIDs->setItem(i, 1, new QTableWidgetItem(it.value()));
    }
    ui_->tableWidget_baseStatusIDs->setColumnWidth(1, 600);
    ui_->tableWidget_baseStatusIDs->sortByColumn(0, Qt::AscendingOrder);

    // Fill in fighter specific status mapping info
    const auto& specificStatusMappings = recording_->mappingInfo().fighterStatus.fighterSpecificEnumNames();
    ui_->tableWidget_specificStatusIDs->clearContents();
    i = 0;
    for (auto fighter = specificStatusMappings.begin(); fighter != specificStatusMappings.end(); ++fighter)
    {
        ui_->tableWidget_specificStatusIDs->setRowCount(
                    ui_->tableWidget_specificStatusIDs->rowCount() + fighter.value().count());
        for (auto it = fighter.value().begin(); it != fighter.value().end(); ++it, ++i)
        {
            ui_->tableWidget_specificStatusIDs->setItem(i, 0, new DoubleIntegerTableWidgetItem(fighter.key(), it.key()));
            ui_->tableWidget_specificStatusIDs->setItem(i, 1, new IntegerTableWidgetItem(it.key()));
            ui_->tableWidget_specificStatusIDs->setItem(i, 2, new QTableWidgetItem(it.value()));
        }
    }
    ui_->tableWidget_specificStatusIDs->setColumnWidth(2, 600);
    ui_->tableWidget_specificStatusIDs->sortByColumn(0, Qt::AscendingOrder);

    clearStackedWidget(ui_->stackedWidget_playerData);
    playerData_.clear();
    playerDataTable_.clear();
    for (i = 0; i != recording_->playerCount(); ++i)
    {
        const auto& statusMapping = recording_->mappingInfo().fighterStatus;
        const auto& states = recording_->playerStates(i);
        QTableWidget* table = new QTableWidget(states.count(), 7);
        table->setHorizontalHeaderLabels({"Frame", "Damage", "Stocks", "Status ID", "Enum Name", "Short Name", "Custom Name"});
        table->setColumnWidth(4, 600);

        int s = 0;
        for (const auto& state : states)
        {
            const QString* baseEnum = statusMapping.statusToBaseEnumName(state.status());
            const QString* specificEnum = statusMapping.statusToFighterSpecificEnumName(state.status(), recording_->playerFighterID(i));
            table->setItem(s, 0, new QTableWidgetItem(QString::number(state.frame())));
            table->setItem(s, 1, new QTableWidgetItem(QString::number(state.damage())));
            table->setItem(s, 2, new QTableWidgetItem(QString::number(state.stocks())));
            table->setItem(s, 3, new QTableWidgetItem(QString::number(state.status())));
            table->setItem(s, 4, new QTableWidgetItem(baseEnum ? *baseEnum : (specificEnum ? *specificEnum : "")));
            s++;
        }

        playerData_.push_back(new QTreeWidgetItem({recording_->playerName(i)}));
        playerData->addChild(playerData_.back());
        ui_->stackedWidget_playerData->addWidget(table);
        playerDataTable_.push_back(table);
    }

    ui_->stackedWidget->setCurrentIndex(storeCurrentPageIndex);

    recording_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
void RecordingView::onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
    (void)previous;

    if (current == gameInfo_)
        ui_->stackedWidget->setCurrentWidget(ui_->page_gameInfo);
    else if (current == stageIDMappings_)
        ui_->stackedWidget->setCurrentWidget(ui_->page_stageIDs);
    else if (current == fighterIDMappings_)
        ui_->stackedWidget->setCurrentWidget(ui_->page_fighterIDs);
    else if (current == baseStatusIDMappings_)
        ui_->stackedWidget->setCurrentWidget(ui_->page_baseStatusIDs);
    else if (current == specificStatusIDMappings_)
        ui_->stackedWidget->setCurrentWidget(ui_->page_specificStatusIDs);
    else
    {
        int i = 0;
        for (const auto& item : playerData_)
        {
            if (current == item)
            {
                ui_->stackedWidget->setCurrentWidget(ui_->page_playerData);
                ui_->stackedWidget_playerData->setCurrentIndex(i);
                break;
            }
            i++;
        }
    }
}

// ----------------------------------------------------------------------------
void RecordingView::onActiveRecordingPlayerNameChanged(int player, const QString& name)
{
    plot_->setPlayerName(player, name);
}

// ----------------------------------------------------------------------------
void RecordingView::onActiveRecordingSetNumberChanged(int number)
{
    (void)number;
}

// ----------------------------------------------------------------------------
void RecordingView::onActiveRecordingGameNumberChanged(int number)
{
    (void)number;
}

// ----------------------------------------------------------------------------
void RecordingView::onActiveRecordingFormatChanged(const SetFormat& format)
{
    (void)format;
}

// ----------------------------------------------------------------------------
void RecordingView::onActiveRecordingNewUniquePlayerState(int player, const PlayerState& state)
{
    // Append to player data table
    QTableWidget* table = playerDataTable_[player];
    int row = table->rowCount();
    const auto& statusMapping = recording_->mappingInfo().fighterStatus;
    const QString* baseEnum = statusMapping.statusToBaseEnumName(state.status());
    const QString* specificEnum = statusMapping.statusToFighterSpecificEnumName(state.status(), recording_->playerFighterID(player));
    table->setRowCount(row + 1);
    table->setItem(row, 0, new QTableWidgetItem(QString::number(state.frame())));
    table->setItem(row, 1, new QTableWidgetItem(QString::number(state.damage())));
    table->setItem(row, 2, new QTableWidgetItem(QString::number(state.stocks())));
    table->setItem(row, 3, new QTableWidgetItem(QString::number(state.status())));
    table->setItem(row, 4, new QTableWidgetItem(baseEnum ? *baseEnum : (specificEnum ? *specificEnum : "")));
    table->scrollToBottom();
}

// ----------------------------------------------------------------------------
void RecordingView::onActiveRecordingNewPlayerState(int player, const PlayerState& state)
{
    // Update plot
    plot_->addPlayerDamageValue(player, state.frame(), state.damage());
    plot_->replotAndAutoScale();
}

}
