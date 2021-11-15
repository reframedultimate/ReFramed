#pragma once

#include "uh/SessionListener.hpp"
#include "uh/Reference.hpp"
#include <QWidget>
#include <QVector>

class QTreeWidgetItem;
class QTableWidget;

namespace Ui {
    class SessionDataView;
}

namespace uh {
    class Session;
}

namespace uhapp {

class SessionDataView : public QWidget
                      , public uh::SessionListener
{
    Q_OBJECT

public:
    explicit SessionDataView(QWidget* parent=nullptr);
    ~SessionDataView();

public slots:
    void setSession(uh::Session* session);
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
    void setPlayerDataTableRow(int player, int row, const uh::PlayerState& state);

private:
    void onRunningSessionNewUniquePlayerState(int player, const uh::PlayerState& state) override;
    void onRunningSessionNewPlayerState(int player, const uh::PlayerState& state) override;

    void onRunningGameSessionPlayerNameChanged(int player, const uh::SmallString<15>& name) override;
    void onRunningGameSessionSetNumberChanged(uh::SetNumber number) override;
    void onRunningGameSessionGameNumberChanged(uh::GameNumber number) override;
    void onRunningGameSessionFormatChanged(const uh::SetFormat& format) override;
    void onRunningGameSessionWinnerChanged(int winner) override;

private:
    Ui::SessionDataView* ui_;
    QTreeWidgetItem* gameInfoItem_ = nullptr;
    QTreeWidgetItem* stageIDMappingsItem_ = nullptr;
    QTreeWidgetItem* fighterIDMappingsItem_ = nullptr;
    QTreeWidgetItem* baseStatusIDMappingsItem_ = nullptr;
    QTreeWidgetItem* specificStatusIDMappingsItem_ = nullptr;
    QTreeWidgetItem* hitStatusIDMappingsItem_ = nullptr;
    uh::SmallVector<QTreeWidgetItem*, 8> playerDataItems_;
    uh::SmallVector<QTableWidget*, 8> playerDataTables_;
    uh::Reference<uh::Session> session_;

    // When a new recording is set, we want to remember which player was selected so the
    // user doesn't have to keep clicking on the player when browsing recordings
    int storeCurrentPageIndex_ = 0;

    bool playerDataTableRowsDirty_ = true;
    uint64_t lastTimePlayerDataTablesUpdated_ = 0;
    uint64_t playerDataTablesUpdateTime_ = 0;
};

}
