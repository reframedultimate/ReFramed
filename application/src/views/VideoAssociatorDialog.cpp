#include "application/ui_VideoAssociatorDialog.h"
#include "application/views/VideoAssociatorDialog.hpp"
#include "application/models/PluginManager.hpp"
#include "rfcommon/Plugin.hpp"
#include "rfcommon/Session.hpp"

#include <QPushButton>

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

    for (const auto& factoryName : pluginManager_->availableFactoryNames(RFPluginType::UI | RFPluginType::VIDEO_PLAYER))
    {
        videoPlugin_ = pluginManager_->create(factoryName);
        if (videoPlugin_)
            if (auto i = videoPlugin_->uiInterface())
                if ((videoView_ = i->createView()))
                    break;

        if (videoPlugin_)
            pluginManager_->destroy(videoPlugin_);
        videoPlugin_ = nullptr;
    }

    if (videoView_)
        ui_->layout_videoPlugin->addWidget(videoView_);

    connect(ui_->pushButton_cancel, &QPushButton::released, this, &VideoAssociatorDialog::close);
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

}
