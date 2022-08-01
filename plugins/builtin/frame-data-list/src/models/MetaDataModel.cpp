#include "frame-data-list/models/MetaDataModel.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/SessionMetaData.hpp"
#include <QDateTime>

#include <QDebug>

#define NUM_FIXED_ROWS 7

// ----------------------------------------------------------------------------
void MetaDataModel::setMetaData(rfcommon::MappingInfo* mappingInfo, rfcommon::SessionMetaData* metaData)
{
    beginResetModel();
    map_ = mappingInfo;
    meta_ = metaData;
    endResetModel();
}

// ----------------------------------------------------------------------------
void MetaDataModel::clearMetaData(rfcommon::MappingInfo* map, rfcommon::SessionMetaData* metaData)
{
    beginResetModel();
    map_.drop();
    meta_.drop();
    endResetModel();
}

// ----------------------------------------------------------------------------
int MetaDataModel::rowCount(const QModelIndex& parent) const
{
    if (meta_)
        return NUM_FIXED_ROWS + meta_->fighterCount();
    return 0;
}

// ----------------------------------------------------------------------------
int MetaDataModel::columnCount(const QModelIndex& parent) const
{
    return 2;
}

// ----------------------------------------------------------------------------
QVariant MetaDataModel::data(const QModelIndex& index, int role) const
{
    if (meta_.notNull())
    {
        switch (meta_->type())
        {
            case rfcommon::SessionMetaData::GAME:
                return gameData(static_cast<rfcommon::GameSessionMetaData*>(meta_.get()), index, role);

            case rfcommon::SessionMetaData::TRAINING:
                return trainingData(static_cast<rfcommon::TrainingSessionMetaData*>(meta_.get()), index, role);
        }
    }

    return QVariant();
}

// ----------------------------------------------------------------------------
QVariant MetaDataModel::gameData(const rfcommon::GameSessionMetaData* meta, const QModelIndex& index, int role) const
{
    switch (role)
    {
        case Qt::DisplayRole:
            if (index.column() == 0)
            {
                auto playerName = [meta](int fighterIdx) -> QString {
                    return meta->name(fighterIdx).cStr();
                };

                if (index.row() == 0) return "Time Started";
                if (index.row() == 1) return "Time Ended";
                if (index.row() == 2) return "Format";
                if (index.row() == 3) return "Set Number";
                if (index.row() == 4) return "Game Number";
                if (index.row() == 5) return "Stage ID";
                if (index.row() == 6) return "Winner";

                return playerName(index.row() - NUM_FIXED_ROWS);
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

                if (index.row() == 0) return QDateTime::fromMSecsSinceEpoch(meta->timeStarted().millisSinceEpoch()).toString();
                if (index.row() == 1) return QDateTime::fromMSecsSinceEpoch(meta->timeEnded().millisSinceEpoch()).toString();
                if (index.row() == 2) return meta->setFormat().description().cStr();
                if (index.row() == 3) return QString::number(meta->setNumber().value());
                if (index.row() == 4) return QString::number(meta->gameNumber().value());
                if (index.row() == 5) return formatStageID(meta->stageID());
                if (index.row() == 6) return formatWinner(meta->winner());

                return formatPlayer(index.row() - NUM_FIXED_ROWS);
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
QVariant MetaDataModel::trainingData(const rfcommon::TrainingSessionMetaData* meta, const QModelIndex& index, int role) const
{
    return QVariant();
}

// ----------------------------------------------------------------------------
void MetaDataModel::onSessionMetaDataTimeStartedChanged(rfcommon::TimeStamp timeStarted)
{

}

// ----------------------------------------------------------------------------
void MetaDataModel::onSessionMetaDataTimeEndedChanged(rfcommon::TimeStamp timeEnded)
{

}

// ----------------------------------------------------------------------------
void MetaDataModel::onSessionMetaDataPlayerNameChanged(int fighterIdx, const rfcommon::SmallString<15>& name)
{

}

// ----------------------------------------------------------------------------
void MetaDataModel::onSessionMetaDataSetNumberChanged(rfcommon::SetNumber number)
{

}

// ----------------------------------------------------------------------------
void MetaDataModel::onSessionMetaDataGameNumberChanged(rfcommon::GameNumber number)
{

}

// ----------------------------------------------------------------------------
void MetaDataModel::onSessionMetaDataSetFormatChanged(const rfcommon::SetFormat& format)
{

}

// ----------------------------------------------------------------------------
void MetaDataModel::onSessionMetaDataWinnerChanged(int winnerPlayerIdx)
{
    emit dataChanged(index(5, 0), index(5, 1));
}

// ----------------------------------------------------------------------------
void MetaDataModel::onSessionMetaDataTrainingSessionNumberChanged(rfcommon::GameNumber number)
{
    (void)number;
}
