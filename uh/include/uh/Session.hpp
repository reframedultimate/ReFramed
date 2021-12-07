#pragma once

#pragma once

#include "uh/config.hpp"
#include "uh/Types.hpp"
#include "uh/MappingInfo.hpp"
#include "uh/RefCounted.hpp"
#include "uh/ListenerDispatcher.hpp"

namespace uh {

class PlayerState;
class SessionListener;

extern template class UH_TEMPLATE_API SmallVector<FighterID, 8>;
extern template class UH_TEMPLATE_API SmallVector<SmallString<15>, 8>;
extern template class UH_TEMPLATE_API Vector<PlayerState>;
extern template class UH_TEMPLATE_API SmallVector<Vector<PlayerState>, 8>;
extern template class UH_TEMPLATE_API SmallVector<SessionListener*, 4>;
extern template class UH_TEMPLATE_API ListenerDispatcher<SessionListener>;

class UH_PUBLIC_API Session : public RefCounted
{
public:
    Session(MappingInfo&& mapping,
            StageID stageID,
            SmallVector<FighterID, 8>&& playerFighterIDs,
            SmallVector<SmallString<15>, 8>&& playerTags);

    /*!
     * \brief Returns information on how to map fighter/stage/state IDs to
     * strings.
     */
    const MappingInfo& mappingInfo() const { return mappingInfo_; }

    /*!
     * \brief Gets the number of players
     */
    int playerCount() const;

    /*!
     * \brief Gets the tag used by the player. This is the string that appears
     * above the player in-game and is created when the player sets their controls.
     * \param index Which player to get
     */
    const SmallString<15>& playerTag(int playerIdx) const;

    /*!
     * \brief Gets the name of the player. By default this will be the same as
     * the tag, but many players like to create tags that are shorter or a
     * variation of their real name. This string is their real name.
     * Unlike tags, there is also no character limit to a player's name.
     * \param index Which player to get
     */
    virtual const SmallString<15>& playerName(int playerIdx) const = 0;

    /*!
     * \brief Gets the fighter ID being used by the specified player.
     * \param index The player to get
     */
    FighterID playerFighterID(int playerIdx) const;

    /*!
     * \brief Gets the stage ID being played on.
     */
    StageID stageID() const
        { return stageID_; }

    int playerStateCount(int playerIdx) const;

    const PlayerState& playerStateAt(int playerIdx, int stateIdx) const;

    const PlayerState& firstPlayerState(int playerIdx) const;

    const PlayerState& lastPlayerState(int playerIdx) const;

    const PlayerState* playerStatesBegin(int playerIdx) const;

    const PlayerState* playerStatesEnd(int playerIdx) const;

    virtual int winner() const = 0;

    /*!
     * \brief Gets the absolute time of when the session started in unix time
     * (milli-seconds since Jan 1 1970). May be slightly off by 1 second or so
     * depending on latency.
     *
     * In the case of a game session, this marks the first frame of gameplay,
     * immediately after the 3-2-1-Go countdown completes.
     *
     * In the case of a training session, this marks the first frame that was
     * received (may not be the first frame of training mode depending on when
     * the user connected).
     */
    virtual TimeStampMS timeStampStartedMs() const = 0;

    ListenerDispatcher<SessionListener> dispatcher;

protected:
    int findWinner() const;

protected:
    MappingInfo mappingInfo_;
    StageID stageID_;
    SmallVector<FighterID, 8> playerFighterIDs_;
    SmallVector<SmallString<15>, 8> playerTags_;
    SmallVector<Vector<PlayerState>, 8> playerStates_;
};

}
