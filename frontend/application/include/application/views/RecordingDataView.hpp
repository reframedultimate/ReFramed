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

private slots:
    void onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);

private:
    void repopulateTree();
    void repopulateGameInfoTable();
    void repopulateStageMappingTable();
    void repopulateFighterMappingTable();
    void repopulateStatusMappingTable();
    void repopulatePlayerDataTables();
    void ensurePlayerDataTablesPopulated();
    void setPlayerDataTableRow(int player, int row, const uh::PlayerState& state);

private:
    void onActiveRecordingPlayerNameChanged(int player, const std::string& name) override;
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
    std::vector<QTreeWidgetItem*> playerDataItems_;
    std::vector<QTableWidget*> playerDataTables_;
    uh::Reference<uh::Recording> recording_;
    bool playerDataTableRowsLoaded_ = false;
};

}
