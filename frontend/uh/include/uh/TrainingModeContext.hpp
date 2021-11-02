#pragma once

#include "uh/config.hpp"
#include "uh/Types.hpp"
#include "uh/ListenerDispatcher.hpp"
#include "uh/MappingInfo.hpp"
#include "uh/RefCounted.hpp"

namespace uh {

class PlayerState;
class TrainingModeContextListener;

class UH_PUBLIC_API TrainingModeContext : public RefCounted
{
public:
    TrainingModeContext(MappingInfo&& mapping,
            FighterID playerFighterID,
            FighterID cpuFighterID,
            StageID stageID);

    /*!
     * \brief Returns information on how to map fighter/stage/state IDs to
     * strings.
     */
    const MappingInfo& mappingInfo() const { return mappingInfo_; }

    /*!
     * \brief Gets the fighter ID being used by the player in training mode.
     */
    FighterID playerFighterID() const { return playerFighterID_; }

    /*!
     * \brief Gets the fighter ID of the CPU character in training mode.
     */
    FighterID cpuFighterID() const { return cpuFighterID_; }

    /*!
     * \brief Gets the stage ID being played on.
     */
    StageID stageID() const { return stageID_; }

    void addPlayerState(int index, PlayerState&& state);

    ListenerDispatcher<TrainingModeContextListener> dispatcher;

protected:
    MappingInfo mappingInfo_;
    FighterID playerFighterID_;
    FighterID cpuFighterID_;
    StageID stageID_;
};

}
