#include "uh/models/SavedRecording.hpp"
#include "uh/models/MappingInfo.hpp"
#include "uh/models/PlayerState.hpp"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QDataStream>

namespace uh {

// ----------------------------------------------------------------------------
SavedRecording::SavedRecording(MappingInfo&& mapping,
                               QVector<uint8_t>&& playerFighterIDs,
                               QVector<QString>&& playerTags,
                               uint16_t stageID)
    : Recording(std::move(mapping),
                std::move(playerFighterIDs),
                std::move(playerTags),
                stageID)
{
}

// ----------------------------------------------------------------------------
SavedRecording* SavedRecording::load(const QString& fileName)
{

    QFile f(fileName);
    if (!f.open(QIODevice::ReadOnly))
        return nullptr;

    const QJsonObject json = QJsonDocument::fromJson(f.readAll()).object();
    if (json.contains("version") == false || json["version"].isString() == false)
        return nullptr;

    SavedRecording* recording;
    QString version = json["version"].toString();
    if (version == "1.0")
    {
        if ((recording = loadVersion_1_0(json)) != nullptr)
            return recording;
    }
    else if (version == "1.1")
    {
        if ((recording = loadVersion_1_1(json)) != nullptr)
            return recording;
    }

    return nullptr;
}

// ----------------------------------------------------------------------------
SavedRecording* SavedRecording::loadVersion_1_0(const QJsonObject& json)
{
    if (json.contains("mappinginfo") == false || json["mappinginfo"].isObject() == false)
        return nullptr;
    if (json.contains("gameinfo") == false || json["gameinfo"].isObject() == false)
        return nullptr;
    if (json.contains("playerinfo") == false || json["playerinfo"].isArray() == false)
        return nullptr;
    if (json.contains("playerstates") == false || json["playerstates"].isString() == false)
        return nullptr;

    const QJsonObject jsonMappingInfo = json["mappinginfo"].toObject();
    const QJsonObject jsonGameInfo = json["gameinfo"].toObject();
    const QJsonArray jsonPlayerInfo = json["playerinfo"].toArray();
    const QString jsonPlayerStates = json["playerstates"].toString();

    if (jsonMappingInfo.contains("fighterstatus") == false || jsonMappingInfo["fighterstatus"].isObject() == false)
        return nullptr;
    if (jsonMappingInfo.contains("fighterid") == false || jsonMappingInfo["fighterid"].isObject() == false)
        return nullptr;
    if (jsonMappingInfo.contains("stageid") == false || jsonMappingInfo["stageid"].isObject() == false)
        return nullptr;

    MappingInfo mappingInfo;

    const QJsonObject jsonFighterIDMapping = jsonMappingInfo["fighterid"].toObject();
    for (auto it = jsonFighterIDMapping.begin(); it != jsonFighterIDMapping.end(); ++it)
    {
        bool ok;
        uint8_t id = it.key().toUInt(&ok);
        if (!ok)
            return nullptr;
        if (it.value().isString() == false)
            return nullptr;

        mappingInfo.fighterID.add(id, it.value().toString());
    }

    const QJsonObject jsonStageIDMapping = jsonMappingInfo["stageid"].toObject();
    for (auto it = jsonStageIDMapping.begin(); it != jsonStageIDMapping.end(); ++it)
    {
        bool ok;
        uint16_t id = it.key().toUInt(&ok);
        if (!ok)
            return nullptr;
        if (it.value().isString() == false)
            return nullptr;

        mappingInfo.stageID.add(id, it.value().toString());
    }

    // skip loading status mappings, it was broken in 1.0
    /*const QJsonObject jsonFighterStatusMapping = jsonMappingInfo["fighterstatus"];*/

    QVector<uint8_t> playerFighterIDs;
    QVector<QString> playerTags;
    for (const auto& infoItem : jsonPlayerInfo)
    {
        const QJsonObject info = infoItem.toObject();
        if (info.contains("fighterid") == false || info["fighterid"].isDouble() == false)
            return nullptr;
        if (info.contains("tag") == false || info["tag"].isString() == false)
            return nullptr;

        playerFighterIDs.push_back(static_cast<uint8_t>(info["fighterid"].toInt()));
        playerTags.push_back(info["tag"].toString());
    }

    if (jsonGameInfo.contains("date") == false || jsonGameInfo["date"].isString() == false)
        return nullptr;
    if (jsonGameInfo.contains("format") == false || jsonGameInfo["format"].isString() == false)
        return nullptr;
    if (jsonGameInfo.contains("number") == false || jsonGameInfo["number"].isDouble() == false)
        return nullptr;
    if (jsonGameInfo.contains("stageid") == false || jsonGameInfo["stageid"].isDouble() == false)
        return nullptr;

    QScopedPointer<SavedRecording> recording(new SavedRecording(
        std::move(mappingInfo),
        std::move(playerFighterIDs),
        std::move(playerTags),
        jsonGameInfo["stageid"].toInt()
    ));

    recording->timeStarted_ = QDateTime::fromString(jsonGameInfo["date"].toString()).toLocalTime();
    recording->format_ = SetFormat(jsonGameInfo["format"].toString());
    recording->gameNumber_ = jsonGameInfo["number"].toInt();

    QByteArray stream_data = QByteArray::fromBase64(jsonPlayerStates.toUtf8(), QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
    QDataStream stream(&stream_data, QIODevice::ReadOnly);
    for (int i = 0; i < recording->playerCount(); ++i)
    {
        quint32 frameCount; stream >> frameCount;
        for (quint32 f = 0; f < frameCount; ++f)
        {
            quint32 frame;  stream >> frame;
            quint16 status; stream >> status;
            qreal damage;   stream >> damage;
            quint8 stocks;  stream >> stocks;
            recording->playerStates_[i].push_back(PlayerState(frame, status, damage, stocks));
        }
    }

    return recording.take();
}

// ----------------------------------------------------------------------------
SavedRecording* SavedRecording::loadVersion_1_1(const QJsonObject& json)
{
    if (json.contains("mappinginfo") == false || json["mappinginfo"].isObject() == false)
        return nullptr;
    if (json.contains("gameinfo") == false || json["gameinfo"].isObject() == false)
        return nullptr;
    if (json.contains("playerinfo") == false || json["playerinfo"].isArray() == false)
        return nullptr;
    if (json.contains("playerstates") == false || json["playerstates"].isString() == false)
        return nullptr;

    const QJsonObject jsonMappingInfo = json["mappinginfo"].toObject();
    const QJsonObject jsonGameInfo = json["gameinfo"].toObject();
    const QJsonArray jsonPlayerInfo = json["playerinfo"].toArray();
    const QString jsonPlayerStates = json["playerstates"].toString();

    if (jsonMappingInfo.contains("fighterstatus") == false || jsonMappingInfo["fighterstatus"].isObject() == false)
        return nullptr;
    if (jsonMappingInfo.contains("fighterid") == false || jsonMappingInfo["fighterid"].isObject() == false)
        return nullptr;
    if (jsonMappingInfo.contains("stageid") == false || jsonMappingInfo["stageid"].isObject() == false)
        return nullptr;

    MappingInfo mappingInfo;

    const QJsonObject jsonFighterStatusMapping = jsonMappingInfo["fighterstatus"].toObject();
    if (jsonFighterStatusMapping.contains("base") == false || jsonFighterStatusMapping["base"].isObject() == false)
        return nullptr;
    if (jsonFighterStatusMapping.contains("specific") == false || jsonFighterStatusMapping["specific"].isObject() == false)
        return nullptr;

    const QJsonObject jsonFighterBaseStatusMapping = jsonFighterStatusMapping["base"].toObject();
    const QJsonObject jsonFighterSpecificStatusMapping = jsonFighterStatusMapping["specific"].toObject();
    for (auto it = jsonFighterBaseStatusMapping.begin(); it != jsonFighterBaseStatusMapping.end(); ++it)
    {
        bool ok;
        uint16_t status = it.key().toUInt(&ok);
        if (!ok)
            return nullptr;
        if (it.value().isString() == false)
            return nullptr;

        mappingInfo.fighterStatus.addBaseEnumName(status, it.value().toString());
    }
    for (auto fighterit = jsonFighterSpecificStatusMapping.begin(); fighterit != jsonFighterSpecificStatusMapping.end(); ++fighterit)
    {
        bool ok;
        uint8_t fighterID = fighterit.key().toUInt(&ok);
        if (!ok)
            return nullptr;
        if (fighterit.value().isObject() == false)
            return nullptr;

        const QJsonObject jsonSpecificMapping = fighterit.value().toObject();
        for (auto it = jsonSpecificMapping.begin(); it != jsonSpecificMapping.end(); ++it)
        {
            uint16_t status = it.key().toUInt(&ok);
            if (!ok)
                return nullptr;
            if (it.value().isString() == false)
                return nullptr;

            mappingInfo.fighterStatus.addFighterSpecificEnumName(status, fighterID, it.value().toString());
        }
    }

    const QJsonObject jsonFighterIDMapping = jsonMappingInfo["fighterid"].toObject();
    for (auto it = jsonFighterIDMapping.begin(); it != jsonFighterIDMapping.end(); ++it)
    {
        bool ok;
        uint8_t id = it.key().toUInt(&ok);
        if (!ok)
            return nullptr;
        if (it.value().isString() == false)
            return nullptr;

        mappingInfo.fighterID.add(id, it.value().toString());
    }

    const QJsonObject jsonStageIDMapping = jsonMappingInfo["stageid"].toObject();
    for (auto it = jsonStageIDMapping.begin(); it != jsonStageIDMapping.end(); ++it)
    {
        bool ok;
        uint16_t id = it.key().toUInt(&ok);
        if (!ok)
            return nullptr;
        if (it.value().isString() == false)
            return nullptr;

        mappingInfo.stageID.add(id, it.value().toString());
    }

    QVector<uint8_t> playerFighterIDs;
    QVector<QString> playerTags;
    for (const auto& infoValue : jsonPlayerInfo)
    {
        const QJsonObject info = infoValue.toObject();
        if (info.contains("fighterid") == false || info["fighterid"].isDouble() == false)
            return nullptr;
        if (info.contains("tag") == false || info["tag"].isString() == false)
            return nullptr;

        playerFighterIDs.push_back(static_cast<uint8_t>(info["fighterid"].toInt()));
        playerTags.push_back(info["tag"].toString());
    }

    if (jsonGameInfo.contains("date") == false || jsonGameInfo["date"].isString() == false)
        return nullptr;
    if (jsonGameInfo.contains("format") == false || jsonGameInfo["format"].isString() == false)
        return nullptr;
    if (jsonGameInfo.contains("number") == false || jsonGameInfo["number"].isDouble() == false)
        return nullptr;
    if (jsonGameInfo.contains("stageid") == false || jsonGameInfo["stageid"].isDouble() == false)
        return nullptr;

    QScopedPointer<SavedRecording> recording(new SavedRecording(
        std::move(mappingInfo),
        std::move(playerFighterIDs),
        std::move(playerTags),
        jsonGameInfo["stageid"].toInt()
    ));

    recording->timeStarted_ = QDateTime::fromString(jsonGameInfo["date"].toString()).toLocalTime();
    recording->format_ = SetFormat(jsonGameInfo["format"].toString());
    recording->gameNumber_ = jsonGameInfo["number"].toInt();

    QByteArray stream_data = QByteArray::fromBase64(jsonPlayerStates.toUtf8(), QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
    QDataStream stream(&stream_data, QIODevice::ReadOnly);
    for (int i = 0; i < recording->playerCount(); ++i)
    {
        quint32 frameCount; stream >> frameCount;
        for (quint32 f = 0; f < frameCount; ++f)
        {
            quint32 frame;  stream >> frame;
            quint16 status; stream >> status;
            qreal damage;   stream >> damage;
            quint8 stocks;  stream >> stocks;
            recording->playerStates_[i].push_back(PlayerState(frame, status, damage, stocks));
        }
    }

    return recording.take();
}

}
