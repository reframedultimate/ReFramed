#pragma once

#include "frame-data-list/listeners/FrameDataListListener.hpp"
#include "rfcommon/Vector.hpp"
#include <QWidget>

class FrameDataListModel;
class QTreeWidgetItem;
class QTableView;

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
    void repopulateTree(rfcommon::MappingInfo* map, rfcommon::SessionMetaData* meta, rfcommon::FrameData* frames);
    void repopulatePlayerDataTables();

private:
    void onNewData(rfcommon::MappingInfo* map, rfcommon::SessionMetaData* meta, rfcommon::FrameData* frames) override;
    void onDataFinalized(rfcommon::MappingInfo* map, rfcommon::SessionMetaData* meta, rfcommon::FrameData* frames) override;
    void onNewFrame() override;

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
    rfcommon::SmallVector<QTableView*, 8> playerDataTables_;
};
