#pragma once

#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Types.hpp"
#include "rfcommon/Reference.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/Frame.hpp"
#include "rfcommon/Vector.hpp"

namespace rfcommon {

class MappingInfo;
class SessionListener;
class SessionMetaData;
class GameSessionMetaData;
class TrainingSessionMetaData;

class RFCOMMON_PUBLIC_API Session : public RefCounted
{
public:
    Session();
    virtual ~Session();

    static Session* load(const String& fileName);
    bool save(const String& fileName);

    /*!
     * \brief Returns information on how to map fighter/stage/state IDs to
     * strings.
     */
    MappingInfo* mappingInfo() const
        { return mappingInfo_; }

    virtual SessionMetaData* metaData() const = 0;

    int frameCount() const
        { return frames_.count(); }

    const Frame& frame(int frameIdx) const
        { return frames_[frameIdx]; }

    const Frame& firstFrame() const
    {
        assert(frameCount() > 0);
        return frames_.front();
    }

    const Frame& lastFrame() const
    {
        assert(frameCount() > 0);
        return frames_.back();
    }

    const FighterState& state(int frameIdx, int fighterIdx) const
    {
        assert(fighterIdx < fighterCount());
        assert(frameIdx < frameCount());
        return frames_[frameIdx].fighter(fighterIdx);
    }

    void addFrame(Frame&& frame);

    ListenerDispatcher<SessionListener> dispatcher;

protected:
    int findWinner() const;

private:
    Reference<MappingInfo> mappingInfo_;
    Vector<Frame> frames_;
};

class RFCOMMON_PUBLIC_API GameSession : public Session
{
public:
    GameSessionMetaData* gameMetaData() const
        { return metaData_; }

    SessionMetaData* metaData() const override;

private:
    Reference<GameSessionMetaData> metaData_;
};

class RFCOMMON_PUBLIC_API TrainingSession : public Session
{
public:
    TrainingSessionMetaData* gameMetaData() const
        { return metaData_; }

    SessionMetaData* metaData() const override;

private:
    Reference<TrainingSessionMetaData> metaData_;
};

}
