#pragma once

#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Types.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/RefCounted.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/Frame.hpp"

namespace rfcommon {

class FighterState;
class SessionListener;

extern template class RFCOMMON_TEMPLATE_API SmallVector<FighterID, 2>;
extern template class RFCOMMON_TEMPLATE_API SmallVector<SmallString<15>, 2>;
extern template class RFCOMMON_TEMPLATE_API SmallVector<SessionListener*, 4>;
extern template class RFCOMMON_TEMPLATE_API ListenerDispatcher<SessionListener>;

class RFCOMMON_PUBLIC_API Session : public RefCounted
{
public:
    Session(MappingInfo&& mapping,
            StageID stageID,
            SmallVector<FighterID, 2>&& fighterIDs,
            SmallVector<SmallString<15>, 2>&& tags);
    virtual ~Session();

    bool save(const String& fileName);

    /*!
     * \brief Returns information on how to map fighter/stage/state IDs to
     * strings.
     */
    const MappingInfo& mappingInfo() const
        { return mappingInfo_; }

    /*!
     * \brief Gets the number of fighters in this session. Usually 2, but can
     * go up to 8.
     */
    int fighterCount() const
        { return fighterIDs_.count(); }

    /*!
     * \brief Gets the tag used by the player. This is the string that appears
     * above the player in-game and is created when the player sets their controls.
     * \param fighterIdx Which player to get
     */
    const SmallString<15>& tag(int fighterIdx) const
        { return tags_[fighterIdx]; }

    /*!
     * \brief Gets the name of the player. By default this will be the same as
     * the tag, but many players like to create tags that are shorter or a
     * variation of their real name. This string is their real name.
     * Unlike tags, there is also no character limit to a player's name.
     * \note If training mode, this will always be the same as the tag.
     * \param fighterIdx Which player to get
     */
    virtual const SmallString<15>& name(int fighterIdx) const = 0;

    /*!
     * \brief Gets the fighter ID being used by the specified player. The ID
     * can be used to look up the character's type or name by using the
     * MappingInfo structure.
     * \param fighterIdx The fighter index, from 0 to fighterCount() - 1.
     */
    FighterID fighterID(int fighterIdx) const
        { return fighterIDs_[fighterIdx]; }

    /*!
     * \brief Gets the stage ID being played on. The ID can be used to look up
     * the stage name by using the MappingInfo structure.
     */
    StageID stageID() const
        { return stageID_; }

    int frameCount() const
        { return frames_.count(); }

    const Frame& firstFrame() const
        { assert(frameCount() > 0); return frames_.front(); }

    const Frame& lastFrame() const
        { assert(frameCount() > 0); return frames_.back(); }

    const FighterState& state(int frameIdx, int fighterIdx) const
        { assert(fighterIdx < fighterCount()); assert(frameIdx < frameCount()); return frames_[frameIdx].fighter(fighterIdx); }

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
    //TimeStampMS timeStampStartedMs() const;

    /*!
     * \brief Gets the absolute time of when the last timestamp was received in
     * unix time (milli-seconds since Jan 1 1970). May be slightly off by 1
     * second or so depending on latency.
     *
     * In the case of a running session, this will be the timestamp of the most
     * current frame received.
     *
     * In the case of a saved session, this will be the timestamp of the last
     * frame received.
     */
    //TimeStampMS timeStampEndedMs() const;

    /*DeltaTimeMS lengthMs() const
        { return timeStampStartedMs() - timeStampEndedMs(); }*/

    ListenerDispatcher<SessionListener> dispatcher;

protected:
    int findWinner() const;

protected:
    MappingInfo mappingInfo_;
    StageID stageID_;
    SmallVector<FighterID, 2> fighterIDs_;
    SmallVector<SmallString<15>, 2> tags_;
    Vector<Frame> frames_;
};

}
