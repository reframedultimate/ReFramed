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
#include "rfcommon/VisualizerContext.hpp"

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

    // Window icon
    setWindowIcon(QIcon(":/icons/reframed-icon.ico"));

    // Start video plugin and add it to the UI
    const auto availableVideoPlugins = pluginManager_->availableFactoryNames(RFPluginType::UI | RFPluginType::VIDEO_PLAYER);
    for (const auto& factoryName : availableVideoPlugins)
    {
        rfcommon::Reference<rfcommon::VisualizerContext> visCtx(new rfcommon::VisualizerContext);
        videoPlugin_ = pluginManager_->create(factoryName, visCtx);
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
    auto embed = session_->tryGetVideoEmbed();
    if (embed)
    {
        ui_->lineEdit_fileName->setEnabled(false);
        ui_->pushButton_extractOrEmbed->setText("Extract Embed...");
    }
    else
    {
        if (vmeta)
        {
            currentVideoFileName_ = vmeta->fileName();
            ui_->lineEdit_fileName->setText(currentVideoFileName_);
        }

        ui_->pushButton_extractOrEmbed->setText("Embed File");
    }

    if (vmeta)
    {
        ui_->spinBox_frameOffset->setValue(vmeta->frameOffset().index());
        ui_->timeEdit_timeOffset->setTime(QTime(0, 0).addMSecs(vmeta->frameOffset().secondsPassed() * 1000));
    }

    // Load video if possible
    if (videoView_)
        if (auto video = session_->tryGetVideo())
            if (auto i = videoPlugin_->videoPlayerInterface())
                if (i->openVideoFromMemory(video->address(), video->size()))
                    if (vmeta)
                        i->seekVideoToGameFrame(vmeta->frameOffset());

    connect(ui_->pushButton_cancel, &QPushButton::released, this, &VideoAssociatorDialog::close);
    connect(ui_->pushButton_save, &QPushButton::released, this, &VideoAssociatorDialog::onSaveReleased);
    connect(ui_->pushButton_chooseFile, &QPushButton::released, this, &VideoAssociatorDialog::onChooseFileReleased);
    connect(ui_->spinBox_frameOffset, qOverload<int>(&QSpinBox::valueChanged), this, &VideoAssociatorDialog::onFrameOffsetChanged);
    connect(ui_->timeEdit_timeOffset, &QTimeEdit::timeChanged, this, &VideoAssociatorDialog::onTimeOffsetChanged);
    connect(ui_->pushButton_stepForwards, &QPushButton::released, this, &VideoAssociatorDialog::onNextFrame);
    connect(ui_->pushButton_stepBackwards, &QPushButton::released, this, &VideoAssociatorDialog::onPrevFrame);
    connect(ui_->pushButton_playPause, &QPushButton::released, this, &VideoAssociatorDialog::onPlayToggled);

    // Kind of ugly, but we don't have a callback for when the next video frame
    // is decoded, so the best we can do is to update the UI frequently while
    // the video is playing
    timer_.setInterval(50);
    connect(&timer_, &QTimer::timeout, this, &VideoAssociatorDialog::updateUIOffsets);
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
        if (replayManager_->videoPathExists(currentVideoFilePath_) == false)
            if (QMessageBox::question(this, "New Video Source Path",
                    "Would you like to add \"" + currentVideoFilePath_ + "\" to the video search path?\n\n"
                    ""
                    "If you say no, the information will still be saved, but ReFramed will be unable to locate the video "
                    "file when loading the replay.") == QMessageBox::Yes)
            {
                replayManager_->addVideoPath(currentVideoFilePath_, currentVideoFilePath_);
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
            timer_.stop();
            i->pauseVideo();
            updateUIVideoButtons();

            i->openVideoFromMemory(currentVideoFile_->address(), currentVideoFile_->size());
            updateUIOffsets();
        }
}

// ----------------------------------------------------------------------------
void VideoAssociatorDialog::onFrameOffsetChanged(int value)
{
    if (videoPlugin_ == nullptr)
        return;
    if (auto i = videoPlugin_->videoPlayerInterface())
    {
        timer_.stop();
        i->pauseVideo();
        updateUIVideoButtons();

        i->seekVideoToGameFrame(rfcommon::FrameIndex::fromValue(value));
        updateUIOffsets();
    }
}

// ----------------------------------------------------------------------------
void VideoAssociatorDialog::onTimeOffsetChanged(const QTime& time)
{
    if (videoPlugin_ == nullptr)
        return;
    if (auto i = videoPlugin_->videoPlayerInterface())
    {
        int offsetMS = QTime(0, 0).msecsTo(time);
        timer_.stop();
        i->pauseVideo();
        updateUIVideoButtons();

        i->seekVideoToGameFrame(rfcommon::FrameIndex::fromSeconds(offsetMS / 1000.0));
        updateUIOffsets();
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
            timer_.stop();
            i->pauseVideo();
        }
        else
        {
            timer_.start();
            i->playVideo();
        }

        updateUIVideoButtons();
        updateUIOffsets();
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
        updateUIVideoButtons();

        i->stepVideo(1);
        updateUIOffsets();
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
        updateUIVideoButtons();

        i->stepVideo(-1);
        updateUIOffsets();
    }
}

// ----------------------------------------------------------------------------
void VideoAssociatorDialog::updateUIOffsets()
{
    if (videoPlugin_ == nullptr)
        return;
    if (auto i = videoPlugin_->videoPlayerInterface())
    {
        auto frame = i->currentVideoGameFrame();

        bool save1 = ui_->spinBox_frameOffset->blockSignals(true);
        bool save2 = ui_->timeEdit_timeOffset->blockSignals(true);

        ui_->spinBox_frameOffset->setValue(frame.index());
        ui_->timeEdit_timeOffset->setTime(QTime(0, 0).addMSecs(frame.secondsPassed() * 1000));

        ui_->spinBox_frameOffset->blockSignals(save1);
        ui_->timeEdit_timeOffset->blockSignals(save2);
    }
}

// ----------------------------------------------------------------------------
void VideoAssociatorDialog::updateUIVideoButtons()
{
    if (videoPlugin_ == nullptr)
        return;
    if (auto i = videoPlugin_->videoPlayerInterface())
    {
        ui_->pushButton_playPause->setText(i->isVideoPlaying() ? "Pause" : "Play");
    }
}

}
