#include "rfcommon/RunningSession.hpp"
#include "rfcommon/SessionListener.hpp"
#include "rfcommon/FighterFrame.hpp"


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
void RunningSession::addFrame(SmallVector<FighterFrame, 2>&& frame)
{
    frames_.push(std::move(frame));

    // If any fighter state is different from the previous one, notify
    for (int i = 0; i != frames_.count(); ++i)
        if (frames_[i].count() < 2 || frames_[i].back(1) != frames_[i].back(2))
        {
            dispatcher.dispatch(&SessionListener::onRunningSessionNewUniqueFrame);
            break;
        }

    // The UI cares about every frame
    dispatcher.dispatch(&SessionListener::onRunningSessionNewFrame);
}

}
