#pragma once

#include <QAbstractTableModel>
#include "rfcommon/String.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/StageID.hpp"

namespace rfcommon {
    class MappingInfo;
}

class StageIDModel : public QAbstractTableModel
{
public:
    void setMappingInfo(rfcommon::MappingInfo* mappingInfo);

    int rowCount(const QModelIndex& parent=QModelIndex()) const override;
    int columnCount(const QModelIndex& parent=QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const override;
    QVariant data(const QModelIndex& index, int role=Qt::DisplayRole) const override;

private:
    rfcommon::SmallVector<rfcommon::StageID, 10> ids_;
    rfcommon::SmallVector<rfcommon::String, 10> names_;
};
