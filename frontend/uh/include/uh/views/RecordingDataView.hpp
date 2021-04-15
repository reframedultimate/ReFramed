#pragma once

#include "uh/listeners/RecordingListener.hpp"
#include <QWidget>
#include <QVector>
#include <QExplicitlySharedDataPointer>

class QTreeWidgetItem;
class QTableWidget;

namespace Ui {
    class RecordingDataView;
}

namespace uh {

class Recording;

class RecordingDataView : public QWidget
                        , public RecordingListener
{
    Q_OBJECT

public:
    explicit RecordingDataView(QWidget* parent=nullptr);
    ~RecordingDataView();

public slots:
    void setRecording(Recording* recording);

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
    void setPlayerDataTableRow(int player, int row, const PlayerState& state);

private:
    void onActiveRecordingPlayerNameChanged(int player, const QString& name) override;
    void onActiveRecordingSetNumberChanged(int number) override;
    void onActiveRecordingGameNumberChanged(int number) override;
    void onActiveRecordingFormatChanged(const SetFormat& format) override;
    void onActiveRecordingNewUniquePlayerState(int player, const PlayerState& state) override;
    void onActiveRecordingNewPlayerState(int player, const PlayerState& state) override;

private:
    Ui::RecordingDataView* ui_;
    QTreeWidgetItem* gameInfoItem_ = nullptr;
    QTreeWidgetItem* stageIDMappingsItem_ = nullptr;
    QTreeWidgetItem* fighterIDMappingsItem_ = nullptr;
    QTreeWidgetItem* baseStatusIDMappingsItem_ = nullptr;
    QTreeWidgetItem* specificStatusIDMappingsItem_ = nullptr;
    QTreeWidgetItem* hitStatusIDMappingsItem_ = nullptr;
    QVector<QTreeWidgetItem*> playerDataItems_;
    QVector<QTableWidget*> playerDataTables_;
    QExplicitlySharedDataPointer<Recording> recording_;
    bool playerDataTableRowsLoaded_ = false;
};

}
