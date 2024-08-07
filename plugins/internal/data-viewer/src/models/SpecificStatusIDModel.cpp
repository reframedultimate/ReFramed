#include "data-viewer/models/SpecificStatusIDModel.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/Profiler.hpp"
#include <algorithm>

// ----------------------------------------------------------------------------
void SpecificStatusIDModel::setMappingInfo(rfcommon::MappingInfo* mappingInfo)
{
    PROFILE(SpecificStatusIDModel, setMappingInfo);

    beginResetModel();
        table_.clearCompact();
        if (mappingInfo)
        {
            auto fighterIDs = mappingInfo->status.fighterIDs();
            std::sort(fighterIDs.begin(), fighterIDs.end());

            for (auto fighterID : fighterIDs)
            {
                auto names = mappingInfo->status.specificNames(fighterID);
                auto statuses = mappingInfo->status.specificStatuses(fighterID);

                int beginIdx = table_.count();
                for (int i = 0; i != names.count(); ++i)
                    table_.push({names[i], statuses[i], fighterID});
                int endIdx = table_.count();

                std::sort(table_.begin() + beginIdx, table_.begin() + endIdx, [](const Row& lhs, const Row& rhs){
                    return lhs.status < rhs.status;
                });
            }
        }
    endResetModel();
}

// ----------------------------------------------------------------------------
int SpecificStatusIDModel::rowCount(const QModelIndex& parent) const
{
    PROFILE(SpecificStatusIDModel, rowCount);

    return table_.count();
}

// ----------------------------------------------------------------------------
int SpecificStatusIDModel::columnCount(const QModelIndex& parent) const
{
    PROFILE(SpecificStatusIDModel, columnCount);

    return 3;
}

// ----------------------------------------------------------------------------
QVariant SpecificStatusIDModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    PROFILE(SpecificStatusIDModel, headerData);

    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
        {
            if (section == 0) return "Fighter ID";
            if (section == 1) return "Status ID";
            if (section == 2) return "Enum Name";
        }
        else if (orientation == Qt::Vertical)
        {
            return QString::number(section + 1);
        }
    }

    return QVariant();
}

// ----------------------------------------------------------------------------
QVariant SpecificStatusIDModel::data(const QModelIndex& index, int role) const
{
    PROFILE(SpecificStatusIDModel, data);

    switch (role)
    {
        case Qt::DisplayRole:
            switch (index.column())
            {
                case 0: return QString::number(table_[index.row()].fighter.value());
                case 1: return QString::number(table_[index.row()].status.value());
                case 2: return table_[index.row()].name.cStr();
            }
            break;

        case Qt::TextAlignmentRole:
            switch (index.column())
            {
                case 0: return static_cast<Qt::Alignment::Int>(Qt::AlignHCenter | Qt::AlignVCenter);
                case 1: return static_cast<Qt::Alignment::Int>(Qt::AlignHCenter | Qt::AlignVCenter);
                case 2: return static_cast<Qt::Alignment::Int>(Qt::AlignLeft | Qt::AlignVCenter);
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
