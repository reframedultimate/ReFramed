#pragma once

#include "uh/config.hpp"
#include "uh/Session.hpp"

namespace uh {

class TrainingSession : public Session
{
public:
    TrainingSession(MappingInfo&& mapping,
                    SmallVector<FighterID, 8>&& playerFighterIDs,
                    SmallVector<SmallString<15>, 8>&& playerTags,
                    StageID stageID);

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
    uint64_t timeStampStartedMs() { return timeStarted_; }

private:
    const uint64_t timeStarted_;
};

}
