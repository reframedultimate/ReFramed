#include "uh/SavedRecording.hpp"
#include "uh/MappingInfo.hpp"
#include "uh/PlayerState.hpp"
#include "uh/StreamBuffer.hpp"
#include "uh/time.h"
#include "nlohmann/json.hpp"
#include "cpp-base64/base64.h"
#include "zlib.h"

namespace uh {

using nlohmann::json;

// ----------------------------------------------------------------------------
SavedRecording::SavedRecording(MappingInfo&& mapping,
                               std::vector<uint8_t>&& playerFighterIDs,
                               std::vector<std::string>&& playerTags,
                               uint16_t stageID)
    : Recording(std::move(mapping),
                std::move(playerFighterIDs),
                std::move(playerTags),
                stageID)
{
}

// ----------------------------------------------------------------------------
SavedRecording* SavedRecording::load(const std::string& fileName)
{

    auto decompressFile = [](const std::string& fileName) -> std::string {
        gzFile f = gzopen(fileName.c_str(), "rb");
        if (f == nullptr)
            return "";
        if (gzbuffer(f, 128 * 1024) != 0)
        {
            gzclose(f);
            return "";
        }

        std::string buf;
        while (true)
        {
            size_t prevSize = buf.size();
            buf.resize(buf.size() + 0x1000, '\0');
            void* dst = static_cast<void*>(buf.data() + prevSize);
            int len = gzread(f, dst, 0x1000 - 1);
            if (len < 0)
                return "";
            if (len < 0x1000 - 1)
            {
                if (gzeof(f))
                    return buf;
                return "";
            }
        }
    };

    std::string s = decompressFile(fileName);
    if (s.length() == 0)
        return nullptr;
    json j = json::parse(std::move(s));

    if (j.contains("version") == false || j["version"].is_string() == false)
        return nullptr;

    SavedRecording* recording;
    std::string version = j["version"];
    if (version == "1.4")
    {
        if ((recording = loadVersion_1_4(static_cast<const void*>(&j))) != nullptr)
            return recording;
    }
    else if (version == "1.3")
    {
        if ((recording = loadVersion_1_3(static_cast<const void*>(&j))) != nullptr)
            return recording;
    }
    else if (version == "1.2")
    {
        if ((recording = loadVersion_1_2(static_cast<const void*>(&j))) != nullptr)
            return recording;
    }
    else if (version == "1.1")
    {
        if ((recording = loadVersion_1_1(static_cast<const void*>(&j))) != nullptr)
            return recording;
    }
    else if (version == "1.0")
    {
        if ((recording = loadVersion_1_0(static_cast<const void*>(&j))) != nullptr)
            return recording;
    }

    return nullptr;
}

// ----------------------------------------------------------------------------
SavedRecording* SavedRecording::loadVersion_1_0(const void* jptr)
{
    const json& j = *static_cast<const json*>(jptr);
    if (j.contains("mappinginfo") == false || j["mappinginfo"].is_object() == false)
        return nullptr;
    if (j.contains("gameinfo") == false || j["gameinfo"].is_object() == false)
        return nullptr;
    if (j.contains("playerinfo") == false || j["playerinfo"].is_array() == false)
        return nullptr;
    if (j.contains("playerstates") == false || j["playerstates"].is_string() == false)
        return nullptr;

    const json jsonMappingInfo = j["mappinginfo"];
    const json jsonGameInfo = j["gameinfo"];
    const json jsonPlayerInfo = j["playerinfo"];

    if (jsonMappingInfo.contains("fighterstatus") == false || jsonMappingInfo["fighterstatus"].is_object() == false)
        return nullptr;
    if (jsonMappingInfo.contains("fighterid") == false || jsonMappingInfo["fighterid"].is_object() == false)
        return nullptr;
    if (jsonMappingInfo.contains("stageid") == false || jsonMappingInfo["stageid"].is_object() == false)
        return nullptr;

    MappingInfo mappingInfo;

    const json jsonFighterIDMapping = jsonMappingInfo["fighterid"];
    for (const auto& [key, value] : jsonFighterIDMapping.items())
    {
        std::size_t pos;
        uint8_t id = std::stoul(key, &pos);
        if (pos != key.length())
            return nullptr;
        if (value.is_string() == false)
            return nullptr;

        mappingInfo.fighterID.add(id, value.get<std::string>());
    }

    const json jsonStageIDMapping = jsonMappingInfo["stageid"];
    for (const auto& [key, value] : jsonStageIDMapping.items())
    {
        std::size_t pos;
        uint8_t id = std::stoul(key, &pos);
        if (pos != key.length())
            return nullptr;
        if (value.is_string() == false)
            return nullptr;

        mappingInfo.stageID.add(id, value.get<std::string>());
    }

    // skip loading status mappings, it was broken in 1.0
    /*const QJsonObject jsonFighterStatusMapping = jsonMappingInfo["fighterstatus"];*/

    std::vector<uint8_t> playerFighterIDs;
    std::vector<std::string> playerTags;
    for (const auto& info : jsonPlayerInfo)
    {
        if (info.contains("fighterid") == false || info["fighterid"].is_number() == false)
            return nullptr;
        if (info.contains("tag") == false || info["tag"].is_string() == false)
            return nullptr;

        playerFighterIDs.push_back(info["fighterid"].get<uint8_t>());
        playerTags.push_back(info["tag"].get<std::string>());
    }

    if (jsonGameInfo.contains("date") == false || jsonGameInfo["date"].is_string() == false)
        return nullptr;
    if (jsonGameInfo.contains("format") == false || jsonGameInfo["format"].is_string() == false)
        return nullptr;
    if (jsonGameInfo.contains("number") == false || jsonGameInfo["number"].is_number() == false)
        return nullptr;
    if (jsonGameInfo.contains("stageid") == false || jsonGameInfo["stageid"].is_number() == false)
        return nullptr;

    std::unique_ptr<SavedRecording> recording(new SavedRecording(
        std::move(mappingInfo),
        std::move(playerFighterIDs),
        std::move(playerTags),
        jsonGameInfo["stageid"].get<uint16_t>()
    ));


    recording->timeStarted_ = time_qt_to_milli_seconds_since_epoch(jsonGameInfo["date"].get<std::string>().c_str());
    recording->format_ = SetFormat(jsonGameInfo["format"].get<std::string>());
    recording->gameNumber_ = jsonGameInfo["number"].get<uint8_t>();

    StreamBuffer stream(base64_decode(j["playerstates"].get<std::string>()));
    for (int i = 0; i < recording->playerCount(); ++i)
    {
        if (stream.readBytesLeft() < 4)
            return nullptr;
        uint32_t frameCount = stream.readU32();
        if (stream.readBytesLeft() < (4+2+8+1) * frameCount)
            return nullptr;

        for (uint32_t f = 0; f < frameCount; ++f)
        {
            uint32_t frame = stream.readU32();
            uint16_t status = stream.readU16();
            float damage = stream.readF64();
            uint8_t stocks = stream.readU8();
            recording->playerStates_[i].push_back(PlayerState(frame, 0.0, 0.0, damage, 0.0, 50.0, status, 0, 0, stocks, false, false));
        }
    }

    recording->winner_ = recording->findWinner();

    return recording.release();
}

// ----------------------------------------------------------------------------
SavedRecording* SavedRecording::loadVersion_1_1(const QJsonObject& json)
{
    if (json.contains("mappinginfo") == false || json["mappinginfo"].is_object() == false)
        return nullptr;
    if (json.contains("gameinfo") == false || json["gameinfo"].is_object() == false)
        return nullptr;
    if (json.contains("playerinfo") == false || json["playerinfo"].is_array() == false)
        return nullptr;
    if (json.contains("playerstates") == false || json["playerstates"].is_string() == false)
        return nullptr;

    const QJsonObject jsonMappingInfo = json["mappinginfo"];
    const QJsonObject jsonGameInfo = json["gameinfo"];
    const QJsonArray jsonPlayerInfo = json["playerinfo"].toArray();
    const QString jsonPlayerStates = json["playerstates"].get<std::string>();

    if (jsonMappingInfo.contains("fighterstatus") == false || jsonMappingInfo["fighterstatus"].is_object() == false)
        return nullptr;
    if (jsonMappingInfo.contains("fighterid") == false || jsonMappingInfo["fighterid"].is_object() == false)
        return nullptr;
    if (jsonMappingInfo.contains("stageid") == false || jsonMappingInfo["stageid"].is_object() == false)
        return nullptr;

    MappingInfo mappingInfo;

    const QJsonObject jsonFighterStatusMapping = jsonMappingInfo["fighterstatus"];
    if (jsonFighterStatusMapping.contains("base") == false || jsonFighterStatusMapping["base"].is_object() == false)
        return nullptr;
    if (jsonFighterStatusMapping.contains("specific") == false || jsonFighterStatusMapping["specific"].is_object() == false)
        return nullptr;

    const QJsonObject jsonFighterBaseStatusMapping = jsonFighterStatusMapping["base"];
    const QJsonObject jsonFighterSpecificStatusMapping = jsonFighterStatusMapping["specific"];
    for (auto it = jsonFighterBaseStatusMapping.begin(); it != jsonFighterBaseStatusMapping.end(); ++it)
    {
        bool ok;
        uint16_t status = it.key().toUInt(&ok);
        if (!ok)
            return nullptr;
        if (it.value().is_array() == false)
            return nullptr;

        QJsonArray arr = it.value().toArray();
        if (arr.size() != 3)
            return nullptr;
        if (arr[0].is_string() == false || arr[1].is_string() == false || arr[2].is_string() == false)
            return nullptr;

        QString enumName   = arr[0].get<std::string>();
        /*QString shortName  = arr[1].get<std::string>();
        QString customName = arr[2].get<std::string>();*/

        mappingInfo.fighterStatus.addBaseEnumName(status, enumName);
    }
    for (auto fighterit = jsonFighterSpecificStatusMapping.begin(); fighterit != jsonFighterSpecificStatusMapping.end(); ++fighterit)
    {
        bool ok;
        uint8_t fighterID = fighterit.key().toUInt(&ok);
        if (!ok)
            return nullptr;
        if (fighterit.value().is_object() == false)
            return nullptr;

        const QJsonObject jsonSpecificMapping = fighterit.value();
        for (auto it = jsonSpecificMapping.begin(); it != jsonSpecificMapping.end(); ++it)
        {
            uint16_t status = it.key().toUInt(&ok);
            if (!ok)
                return nullptr;
            if (it.value().is_array() == false)
                return nullptr;

            QJsonArray arr = it.value().toArray();
            if (arr.size() != 3)
                return nullptr;
            if (arr[0].is_string() == false || arr[1].is_string() == false || arr[2].is_string() == false)
                return nullptr;

            QString enumName   = arr[0].get<std::string>();
            /*QString shortName  = arr[1].get<std::string>();
            QString customName = arr[2].get<std::string>();*/

            mappingInfo.fighterStatus.addFighterSpecificEnumName(status, fighterID, enumName);
        }
    }

    const QJsonObject jsonFighterIDMapping = jsonMappingInfo["fighterid"];
    for (auto it = jsonFighterIDMapping.begin(); it != jsonFighterIDMapping.end(); ++it)
    {
        bool ok;
        uint8_t id = it.key().toUInt(&ok);
        if (!ok)
            return nullptr;
        if (it.value().is_string() == false)
            return nullptr;

        mappingInfo.fighterID.add(id, it.value().get<std::string>());
    }

    const QJsonObject jsonStageIDMapping = jsonMappingInfo["stageid"];
    for (auto it = jsonStageIDMapping.begin(); it != jsonStageIDMapping.end(); ++it)
    {
        bool ok;
        uint16_t id = it.key().toUInt(&ok);
        if (!ok)
            return nullptr;
        if (it.value().is_string() == false)
            return nullptr;

        mappingInfo.stageID.add(id, it.value().get<std::string>());
    }

    QVector<uint8_t> playerFighterIDs;
    QVector<QString> playerTags;
    for (const auto& infoValue : jsonPlayerInfo)
    {
        const QJsonObject info = infoValue;
        if (info.contains("fighterid") == false || info["fighterid"].is_number() == false)
            return nullptr;
        if (info.contains("tag") == false || info["tag"].is_string() == false)
            return nullptr;

        playerFighterIDs.push_back(static_cast<uint8_t>(info["fighterid"].toInt()));
        playerTags.push_back(info["tag"].get<std::string>());
    }

    if (jsonGameInfo.contains("date") == false || jsonGameInfo["date"].is_string() == false)
        return nullptr;
    if (jsonGameInfo.contains("format") == false || jsonGameInfo["format"].is_string() == false)
        return nullptr;
    if (jsonGameInfo.contains("number") == false || jsonGameInfo["number"].is_number() == false)
        return nullptr;
    if (jsonGameInfo.contains("stageid") == false || jsonGameInfo["stageid"].is_number() == false)
        return nullptr;

    QScopedPointer<SavedRecording> recording(new SavedRecording(
        std::move(mappingInfo),
        std::move(playerFighterIDs),
        std::move(playerTags),
        jsonGameInfo["stageid"].toInt()
    ));

    recording->timeStarted_ = QDateTime::fromString(jsonGameInfo["date"].get<std::string>()).toLocalTime();
    recording->format_ = SetFormat(jsonGameInfo["format"].get<std::string>());
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
            recording->playerStates_[i].push_back(PlayerState(frame, 0.0, 0.0, damage, 0.0, 50.0, status, 0, 0, stocks, false, false));
        }
    }

    recording->winner_ = recording->findWinner();

    return recording.take();
}

