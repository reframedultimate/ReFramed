#include "data-viewer/models/FighterIDModel.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/Profiler.hpp"
#include <algorithm>

// ----------------------------------------------------------------------------
void FighterIDModel::setMappingInfo(rfcommon::MappingInfo* mappingInfo)
{
    PROFILE(FighterIDModel, setMappingInfo);

    beginResetModel();
        table_.clearCompact();
        if (mappingInfo)
        {
            auto names = mappingInfo->fighter.names();
            auto ids = mappingInfo->fighter.IDs();

            for (int i = 0; i != ids.count(); ++i)
                table_.push({names[i], ids[i]});

            std::sort(table_.begin(), table_.end(), [](const Row& lhs, const Row& rhs){
                return lhs.id < rhs.id;
            });
        }
    endResetModel();
}

// ----------------------------------------------------------------------------
int FighterIDModel::rowCount(const QModelIndex& parent) const
{
    PROFILE(FighterIDModel, rowCount);

    return table_.count();
}

// ----------------------------------------------------------------------------
int FighterIDModel::columnCount(const QModelIndex& parent) const
{
    PROFILE(FighterIDModel, columnCount);

    return 2;
}

// ----------------------------------------------------------------------------
QVariant FighterIDModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    PROFILE(FighterIDModel, headerData);

    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        if (section == 0) return "Fighter ID";
        if (section == 1) return "Name";
    }

    return QVariant();
}

// ----------------------------------------------------------------------------
QVariant FighterIDModel::data(const QModelIndex& index, int role) const
{
    PROFILE(FighterIDModel, data);

    switch (role)
    {
        case Qt::DisplayRole:
            switch (index.column())
            {
                case 0: return QString::number(table_[index.row()].id.value());
                case 1: return table_[index.row()].name.cStr();
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
