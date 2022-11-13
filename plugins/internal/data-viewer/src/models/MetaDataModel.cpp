#include "rfcommon/GameMetaData.hpp"
#include "data-viewer/models/MetaDataModel.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/TrainingMetaData.hpp"
#include <QDateTime>

namespace Game {
    enum GameRowType
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
}

namespace Training {
    enum TrainingRowType
    {
        TimeStarted,
        TimeEnded,
        Stage,

        FixedRowCount
    };
}

// ----------------------------------------------------------------------------
MetaDataModel::~MetaDataModel()
{
    if (meta_)
        meta_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void MetaDataModel::setMetaData(rfcommon::MappingInfo* mappingInfo, rfcommon::MetaData* metaData)
{
    PROFILE(MetaDataModel, setMetaData);

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
    PROFILE(MetaDataModel, rowCount);

    if (meta_)
        return meta_->type() == rfcommon::MetaData::GAME ?
            Game::FixedRowCount + meta_->fighterCount() :
            Training::FixedRowCount + meta_->fighterCount();
    return 0;
}

// ----------------------------------------------------------------------------
int MetaDataModel::columnCount(const QModelIndex& parent) const
{
    PROFILE(MetaDataModel, columnCount);

    return 2;
}

// ----------------------------------------------------------------------------
QVariant MetaDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    PROFILE(MetaDataModel, headerData);

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
    PROFILE(MetaDataModel, data);

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
    PROFILE(MetaDataModel, gameData);

    switch (role)
    {
        case Qt::DisplayRole:
            if (index.column() == 0)
            {
                auto playerName = [meta](int fighterIdx) -> QString {
                    return meta->playerName(fighterIdx).cStr();
                };

                switch (index.row())
                {
                    case Game::TimeStarted: return "Time Started";
                    case Game::TimeEnded: return "Time Ended";
                    case Game::Format: return "Format";
                    case Game::SetNumber: return "Set Number";
                    case Game::GameNumber: return "Game Number";
                    case Game::Stage: return "Stage ID";
                    case Game::Winner: return "Winner";
                    default: return playerName(index.row() - Game::FixedRowCount);
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
                        return meta->playerName(winner).cStr();
                    return "(unknown)";
                };

                auto formatPlayer = [this, meta](int fighterIdx) -> QString {
                    auto fighterID = meta->playerFighterID(fighterIdx);
                    if (map_)
                        return QString::number(fighterID.value()) + " (" + map_->fighter.toName(fighterID) + ")";
                    else
                        return QString::number(fighterID.value()) + " (No mapping info)";
                };

                switch (index.row())
                {
                    case Game::TimeStarted: return QDateTime::fromMSecsSinceEpoch(meta->timeStarted().millisSinceEpoch()).toString();
                    case Game::TimeEnded: return QDateTime::fromMSecsSinceEpoch(meta->timeEnded().millisSinceEpoch()).toString();
                    case Game::Format: return meta->setFormat().shortDescription();
                    case Game::SetNumber: return QString::fromUtf8(meta->round().longDescription().cStr());
                    case Game::GameNumber: return QString::number(meta->score().gameNumber().value());
                    case Game::Stage: return formatStageID(meta->stageID());
                    case Game::Winner: return formatWinner(meta->winner());
                    default: return formatPlayer(index.row() - Game::FixedRowCount);
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
    PROFILE(MetaDataModel, trainingData);

    switch (role)
    {
        case Qt::DisplayRole:
            if (index.column() == 0)
            {
                auto playerName = [meta](int fighterIdx) -> QString {
                    return meta->playerTag(fighterIdx).cStr();
                };

                switch (index.row())
                {
                    case Training::TimeStarted: return "Time Started";
                    case Training::TimeEnded: return "Time Ended";
                    case Training::Stage: return "Stage ID";
                    default: return playerName(index.row() - Training::FixedRowCount);
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

                auto formatPlayer = [this, meta](int fighterIdx) -> QString {
                    auto fighterID = meta->playerFighterID(fighterIdx);
                    if (map_)
                        return QString::number(fighterID.value()) + " (" + map_->fighter.toName(fighterID) + ")";
                    else
                        return QString::number(fighterID.value()) + " (No mapping info)";
                };

                switch (index.row())
                {
                    case Training::TimeStarted: return QDateTime::fromMSecsSinceEpoch(meta->timeStarted().millisSinceEpoch()).toString();
                    case Training::TimeEnded: return QDateTime::fromMSecsSinceEpoch(meta->timeEnded().millisSinceEpoch()).toString();
                    case Training::Stage: return formatStageID(meta->stageID());
                    default: return formatPlayer(index.row() - Training::FixedRowCount);
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
void MetaDataModel::onMetaDataTimeChanged(rfcommon::TimeStamp timeStarted, rfcommon::TimeStamp timeEnded)
{
    PROFILE(MetaDataModel, onMetaDataTimeStartedChanged);

    switch (meta_->type())
    {
        case rfcommon::MetaData::GAME:
            emit dataChanged(index(Game::TimeStarted, 1), index(Game::TimeEnded, 1));
            break;
        case rfcommon::MetaData::TRAINING:
            emit dataChanged(index(Training::TimeStarted, 1), index(Training::TimeEnded, 1));
            break;
    }
}

// ----------------------------------------------------------------------------
void MetaDataModel::onMetaDataTournamentDetailsChanged() {}

// ----------------------------------------------------------------------------
void MetaDataModel::onMetaDataEventDetailsChanged() {}

// ----------------------------------------------------------------------------
void MetaDataModel::onMetaDataCommentatorsChanged() {}

// ----------------------------------------------------------------------------
void MetaDataModel::onMetaDataGameDetailsChanged()
{
    PROFILE(MetaDataModel, onMetaDataSetNumberChanged);

    switch (meta_->type())
    {
        case rfcommon::MetaData::GAME:
            emit dataChanged(index(Game::Format, 1), index(Game::GameNumber, 1));
            break;
        case rfcommon::MetaData::TRAINING: break;
    }
}

// ----------------------------------------------------------------------------
void MetaDataModel::onMetaDataPlayerDetailsChanged()
{
    PROFILE(MetaDataModel, onMetaDataPlayerNameChanged);

    switch (meta_->type())
    {
        case rfcommon::MetaData::GAME:
            emit dataChanged(index(Game::FixedRowCount, 0), index(Game::FixedRowCount + meta_->fighterCount(), 0));
            break;
        case rfcommon::MetaData::TRAINING:
            emit dataChanged(index(Training::FixedRowCount, 0), index(Training::FixedRowCount + meta_->fighterCount(), 0));
            break;
    }
}

// ----------------------------------------------------------------------------
void MetaDataModel::onMetaDataWinnerChanged(int winnerPlayerIdx)
{
    PROFILE(MetaDataModel, onMetaDataWinnerChanged);

    switch (meta_->type())
    {
        case rfcommon::MetaData::GAME:
            emit dataChanged(index(Game::Winner, 1), index(Game::Winner, 1));
            break;
        case rfcommon::MetaData::TRAINING: break;
    }
}

// ----------------------------------------------------------------------------
void MetaDataModel::onMetaDataTrainingSessionNumberChanged(rfcommon::SessionNumber number) {}
