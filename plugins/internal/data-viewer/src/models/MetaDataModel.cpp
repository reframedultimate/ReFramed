#include "data-viewer/models/MetaDataModel.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/MetaData.hpp"
#include <QDateTime>

enum RowType
{
    TimeStarted,
    TimeEnded,
    Format,
    SetNumber,
    GameNumber,
    Stage,
    Winner,

    FixedRowCount
};

// ----------------------------------------------------------------------------
MetaDataModel::~MetaDataModel()
{
    if (meta_)
        meta_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void MetaDataModel::setMetaData(rfcommon::MappingInfo* mappingInfo, rfcommon::MetaData* metaData)
{
    beginResetModel();
        if (meta_)
            meta_->dispatcher.removeListener(this);

        map_ = mappingInfo;
        meta_ = metaData;

        if (meta_)
            meta_->dispatcher.addListener(this);
    endResetModel();
}

// ----------------------------------------------------------------------------
int MetaDataModel::rowCount(const QModelIndex& parent) const
{
    if (meta_)
        return FixedRowCount + meta_->fighterCount();
    return 0;
}

// ----------------------------------------------------------------------------
int MetaDataModel::columnCount(const QModelIndex& parent) const
{
    return 2;
}

// ----------------------------------------------------------------------------
QVariant MetaDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        if (section == 0) return "Key";
        if (section == 1) return "Value";
    }

    return QVariant();
}

// ----------------------------------------------------------------------------
QVariant MetaDataModel::data(const QModelIndex& index, int role) const
{
    if (meta_.notNull())
    {
        switch (meta_->type())
        {
            case rfcommon::MetaData::GAME:
                return gameData(static_cast<rfcommon::GameMetaData*>(meta_.get()), index, role);

            case rfcommon::MetaData::TRAINING:
                return trainingData(static_cast<rfcommon::TrainingMetaData*>(meta_.get()), index, role);
        }
    }

    return QVariant();
}

// ----------------------------------------------------------------------------
QVariant MetaDataModel::gameData(const rfcommon::GameMetaData* meta, const QModelIndex& index, int role) const
{
    switch (role)
    {
        case Qt::DisplayRole:
            if (index.column() == 0)
            {
                auto playerName = [meta](int fighterIdx) -> QString {
                    return meta->name(fighterIdx).cStr();
                };

                switch (index.row())
                {
                    case TimeStarted: return "Time Started";
                    case TimeEnded: return "Time Ended";
                    case Format: return "Format";
                    case SetNumber: return "Set Number";
                    case GameNumber: return "Game Number";
                    case Stage: return "Stage ID";
                    case Winner: return "Winner";
                    default: return playerName(index.row() - FixedRowCount);
                }
            }
            else if (index.column() == 1)
            {
                auto formatStageID = [this](rfcommon::StageID stageID) -> QString {
                    if (map_)
                        return QString::number(stageID.value()) + " (" + map_->stage.toName(stageID) + ")";
                    else
                        return QString::number(stageID.value()) + " (No mapping info)";
                };

                auto formatWinner = [meta](int winner) -> QString {
                    if (winner > -1)
                        return meta->name(winner).cStr();
                    return "(unknown)";
                };

                auto formatPlayer = [this, meta](int fighterIdx) -> QString {
                    auto fighterID = meta->fighterID(fighterIdx);
                    if (map_)
                        return QString::number(fighterID.value()) + " (" + map_->fighter.toName(fighterID) + ")";
                    else
                        return QString::number(fighterID.value()) + " (No mapping info)";
                };

                switch (index.row())
                {
                    case TimeStarted: return QDateTime::fromMSecsSinceEpoch(meta->timeStarted().millisSinceEpoch()).toString();
                    case TimeEnded: return QDateTime::fromMSecsSinceEpoch(meta->timeEnded().millisSinceEpoch()).toString();
                    case Format: return meta->setFormat().description().cStr();
                    case SetNumber: return QString::number(meta->setNumber().value());
                    case GameNumber: return QString::number(meta->gameNumber().value());
                    case Stage: return formatStageID(meta->stageID());
                    case Winner: return formatWinner(meta->winner());
                    default: return formatPlayer(index.row() - FixedRowCount);
                }
            }
            break;

        case Qt::TextAlignmentRole:
            return Qt::AlignLeft + Qt::AlignVCenter;

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

// ----------------------------------------------------------------------------
QVariant MetaDataModel::trainingData(const rfcommon::TrainingMetaData* meta, const QModelIndex& index, int role) const
{
    return QVariant();
}

// ----------------------------------------------------------------------------
void MetaDataModel::onMetaDataTimeStartedChanged(rfcommon::TimeStamp timeStarted)
{
    emit dataChanged(index(TimeStarted, 1), index(TimeStarted, 1));
}

// ----------------------------------------------------------------------------
void MetaDataModel::onMetaDataTimeEndedChanged(rfcommon::TimeStamp timeEnded)
{
    emit dataChanged(index(TimeEnded, 1), index(TimeEnded, 1));
}

// ----------------------------------------------------------------------------
void MetaDataModel::onMetaDataPlayerNameChanged(int fighterIdx, const rfcommon::SmallString<15>& name)
{
    emit dataChanged(index(FixedRowCount + fighterIdx, 0), index(FixedRowCount + fighterIdx, 0));
}

// ----------------------------------------------------------------------------
void MetaDataModel::onMetaDataSetNumberChanged(rfcommon::SetNumber number)
{
    emit dataChanged(index(SetNumber, 1), index(SetNumber, 1));
}

// ----------------------------------------------------------------------------
void MetaDataModel::onMetaDataGameNumberChanged(rfcommon::GameNumber number)
{
    emit dataChanged(index(GameNumber, 1), index(GameNumber, 1));
}

// ----------------------------------------------------------------------------
void MetaDataModel::onMetaDataSetFormatChanged(const rfcommon::SetFormat& format)
{
    emit dataChanged(index(Format, 1), index(Format, 1));
}

// ----------------------------------------------------------------------------
void MetaDataModel::onMetaDataWinnerChanged(int winnerPlayerIdx)
{
    emit dataChanged(index(Winner, 1), index(Winner, 1));
}

// ----------------------------------------------------------------------------
void MetaDataModel::onMetaDataTrainingSessionNumberChanged(rfcommon::GameNumber number)
{
    (void)number;
}
