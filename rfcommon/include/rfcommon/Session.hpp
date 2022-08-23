#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Reference.hpp"
#include "rfcommon/RefCounted.hpp"
#include "rfcommon/FrameDataListener.hpp"
#include "rfcommon/Vector.hpp"

#define RFCOMMON_SESSION_SECTIONS_LIST \
    X(MappingInfo, 0x01)               \
    X(MetaData, 0x02)                  \
    X(FrameData, 0x04)                 \
    X(VideoMeta, 0x08)                 \
    X(VideoEmbed, 0x10)

namespace rfcommon {

class MappedFile;
class MappingInfo;
class MetaData;
class FrameData;
class VideoMeta;
class VideoEmbed;

class RFCOMMON_PUBLIC_API Session : public RefCounted, public FrameDataListener
{
    Session(MappedFile* file, MappingInfo* mappingInfo, MetaData* metaData, FrameData* frameData);

public:
    struct Flags {
        enum Flag
        {
#define X(name, value) name = value,
            RFCOMMON_SESSION_SECTIONS_LIST
#undef X
            None = 0x00,
            All = 0xFF
        };
    };

    ~Session();

    static Session* newModernSavedSession(MappedFile* file);
    static Session* newLegacySavedSession(MappingInfo* mappingInfo, MetaData* metaData, FrameData* frameData);
    static Session* newActiveSession(MappingInfo* globalMappingInfo, MetaData* metaData);
    static Session* load(const char* fileName, uint8_t loadFlags=Flags::None);
    bool save(const char* fileNamem, uint8_t saveFlags=Flags::All);

    bool existsInContentTable(Flags::Flag flag) const;

    /*!
     * \brief Returns information on how to map fighter/stage/state IDs to
     * strings.
     */
    MappingInfo* tryGetMappingInfo();

    MetaData* tryGetMetaData();

    FrameData* tryGetFrameData();

    VideoMeta* tryGetVideoMeta();

    VideoEmbed* tryGetVideoEmbed();

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
    SmallVector<ContentTableEntry, 5> contentTable_;

    Reference<MappedFile> file_;
    Reference<MappingInfo> mappingInfo_;
    Reference<MetaData> metaData_;
    Reference<FrameData> frameData_;
    Reference<VideoMeta> videoMeta_;
    Reference<VideoEmbed> videoEmbed_;
};

}
