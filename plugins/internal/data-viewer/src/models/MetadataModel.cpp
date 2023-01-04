#include "data-viewer/models/MetadataModel.hpp"

#include "rfcommon/GameMetadata.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/TrainingMetadata.hpp"

#include <QDateTime>

namespace Game {
enum Game
{
    TournamentNode,
        TournamentName,
        TournamentWebsite,
        TournamentOrganizersNode,
            TournamentOrganizer,
        TournamentSponsorsNode,
            TournamentSponsor,
    CommentatorsNode,
        Commentator,
    EventNode,
        EventBracketType,
        EventBracketURL,
    GameNode,
        TimeStarted,
        TimeEnded,
        Duration,
        Stage,
        Round,
        SetFormat,
        GameNumber,
        Score,
        Winner,
    PlayersNode,
        PlayerNode,
            PlayerName,
            PlayerFighter,
            PlayerCostume,
            PlayerSocial,
            PlayerPronouns
};
}

// ----------------------------------------------------------------------------
MetadataModel::~MetadataModel()
{
    if (meta_)
        meta_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void MetadataModel::setMetadata(rfcommon::MappingInfo* mappingInfo, rfcommon::Metadata* metadata)
{
    PROFILE(MetadataModel, setMetadata);

    beginResetModel();
        if (meta_)
            meta_->dispatcher.removeListener(this);

        map_ = mappingInfo;
        meta_ = metadata;

        if (meta_)
            meta_->dispatcher.addListener(this);
    endResetModel();
}

// ----------------------------------------------------------------------------
QModelIndex MetadataModel::index(int row, int column, const QModelIndex& parent) const
{
    PROFILE(MetadataModel, index);

    if (parent.isValid() == false)
        switch (row)
        {
            case 0: return createIndex(row, column, Game::TournamentNode); break;
            case 1: return createIndex(row, column, Game::CommentatorsNode); break;
            case 2: return createIndex(row, column, Game::EventNode); break;
            case 3: return createIndex(row, column, Game::GameNode); break;
            case 4: return createIndex(row, column, Game::PlayersNode); break;
        }
    else switch (parent.internalId() & 0xFFFF)
    {
        case Game::TournamentNode:
            switch (row)
            {
                case 0: return createIndex(row, column, Game::TournamentName);
                case 1: return createIndex(row, column, Game::TournamentWebsite);
                case 2: return createIndex(row, column, Game::TournamentOrganizersNode); break;
                case 3: return createIndex(row, column, Game::TournamentSponsorsNode); break;
            }
            break;

        case Game::TournamentOrganizersNode:
            return createIndex(row, column, Game::TournamentOrganizer);

        case Game::TournamentSponsorsNode:
            return createIndex(row, column, Game::TournamentSponsor);

        case Game::CommentatorsNode:
            return createIndex(row, column, Game::Commentator);

        case Game::EventNode:
            switch (row)
            {
                case 0: return createIndex(row, column, Game::EventBracketType);
                case 1: return createIndex(row, column, Game::EventBracketURL);
            }
            break;

        case Game::GameNode:
            switch (row)
            {
                case 0: return createIndex(row, column, Game::TimeStarted);
                case 1: return createIndex(row, column, Game::TimeEnded);
                case 2: return createIndex(row, column, Game::Duration);
                case 3: return createIndex(row, column, Game::Stage);
                case 4: return createIndex(row, column, Game::Round);
                case 5: return createIndex(row, column, Game::SetFormat);
                case 6: return createIndex(row, column, Game::GameNumber);
                case 7: return createIndex(row, column, Game::Score);
                case 8: return createIndex(row, column, Game::Winner);
            }
            break;

        case Game::PlayersNode:
            return createIndex(row, column, Game::PlayerNode);

        case Game::PlayerNode:
            switch (row)
            {
                case 0: return createIndex(row, column, quint16(Game::PlayerName) | (quint16(parent.row()) << 16));
                case 1: return createIndex(row, column, quint16(Game::PlayerFighter) | (quint16(parent.row()) << 16));
                case 2: return createIndex(row, column, quint16(Game::PlayerCostume) | (quint16(parent.row()) << 16));
                case 3: return createIndex(row, column, quint16(Game::PlayerSocial) | (quint16(parent.row()) << 16));
                case 4: return createIndex(row, column, quint16(Game::PlayerPronouns) | (quint16(parent.row()) << 16));
            }
            break;
    }

    return QModelIndex();
}

// ----------------------------------------------------------------------------
QModelIndex MetadataModel::parent(const QModelIndex& index) const
{
    PROFILE(MetadataModel, parent);

    if (index.isValid() == false)
        return QModelIndex();
    switch (index.internalId() & 0xFFFF)
    {
        case Game::TournamentName: return createIndex(0, 0, Game::TournamentNode);
        case Game::TournamentWebsite: return createIndex(0, 0, Game::TournamentNode);
        case Game::TournamentOrganizersNode: return createIndex(0, 0, Game::TournamentNode);
        case Game::TournamentOrganizer: return createIndex(2, 0, Game::TournamentOrganizersNode);
        case Game::TournamentSponsorsNode: return createIndex(0, 0, Game::TournamentNode);
        case Game::TournamentSponsor: return createIndex(3, 0, Game::TournamentSponsorsNode);

        case Game::Commentator: return createIndex(1, 0, Game::CommentatorsNode);

        case Game::EventBracketType: return createIndex(2, 0, Game::EventNode);
        case Game::EventBracketURL: return createIndex(2, 0, Game::EventNode);

        case Game::TimeStarted: return createIndex(3, 0, Game::GameNode);
        case Game::TimeEnded: return createIndex(3, 0, Game::GameNode);
        case Game::Duration: return createIndex(3, 0, Game::GameNode);
        case Game::Stage: return createIndex(3, 0, Game::GameNode);
        case Game::Round: return createIndex(3, 0, Game::GameNode);
        case Game::SetFormat: return createIndex(3, 0, Game::GameNode);
        case Game::GameNumber: return createIndex(4, 0, Game::GameNode);
        case Game::Score: return createIndex(4, 0, Game::GameNode);
        case Game::Winner: return createIndex(4, 0, Game::GameNode);

        case Game::PlayerNode: return createIndex(4, 0, Game::PlayersNode);

        case Game::PlayerName: return createIndex((index.internalId() >> 16) & 0xFFFF, 0, Game::PlayerNode);
        case Game::PlayerFighter: return createIndex((index.internalId() >> 16) & 0xFFFF, 0, Game::PlayerNode);
        case Game::PlayerCostume: return createIndex((index.internalId() >> 16) & 0xFFFF, 0, Game::PlayerNode);
        case Game::PlayerSocial: return createIndex((index.internalId() >> 16) & 0xFFFF, 0, Game::PlayerNode);
        case Game::PlayerPronouns: return createIndex((index.internalId() >> 16) & 0xFFFF, 0, Game::PlayerNode);
    }

    return QModelIndex();
}

// ----------------------------------------------------------------------------
int MetadataModel::rowCount(const QModelIndex& parent) const
{
    PROFILE(MetadataModel, rowCount);

    if (meta_.isNull())
        return 0;

    if (parent.isValid() == false)
        return 5;

    switch (meta_->type())
    {
        case rfcommon::Metadata::GAME: return gameRowCount(meta_->asGame(), parent);
        case rfcommon::Metadata::TRAINING: return trainingRowCount(meta_->asTraining(), parent);
    }

    return 0;
}

// ----------------------------------------------------------------------------
int MetadataModel::gameRowCount(const rfcommon::GameMetadata* meta, const QModelIndex& parent) const
{
    PROFILE(MetadataModel, gameRowCount);

    auto bracketTypeHasURL = [](const rfcommon::BracketType bracketType) {
        switch (bracketType.type())
        {
            case rfcommon::BracketType::SINGLES:
            case rfcommon::BracketType::DOUBLES:
            case rfcommon::BracketType::AMATEURS:
            case rfcommon::BracketType::SIDE:
                return true;

            case rfcommon::BracketType::FRIENDLIES:
            case rfcommon::BracketType::PRACTICE:
            case rfcommon::BracketType::MONEYMATCH:
            case rfcommon::BracketType::OTHER:
                return false;
        }
        return false;
    };

    switch (parent.internalId() & 0xFFFF)
    {
        case Game::TournamentNode: return 4;
        case Game::TournamentOrganizersNode: return meta->tournamentOrganizerCount();
        case Game::TournamentSponsorsNode: return meta->sponsorCount();
        case Game::CommentatorsNode: return meta->commentatorCount();
        case Game::EventNode: return bracketTypeHasURL(meta->bracketType()) ? 2 : 1;
        case Game::GameNode: return 9;
        case Game::PlayersNode: return meta->fighterCount();
        case Game::PlayerNode: return 5;
    }

    return 0;
}

// ----------------------------------------------------------------------------
int MetadataModel::trainingRowCount(const rfcommon::TrainingMetadata* meta, const QModelIndex& parent) const
{
    PROFILE(MetadataModel, trainingRowCount);

    return 0;
}

// ----------------------------------------------------------------------------
int MetadataModel::columnCount(const QModelIndex& parent) const
{
    PROFILE(MetadataModel, columnCount);

    return 2;
}

// ----------------------------------------------------------------------------
QVariant MetadataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    PROFILE(MetadataModel, headerData);

    switch (role)
    {
        case Qt::DisplayRole: {
            if (orientation == Qt::Horizontal)
            {
                switch (section)
                {
                    case 0: return "Key";
                    case 1: return "Value";
                }
            }
        } break;
    }

    return QVariant();
}

