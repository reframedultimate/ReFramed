#pragma once

#include "uh/config.hpp"
#include "uh/GameSession.hpp"

namespace uh {

class UH_PUBLIC_API SavedGameSession : public GameSession
{
public:
    static SavedGameSession* load(const String& fileName);

    /*!
     * \brief Gets the absolute time of when the session ended in unix time
     * (milli-seconds since Jan 1 1970). May be slightly off by 1 second or so
     * depending on latency.
     *
     * In the case of a game session, this marks the last frame of gameplay.
     *
     * In the case of a training session, this marks the last frame of training
     * mode.
     */
    uint64_t timeStampEndedMs() const;

    uint64_t gameLengthMs() const
        { return timeStampEndedMs() - timeStampStartedMs(); }

    int playerStateCount(int playerIdx) const
        { return static_cast<int>(playerStates_[playerIdx].count()); }

    const PlayerState& playerStateAt(int playerIdx, int stateIdx) const
        { return playerStates_[playerIdx][stateIdx]; }

    const PlayerState& firstPlayerState(int playerIdx) const
        { return playerStates_[playerIdx][0]; }

    const PlayerState& lastPlayerState(int playerIdx) const
        { return playerStateAt(playerIdx, playerStateCount(playerIdx) - 1); }

    const PlayerState* playerStatesBegin(int playerIdx) const
        { return playerStates_[playerIdx].data(); }

    const PlayerState* playerStatesEnd(int playerIdx) const;

private:
    SavedGameSession(MappingInfo&& mapping,
                     SmallVector<FighterID, 8>&& playerFighterIDs,
                     SmallVector<SmallString<15>, 8>&& playerTags,
                     SmallVector<SmallString<15>, 8>&& playerNames,
                     StageID stageID);

    // Using void* here to avoid json objects leaking into the rest of the
    // program
    static SavedGameSession* loadVersion_1_0(const void* jptr);
    static SavedGameSession* loadVersion_1_1(const void* jptr);
    static SavedGameSession* loadVersion_1_2(const void* jptr);
    static SavedGameSession* loadVersion_1_3(const void* jptr);
    static SavedGameSession* loadVersion_1_4(const void* jptr);

private:
    int winner_;
};

}
