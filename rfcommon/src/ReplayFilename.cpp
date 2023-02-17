#include "rfcommon/GameMetadata.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Replayfilename.hpp"
#include "rfcommon/TrainingMetadata.hpp"
#include <cctype>
#include <ctime>

namespace rfcommon {

// ----------------------------------------------------------------------------
static String fromTrainingMetadata(
    const rfcommon::MappingInfo* map,
    const rfcommon::TrainingMetadata* mdata,
    const char* datetime)
{
    return String(datetime) +
        " - Training (" + String::decimal(mdata->sessionNumber().value()) + ") - "
        + mdata->playerTag(0) + " vs CPU - "
        + map->stage.toName(mdata->stageID())
        +".rfr";
}

// ----------------------------------------------------------------------------
static String fromGameMetadata(
    const rfcommon::MappingInfo* map,
    const rfcommon::GameMetadata* mdata,
    const char* datetime)
{
    // Examples:
    //   2020-05-23_19-45-01 - Singles - Bo3 (WR2) - TheComet (Pika) vs TAEL (Falcon) - Game 3 (1-1) - Kalos.rfr
    //   2020-05-23_19-45-01 - Friendlies - Free (5) - TheComet (Pika) vs TAEL (Falcon) - Game 2 (0-1) - Kalos.rfr

    Vector<String> playerNames, fighterNames;
    for (int i = 0; i != mdata->fighterCount(); ++i)
    {
        playerNames.emplace(mdata->playerName(i));
        fighterNames.emplace(map->fighter.toName(mdata->playerFighterID(i)));
    }

    String playerChars;
    for (int i = 0; i != playerNames.count(); ++i)
    {
        if (i != 0)
            playerChars += " vs ";
        if (mdata->playerName(i).length() > 0)
            playerChars += mdata->playerName(i);
        else
            playerChars += "Player " + String::decimal(i);
        playerChars += " (";
        playerChars += map->fighter.toName(mdata->playerFighterID(i));
        playerChars += ")";
        if (mdata->playerIsLoserSide(i))
            playerChars += " [L]";
    }

    return String(datetime) + " - "
        + mdata->bracketType().description() + " - "
        + mdata->setFormat().shortDescription() + " ("
        + mdata->round().shortDescription() + ") - "
        + playerChars + " - "
        + "Game " + String::decimal(mdata->score().gameNumber().value()) + " ("
        + String::decimal(mdata->score().left()) + "-" + String::decimal(mdata->score().right()) + ") - "
        + map->stage.toName(mdata->stageID())
        + ".rfr";
}

// ----------------------------------------------------------------------------
String ReplayFilename::fromMetadata(const rfcommon::MappingInfo* map, const rfcommon::Metadata* mdata)
{
    PROFILE(ReplayFilename, fromMetadata);

    const auto stampMs = mdata->timeStarted().millisSinceEpoch();
    std::time_t t = (std::time_t)(stampMs / 1000);
    std::tm* tm = std::localtime(&t);
    char datetime[20];
    std::strftime(datetime, 20, "%Y-%m-%d_%H-%M-%S", tm);

    switch (mdata->type())
    {
        case Metadata::TRAINING: return fromTrainingMetadata(map, mdata->asTraining(), datetime);
        case Metadata::GAME: return fromGameMetadata(map, mdata->asGame(), datetime);
    }

    return "";
}

}
