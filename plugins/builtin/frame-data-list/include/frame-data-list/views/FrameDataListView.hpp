#pragma once

#include "frame-data-list/listeners/FrameDataListListener.hpp"
#include "rfcommon/SessionListener.hpp"
#include "rfcommon/Vector.hpp"
#include <QWidget>

class FrameDataListModel;
class QTreeWidgetItem;
class QTableWidget;

namespace Ui {
    class FrameDataListView;
}

class FrameDataListView : public QWidget
                        , public FrameDataListListener
                        , public rfcommon::SessionListener
{
    Q_OBJECT

public:
    explicit FrameDataListView(FrameDataListModel* model, QWidget* parent=nullptr);
    ~FrameDataListView();

private slots:
    void onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);

private:
    void clearUI();
    void repopulateTree();
    void repopulateGameInfoTable();
    void repopulateStageMappingTable();
    void repopulateFighterMappingTable();
    void repopulateStatusMappingTable();
    void repopulatePlayerDataTables();
    void repopulateHitStatusMappingTable();

    void updatePlayerDataTableRowsIfDirty();
    void setPlayerDataTableRow(int playerIdx, int row, const rfcommon::PlayerState& state);

private:
    void onFrameDataListSessionSet(rfcommon::Session* session) override;
    void onFrameDataListSessionCleared(rfcommon::Session* session) override;

private:
    void onRunningGameSessionPlayerNameChanged(int playerIdx, const rfcommon::SmallString<15>& name) override;
    void onRunningGameSessionSetNumberChanged(rfcommon::SetNumber number) override;
    void onRunningGameSessionGameNumberChanged(rfcommon::GameNumber number) override;
    void onRunningGameSessionFormatChanged(const rfcommon::SetFormat& format) override;
    void onRunningGameSessionWinnerChanged(int winnerPlayerIdx) override;

    // RunningSession events
    void onRunningSessionNewUniquePlayerState(int playerIdx, const rfcommon::PlayerState& state) override;
    void onRunningSessionNewPlayerState(int playerIdx, const rfcommon::PlayerState& state) override;
    void onRunningSessionNewUniqueFrame(const rfcommon::SmallVector<rfcommon::PlayerState, 8>& states) override;
    void onRunningSessionNewFrame(const rfcommon::SmallVector<rfcommon::PlayerState, 8>& states) override;

private:
    FrameDataListModel* model_;
    Ui::FrameDataListView* ui_;
    QTreeWidgetItem* gameInfoItem_ = nullptr;
    QTreeWidgetItem* stageIDMappingsItem_ = nullptr;
    QTreeWidgetItem* fighterIDMappingsItem_ = nullptr;
    QTreeWidgetItem* baseStatusIDMappingsItem_ = nullptr;
    QTreeWidgetItem* specificStatusIDMappingsItem_ = nullptr;
    QTreeWidgetItem* hitStatusIDMappingsItem_ = nullptr;
    rfcommon::SmallVector<QTreeWidgetItem*, 8> playerDataItems_;
    rfcommon::SmallVector<QTableWidget*, 8> playerDataTables_;

    // When a new recording is set, we want to remember which player was selected so the
    // user doesn't have to keep clicking on the player when browsing recordings
    int storeCurrentPageIndex_ = 0;

    bool playerDataTableRowsDirty_ = true;
    uint64_t lastTimePlayerDataTablesUpdated_ = 0;
    uint64_t playerDataTablesUpdateTime_ = 0;
};
