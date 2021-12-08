#pragma once

#include "rfcommon/SessionListener.hpp"
#include "rfcommon/Reference.hpp"
#include <QWidget>
#include <QVector>

class QTreeWidgetItem;
class QTableWidget;

namespace Ui {
    class SessionDataView;
}

namespace rfcommon {
    class Session;
}

namespace rfapp {

class SessionDataView : public QWidget
                      , public rfcommon::SessionListener
{
    Q_OBJECT

public:
    explicit SessionDataView(QWidget* parent=nullptr);
    ~SessionDataView();

public slots:
    void setSession(rfcommon::Session* session);
    void clear();

private slots:
    void onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);

private:
    void repopulateTree();
    void repopulateGameInfoTable();
    void repopulateStageMappingTable();
    void repopulateFighterMappingTable();
    void repopulateStatusMappingTable();
    void repopulatePlayerDataTables();
    void repopulateHitStatusMappingTable();

    void updatePlayerDataTableRowsIfDirty();
    void setPlayerDataTableRow(int player, int row, const rfcommon::PlayerState& state);

private:
    void onRunningSessionNewUniquePlayerState(int player, const rfcommon::PlayerState& state) override;
    void onRunningSessionNewPlayerState(int player, const rfcommon::PlayerState& state) override;

    void onRunningGameSessionPlayerNameChanged(int player, const rfcommon::SmallString<15>& name) override;
    void onRunningGameSessionSetNumberChanged(rfcommon::SetNumber number) override;
    void onRunningGameSessionGameNumberChanged(rfcommon::GameNumber number) override;
    void onRunningGameSessionFormatChanged(const rfcommon::SetFormat& format) override;
    void onRunningGameSessionWinnerChanged(int winner) override;

    void onRunningTrainingSessionTrainingReset() override;

private:
    Ui::SessionDataView* ui_;
    QTreeWidgetItem* gameInfoItem_ = nullptr;
    QTreeWidgetItem* stageIDMappingsItem_ = nullptr;
    QTreeWidgetItem* fighterIDMappingsItem_ = nullptr;
    QTreeWidgetItem* baseStatusIDMappingsItem_ = nullptr;
    QTreeWidgetItem* specificStatusIDMappingsItem_ = nullptr;
    QTreeWidgetItem* hitStatusIDMappingsItem_ = nullptr;
    rfcommon::SmallVector<QTreeWidgetItem*, 8> playerDataItems_;
    rfcommon::SmallVector<QTableWidget*, 8> playerDataTables_;
    rfcommon::Reference<rfcommon::Session> session_;

    // When a new recording is set, we want to remember which player was selected so the
    // user doesn't have to keep clicking on the player when browsing recordings
    int storeCurrentPageIndex_ = 0;

    bool playerDataTableRowsDirty_ = true;
    uint64_t lastTimePlayerDataTablesUpdated_ = 0;
    uint64_t playerDataTablesUpdateTime_ = 0;
};

}
