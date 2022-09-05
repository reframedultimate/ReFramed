#include "application/ui_VideoAssociatorDialog.h"
#include "application/views/VideoAssociatorDialog.hpp"
#include "application/models/PluginManager.hpp"
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
    , currentFileName_(currentFileName)
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
    auto embed = session_->tryGetVideoEmbed();
    if (embed)
    {
        ui_->lineEdit_fileName->setEnabled(false);
        ui_->lineEdit_filePath->setEnabled(false);
        ui_->pushButton_extractOrEmbed->setText("Extract Embed...");
    }
    else
    {
        if (vmeta)
        {
            ui_->lineEdit_fileName->setText(vmeta->fileName());
            ui_->lineEdit_filePath->setText(vmeta->filePath());
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


}

// ----------------------------------------------------------------------------
void VideoAssociatorDialog::onChooseFileReleased()
{
    PROFILE(VideoAssociatorDialog, onChooseFileReleased);

    /*
    QString fileName = QFileDialog::getOpenFileName(
        this, "Open Video File", "", "Video Files (*.mp4 *.mkv *.avi *.webm)");
    if (fileName.length() == 0)
        return;*/
    //QString fileName = "/media/ssbu/2022-08-04 - Basel Summer Weekly/2022-08-04 - Bo3 - TheComet (Pika) vs DeepFreeze (Joker) Game 1.mp4";
    QString fileName = "/media/ssbu/2021-04-18 - NullSpace+TAEL/Weekly 2021-04-18 - Bo5 - TheComet (Pika) vs NullSpace (Wolf) Game 1.mp4";

    rfcommon::Reference<rfcommon::MappedFile> f = new rfcommon::MappedFile;
    QByteArray ba = fileName.toUtf8();
    if (f->open(ba.constData()) == false)
        return;

    currentVideoFile_ = f;

    if (videoPlugin_)
        videoPlugin_->videoPlayerInterface()->openVideoFromMemory(currentVideoFile_->address(), currentVideoFile_->size());
}

// ----------------------------------------------------------------------------
void VideoAssociatorDialog::onPlayToggled()
{
    if (videoPlugin_ == nullptr)
        return;
    if (auto i = videoPlugin_->videoPlayerInterface())
    {
        if (i->isVideoPlaying())
            i->pauseVideo();
        else
            i->playVideo();
    }
}

// ----------------------------------------------------------------------------
void VideoAssociatorDialog::onNextFrame()
{
    if (videoPlugin_ == nullptr)
        return;
    if (auto i = videoPlugin_->videoPlayerInterface())
        if (auto fdata = session_->tryGetFrameData())
        {
            i->pauseVideo();
            i->stepVideo(1);
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
    }
}

}
