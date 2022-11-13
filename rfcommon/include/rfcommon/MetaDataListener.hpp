#pragma once

#include "rfcommon/TimeStamp.hpp"
#include "rfcommon/SessionNumber.hpp"

namespace rfcommon {

class SetFormat;

class MetaDataListener
{
public:
    virtual void onMetaDataTimeChanged(TimeStamp timeStarted, TimeStamp timeEnded) = 0;

    // Game related events

    /*!
     * Called when any of the following things change:
     *   - Tournament name
     *   - Tournament website URL
     *   - TO is added, removed or modified
     *   - Sponsor is added, removed or modified
     */
    virtual void onMetaDataTournamentDetailsChanged() = 0;

    /*!
     * Called when any of the following things change:
     *   - Event type is changed
     *   - Event URL is changed
     */
    virtual void onMetaDataEventDetailsChanged() = 0;

    /*!
     * Called when any of the following things change:
     *   - Commentator is added, removed or modified
     */
    virtual void onMetaDataCommentatorsChanged() = 0;

    /*!
     * Called when any of the following things change:
     *   - Round type or number is changed
     *   - Set format is changed
     *   - Score (and therefore game number) is changed
     */
    virtual void onMetaDataGameDetailsChanged() = 0;

    /*!
     * Called when any of the following things change:
     *   - Player name, sponsor, social, or pronouns change
     *   - Player's isLoserSide() changes
     */
    virtual void onMetaDataPlayerDetailsChanged() = 0;

    /*! Called whenever the winner changes. This only makes sense during a live session */
    virtual void onMetaDataWinnerChanged(int winnerPlayerIdx) = 0;

    // Training mode related events

    /*! Whenever a new training room is loaded, the session number will increment */
    virtual void onMetaDataTrainingSessionNumberChanged(SessionNumber number) = 0;
};

}
