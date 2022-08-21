#include "data-viewer/models/BaseStatusIDModel.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/Profiler.hpp"
#include <algorithm>

// ----------------------------------------------------------------------------
void BaseStatusIDModel::setMappingInfo(rfcommon::MappingInfo* mappingInfo)
{
    PROFILE(BaseStatusIDModel, setMappingInfo);

    beginResetModel();
        table_.clearCompact();
        if (mappingInfo)
        {
            auto statuses = mappingInfo->status.baseStatuses();
            auto names = mappingInfo->status.baseNames();

            for (int i = 0; i != statuses.count(); ++i)
                table_.push({names[i], statuses[i]});

            std::sort(table_.begin(), table_.end(), [](const Row& lhs, const Row& rhs){
                return lhs.status < rhs.status;
            });
        }
    endResetModel();
}

// ----------------------------------------------------------------------------
int BaseStatusIDModel::rowCount(const QModelIndex& parent) const
{
    PROFILE(BaseStatusIDModel, rowCount);

    return table_.count();
}

// ----------------------------------------------------------------------------
int BaseStatusIDModel::columnCount(const QModelIndex& parent) const
{
    PROFILE(BaseStatusIDModel, columnCount);

    return 2;
}

// ----------------------------------------------------------------------------
QVariant BaseStatusIDModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    PROFILE(BaseStatusIDModel, headerData);

    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        if (section == 0) return "Status ID";
        if (section == 1) return "Enum Name";
    }

    return QVariant();
}

// ----------------------------------------------------------------------------
QVariant BaseStatusIDModel::data(const QModelIndex& index, int role) const
{
    PROFILE(BaseStatusIDModel, data);

    switch (role)
    {
        case Qt::DisplayRole:
            switch (index.column())
            {
                case 0: return QString::number(table_[index.row()].status.value());
                case 1: return table_[index.row()].name.cStr();
            }
            break;

        case Qt::TextAlignmentRole:
            switch (index.column())
            {
                case 0: return Qt::AlignHCenter + Qt::AlignVCenter;
                case 1: return Qt::AlignLeft + Qt::AlignVCenter;
            }
            break;

        case Qt::CheckStateRole:
        case Qt::DecorationRole:
        case Qt::EditRole:
        case Qt::ToolTipRole:
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
        case Qt::SizeHintRole:
            break;
    }

    return QVariant();
}
