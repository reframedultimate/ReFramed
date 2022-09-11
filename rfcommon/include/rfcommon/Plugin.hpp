#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/FrameIndex.hpp"
#include "rfcommon/ProtocolListener.hpp"
#include "rfcommon/Reference.hpp"
#include "rfcommon/String.hpp"
#include "rfcommon/Vector.hpp"

class QWidget;
struct RFPluginFactory;

namespace rfcommon {

class Session;
class VisualizerContext;
class VisualizerData;

class RFCOMMON_PUBLIC_API Plugin
{
public:
    class UIInterface
    {
    public:
        virtual QWidget* createView() = 0;
        virtual void destroyView(QWidget* view) = 0;
    };

    class ReplayInterface
    {
    public:
        virtual void onGameSessionLoaded(rfcommon::Session* game) = 0;
        virtual void onGameSessionUnloaded(rfcommon::Session* game) = 0;
        virtual void onTrainingSessionLoaded(rfcommon::Session* training) = 0;
        virtual void onTrainingSessionUnloaded(rfcommon::Session* training) = 0;

        virtual void onGameSessionSetLoaded(rfcommon::Session** games, int numGames) = 0;
        virtual void onGameSessionSetUnloaded(rfcommon::Session** games, int numGames) = 0;
    };

    class RFCOMMON_PUBLIC_API VisualizerInterface
    {
    public:
        VisualizerInterface(VisualizerContext* visCtx, const RFPluginFactory* factory);
        virtual ~VisualizerInterface();

        int visualizerSourceCount() const;
        const VisualizerData& visualizerData(int sourceIdx) const;
        void setVisualizerData(VisualizerData&& data);

        virtual void onVisualizerDataChanged() = 0;

    private:
        Reference<VisualizerContext> visCtx_;
        const String name_;
    };

    class RealtimeInterface : public ProtocolListener
    {
    public:
    };

    class VideoPlayerInterface
    {
    public:
        /*!
         * \brief Open a video file and display the first frame. Video
         * player should pause.
         *
         * In ReFramed, video files are mapped into memory instead of
         * dealing with file pointers and file names. This makes it possible
         * to use the same API for playing embedded videos and external
         * videos.
         *
         * \note ReFramed will guarantee that this function won't be called
         * twice in a row. close() will always be called first if necessary.
         */
        virtual bool openVideoFromMemory(const void* address, uint64_t size) = 0;

        /*!
         * \brief Close the video. Player should reset everything.
         * \note ReFramed will guarantee that this function won't be called
         * twice in a row.
         */
        virtual void closeVideo() = 0;

        /*!
         * \brief Begin normal playback of the video stream.
         */
        virtual void playVideo() = 0;

        /*!
         * \brief Pause the video stream.
         */
        virtual void pauseVideo() = 0;

        /*!
         * \brief Return true if the video is playing, otherwise false
         */
        virtual bool isVideoPlaying() const = 0;

        /*!
         * \brief Set the volume in percent.
         */
        virtual void setVideoVolume(int percent) = 0;

        /*!
         * \brief Advance by N number of video-frames (not game-frames).
         * \note N can be negative, which means to go backwards N frames.
         *
         * The video player should avoid seeking here if possible, but instead,
         * decode each successive frame as needed. In the case of advancing
         * backwards (negative value for N), the video player can seek
         * if required to buffer the previous frames.
         *
         * \param frames The number of frames to seek. Can be negative. This
         * value is guaranteed to be "small", i.e. in the range of -30 to 30.
         */
        virtual void stepVideo(int videoFrames) = 0;

        /*!
         * \brief Try to seek to a specific game-frame (not video frame). Due
         * to the nature of decoding video streams, it's OK to not be 100%
         * accurate here, but you should try for best effort.
         */
        virtual void seekVideoToGameFrame(rfcommon::FrameIndex frameNumber) = 0;

        virtual rfcommon::FrameIndex currentVideoGameFrame() = 0;
    };

    Plugin(const RFPluginFactory* factory);
    virtual ~Plugin();

    // These avoid the need for ReFramed to dynamic_cast the plugin to a
    // derived type. Plugins should simply return "this" if they implement
    // the interface, and nullptr if they don't. Plugins may also separate
    // the interfaces into individual objects and return those, if it's more
    // convenient.
    virtual Plugin::UIInterface* uiInterface() = 0;
    virtual Plugin::ReplayInterface* replayInterface() = 0;
    virtual Plugin::VisualizerInterface* visualizerInterface() = 0;
    virtual Plugin::RealtimeInterface* realtimeInterface() = 0;
    virtual Plugin::VideoPlayerInterface* videoPlayerInterface() = 0;

    const RFPluginFactory* factory() const;

private:
    const RFPluginFactory* const factory_;
};

}
