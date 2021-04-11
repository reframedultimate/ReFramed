#include "uh/models/Recording.hpp"
#include "uh/models/PlayerState.hpp"
#include <QFile>
#include <QDataStream>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMessageBox>
#include <QFileDialog>

namespace uh {

// ----------------------------------------------------------------------------
Recording::Recording(MappingInfo&& mapping,
                     QVector<uint8_t>&& playerFighterIDs,
                     QVector<QString>&& playerTags,
                     uint16_t stageID)
    : mappingInfo_(std::move(mapping))
    , timeStarted_(QDateTime::currentDateTime())
    , playerTags_(playerTags)
    , playerNames_(playerTags)
    , playerFighterIDs_(std::move(playerFighterIDs))
    , playerStates_(playerTags.size())
    , format_(SetFormat::FRIENDLIES)
    , stageID_(stageID)
{
    assert(playerTags_.size() == playerNames_.size());
    assert(playerTags_.size() == playerFighterIDs_.size());
    assert(playerStates_.size() == playerFighterIDs_.size());
}

// ----------------------------------------------------------------------------
bool Recording::saveAs(const QString& fileName)
{
    QSet<uint16_t> usedStatuses;
    for (const auto& player : playerStates_)
        for (const auto& state : player)
            usedStatuses.insert(state.status());

    QSet<uint8_t> usedFighterIDs;
    for (const auto& fighterID : playerFighterIDs_)
        usedFighterIDs.insert(fighterID);

    QJsonObject gameInfo;
    gameInfo["stageid"] = stageID_;
    gameInfo["date"] = timeStarted_.toUTC().toString();
    gameInfo["format"] = format_.description();
    gameInfo["number"] = gameNumber_;
    gameInfo["set"] = setNumber_;

    QJsonObject fighterBaseStatusMapping;
    const auto& baseEnumNames = mappingInfo_.fighterStatus.baseEnumNames();
    for (auto it = baseEnumNames.begin(); it != baseEnumNames.end(); ++it)
    {
        // Skip saving enums that aren't actually used in the set of player states
        if (usedStatuses.contains(it.key()) == false)
            continue;

        /*const QString* shortName = mappingInfo_.fighterStatus.mapToShortName(it.key());
        const QString* customName = mappingInfo_.fighterStatus.mapToCustom(it.key());*/

        QJsonArray mappings;
        mappings.append(it.value());
        mappings.append(/*shortName ? *shortName :*/ QString());
        mappings.append(/*customName ? *customName :*/ QString());
        fighterBaseStatusMapping[QString::number(it.key())] = mappings;
    }

    QJsonObject fighterSpecificStatusMapping;
    const auto& specificEnumNames = mappingInfo_.fighterStatus.fighterSpecificEnumNames();
    for (auto fighter = specificEnumNames.begin(); fighter != specificEnumNames.end(); ++fighter)
    {
        // Skip saving enums for fighters that aren't being used
        if (usedFighterIDs.contains(fighter.key()) == false)
            continue;

        QJsonObject specificMapping;
        for (auto it = fighter.value().begin(); it != fighter.value().end(); ++it)
        {
            // Skip saving enums that aren't actually used in the set of player states
            if (usedStatuses.contains(it.key()) == false)
                continue;

            /*const QString* shortName = mappingInfo_.fighterStatus.mapToShortName(it.key());
            const QString* customName = mappingInfo_.fighterStatus.mapToCustom(it.key());*/

            QJsonArray mappings;
            mappings.append(it.value());
            mappings.append(/*shortName ? *shortName :*/ QString());
            mappings.append(/*customName ? *customName :*/ QString());
            specificMapping[QString::number(it.key())] = mappings;
        }

        if (specificMapping.size() > 0)
            fighterSpecificStatusMapping[QString::number(fighter.key())] = specificMapping;
    }

    QJsonObject fighterStatusMapping;
    fighterStatusMapping["base"] = fighterBaseStatusMapping;
    fighterStatusMapping["specific"] = fighterSpecificStatusMapping;

    QJsonObject fighterIDMapping;
    const auto& fighterIDMap = mappingInfo_.fighterID.get();
    for (auto it = fighterIDMap.begin(); it != fighterIDMap.end(); ++it)
        if (usedFighterIDs.contains(it.key()))
            fighterIDMapping[QString::number(it.key())] = it.value();

    QJsonObject stageIDMapping;
    const auto& stageIDMap = mappingInfo_.stageID.get();
    for (auto it = stageIDMap.begin(); it != stageIDMap.end(); ++it)
        if (it.key() == stageID_)
            stageIDMapping[QString::number(it.key())] = it.value();

    QJsonObject mappingInfo;
    mappingInfo["fighterstatus"] = fighterStatusMapping;
    mappingInfo["fighterid"] = fighterIDMapping;
    mappingInfo["stageid"] = stageIDMapping;

    QJsonArray playerInfo;
    for (int i = 0; i < playerCount(); ++i)
    {
        QJsonObject player;
        player["tag"] = playerTags_[i];
        player["name"] = playerNames_[i];
        player["fighterid"] = playerFighterIDs_[i];
        playerInfo.append(player);
    }

    QByteArray stream_data;
    QDataStream stream(&stream_data, QIODevice::WriteOnly);
    for (const auto& states : playerStates_)
    {
        stream << static_cast<quint32>(states.size());
        for (const auto& state : states)
        {
            stream << static_cast<quint32>(state.frame());
            stream << static_cast<quint16>(state.status());
            stream << static_cast<qreal>(state.damage());
            stream << static_cast<quint8>(state.stocks());
        }
    }

    QJsonObject json;
    json["version"] = "1.1";
    json["mappinginfo"] = mappingInfo;
    json["gameinfo"] = gameInfo;
    json["playerinfo"] = playerInfo;
    json["playerstates"] = QString::fromUtf8(stream_data.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));

    QFile f(fileName);
    if (!f.open(QIODevice::WriteOnly))
    {/*
        if (QMessageBox::warning(nullptr, "Failed to save recording", QString("Failed to open file for writing: ") + f.fileName() + "\n\nWould you like to save the file manually?", QMessageBox::Save | QMessageBox::Discard) == QMessageBox::Save)
        {
            QFileDialog::getSaveFileName(nullptr, "Save Recording", f.fileName());
        }*/
        return false;
    }
    f.write(QJsonDocument(json).toJson());

    return true;
}

}
