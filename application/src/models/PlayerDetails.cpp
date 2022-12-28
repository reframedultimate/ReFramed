#include "application/models/PlayerDetails.hpp"

#include "rfcommon/Log.hpp"

#include "nlohmann/json.hpp"

#include <QDir>
#include <QStandardPaths>
#include <QDebug>

namespace rfapp {

using namespace nlohmann;

// ----------------------------------------------------------------------------
PlayerDetails::PlayerDetails()
{
    load();
}

// ----------------------------------------------------------------------------
PlayerDetails::~PlayerDetails()
{
    save();
}

// ----------------------------------------------------------------------------
void PlayerDetails::load()
{
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
    QString fileName = dir.absoluteFilePath("playerDetails.json");

    rfcommon::Log::root()->info("Opening file %s", fileName.toUtf8().constData());
    QFile f(fileName);
    if (!f.open(QIODevice::ReadOnly))
    {
        rfcommon::Log::root()->notice("playerDetails.json doesn't exist. Player details will be empty.", fileName.toUtf8().constData());
        return;
    }

    QByteArray ba = f.readAll();
    json root = json::parse(ba.begin(), ba.end(), nullptr, false);
    if (root.is_discarded())
    {
        rfcommon::Log::root()->error("Failed to parse playerDetails.json");
        return;
    }

    if (root.is_object() == false)
    {
        rfcommon::Log::root()->error("Root invalid in playerDetails.json");
        return;
    }

    json jPlayers = root["players"];
    if (jPlayers.is_object() == false)
    {
        rfcommon::Log::root()->error("\"players\" node invalid in playerDetails.json");
        return;
    }

    for (const auto& [key, value] : jPlayers.items())
    {
        if (value.is_object() == false)
            continue;

        json jName = value["name"];
        json jSponsor = value["sponsor"];
        json jSocial = value["social"];
        json jPronouns = value["pronouns"];
        if (jName.is_string() == false || jSponsor.is_string() == false || jSocial.is_string() == false || jPronouns.is_string() == false)
            continue;

        players_.insertAlways(key.c_str(), Player{
            jName.get<std::string>().c_str(),
            jSponsor.get<std::string>().c_str(),
            jSocial.get<std::string>().c_str(),
            jPronouns.get<std::string>().c_str()
        });
    }

    rfcommon::Log::root()->info("Player details loaded");
}

// ----------------------------------------------------------------------------
void PlayerDetails::save()
{
    auto log = rfcommon::Log::root();

    QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
    QFileInfo pathInfo(dir.path());
    if (!pathInfo.exists())
        QDir().mkdir(pathInfo.filePath());
    QString fileName = dir.absoluteFilePath("playerDetails.json");

    log->info("Saving player details to file %s", fileName.toUtf8().constData());
    QFile f(fileName);
    if (!f.open(QIODevice::WriteOnly))
    {
        log->error("Failed to open file %s for writing. Can't save player details.", fileName.toUtf8().constData());
        return;
    }

    json jPlayers;
    for (const auto& it : players_)
    {
        jPlayers[it.key().cStr()] = {
            {"name", it.value().name.cStr()},
            {"sponsor", it.value().sponsor.cStr()},
            {"social", it.value().social.cStr()},
            {"pronouns", it.value().pronouns.cStr()}
        };
    }

    json root = {
        {"players", jPlayers}
    };

    const std::string jsonStr = root.dump(2);
    if (f.write(jsonStr.data(), jsonStr.length()) != jsonStr.length())
    {
        log->error("Failed to write data to playerDetails.json");
        return;
    }

    log->info("Player details saved");
}

// ----------------------------------------------------------------------------
const PlayerDetails::Player* PlayerDetails::findTag(const rfcommon::String& tag) const
{
    const auto it = players_.find(tag);
    if (it == players_.end())
        return nullptr;

    return &it->value();
}

// ----------------------------------------------------------------------------
const PlayerDetails::Player* PlayerDetails::findName(const rfcommon::String& name) const
{
    for (const auto it : players_)
        if (it.value().name == name)
            return &it.value();

    return nullptr;
}

// ----------------------------------------------------------------------------
void PlayerDetails::addOrModifyPlayer(const char* tag, const char* name, const char* sponsor, const char* social, const char* pronouns)
{
    players_.insertAlways(tag, Player{name, sponsor, social, pronouns});
}

}
