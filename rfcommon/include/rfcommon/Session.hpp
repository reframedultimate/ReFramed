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
class VideoFileResolver;

class RFCOMMON_PUBLIC_API Session : public RefCounted, public FrameDataListener
{
    Session(
            VideoFileResolver* vfr,
            MappedFile* file,
            MappingInfo* mappingInfo,
            MetaData* metaData,
            FrameData* frameData,
            VideoMeta* videoMeta,
            VideoEmbed* video);

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

    static Session* newModernSavedSession(VideoFileResolver* vfr, MappedFile* file);
    static Session* newLegacySavedSession(MappingInfo* mappingInfo, MetaData* metaData, FrameData* frameData);
    static Session* newActiveSession(MappingInfo* globalMappingInfo, MetaData* metaData);
    static Session* load(VideoFileResolver* vfr, const char* fileName, uint8_t loadFlags=Flags::None);
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
    VideoEmbed* tryGetVideo();

    void setNewVideo(VideoMeta* meta, VideoEmbed* embed);

private:
    void eraseFromContentTable(Flags::Flag flag);
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

    const VideoFileResolver* vfr_;
    Reference<MappedFile> file_;
    Reference<MappingInfo> mappingInfo_;
    Reference<MetaData> metaData_;
    Reference<FrameData> frameData_;
    Reference<VideoMeta> videoMeta_;
    Reference<VideoEmbed> videoEmbed_;
};

}
