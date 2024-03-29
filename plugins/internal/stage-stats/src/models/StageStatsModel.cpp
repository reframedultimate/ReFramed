#include "stage-stats/listeners/StageStatsListener.hpp"
#include "stage-stats/models/StageStatsModel.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/GameMetadata.hpp"
#include "rfcommon/Profiler.hpp"

// ----------------------------------------------------------------------------
StageStatsModel::StageStatsModel()
{
}

// ----------------------------------------------------------------------------
StageStatsModel::~StageStatsModel()
{
}

// ----------------------------------------------------------------------------
void StageStatsModel::clearStats()
{
    PROFILE(StageStatsModel, clearStats);

    fighters_.clearCompact();
}

// ----------------------------------------------------------------------------
void StageStatsModel::addSessionData(rfcommon::MappingInfo* map, rfcommon::GameMetadata* mdata)
{
    PROFILE(StageStatsModel, addSessionData);

    stageNames_.insertIfNew(mdata->stageID(), map->stage.toName(mdata->stageID()));

    for (int fighterIdx = 0; fighterIdx != mdata->fighterCount(); ++fighterIdx)
    {
        const rfcommon::FighterID fighterID = mdata->playerFighterID(fighterIdx);
        const rfcommon::StageID stageID = mdata->stageID();

        auto& fighterData = fighters_.insertOrGet(fighterID, FighterData())->value();
        auto& stageData = fighterData.stageData.insertOrGet(stageID, StageData())->value();

        fighterData.name = mdata->playerName(fighterIdx);
        fighterData.character = map->fighter.toName(fighterID);

        if (mdata->winner() == fighterIdx)
            stageData.wins++;
        else
            stageData.losses++;
    }
}

// ----------------------------------------------------------------------------
void StageStatsModel::notifyUpdated()
{
    PROFILE(StageStatsModel, notifyUpdated);

    dispatcher.dispatch(&StageStatsListener::onDataUpdated);
}

// ----------------------------------------------------------------------------
int StageStatsModel::fighterCount() const
{
    PROFILE(StageStatsModel, fighterCount);

    return fighters_.count();
}

// ----------------------------------------------------------------------------
const char* StageStatsModel::playerName(int fighterIdx) const
{
    PROFILE(StageStatsModel, playerName);

    const auto& fighterData = (fighters_.begin() + fighterIdx)->value();
    return fighterData.name.cStr();
}

// ----------------------------------------------------------------------------
const char* StageStatsModel::characterName(int fighterIdx) const
{
    PROFILE(StageStatsModel, characterName);

    const auto& fighterData = (fighters_.begin() + fighterIdx)->value();
    return fighterData.character.cStr();
}

// ----------------------------------------------------------------------------
int StageStatsModel::stageCount(int fighterIdx) const
{
    PROFILE(StageStatsModel, stageCount);

    const auto& stageData = (fighters_.begin() + fighterIdx)->value().stageData;
    return stageData.count();
}

// ----------------------------------------------------------------------------
int StageStatsModel::stageWins(int fighterIdx, int stageIdx) const
{
    PROFILE(StageStatsModel, stageWins);

    const auto& stageData = (fighters_.begin() + fighterIdx)->value().stageData;
    return (stageData.begin() + stageIdx)->value().wins;
}

// ----------------------------------------------------------------------------
int StageStatsModel::stageLosses(int fighterIdx, int stageIdx) const
{
    PROFILE(StageStatsModel, stageLosses);

    const auto& stageData = (fighters_.begin() + fighterIdx)->value().stageData;
    return (stageData.begin() + stageIdx)->value().losses;
}

// ----------------------------------------------------------------------------
rfcommon::SmallVector<rfcommon::StageID, 3> StageStatsModel::top3Stages(int fighterIdx) const
{
    PROFILE(StageStatsModel, top3Stages);

    rfcommon::SmallVector<rfcommon::StageID, 3> result;
    const auto& stageData = (fighters_.begin() + fighterIdx)->value().stageData;

    while (result.count() < 3)
    {
        int wins = 0;
        auto stageID = rfcommon::StageID::makeInvalid();
        for (const auto& it : stageData)
        {
            if (result.findFirst(it.key()) != result.end())
                continue;

            if (wins < it.value().wins)
            {
                wins = it.value().wins;
                stageID = it.key();
            }
        }

        if (wins == 0)
            break;

        result.push(stageID);
    }

    return result;
}

// ----------------------------------------------------------------------------
rfcommon::SmallVector<rfcommon::StageID, 3> StageStatsModel::bottom3Stages(int fighterIdx) const
{
    PROFILE(StageStatsModel, bottom3Stages);

     rfcommon::SmallVector<rfcommon::StageID, 3> result;
     const auto& stageData = (fighters_.begin() + fighterIdx)->value().stageData;

     while (result.count() < 3)
     {
         int losses = 0;
         auto stageID = rfcommon::StageID::makeInvalid();
         for (const auto& it : stageData)
         {
             if (losses < it.value().losses)
             {
                 losses = it.value().losses;
                 stageID = it.key();
             }
         }

         if (losses == 0)
             break;

         result.push(stageID);
     }

     return result;
}

// ----------------------------------------------------------------------------
const char* StageStatsModel::stageName(rfcommon::StageID stageID) const
{
    PROFILE(StageStatsModel, stageName);

    auto it = stageNames_.findKey(stageID);
    if (it == stageNames_.end())
        return "(unknown stage)";
    return it->value().cStr();
}
