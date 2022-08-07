#include "data-viewer/models/HitStatusIDModel.hpp"
#include "rfcommon/MappingInfo.hpp"
#include <algorithm>

// ----------------------------------------------------------------------------
void HitStatusIDModel::setMappingInfo(rfcommon::MappingInfo* mappingInfo)
{
    beginResetModel();
        table_.clearCompact();
        if (mappingInfo)
        {
            auto names = mappingInfo->hitStatus.names();
            auto statuses = mappingInfo->hitStatus.statuses();

            for (int i = 0; i != statuses.count(); ++i)
                table_.push({names[i], statuses[i]});

            std::sort(table_.begin(), table_.end(), [](const Row& lhs, const Row& rhs){
                return lhs.status < rhs.status;
            });
        }
    endResetModel();
}

// ----------------------------------------------------------------------------
int HitStatusIDModel::rowCount(const QModelIndex& parent) const
{
    return table_.count();
}

// ----------------------------------------------------------------------------
int HitStatusIDModel::columnCount(const QModelIndex& parent) const
{
    return 2;
}

// ----------------------------------------------------------------------------
QVariant HitStatusIDModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        if (section == 0) return "Hit Status ID";
        if (section == 1) return "Enum Name";
    }

    return QVariant();
}

// ----------------------------------------------------------------------------
QVariant HitStatusIDModel::data(const QModelIndex& index, int role) const
{
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
