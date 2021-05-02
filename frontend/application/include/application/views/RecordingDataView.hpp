#pragma once

#include "uh/RecordingListener.hpp"
#include "uh/Reference.hpp"
#include <QWidget>
#include <QVector>

class QTreeWidgetItem;
class QTableWidget;

namespace Ui {
    class RecordingDataView;
}

namespace uh {
    class Recording;
}

namespace uhapp {

class RecordingDataView : public QWidget
                        , public uh::RecordingListener
{
    Q_OBJECT

public:
    explicit RecordingDataView(QWidget* parent=nullptr);
    ~RecordingDataView();

public slots:
    void setRecording(uh::Recording* recording);
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
    void onActiveRecordingPlayerNameChanged(int player, const uh::SmallString<15>& name) override;
    void onActiveRecordingSetNumberChanged(uh::SetNumber number) override;
    void onActiveRecordingGameNumberChanged(uh::GameNumber number) override;
    void onActiveRecordingFormatChanged(const uh::SetFormat& format) override;
    void onActiveRecordingNewUniquePlayerState(int player, const uh::PlayerState& state) override;
    void onActiveRecordingNewPlayerState(int player, const uh::PlayerState& state) override;
    void onRecordingWinnerChanged(int winner) override;

private:
    Ui::RecordingDataView* ui_;
    QTreeWidgetItem* gameInfoItem_ = nullptr;
    QTreeWidgetItem* stageIDMappingsItem_ = nullptr;
    QTreeWidgetItem* fighterIDMappingsItem_ = nullptr;
    QTreeWidgetItem* baseStatusIDMappingsItem_ = nullptr;
    QTreeWidgetItem* specificStatusIDMappingsItem_ = nullptr;
    QTreeWidgetItem* hitStatusIDMappingsItem_ = nullptr;
    uh::SmallVector<QTreeWidgetItem*, 8> playerDataItems_;
    uh::SmallVector<QTableWidget*, 8> playerDataTables_;
    uh::Reference<uh::Recording> recording_;

    // When a new recording is set, we want to remember which player was selected so the
    // user doesn't have to keep clicking on the player when browsing recordings
    int storeCurrentPageIndex_ = 0;

    bool playerDataTableRowsDirty_ = true;
    uint64_t lastTimePlayerDataTablesUpdated_ = 0;
    uint64_t playerDataTablesUpdateTime_ = 0;
};

}
