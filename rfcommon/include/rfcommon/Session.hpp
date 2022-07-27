#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Reference.hpp"
#include "rfcommon/FrameDataListener.hpp"

namespace rfcommon {

class MappingInfo;
class SessionMetaData;
class FrameData;

class RFCOMMON_PUBLIC_API Session : public RefCounted, public FrameDataListener
{
public:
    Session();
    ~Session();

    static Session* load(const char* fileName);
    bool save(const char* fileName);

    /*!
     * \brief Returns information on how to map fighter/stage/state IDs to
     * strings.
     */
    MappingInfo* mappingInfo() const;

    SessionMetaData* metaData() const;

    FrameData* frameData() const;

private:
    int findWinner() const;

private:
    virtual void onFrameDataNewUniqueFrame(int frameIdx, const Frame& frame);
    virtual void onFrameDataNewFrame(int frameIdx, const Frame& frame);

private:
    Reference<MappingInfo> mappingInfo_;
    Reference<SessionMetaData> metaData_;
    Reference<FrameData> frameData_;
};

}
