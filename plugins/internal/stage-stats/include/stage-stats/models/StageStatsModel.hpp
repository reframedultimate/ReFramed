#pragma once

#include "rfcommon/FighterID.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/LinearMap.hpp"
#include "rfcommon/StageID.hpp"
#include "rfcommon/Vector.hpp"

class StageStatsListener;

namespace rfcommon {
    class MappingInfo;
    class GameMetadata;
}

class FighterPair
{
public:
    FighterPair(rfcommon::FighterID p1, rfcommon::FighterID p2)
        : pair_((p2.value() << 8) + p1.value())
    {}

    rfcommon::FighterID p1() const { return rfcommon::FighterID::fromValue(pair_ & 0xFF); }
    rfcommon::FighterID p2() const { return rfcommon::FighterID::fromValue((pair_ >> 8) & 0xFF); }

    bool operator< (FighterPair other) { return pair_ < other.pair_; }
    bool operator==(FighterPair other) { return pair_ == other.pair_; }
    bool operator!=(FighterPair other) { return pair_ != other.pair_; }

private:
    uint16_t pair_;
};

class StageStatsModel
{
public:
    StageStatsModel();
    ~StageStatsModel();

    void clearStats();
    void addSessionData(rfcommon::MappingInfo* map, rfcommon::GameMetadata* mdata);
    void notifyUpdated();

    int fighterCount() const;
    const char* playerName(int fighterIdx) const;
    const char* characterName(int fighterIdx) const;
    int stageCount(int fighterIdx) const;
    int stageWins(int fighterIdx, int stageIdx) const;
    int stageLosses(int fighterIdx, int stageIdx) const;
    rfcommon::SmallVector<rfcommon::StageID, 3> top3Stages(int fighterIdx) const;
    rfcommon::SmallVector<rfcommon::StageID, 3> bottom3Stages(int fighterIdx) const;

    const char* stageName(rfcommon::StageID stageID) const;

    rfcommon::ListenerDispatcher<StageStatsListener> dispatcher;

private:
    struct StageData
    {
        int wins, losses;
    };

    struct FighterData
    {
        rfcommon::SmallLinearMap<rfcommon::StageID, StageData, 12> stageData;
        rfcommon::String name;
        rfcommon::String character;
    };

    rfcommon::SmallLinearMap<rfcommon::FighterID, FighterData, 4> fighters_;
    rfcommon::SmallLinearMap<rfcommon::StageID, rfcommon::String, 12> stageNames_;
};
