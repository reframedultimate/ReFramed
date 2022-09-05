#include "application/ui_VideoAssociatorDialog.h"
#include "application/views/VideoAssociatorDialog.hpp"
#include "application/models/PluginManager.hpp"
#include "application/models/ReplayManager.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/MappedFile.hpp"
#include "rfcommon/Plugin.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/VideoMeta.hpp"
#include "rfcommon/VideoEmbed.hpp"

#include <QShortcut>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>

namespace rfapp {

// ----------------------------------------------------------------------------
VideoAssociatorDialog::VideoAssociatorDialog(
        PluginManager* pluginManager,
        ReplayManager* replayManager,
        rfcommon::Session* session,
        const QString& currentFileName,
        QWidget* parent)
    : QDialog(parent)
    , ui_(new Ui::VideoAssociatorDialog)
    , pluginManager_(pluginManager)
    , replayManager_(replayManager)
    , videoPlugin_(nullptr)
    , videoView_(nullptr)
    , session_(session)
    , currentSessionFileName_(currentFileName)
{
    ui_->setupUi(this);

    // Start video plugin and add it to the UI
    const auto availableVideoPlugins = pluginManager_->availableFactoryNames(RFPluginType::UI | RFPluginType::VIDEO_PLAYER);
    for (const auto& factoryName : availableVideoPlugins)
    {
        videoPlugin_ = pluginManager_->create(factoryName);
        if (videoPlugin_)
            if (videoPlugin_->videoPlayerInterface())
                if (auto i = videoPlugin_->uiInterface())
                    if ((videoView_ = i->createView()))
                        break;

        if (videoPlugin_)
            pluginManager_->destroy(videoPlugin_);
        videoPlugin_ = nullptr;
    }
    if (videoView_)
    {
        togglePlayShortcut_ = new QShortcut(QKeySequence(Qt::Key_Space), videoView_);
        nextFrameShortcut_ = new QShortcut(QKeySequence(Qt::Key_Right), videoView_);
        prevFrameShortcut_ = new QShortcut(QKeySequence(Qt::Key_Left), videoView_);
        ui_->layout_videoPlugin->addWidget(videoView_);

        connect(togglePlayShortcut_, &QShortcut::activated, this, &VideoAssociatorDialog::onPlayToggled);
        connect(nextFrameShortcut_, &QShortcut::activated, this, &VideoAssociatorDialog::onNextFrame);
        connect(prevFrameShortcut_, &QShortcut::activated, this, &VideoAssociatorDialog::onPrevFrame);
    }
    else
    {
        QLabel* label = new QLabel;
        if (availableVideoPlugins.size() == 0)
            label->setText("No video plugin was found! Make sure you enabled the built-in video plugin, and if you built from source, ensure that you specified -DREFRAMED_plugin-video-player:BOOL=ON");
        else
            label->setText("Failed to create video player view!");
        label->setWordWrap(true);
        ui_->layout_videoPlugin->addWidget(label);
    }

    // Fill in UI with values from the session object
    auto vmeta = session_->tryGetVideoMeta();
    auto embed = session_->tryGetVideo();
    if (embed)
    {
        ui_->lineEdit_fileName->setEnabled(false);
        ui_->pushButton_extractOrEmbed->setText("Extract Embed...");
    }
    else
    {
        if (vmeta)
        {
            ui_->lineEdit_fileName->setText(vmeta->fileName());
        }
        ui_->pushButton_extractOrEmbed->setText("Embed File");
    }

    /*
    if (vmeta)
    {
        QTime time;
        time.addSecs(vmeta->frameOffset().seconds());

        ui_->spinBox_frameOffset->setValue(vmeta->frameOffset().frames());
        ui_->timeEdit_timeOffset->setTime(time);
    }*/

    connect(ui_->pushButton_cancel, &QPushButton::released, this, &VideoAssociatorDialog::close);
    connect(ui_->pushButton_save, &QPushButton::released, this, &VideoAssociatorDialog::onSaveReleased);
    connect(ui_->pushButton_chooseFile, &QPushButton::released, this, &VideoAssociatorDialog::onChooseFileReleased);

    // Kind of ugly, but we don't have a callback for when the next video frame
    // is decoded, so the best we can do is to update the UI frequently while
    // the video is playing
    timer_.setInterval(50);
    connect(&timer_, &QTimer::timeout, this, &VideoAssociatorDialog::updateTimesFromVideo);
}

// ----------------------------------------------------------------------------
VideoAssociatorDialog::~VideoAssociatorDialog()
{
    if (videoView_)
    {
        videoView_->setParent(nullptr);
        videoPlugin_->uiInterface()->destroyView(videoView_);
        pluginManager_->destroy(videoPlugin_);
    }

    delete ui_;
}

// ----------------------------------------------------------------------------
void VideoAssociatorDialog::onSaveReleased()
{
    PROFILE(VideoAssociatorDialog, onSaveReleased);

    if (currentVideoFilePath_.length() > 0)
    {
        if ([this]{
            for (int i = 0; i != replayManager_->videoSourcesCount(); ++i)
                if (replayManager_->videoSourcePath(i) == currentVideoFilePath_)
                    return false;
            return true;
        }()) {
            if (QMessageBox::question(this, "New Video Source Path",
                    "Would you like to add \"" + currentVideoFilePath_ + "\" to the video search path?\n\n"
                    ""
                    "If you say no, the information will still be saved, but ReFramed will be unable to locate the video"
                    "file when loading the replay.") == QMessageBox::Yes)
            {
                replayManager_->addVideoSource(currentVideoFilePath_, currentVideoFilePath_);
            }
        }
    }

    rfcommon::Reference<rfcommon::VideoMeta> meta = new rfcommon::VideoMeta(
                currentVideoFileName_.toUtf8().constData(),
                rfcommon::FrameIndex::fromValue(ui_->spinBox_frameOffset->value()),
                false);

    rfcommon::Reference<rfcommon::VideoEmbed> embed = nullptr;  // TODO support embeds

    session_->setNewVideo(meta, embed);

    if (replayManager_->saveReplayOver(session_, currentSessionFileName_))
        close();
    else
        QMessageBox::critical(this, "Error", "Failed to save file");
}

// ----------------------------------------------------------------------------
void VideoAssociatorDialog::onChooseFileReleased()
{
    PROFILE(VideoAssociatorDialog, onChooseFileReleased);

    QString fileName = QFileDialog::getOpenFileName(
        this, "Open Video File", "", "Video Files (*.mp4 *.mkv *.avi *.webm)");
    if (fileName.length() == 0)
        return;

    rfcommon::Reference<rfcommon::MappedFile> f = new rfcommon::MappedFile;
    QByteArray ba = fileName.toUtf8();
    if (f->open(ba.constData()) == false)
        return;

    QFileInfo fi(fileName);
    currentVideoFileName_ = fi.fileName();
    currentVideoFilePath_ = fi.absolutePath();
    ui_->lineEdit_fileName->setText(currentVideoFileName_);

    currentVideoFile_ = f;

    if (videoPlugin_)
        if (auto i = videoPlugin_->videoPlayerInterface())
        {
            i->openVideoFromMemory(currentVideoFile_->address(), currentVideoFile_->size());
            i->stepVideo(1);
        }
}

// ----------------------------------------------------------------------------
void VideoAssociatorDialog::onPlayToggled()
{
    if (videoPlugin_ == nullptr)
        return;
    if (auto i = videoPlugin_->videoPlayerInterface())
    {
        if (i->isVideoPlaying())
        {
            i->pauseVideo();
            timer_.stop();
        }
        else
        {
            i->playVideo();
            timer_.start();
        }

        updateTimesFromVideo();
    }
}

// ----------------------------------------------------------------------------
void VideoAssociatorDialog::onNextFrame()
{
    if (videoPlugin_ == nullptr)
        return;
    if (auto i = videoPlugin_->videoPlayerInterface())
    {
        i->pauseVideo();
        i->stepVideo(1);
        updateTimesFromVideo();
    }
}

// ----------------------------------------------------------------------------
void VideoAssociatorDialog::onPrevFrame()
{
    if (videoPlugin_ == nullptr)
        return;
    if (auto i = videoPlugin_->videoPlayerInterface())
    {
        i->pauseVideo();
        i->stepVideo(-1);
        updateTimesFromVideo();
    }
}

// ----------------------------------------------------------------------------
void VideoAssociatorDialog::updateTimesFromVideo()
{
    if (videoPlugin_ == nullptr)
        return;
    if (auto i = videoPlugin_->videoPlayerInterface())
    {
        auto frame = i->currentVideoGameFrame();

        ui_->spinBox_frameOffset->setValue(frame.index());
        ui_->timeEdit_timeOffset->setTime(QTime(0, 0).addMSecs(frame.secondsPassed() * 1000));
    }
}

}