// ----------------------------------------------------------------------------
QVariant MetadataModel::data(const QModelIndex& index, int role) const
{
    PROFILE(MetadataModel, data);

    if (meta_.notNull())
        switch (meta_->type())
        {
            case rfcommon::Metadata::GAME: return gameData(meta_->asGame(), index, role);
            case rfcommon::Metadata::TRAINING: return trainingData(meta_->asTraining(), index, role);
        }

    return QVariant();
}

// ----------------------------------------------------------------------------
QVariant MetadataModel::gameData(const rfcommon::GameMetadata* meta, const QModelIndex& index, int role) const
{
    PROFILE(MetadataModel, gameData);

    switch (role)
    {
        case Qt::DisplayRole: {
            switch (index.internalId() & 0xFFFF)
            {
                case Game::TournamentNode:
                    if (index.column() == 0) return "Tournament";
                    break;

                case Game::TournamentName:
                    if (index.column() == 0) return "Name";
                    if (index.column() == 1) return QString::fromUtf8(meta->tournamentName().cStr());
                    break;

                case Game::TournamentWebsite:
                    if (index.column() == 0) return "Website";
                    if (index.column() == 1) return QString::fromUtf8(meta->tournamentWebsite().cStr());
                    break;

                case Game::TournamentOrganizersNode:
                    if (index.column() == 0) return "Tournament Organizers";
                    break;

                case Game::TournamentOrganizer:
                    if (index.column() == 0) return QString::fromUtf8(meta->tournamentOrganizerName(index.row()).cStr());
                    if (index.column() == 1) return
                            QString::fromUtf8(meta->tournamentOrganizerSocial(index.row()).cStr()) + ", " +
                            QString::fromUtf8(meta->tournamentOrganizerPronouns(index.row()).cStr());
                    break;

                case Game::TournamentSponsorsNode:
                    if (index.column() == 0) return "Sponsors";
                    break;

                case Game::TournamentSponsor:
                    if (index.column() == 0) return QString::fromUtf8(meta->sponsorName(index.row()).cStr());
                    if (index.column() == 1) return QString::fromUtf8(meta->sponsorWebsite(index.row()).cStr());
                    break;

                case Game::CommentatorsNode:
                    if (index.column() == 0) return "Commentators";
                    break;

                case Game::Commentator:
                    if (index.column() == 0) return QString::fromUtf8(meta->commentatorName(index.row()).cStr());
                    if (index.column() == 1) return
                            QString::fromUtf8(meta->commentatorSocial(index.row()).cStr()) + ", " +
                            QString::fromUtf8(meta->commentatorPronouns(index.row()).cStr());
                    break;

                case Game::EventNode:
                    if (index.column() == 0) return "Event";
                    break;

                case Game::EventBracketType:
                    if (index.column() == 0) return "Bracket Type";
                    if (index.column() == 1) return QString::fromUtf8(meta->bracketType().description());
                    break;

                case Game::EventBracketURL:
                    if (index.column() == 0) return "Bracket URL";
                    if (index.column() == 1) return QString::fromUtf8(meta->bracketURL().cStr());
                    break;

                case Game::GameNode:
                    if (index.column() == 0) return "Game";
                    break;

                case Game::TimeStarted:
                    if (index.column() == 0) return "Time Started";
                    if (index.column() == 1) return QDateTime::fromMSecsSinceEpoch(meta->timeStarted().millisSinceEpoch()).toString();
                    break;

                case Game::TimeEnded:
                    if (index.column() == 0) return "Time Ended";
                    if (index.column() == 1) return QDateTime::fromMSecsSinceEpoch(meta->timeEnded().millisSinceEpoch()).toString();
                    break;

                case Game::Duration:
                    if (index.column() == 0) return "Duration";
                    if (index.column() == 1) return QTime(0, 0).addSecs(meta->length().seconds()).toString();
                    break;

                case Game::Stage:
                    if (index.column() == 0) return "Stage";
                    if (index.column() == 1) return map_ ?
                                QString::number(meta->stageID().value()) + " (" + QString::fromUtf8(map_->stage.toName(meta->stageID())) + ")" :
                                QString::number(meta->stageID().value()) + " (no mapping info)";
                    break;

                case Game::Round:
                    if (index.column() == 0) return "Round";
                    if (index.column() == 1) return QString::fromUtf8(meta->round().longDescription().cStr());
                    break;

                case Game::SetFormat:
                    if (index.column() == 0) return "Format";
                    if (index.column() == 1) return QString::fromUtf8(meta->setFormat().longDescription());
                    break;

                case Game::PlayersNode:
                    if (index.column() == 0) return "Players";
                    break;

                case Game::GameNumber:
                    if (index.column() == 0) return "Game Number";
                    if (index.column() == 1) return QString::number(meta->score().gameNumber().value());
                    break;

                case Game::Score:
                    if (index.column() == 0) return "Score";
                    if (index.column() == 1) return QString::number(meta->score().left()) + "-" + QString::number(meta->score().right());
                    break;

                case Game::Winner:
                    if (index.column() == 0) return "Winner";
                    if (index.column() == 1) return QString::fromUtf8(meta->playerTag(meta->winner()).cStr());
                    break;

                case Game::PlayerNode: {
                    int fighterIdx = index.row();
                    if (index.column() == 0) return QString::fromUtf8(meta->playerTag(fighterIdx).cStr());
                } break;

                case Game::PlayerName:
                    if (index.column() == 0) return "Name";
                    if (index.column() == 1)
                    {
                        int fighterIdx = (index.internalId() >> 16) & 0xFFFF;
                        const auto sponsor = QString::fromUtf8(meta->playerSponsor(fighterIdx).cStr());
                        const auto name = QString::fromUtf8(meta->playerName(fighterIdx).cStr());
                        return sponsor.isEmpty() ? name : sponsor + " | " + name;
                    }
                    break;

                case Game::PlayerFighter:
                    if (index.column() == 0) return "Fighter";
                    if (index.column() == 1)
                    {
                        int fighterIdx = (index.internalId() >> 16) & 0xFFFF;
                        const auto fighterID = meta->playerFighterID(fighterIdx);
                        const char* fighterName = map_ ?map_->fighter.toName(fighterID) : "no mapping info";
                        return QString::number(fighterID.value()) + " (" + QString::fromUtf8(fighterName) + ")";
                    }
                    break;

                case Game::PlayerCostume:
                    if (index.column() == 0) return "Costume";
                    if (index.column() == 1)
                    {
                        int fighterIdx = (index.internalId() >> 16) & 0xFFFF;
                        const auto costume = meta->playerCostume(fighterIdx);
                        return QString::number(costume.value());
                    }

                case Game::PlayerSocial:
                    if (index.column() == 0) return "Social";
                    if (index.column() == 1)
                    {
                        int fighterIdx = (index.internalId() >> 16) & 0xFFFF;
                        return QString::fromUtf8(meta->playerSocial(fighterIdx).cStr());
                    }
                    break;

                case Game::PlayerPronouns:
                    if (index.column() == 0) return "Pronouns";
                    if (index.column() == 1)
                    {
                        int fighterIdx = (index.internalId() >> 16) & 0xFFFF;
                        return QString::fromUtf8(meta->playerPronouns(fighterIdx).cStr());
                    }
                    break;

            }
        } break;
    }

    return QVariant();
}

