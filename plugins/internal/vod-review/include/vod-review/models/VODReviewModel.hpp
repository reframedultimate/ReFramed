#pragma once

#include "rfcommon/Plugin.hpp"
#include "rfcommon/ListenerDispatcher.hpp"

class VideoPlayerModel;
class VODReviewListener;

namespace rfcommon {
    class Session;
    class VideoEmbed;
    class VideoMeta;
}

class VODReviewModel : public rfcommon::Plugin::VisualizerInterface
{
public:
    VODReviewModel(VideoPlayerModel* videoPlayer, rfcommon::VisualizerContext* visCtx, const RFPluginFactory* factory);
    ~VODReviewModel();

    void setSession(rfcommon::Session* session);
    void clearSession();

    rfcommon::Session* session() const;
    rfcommon::VideoMeta* vmeta() const;

    rfcommon::ListenerDispatcher<VODReviewListener> dispatcher;

private:
    void onVisualizerDataChanged() override;

private:
    VideoPlayerModel* videoPlayer_;
    rfcommon::Reference<rfcommon::Session> session_;
    rfcommon::Reference<rfcommon::VideoEmbed> activeVideo_;
};
