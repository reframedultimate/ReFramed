#include "application/models/AutoAssociateVideosTask.hpp"
#include "application/models/PluginManager.hpp"

#include "rfcommon/DeltaTime.hpp"
#include "rfcommon/Log.hpp"
#include "rfcommon/MappedFile.hpp"
#include "rfcommon/MetaData.hpp"
#include "rfcommon/Plugin.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/TimeStamp.hpp"
#include "rfcommon/VideoEmbed.hpp"
#include "rfcommon/VideoMeta.hpp"
#include "rfcommon/VisualizerContext.hpp"

#include <QDir>
#include <QFileInfo>
#include <QDateTime>

#include <QDebug>

#include <sys/stat.h>
#include <time.h>

namespace rfapp {

// ----------------------------------------------------------------------------
AutoAssociateVideoTask::AutoAssociateVideoTask(
        rfcommon::Session* session,
        const QString& vidDir,
        PluginManager* pluginManager,
        rfcommon::Log* log,
        QObject* parent)
    : QThread(parent)
    , pluginManager_(pluginManager)
    , videoPlugin_(nullptr)
    , log_(log)
    , session_(session)
    , vidDir_(vidDir)
{
    rfcommon::Reference<rfcommon::VisualizerContext> visCtx(new rfcommon::VisualizerContext);
    for (const auto& factoryName : pluginManager_->availableFactoryNames(RFPluginType::VIDEO_PLAYER))
    {
        videoPlugin_ = pluginManager_->create(factoryName, visCtx);
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
    if (videoPlugin_ == nullptr)
    {
        log_->error("No video plugin was found. Can't open video files.");
        emit failure();
        return;
    }
    auto videoInterface = videoPlugin_->videoPlayerInterface();

    rfcommon::Reference<rfcommon::MappedFile> file = new rfcommon::MappedFile;
    auto timeStarted = session_->tryGetMetaData()->timeStarted();

    QDir dir(vidDir_);
    for (const auto& fileName : dir.entryList(QDir::Files))
    {
        QByteArray filePathBA = dir.absoluteFilePath(fileName).toUtf8();
        QByteArray fileNameBA = fileName.toUtf8();
        const char* utf8FileName = fileNameBA.constData();
        const char* utf8FilePath = filePathBA.constData();

        struct stat st;
        stat(utf8FilePath, &st);
        auto videoStarted = rfcommon::TimeStamp::fromMillisSinceEpoch((uint64_t)st.st_atime * 1000);

        // If file is newer than when the game started, ignore
        if (timeStarted < videoStarted)
            continue;

        // If file is older than 48h, ignore as well
        auto diff = timeStarted - videoStarted;
        if (diff.millis() > 48 * 60 * 60 * 1000)
            continue;

        /*
        qDebug() << fileName;

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
        videoInterface->closeVideo();

        auto videoEnd = rfcommon::TimeStamp::fromMillisSinceEpoch(
                    videoStarted.millisSinceEpoch() + endFrame.millisPassed());*/

        auto videoEnd = rfcommon::TimeStamp::fromMillisSinceEpoch((uint64_t)st.st_ctime * 1000);

        qDebug() << "started: " << timeStarted.millisSinceEpoch() << ", video start: " << videoStarted.millisSinceEpoch() << ", video end: " << videoEnd.millisSinceEpoch();

        if (timeStarted < videoEnd)
        {
            auto frameOffset = rfcommon::FrameIndex::fromSeconds(diff.millis() / 1000);
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