// ----------------------------------------------------------------------------
QVariant MetadataModel::trainingData(const rfcommon::TrainingMetadata* meta, const QModelIndex& index, int role) const
{
    PROFILE(MetadataModel, trainingData);

    return QVariant();
}

// ----------------------------------------------------------------------------
void MetadataModel::onMetadataTimeChanged(rfcommon::TimeStamp timeStarted, rfcommon::TimeStamp timeEnded)
{
    PROFILE(MetadataModel, onMetadataTimeChanged);

}

// ----------------------------------------------------------------------------
void MetadataModel::onMetadataTournamentDetailsChanged() {}

// ----------------------------------------------------------------------------
void MetadataModel::onMetadataEventDetailsChanged() {}

// ----------------------------------------------------------------------------
void MetadataModel::onMetadataCommentatorsChanged() {}

// ----------------------------------------------------------------------------
void MetadataModel::onMetadataGameDetailsChanged()
{
    PROFILE(MetadataModel, onMetadataGameDetailsChanged);

}

// ----------------------------------------------------------------------------
void MetadataModel::onMetadataPlayerDetailsChanged()
{
    PROFILE(MetadataModel, onMetadataPlayerDetailsChanged);
}

// ----------------------------------------------------------------------------
void MetadataModel::onMetadataWinnerChanged(int winnerPlayerIdx)
{
    PROFILE(MetadataModel, onMetadataWinnerChanged);
}

// ----------------------------------------------------------------------------
void MetadataModel::onMetadataTrainingSessionNumberChanged(rfcommon::SessionNumber number) {}

