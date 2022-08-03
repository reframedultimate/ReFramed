#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Reference.hpp"
#include "rfcommon/RefCounted.hpp"
#include "rfcommon/FrameDataListener.hpp"

namespace rfcommon {

class MappingInfo;
class MetaData;
class FrameData;

class RFCOMMON_PUBLIC_API Session : public RefCounted, public FrameDataListener
{
    Session(FILE* fp, MappingInfo* mappingInfo, MetaData* metaData, FrameData* frameData);

public:
    enum LoadFlags
    {
        MAPPING_INFO = 0x01,
        META_DATA = 0x02,
        FRAME_DATA = 0x04,
        ALL = 0x07
    };

    ~Session();

    static Session* newModernSavedSession(FILE* fp);
    static Session* newLegacySavedSession(MappingInfo* mappingInfo, MetaData* metaData, FrameData* frameData);
    static Session* newActiveSession(MappingInfo* globalMappingInfo, MetaData* metaData);
    static Session* load(const char* fileName, uint8_t loadFlags=0);
    bool save(const char* fileName);

    bool existsInContentTable(LoadFlags flag) const;

    void setMappingInfo(MappingInfo* mappingInfo);

    /*!
     * \brief Returns information on how to map fighter/stage/state IDs to
     * strings.
     */
    MappingInfo* tryGetMappingInfo();

    MetaData* tryGetMetaData();

    FrameData* tryGetFrameData();

private:
    void onFrameDataNewUniqueFrame(int frameIdx, const Frame<4>& frame) override;
    void onFrameDataNewFrame(int frameIdx, const Frame<4>& frame) override;

private:
    struct ContentTableEntry
    {
        ContentTableEntry();
        ContentTableEntry(const char* typeStr);

        char type[4];
        uint32_t offset;
        uint32_t size;
    };

    FILE* fp_;
    SmallVector<ContentTableEntry, 4> contentTable_;

    Reference<MappingInfo> mappingInfo_;
    Reference<MetaData> metaData_;
    Reference<FrameData> frameData_;
};

}
