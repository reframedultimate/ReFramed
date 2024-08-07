#include "data-viewer/models/StageIDModel.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/Profiler.hpp"

// ----------------------------------------------------------------------------
void StageIDModel::setMappingInfo(rfcommon::MappingInfo* mappingInfo)
{
    PROFILE(StageIDModel, setMappingInfo);

    beginResetModel();
        if (mappingInfo)
        {
            names_ = mappingInfo->stage.names();
            ids_ = mappingInfo->stage.IDs();
        }
        else
        {
            names_.clearCompact();
            ids_.clearCompact();
        }
    endResetModel();
}

// ----------------------------------------------------------------------------
int StageIDModel::rowCount(const QModelIndex& parent) const
{
    PROFILE(StageIDModel, rowCount);

    return ids_.count();
}

// ----------------------------------------------------------------------------
int StageIDModel::columnCount(const QModelIndex& parent) const
{
    PROFILE(StageIDModel, columnCount);

    return 2;
}

// ----------------------------------------------------------------------------
QVariant StageIDModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    PROFILE(StageIDModel, headerData);

    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        if (section == 0) return "Stage ID";
        if (section == 1) return "Name";
    }

    return QVariant();
}

// ----------------------------------------------------------------------------
QVariant StageIDModel::data(const QModelIndex& index, int role) const
{
    PROFILE(StageIDModel, data);

    switch (role)
    {
        case Qt::DisplayRole:
            switch (index.column())
            {
                case 0: return QString::number(ids_[index.row()].value());
                case 1: return names_[index.row()].cStr();
            }
            break;

        case Qt::TextAlignmentRole:
            switch (index.column())
            {
                case 0: return static_cast<Qt::Alignment::Int>(Qt::AlignHCenter | Qt::AlignVCenter);
                case 1: return static_cast<Qt::Alignment::Int>(Qt::AlignLeft | Qt::AlignVCenter);
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
