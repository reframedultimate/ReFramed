#pragma once

#include "frame-data-list/listeners/FrameDataListListener.hpp"
#include "rfcommon/Vector.hpp"
#include <QWidget>

class FrameDataListModel;
class QTreeWidgetItem;
class QTableWidget;

namespace Ui {
    class FrameDataListView;
}

namespace rfcommon {
    class FighterState;
}

class FrameDataListView : public QWidget
                        , public FrameDataListListener
{
    Q_OBJECT

public:
    explicit FrameDataListView(FrameDataListModel* model, QWidget* parent=nullptr);
    ~FrameDataListView();

private slots:
    void onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);

private:
    void clearUI();
    void repopulateTree(rfcommon::SessionMetaData* meta, rfcommon::FrameData* frameData);
    void repopulateStageMappingTable();
    void repopulateFighterMappingTable();
    void repopulateStatusMappingTable();
    void repopulatePlayerDataTables();
    void repopulateHitStatusMappingTable();

    void updatePlayerDataTableRowsIfDirty();
    void setPlayerDataTableRow(int playerIdx, int row, const rfcommon::FighterState& frame);

private:
    void onNewData(rfcommon::MappingInfo* map, rfcommon::SessionMetaData* meta, rfcommon::FrameData* frames) override;
    void onDataFinalized(rfcommon::MappingInfo* map, rfcommon::SessionMetaData* meta, rfcommon::FrameData* frames) override;

private:
    FrameDataListModel* model_;
    Ui::FrameDataListView* ui_;
    QTreeWidgetItem* metaDataItem_ = nullptr;
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
