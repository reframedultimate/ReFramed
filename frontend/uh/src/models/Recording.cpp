#include "uh/listeners/RecordingListener.hpp"
#include "uh/models/Recording.hpp"
#include "uh/models/PlayerState.hpp"
#include <QFile>
#include <QDataStream>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

namespace uh {

// ----------------------------------------------------------------------------
Recording::Recording(const MappingInfo& mapping,
                     const GameInfo& gameInfo,
                     QVector<PlayerInfo>&& playerInfos)
    : mappingInfo_(mapping)
    , gameInfo_(gameInfo)
    , playerInfo_(playerInfos)
    , playerStates_(playerInfos.size())
{
}

// ----------------------------------------------------------------------------
Recording* Recording::load(const QString& fileName)
{
    return nullptr;
}

// ----------------------------------------------------------------------------
bool Recording::saveTo(const QDir& path)
{
    QFile f(findNonExistingFileName(path));
    if (!f.open(QIODevice::WriteOnly))
        return false;

    QJsonObject gameInfo;
    gameInfo["stageid"] = gameInfo_.stageID();
    gameInfo["date"] = gameInfo_.timeStarted().toUTC().toString();
    gameInfo["format"] = gameInfo_.formatDesc();
    gameInfo["number"] = gameInfo_.gameNumber();

    QJsonObject fighterStatusMapping;
    const auto& baseEnumNames = mappingInfo_.fighterStatus.baseEnumNames();
    for (auto it = baseEnumNames.begin(); it != baseEnumNames.end(); ++it)
    {
        /*const QString* shortName = mappingInfo_.fighterStatus.mapToShortName(it.key());
        const QString* customName = mappingInfo_.fighterStatus.mapToCustom(it.key());*/

        QJsonArray mappings;
        mappings.append(it.value());
        mappings.append(/*shortName ? *shortName :*/ QString());
        mappings.append(/*customName ? *customName :*/ QString());
        fighterStatusMapping[QString::number(it.key())] = mappings;
    }

    const auto& specificEnumNames = mappingInfo_.fighterStatus.fighterSpecificEnumNames();
    for (auto fighter = specificEnumNames.begin(); fighter != specificEnumNames.end(); ++fighter)
    {
        QJsonObject specificMapping;
        for (auto it = fighter.value().begin(); it != fighter.value().end(); ++it)
        {
            /*const QString* shortName = mappingInfo_.fighterStatus.mapToShortName(it.key());
            const QString* customName = mappingInfo_.fighterStatus.mapToCustom(it.key());*/

            QJsonArray mappings;
            mappings.append(it.value());
            mappings.append(/*shortName ? *shortName :*/ QString());
            mappings.append(/*customName ? *customName :*/ QString());
            specificMapping[QString::number(it.key())] = mappings;
        }
        fighterStatusMapping[QString::number(fighter.key())] = specificMapping;
    }

    QJsonObject fighterIDMapping;
    const auto& fighterIDMap = mappingInfo_.fighterID.get();
    for (auto it = fighterIDMap.begin(); it != fighterIDMap.end(); ++it)
        fighterIDMapping[QString::number(it.key())] = it.value();

    QJsonObject stageIDMapping;
    const auto& stageIDMap = mappingInfo_.stageID.get();
    for (auto it = stageIDMap.begin(); it != stageIDMap.end(); ++it)
        stageIDMapping[QString::number(it.key())] = it.value();

    QJsonObject mappingInfo;
    mappingInfo["fighterstatus"] = fighterStatusMapping;
    mappingInfo["fighterid"] = fighterIDMapping;
    mappingInfo["stageid"] = stageIDMapping;

    QJsonArray playerInfo;
    for (const auto& info : playerInfo_)
    {
        QJsonObject player;
        player["tag"] = info.tag();
        player["fighterid"] = info.fighterID();
        playerInfo.append(player);
    }

    QJsonObject json;
    json["version"] = "1.0";
    json["date"] = gameInfo_.timeStarted().toUTC().toString();
    json["mappinginfo"] = mappingInfo;
    json["gameinfo"] = gameInfo;
    json["playerinfo"] = playerInfo;

    f.write(QJsonDocument(json).toJson());
    f.write("\n");

    QDataStream stream(&f);
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

    return true;
}

// ----------------------------------------------------------------------------
QString Recording::findNonExistingFileName(const QDir &path)
{
    QString date = gameInfo_.timeStarted().toString("yyyy-MM-dd");
    QString formatDesc = gameInfo_.formatDesc();
    QStringList playerList;
    for (const auto& info : playerInfo_)
    {
        const QString* fighterName = mappingInfo_.fighterID.map(info.fighterID());
        if (fighterName)
            playerList.append(info.tag() + " (" + *fighterName + ")");
        else
            playerList.append(info.tag());
    }
    QString players = playerList.join(" vs ");

    QString fileName = date + " - " + formatDesc + " - " + players + " Game " + QString::number(gameInfo_.gameNumber()) + ".uhr";
    for (int i = 2; QFileInfo::exists(path.absoluteFilePath(fileName)); ++i)
    {
        if (gameInfo_.format() == GameInfo::FRIENDLIES)
            fileName = date + " - " + formatDesc + " - " + players + " Game " + QString::number(i) + ".uhr";
        else
            fileName = date + " - " + formatDesc + "(" + i + ")" + " - " + players + " Game " + QString::number(gameInfo_.gameNumber()) + ".uhr";
    }

    return path.absoluteFilePath(fileName);
}

// ----------------------------------------------------------------------------
void Recording::addPlayerState(int index, PlayerState&& state)
{
    // Only add a new state if the previous one was different
    if (playerStates_[index].count() == 0 || playerStates_[index].back().status() != state.status())
    {
        playerStates_[index].push_back(std::move(state));
        dispatcher.dispatch(&RecordingListener::onRecordingPlayerStateAdded, index, playerStates_[index].back());
    }
    else
    {
        dispatcher.dispatch(&RecordingListener::onRecordingPlayerStateAdded, index, state);
    }
}

}
