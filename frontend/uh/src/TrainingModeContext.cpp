#include "uh/TrainingModeContext.hpp"
#include "uh/TrainingModeContextListener.hpp"

namespace uh {

// ----------------------------------------------------------------------------
TrainingModeContext::TrainingModeContext(MappingInfo&& mapping,
        FighterID playerFighterID,
        FighterID cpuFighterID,
        StageID stageID)
    : mappingInfo_(std::move(mapping))
    , playerFighterID_(playerFighterID)
    , cpuFighterID_(cpuFighterID)
    , stageID_(stageID)
{
}

// ----------------------------------------------------------------------------
void TrainingModeContext::addPlayerState(int index, PlayerState&& state)
{
    dispatcher.dispatch(&TrainingModeContextListener::onTrainingModeContextNewPlayerState, index, state);
}

}
