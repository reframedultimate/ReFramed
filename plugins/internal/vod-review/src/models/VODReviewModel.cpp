#include "vod-review/listeners/VODReviewListener.hpp"
#include "vod-review/models/VideoPlayerModel.hpp"
#include "vod-review/models/VODReviewModel.hpp"

#include "rfcommon/Session.hpp"
#include "rfcommon/VideoMeta.hpp"
#include "rfcommon/VideoEmbed.hpp"
#include "rfcommon/FrameData.hpp"

// ----------------------------------------------------------------------------
VODReviewModel::VODReviewModel(VideoPlayerModel* videoPlayer, rfcommon::PluginContext* pluginCtx, const RFPluginFactory* factory)
    : SharedDataInterface(pluginCtx, factory)
    , videoPlayer_(videoPlayer)
{}

// ----------------------------------------------------------------------------
VODReviewModel::~VODReviewModel()
{}

// ----------------------------------------------------------------------------
void VODReviewModel::setSession(rfcommon::Session* session)
{
    session_ = session;
    activeVideo_ = session_->tryGetVideo();
    if (activeVideo_.isNull())
        return;

    if (videoPlayer_->openVideoFromMemory(activeVideo_->address(), activeVideo_->size()) == false)
    {
        activeVideo_.drop();
        session_.drop();
        return;
    }

    videoPlayer_->seekVideoToGameFrame(vmeta()->frameOffset());

    //videoPlayer_->playVideo();
}

// ----------------------------------------------------------------------------
void VODReviewModel::clearSession()
{
    videoPlayer_->closeVideo();
    session_.drop();
}

// ----------------------------------------------------------------------------
rfcommon::Session* VODReviewModel::session() const
{
    return session_;
}

// ----------------------------------------------------------------------------
rfcommon::VideoMeta* VODReviewModel::vmeta() const
{ 
    return session_->tryGetVideoMeta();
}

// ----------------------------------------------------------------------------
void VODReviewModel::onSharedDataChanged()
{
    dispatcher.dispatch(&VODReviewListener::onVODReviewPluginSharedDataChanged);
}
