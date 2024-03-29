#include "application/models/AutoAssociateVideosTask.hpp"
#include "application/models/PluginManager.hpp"

#include "rfcommon/DeltaTime.hpp"
#include "rfcommon/Log.hpp"
#include "rfcommon/MappedFile.hpp"
#include "rfcommon/Metadata.hpp"
#include "rfcommon/Plugin.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/TimeStamp.hpp"
#include "rfcommon/VideoEmbed.hpp"
#include "rfcommon/VideoMeta.hpp"
#include "rfcommon/PluginContext.hpp"

#include <QDir>
#include <QFileInfo>
#include <QDateTime>

#include <QDebug>

#if defined(RFCOMMON_PLATFORM_WINDOWS)
#   define WIN32_LEAN_AND_MEAN
#   include <Windows.h>
#   include "rfcommon/LastError.hpp"
#   include "rfcommon/time.h"
#   include "rfcommon/Utf8.hpp"
#elif defined(RFCOMMON_PLATFORM_LINUX)
#   include <sys/stat.h>
#   include <time.h>
#endif

namespace rfapp {

// ----------------------------------------------------------------------------
AutoAssociateVideoTask::AutoAssociateVideoTask(
        rfcommon::Session* session,
        const QString& vidDir,
        int frameOffsetCorrection,
        PluginManager* pluginManager,
        rfcommon::Log* log,
        QObject* parent)
    : QThread(parent)
    , pluginManager_(pluginManager)
    , videoPlugin_(nullptr)
    , log_(log)
    , session_(session)
    , vidDir_(vidDir)
    , frameCorrection_(frameOffsetCorrection)
{
    rfcommon::Reference<rfcommon::PluginContext> pluginCtx(new rfcommon::PluginContext);
    for (const auto& factoryName : pluginManager_->availableFactoryNames(RFPluginType::VIDEO_PLAYER))
    {
        videoPlugin_ = pluginManager_->create(factoryName, pluginCtx);
        if (videoPlugin_)
        {
            if (videoPlugin_->videoPlayerInterface())
                break;
            pluginManager_->destroy(videoPlugin_);
            videoPlugin_ = nullptr;
        }
    }
}

// ----------------------------------------------------------------------------
AutoAssociateVideoTask::~AutoAssociateVideoTask()
{
    if (videoPlugin_)
        pluginManager_->destroy(videoPlugin_);
}

// ----------------------------------------------------------------------------
void AutoAssociateVideoTask::run()
{
    PROFILE(AutoAssociateVideoTask, run);

    if (videoPlugin_ == nullptr)
    {
        log_->error("No video plugin was found. Can't open video files.");
        emit failure();
        return;
    }
    auto videoInterface = videoPlugin_->videoPlayerInterface();

    rfcommon::Reference<rfcommon::MappedFile> file = new rfcommon::MappedFile;
    auto gameStarted = session_->tryGetMetadata()->timeStarted();

    QDir dir(vidDir_);
    for (const auto& fileName : dir.entryList(QDir::Files))
    {
        QByteArray filePathBA = dir.absoluteFilePath(fileName).toUtf8();
        QByteArray fileNameBA = fileName.toUtf8();
        const char* utf8FileName = fileNameBA.constData();
        const char* utf8FilePath = filePathBA.constData();

#if defined(RFCOMMON_PLATFORM_WINDOWS)
        wchar_t* utf16FilePath = rfcommon::utf8_to_utf16(utf8FilePath, (int)strlen(utf8FilePath));
        if (utf8FilePath == nullptr)
            continue;

        WIN32_FILE_ATTRIBUTE_DATA fileAttributes;
        if (GetFileAttributesExW(utf16FilePath, GetFileExInfoStandard, (void*)&fileAttributes) == 0)
        {
            log_->error("Failed to get file attributes for file \"%s\": %s", utf8FilePath, rfcommon::LastError().cStr());
            rfcommon::utf16_free(utf16FilePath);
            continue;
        }

        auto videoStarted = rfcommon::TimeStamp::fromMillisSinceEpoch(
                    time_win32_filetime_to_milli_seconds_since_epoch(
                        ((uint64_t)fileAttributes.ftCreationTime.dwHighDateTime << 32) +
                        (uint64_t)fileAttributes.ftCreationTime.dwLowDateTime));
        auto videoEnd = rfcommon::TimeStamp::fromMillisSinceEpoch(
                    time_win32_filetime_to_milli_seconds_since_epoch(
                        ((uint64_t)fileAttributes.ftLastWriteTime.dwHighDateTime << 32) +
                        (uint64_t)fileAttributes.ftLastWriteTime.dwLowDateTime));

        rfcommon::utf16_free(utf16FilePath);
#elif defined(RFCOMMON_PLATFORM_LINUX)
        struct stat st;
        stat(utf8FilePath, &st);
        auto videoStarted = rfcommon::TimeStamp::fromSecondsSinceEpoch((uint64_t)st.st_atime);
        auto videoEnd     = rfcommon::TimeStamp::fromSecondsSinceEpoch((uint64_t)st.st_ctime);
#endif

        // If file is newer than when the game started, ignore
        if (gameStarted < videoStarted)
            continue;

        // If file is older than 48h, ignore as well
        auto offset = gameStarted - videoStarted;
        if (offset.seconds() > 48 * 60 * 60)
            continue;

        // If for whatever reason the file metadata isn't right, we have to fall
        // back to decoding the video to determine its length. Here we make the
        // assumption that a game lasts longer than 10 seconds
        if ((videoEnd - videoStarted).seconds() < 10)
        {
            if (file->open(utf8FilePath) == false)
            {
                log_->error("Failed to open file \"%s\"", utf8FileName);
                continue;
            }
            if (videoInterface->openVideoFromMemory(file->address(), file->size()) == false)
            {
                log_->error("Failed to load file as video: \"%s\"", utf8FileName);
                file->close();
                continue;
            }

            rfcommon::FrameIndex endFrame = videoInterface->videoGameFrameCount();
            videoEnd = rfcommon::TimeStamp::fromMillisSinceEpoch(
                        videoStarted.millisSinceEpoch() + endFrame.millisPassed());

            videoInterface->closeVideo();
        }

        if (gameStarted < videoEnd)
        {
            auto frameOffset = rfcommon::FrameIndex::fromSeconds(offset.seconds());
            frameOffset += frameCorrection_;
            log_->info("Associating file \"%s\" with replay, offset=%d frames", utf8FileName, frameOffset.index());
            rfcommon::VideoMeta* vmeta = new rfcommon::VideoMeta(utf8FileName, frameOffset, false);
            session_->setNewVideo(vmeta, nullptr);

            emit success();
            return;
        }
    }

    emit failure();
}

}
