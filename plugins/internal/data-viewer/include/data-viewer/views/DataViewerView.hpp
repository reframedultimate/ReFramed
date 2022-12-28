#pragma once

#include "data-viewer/listeners/DataViewerListener.hpp"
#include "rfcommon/Vector.hpp"
#include <QWidget>

class DataViewerModel;
class QTreeWidgetItem;
class QTableView;

namespace Ui {
    class DataViewerView;
}

namespace rfcommon {
    class FighterState;
}

class DataViewerView
    : public QWidget
    , public DataViewerListener
{
    Q_OBJECT

public:
    explicit DataViewerView(DataViewerModel* model, QWidget* parent=nullptr);
    ~DataViewerView();

private slots:
    void onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);

private:
    void populateTree(rfcommon::MappingInfo* map, rfcommon::Metadata* meta, rfcommon::VideoMeta* vmeta, rfcommon::FrameData* frames);
    void populatePlayerDataTables();

private:
    void onNewData(rfcommon::MappingInfo* map, rfcommon::Metadata* meta, rfcommon::VideoMeta* vmeta, rfcommon::FrameData* frames) override;
    void onClear() override;
    void onNewFrame() override;

private:
    DataViewerModel* model_;
    Ui::DataViewerView* ui_;
    QTreeWidgetItem* metadataItem_ = nullptr;
    QTreeWidgetItem* videoMetadataItem_ = nullptr;
    QTreeWidgetItem* stageIDMappingsItem_ = nullptr;
    QTreeWidgetItem* fighterIDMappingsItem_ = nullptr;
    QTreeWidgetItem* baseStatusIDMappingsItem_ = nullptr;
    QTreeWidgetItem* specificStatusIDMappingsItem_ = nullptr;
    QTreeWidgetItem* hitStatusIDMappingsItem_ = nullptr;
    rfcommon::SmallVector<QTreeWidgetItem*, 8> playerDataItems_;
    rfcommon::SmallVector<QTableView*, 8> playerDataTables_;

    int playerDataTableIdxOnClear_ = -1;
    int selectedTreeItemOnClear_ = -1;
};
