#include "stats/exporters/WSExporter.hpp"
#include "stats/models/PlayerMeta.hpp"
#include "stats/models/SettingsModel.hpp"
#include "stats/models/StatsCalculator.hpp"
#include "stats/models/WebSocketServer.hpp"
#include "stats/util/StatsFormatter.hpp"

#include "rfcommon/GameMetaData.hpp"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

// ----------------------------------------------------------------------------
WSExporter::WSExporter(
        const PlayerMeta* playerMeta,
        const StatsCalculator* stats,
        const SettingsModel* settings,
        const WebSocketServer* server)
    : playerMeta_(playerMeta)
    , stats_(stats)
    , settings_(settings)
    , server_(server)
{
}

// ----------------------------------------------------------------------------
WSExporter::~WSExporter()
{}

// ----------------------------------------------------------------------------
void WSExporter::writeJSON(bool gameStarted, bool gameEnded) const
{
    // Don't send anything if there's no data
    if (playerMeta_->playerCount() == 0)
        return;

    QJsonArray jPlayers;
    for (int i = 0; i != playerMeta_->playerCount(); ++i)
        jPlayers += QJsonObject({
            {"name", playerMeta_->name(i)},
            {"tag", playerMeta_->tag(i)},
            {"fighter", playerMeta_->character(i)}
        });

    QJsonArray jStatNames;
    for (int s = 0; s != settings_->numStatsEnabled(); ++s)
    {
        StatType type = settings_->statAtIndex(s);
        jStatNames += statTypeToString(type);
    }

    QJsonArray jStatValues;
    StatsFormatter formatter(stats_, playerMeta_);
    for (int s = 0; s != settings_->numStatsEnabled(); ++s)
    {
        StatType type = settings_->statAtIndex(s);
        QJsonArray jPlayerValues;
        for (int i = 0; i != playerMeta_->playerCount(); ++i)
            jPlayerValues += formatter.playerStatAsString(i, type);
        jStatValues += jPlayerValues;
    }

    auto mdata = playerMeta_->latestMetaData();
    QJsonObject jMetaData = mdata ? QJsonObject({
        {"started", qint64(mdata->timeStarted().millisSinceEpoch())},
        {"ended", qint64(mdata->timeEnded().millisSinceEpoch())},
        {"gamenumber", mdata->gameNumber().value()},
        {"setnumber", mdata->setNumber().value()},
        {"format", mdata->setFormat().shortDescription()},
        {"winner", mdata->winner()}
    }) : QJsonObject();

    QJsonObject jState = {
        {"started", gameStarted},
        {"ended", gameEnded}
    };

    QJsonObject jStats = {
        {"version", "1.0"},
        {"metadata", jMetaData},
        {"players", jPlayers},
        {"names", jStatNames},
        {"values", jStatValues},
        {"state", jState}
    };

    QJsonObject j = {
        {"stats", jStats}
    };

    server_->sendBinaryMessage(QJsonDocument(j).toJson(QJsonDocument::Compact));
}
