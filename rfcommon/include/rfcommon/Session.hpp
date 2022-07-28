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
    Session(SessionMetaData* metaData, MappingInfo* mappingInfo, MappingInfo* globalMappingInfo, FrameData* frameData);

public:
    static Session* load(const char* fileName, MappingInfo* globalMappingInfo);
    bool save(const char* fileName);

    /*!
     * \brief Returns information on how to map fighter/stage/state IDs to
     * strings.
     */
    MappingInfo* mappingInfo() const;

    MappingInfo* globalMappingInfo() const;

    SessionMetaData* metaData() const;

    FrameData* frameData() const;

private:
    int findWinner() const;

private:
    void onFrameDataNewUniqueFrame(int frameIdx, const SmallVector<FighterState, 4>& frame) override;
    void onFrameDataNewFrame(int frameIdx, const SmallVector<FighterState, 4>& frame) override;

private:
    Reference<MappingInfo> mappingInfo_;
    Reference<MappingInfo> globalMappingInfo_;
    Reference<SessionMetaData> metaData_;
    Reference<FrameData> frameData_;
};

}