// ----------------------------------------------------------------------------
SavedRecording* SavedRecording::loadVersion_1_2(const QJsonObject& json)
{
    if (json.contains("mappinginfo") == false || json["mappinginfo"].is_object() == false)
        return nullptr;
    if (json.contains("gameinfo") == false || json["gameinfo"].is_object() == false)
        return nullptr;
    if (json.contains("playerinfo") == false || json["playerinfo"].is_array() == false)
        return nullptr;
    if (json.contains("playerstates") == false || json["playerstates"].is_string() == false)
        return nullptr;

    const QJsonObject jsonMappingInfo = json["mappinginfo"];
    const QJsonObject jsonGameInfo = json["gameinfo"];
    const QJsonArray jsonPlayerInfo = json["playerinfo"].toArray();
    const QString jsonPlayerStates = json["playerstates"].get<std::string>();

    if (jsonMappingInfo.contains("fighterstatus") == false || jsonMappingInfo["fighterstatus"].is_object() == false)
        return nullptr;
    if (jsonMappingInfo.contains("fighterid") == false || jsonMappingInfo["fighterid"].is_object() == false)
        return nullptr;
    if (jsonMappingInfo.contains("stageid") == false || jsonMappingInfo["stageid"].is_object() == false)
        return nullptr;

    MappingInfo mappingInfo;

    const QJsonObject jsonFighterStatusMapping = jsonMappingInfo["fighterstatus"];
    if (jsonFighterStatusMapping.contains("base") == false || jsonFighterStatusMapping["base"].is_object() == false)
        return nullptr;
    if (jsonFighterStatusMapping.contains("specific") == false || jsonFighterStatusMapping["specific"].is_object() == false)
        return nullptr;

    const QJsonObject jsonFighterBaseStatusMapping = jsonFighterStatusMapping["base"];
    const QJsonObject jsonFighterSpecificStatusMapping = jsonFighterStatusMapping["specific"];
    for (auto it = jsonFighterBaseStatusMapping.begin(); it != jsonFighterBaseStatusMapping.end(); ++it)
    {
        bool ok;
        uint16_t status = it.key().toUInt(&ok);
        if (!ok)
            return nullptr;
        if (it.value().is_array() == false)
            return nullptr;

        QJsonArray arr = it.value().toArray();
        if (arr.size() != 3)
            return nullptr;
        if (arr[0].is_string() == false || arr[1].is_string() == false || arr[2].is_string() == false)
            return nullptr;

        QString enumName   = arr[0].get<std::string>();
        /*QString shortName  = arr[1].get<std::string>();
        QString customName = arr[2].get<std::string>();*/

        mappingInfo.fighterStatus.addBaseEnumName(status, enumName);
    }
    for (auto fighterit = jsonFighterSpecificStatusMapping.begin(); fighterit != jsonFighterSpecificStatusMapping.end(); ++fighterit)
    {
        bool ok;
        uint8_t fighterID = fighterit.key().toUInt(&ok);
        if (!ok)
            return nullptr;
        if (fighterit.value().is_object() == false)
            return nullptr;

        const QJsonObject jsonSpecificMapping = fighterit.value();
        for (auto it = jsonSpecificMapping.begin(); it != jsonSpecificMapping.end(); ++it)
        {
            uint16_t status = it.key().toUInt(&ok);
            if (!ok)
                return nullptr;
            if (it.value().is_array() == false)
                return nullptr;

            QJsonArray arr = it.value().toArray();
            if (arr.size() != 3)
                return nullptr;
            if (arr[0].is_string() == false || arr[1].is_string() == false || arr[2].is_string() == false)
                return nullptr;

            QString enumName   = arr[0].get<std::string>();
            /*QString shortName  = arr[1].get<std::string>();
            QString customName = arr[2].get<std::string>();*/

            mappingInfo.fighterStatus.addFighterSpecificEnumName(status, fighterID, enumName);
        }
    }

    const QJsonObject jsonFighterIDMapping = jsonMappingInfo["fighterid"];
    for (auto it = jsonFighterIDMapping.begin(); it != jsonFighterIDMapping.end(); ++it)
    {
        bool ok;
        uint8_t id = it.key().toUInt(&ok);
        if (!ok)
            return nullptr;
        if (it.value().is_string() == false)
            return nullptr;

        mappingInfo.fighterID.add(id, it.value().get<std::string>());
    }

    const QJsonObject jsonStageIDMapping = jsonMappingInfo["stageid"];
    for (auto it = jsonStageIDMapping.begin(); it != jsonStageIDMapping.end(); ++it)
    {
        bool ok;
        uint16_t id = it.key().toUInt(&ok);
        if (!ok)
            return nullptr;
        if (it.value().is_string() == false)
            return nullptr;

        mappingInfo.stageID.add(id, it.value().get<std::string>());
    }

    QVector<uint8_t> playerFighterIDs;
    QVector<QString> playerTags;
    QVector<QString> playerNames;
    for (const auto& infoValue : jsonPlayerInfo)
    {
        const QJsonObject info = infoValue;
        if (info.contains("fighterid") == false || info["fighterid"].is_number() == false)
            return nullptr;
        if (info.contains("tag") == false || info["tag"].is_string() == false)
            return nullptr;
        if (info.contains("name") == false || info["name"].is_string() == false)
            return nullptr;

        playerFighterIDs.push_back(static_cast<uint8_t>(info["fighterid"].toInt()));
        playerTags.push_back(info["tag"].get<std::string>());
        playerNames.push_back(info["name"].get<std::string>());
    }

    if (jsonGameInfo.contains("stageid") == false || jsonGameInfo["stageid"].is_number() == false)
        return nullptr;
    if (jsonGameInfo.contains("date") == false || jsonGameInfo["date"].is_string() == false)
        return nullptr;
    if (jsonGameInfo.contains("format") == false || jsonGameInfo["format"].is_string() == false)
        return nullptr;
    if (jsonGameInfo.contains("number") == false || jsonGameInfo["number"].is_number() == false)
        return nullptr;
    if (jsonGameInfo.contains("set") == false || jsonGameInfo["set"].is_number() == false)
        return nullptr;

    QScopedPointer<SavedRecording> recording(new SavedRecording(
        std::move(mappingInfo),
        std::move(playerFighterIDs),
        std::move(playerTags),
        jsonGameInfo["stageid"].toInt()
    ));

    recording->timeStarted_ = QDateTime::fromString(jsonGameInfo["date"].get<std::string>()).toLocalTime();
    recording->format_ = SetFormat(jsonGameInfo["format"].get<std::string>());
    recording->gameNumber_ = jsonGameInfo["number"].toInt();
    recording->setNumber_ = jsonGameInfo["set"].toInt();
    recording->playerNames_ = std::move(playerNames);

    QByteArray stream_data = QByteArray::fromBase64(jsonPlayerStates.toUtf8(), QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
    QDataStream stream(&stream_data, QIODevice::ReadOnly);
    for (int i = 0; i < recording->playerCount(); ++i)
    {
        quint32 frameCount; stream >> frameCount;
        for (quint32 f = 0; f < frameCount; ++f)
        {
            quint32 frame;     stream >> frame;
            float posx;        stream >> posx;
            float posy;        stream >> posy;
            float damage;      stream >> damage;
            float hitstun;     stream >> hitstun;
            float shield;      stream >> shield;
            quint16 status;    stream >> status;
            quint64 motion;    stream >> motion;
            quint8 hit_status; stream >> hit_status;
            quint8 stocks;     stream >> stocks;
            quint8 flags;      stream >> flags;

            bool attack_connected = !!(flags & 0x01);
            bool facing_direction = !!(flags & 0x02);

            recording->playerStates_[i].push_back(PlayerState(frame, posx, posy, damage, hitstun, shield, status, motion, hit_status, stocks, attack_connected, facing_direction));
        }
    }

    recording->winner_ = recording->findWinner();

    return recording.take();
}

// ----------------------------------------------------------------------------
SavedRecording* SavedRecording::loadVersion_1_3(const QJsonObject& json)
{
    if (json.contains("mappinginfo") == false || json["mappinginfo"].is_object() == false)
        return nullptr;
    if (json.contains("gameinfo") == false || json["gameinfo"].is_object() == false)
        return nullptr;
    if (json.contains("playerinfo") == false || json["playerinfo"].is_array() == false)
        return nullptr;
    if (json.contains("playerstates") == false || json["playerstates"].is_string() == false)
        return nullptr;

    const QJsonObject jsonMappingInfo = json["mappinginfo"];
    const QJsonObject jsonGameInfo = json["gameinfo"];
    const QJsonArray jsonPlayerInfo = json["playerinfo"].toArray();
    const QString jsonPlayerStates = json["playerstates"].get<std::string>();

    if (jsonMappingInfo.contains("fighterstatus") == false || jsonMappingInfo["fighterstatus"].is_object() == false)
        return nullptr;
    if (jsonMappingInfo.contains("fighterid") == false || jsonMappingInfo["fighterid"].is_object() == false)
        return nullptr;
    if (jsonMappingInfo.contains("stageid") == false || jsonMappingInfo["stageid"].is_object() == false)
        return nullptr;
    if (jsonMappingInfo.contains("hitstatus") == false || jsonMappingInfo["hitstatus"].is_object() == false)
        return nullptr;

    MappingInfo mappingInfo;

    const QJsonObject jsonFighterStatusMapping = jsonMappingInfo["fighterstatus"];
    if (jsonFighterStatusMapping.contains("base") == false || jsonFighterStatusMapping["base"].is_object() == false)
        return nullptr;
    if (jsonFighterStatusMapping.contains("specific") == false || jsonFighterStatusMapping["specific"].is_object() == false)
        return nullptr;

    const QJsonObject jsonFighterBaseStatusMapping = jsonFighterStatusMapping["base"];
    const QJsonObject jsonFighterSpecificStatusMapping = jsonFighterStatusMapping["specific"];
    for (auto it = jsonFighterBaseStatusMapping.begin(); it != jsonFighterBaseStatusMapping.end(); ++it)
    {
        bool ok;
        uint16_t status = it.key().toUInt(&ok);
        if (!ok)
            return nullptr;
        if (it.value().is_array() == false)
            return nullptr;

        QJsonArray arr = it.value().toArray();
        if (arr.size() != 3)
            return nullptr;
        if (arr[0].is_string() == false || arr[1].is_string() == false || arr[2].is_string() == false)
            return nullptr;

        QString enumName   = arr[0].get<std::string>();
        /*QString shortName  = arr[1].get<std::string>();
        QString customName = arr[2].get<std::string>();*/

        mappingInfo.fighterStatus.addBaseEnumName(status, enumName);
    }
    for (auto fighterit = jsonFighterSpecificStatusMapping.begin(); fighterit != jsonFighterSpecificStatusMapping.end(); ++fighterit)
    {
        bool ok;
        uint8_t fighterID = fighterit.key().toUInt(&ok);
        if (!ok)
            return nullptr;
        if (fighterit.value().is_object() == false)
            return nullptr;

        const QJsonObject jsonSpecificMapping = fighterit.value();
        for (auto it = jsonSpecificMapping.begin(); it != jsonSpecificMapping.end(); ++it)
        {
            uint16_t status = it.key().toUInt(&ok);
            if (!ok)
                return nullptr;
            if (it.value().is_array() == false)
                return nullptr;

            QJsonArray arr = it.value().toArray();
            if (arr.size() != 3)
                return nullptr;
            if (arr[0].is_string() == false || arr[1].is_string() == false || arr[2].is_string() == false)
                return nullptr;

            QString enumName   = arr[0].get<std::string>();
            /*QString shortName  = arr[1].get<std::string>();
            QString customName = arr[2].get<std::string>();*/

            mappingInfo.fighterStatus.addFighterSpecificEnumName(status, fighterID, enumName);
        }
    }

    const QJsonObject jsonFighterIDMapping = jsonMappingInfo["fighterid"];
    for (auto it = jsonFighterIDMapping.begin(); it != jsonFighterIDMapping.end(); ++it)
    {
        bool ok;
        uint8_t id = it.key().toUInt(&ok);
        if (!ok)
            return nullptr;
        if (it.value().is_string() == false)
            return nullptr;

        mappingInfo.fighterID.add(id, it.value().get<std::string>());
    }

    const QJsonObject jsonStageIDMapping = jsonMappingInfo["stageid"];
    for (auto it = jsonStageIDMapping.begin(); it != jsonStageIDMapping.end(); ++it)
    {
        bool ok;
        uint16_t id = it.key().toUInt(&ok);
        if (!ok)
            return nullptr;
        if (it.value().is_string() == false)
            return nullptr;

        mappingInfo.stageID.add(id, it.value().get<std::string>());
    }

    const QJsonObject jsonHitStatusMapping = jsonMappingInfo["hitstatus"];
    for (auto it = jsonHitStatusMapping.begin(); it != jsonHitStatusMapping.end(); ++it)
    {
        bool ok;
        uint8_t id = it.key().toUInt(&ok);
        if (!ok)
            return nullptr;
        if (it.value().is_string() == false)
            return nullptr;

        mappingInfo.hitStatus.add(id, it.value().get<std::string>());
    }

    QVector<uint8_t> playerFighterIDs;
    QVector<QString> playerTags;
    QVector<QString> playerNames;
    for (const auto& infoValue : jsonPlayerInfo)
    {
        const QJsonObject info = infoValue;
        if (info.contains("fighterid") == false || info["fighterid"].is_number() == false)
            return nullptr;
        if (info.contains("tag") == false || info["tag"].is_string() == false)
            return nullptr;
        if (info.contains("name") == false || info["name"].is_string() == false)
            return nullptr;

        playerFighterIDs.push_back(static_cast<uint8_t>(info["fighterid"].toInt()));
        playerTags.push_back(info["tag"].get<std::string>());
        playerNames.push_back(info["name"].get<std::string>());
    }

    if (jsonGameInfo.contains("stageid") == false || jsonGameInfo["stageid"].is_number() == false)
        return nullptr;
    if (jsonGameInfo.contains("date") == false || jsonGameInfo["date"].is_string() == false)
        return nullptr;
    if (jsonGameInfo.contains("format") == false || jsonGameInfo["format"].is_string() == false)
        return nullptr;
    if (jsonGameInfo.contains("number") == false || jsonGameInfo["number"].is_number() == false)
        return nullptr;
    if (jsonGameInfo.contains("set") == false || jsonGameInfo["set"].is_number() == false)
        return nullptr;
    if (jsonGameInfo.contains("winner") == false || jsonGameInfo["winner"].is_number() == false)
        return nullptr;

    QScopedPointer<SavedRecording> recording(new SavedRecording(
        std::move(mappingInfo),
        std::move(playerFighterIDs),
        std::move(playerTags),
        jsonGameInfo["stageid"].toInt()
    ));

    recording->timeStarted_ = QDateTime::fromString(jsonGameInfo["date"].get<std::string>()).toLocalTime();
    recording->format_ = SetFormat(jsonGameInfo["format"].get<std::string>());
    recording->gameNumber_ = jsonGameInfo["number"].toInt();
    recording->setNumber_ = jsonGameInfo["set"].toInt();
    recording->playerNames_ = std::move(playerNames);

    QByteArray stream_data = QByteArray::fromBase64(jsonPlayerStates.toUtf8(), QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
    QDataStream stream(&stream_data, QIODevice::ReadOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream.setVersion(QDataStream::Qt_5_12);
    for (int i = 0; i < recording->playerCount(); ++i)
    {
        quint32 frameCount; stream >> frameCount;
        for (quint32 f = 0; f < frameCount; ++f)
        {
            quint32 frame;     stream >> frame;
            float posx;        stream >> posx;
            float posy;        stream >> posy;
            float damage;      stream >> damage;
            float hitstun;     stream >> hitstun;
            float shield;      stream >> shield;
            quint16 status;    stream >> status;
            quint32 motion_l;  stream >> motion_l;
            quint8 motion_h;   stream >> motion_h;
            quint8 hit_status; stream >> hit_status;
            quint8 stocks;     stream >> stocks;
            quint8 flags;      stream >> flags;

            quint64 motion = static_cast<quint64>(motion_l)
                          | (static_cast<quint64>(motion_h) << 32);

            bool attack_connected = !!(flags & 0x01);
            bool facing_direction = !!(flags & 0x02);

            recording->playerStates_[i].push_back(PlayerState(frame, posx, posy, damage, hitstun, shield, status, motion, hit_status, stocks, attack_connected, facing_direction));
        }
    }

    recording->winner_ = recording->findWinner();

    return recording.take();
}

}
