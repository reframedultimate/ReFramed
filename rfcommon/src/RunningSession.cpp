#include "rfcommon/RunningSession.hpp"
#include "rfcommon/SessionListener.hpp"


namespace rfcommon {

// ----------------------------------------------------------------------------
RunningSession::RunningSession()
{
}

// ----------------------------------------------------------------------------
RunningSession::RunningSession(
        MappingInfo&& mapping,
        StageID stageID,
        SmallVector<FighterID, 2>&& fighterIDs,
        SmallVector<SmallString<15>, 2>&& tags)
    : Session(std::move(mapping), stageID, std::move(fighterIDs), std::move(tags))
{
}

// ----------------------------------------------------------------------------
void RunningSession::addFrame(Frame&& frame)
{
    // Sanity checks
#ifndef NDEBUG
    for (int i = 1; i < frame.fighterCount(); ++i)
    {
        assert(frame.fighter(0).framesLeft() == frame.fighter(i).framesLeft());
        assert(frame.fighter(0).frameNumber() == frame.fighter(i).frameNumber());
    }
#endif

    frames_.push(std::move(frame));

    // If any fighter state is different from the previous one, notify
    if (frames_.count() < 2 || frames_.back(1).hasSameDataAs(frames_.back(2)))
        dispatcher.dispatch(&SessionListener::onRunningSessionNewUniqueFrame, frames_.count() - 1, frames_.back());

    // The UI cares about every frame
    dispatcher.dispatch(&SessionListener::onRunningSessionNewFrame, frames_.count() - 1, frames_.back());
}

}
