#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Reference.hpp"
#include "rfcommon/RefCounted.hpp"
#include "rfcommon/FrameDataListener.hpp"
#include "rfcommon/Vector.hpp"

#define RFCOMMON_SESSION_SECTIONS_LIST \
    X(MappingInfo, 0x01)               \
    X(Metadata, 0x02)                  \
    X(FrameData, 0x04)                 \
    X(VideoMeta, 0x08)                 \
    X(VideoEmbed, 0x10)

namespace rfcommon {
    
class FilePathResolver;
class MappedFile;
class MappingInfo;
class Metadata;
class FrameData;
class VideoMeta;
class VideoEmbed;

class RFCOMMON_PUBLIC_API Session : public RefCounted, public FrameDataListener
{
    Session(
            FilePathResolver* pathResolver,
            MappedFile* file,
            MappingInfo* mappingInfo,
            Metadata* metadata,
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

    static Session* newModernSavedSession(FilePathResolver* pathResolver, MappedFile* file);
    static Session* newLegacySavedSession(MappingInfo* mappingInfo, Metadata* metadata, FrameData* frameData, MappedFile* file);
    static Session* newActiveSession(MappingInfo* globalMappingInfo, Metadata* metadata);
    static Session* load(FilePathResolver* pathResolver, const char* fileName, uint8_t loadFlags=Flags::None);
    bool save(const char* utf8_filename, uint8_t saveFlags=Flags::All);
    uint64_t save(FILE* fp, uint8_t saveFlags=Flags::All);

    bool existsInContentTable(Flags::Flag flag) const;

    MappedFile* file() const { return file_; }

    /*!
     * \brief Returns information on how to map fighter/stage/state IDs to
     * strings.
     */
    MappingInfo* tryGetMappingInfo();

    Metadata* tryGetMetadata();

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

    const FilePathResolver* pathResolver_;
    Reference<MappedFile> file_;
    Reference<MappingInfo> mappingInfo_;
    Reference<Metadata> metadata_;
    Reference<FrameData> frameData_;
    Reference<VideoMeta> videoMeta_;
    Reference<VideoEmbed> videoEmbed_;
};

}
