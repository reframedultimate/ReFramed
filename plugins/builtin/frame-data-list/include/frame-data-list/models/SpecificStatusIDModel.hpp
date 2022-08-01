#pragma once

#include <QAbstractTableModel>
#include "rfcommon/String.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/FighterStatus.hpp"
#include "rfcommon/FighterID.hpp"

namespace rfcommon {
    class MappingInfo;
}

class SpecificStatusIDModel : public QAbstractTableModel
{
public:
    void setMappingInfo(rfcommon::MappingInfo* mappingInfo);

    int rowCount(const QModelIndex& parent=QModelIndex()) const override;
    int columnCount(const QModelIndex& parent=QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const override;
    QVariant data(const QModelIndex& index, int role=Qt::DisplayRole) const override;

private:
    struct Row
    {
        rfcommon::SmallString<31> name;
        rfcommon::FighterStatus status;
        rfcommon::FighterID fighter;
    };

    rfcommon::Vector<Row> table_;
};
