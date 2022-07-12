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

class RFCOMMON_PUBLIC_API Session : public RefCounted
{
public:
    Session();
    virtual ~Session();

    bool save(const String& fileName);

    /*!
     * \brief Returns information on how to map fighter/stage/state IDs to
     * strings.
     */
    const MappingInfo& mappingInfo() const
        { return mappingInfo_; }

    int frameCount() const
        { return frames_.count(); }

    const Frame& frame(int frameIdx) const
        { return frames_[frameIdx]; }

    const Frame& firstFrame() const
        { assert(frameCount() > 0); return frames_.front(); }

    const Frame& lastFrame() const
        { assert(frameCount() > 0); return frames_.back(); }

    const FighterState& state(int frameIdx, int fighterIdx) const
        { assert(fighterIdx < fighterCount()); assert(frameIdx < frameCount()); return frames_[frameIdx].fighter(fighterIdx); }

    void addFrame(Frame&& frame);

    ListenerDispatcher<SessionListener> dispatcher;

protected:
    int findWinner() const;

protected:
    Vector<Frame> frames_;
};

}
