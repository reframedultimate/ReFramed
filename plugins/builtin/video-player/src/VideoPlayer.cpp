#include "video-player/VideoPlayer.hpp"
#include <QPlainTextEdit>
#include <QVBoxLayout>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

// ----------------------------------------------------------------------------
VideoPlayer::VideoPlayer(RFPluginFactory* factory, QWidget *parent)
    : QWidget(parent)
    , VisualizerPlugin(factory)
    , logWidget_(new QPlainTextEdit)
{
    setLayout(new QVBoxLayout);

    logWidget_->setReadOnly(true);
    layout()->addWidget(logWidget_);

    const char* videoFile = "/media/ssbu/Weekly 2021-04-05/2021-04-05_18-42-51.mkv";
    openFile(videoFile);
}

// ----------------------------------------------------------------------------
VideoPlayer::~VideoPlayer()
{
}

// ----------------------------------------------------------------------------
bool VideoPlayer::openFile(const QString& fileName)
{
    info("Opening file " + fileName);

    int videoStreamIdx = -1;
    int audioStreamIdx = -1;
    AVCodec* videoCodec;
    AVCodec* audioCodec;
    AVCodecContext* videoCodecContext;
    AVFrame* videoFrame;
    AVPacket* packet;
    int readFrames;

    // AVFormatContext holds the header information from the format (Container)
    // Allocating memory for this component
    // http://ffmpeg.org/doxygen/trunk/structAVFormatContext.html
    formatContext_ = avformat_alloc_context();
    if (formatContext_ == nullptr)
    {
        error("Failed to allocate avformat context");
        goto alloc_context_failed;
    }

    // Open the file and read its header. The codecs are not opened.
    // The function arguments are:
    // AVFormatContext (the component we allocated memory for),
    // url (filename),
    // AVInputFormat (if you pass NULL it'll do the auto detect)
    // and AVDictionary (which are options to the demuxer)
    // http://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#ga31d601155e9035d5b0e7efedc894ee49
    if (avformat_open_input(&formatContext_, fileName.toStdString().c_str(), NULL, NULL) != 0)
    {
        error("Failed to open input file \"" + fileName + "\"");
        goto open_input_failed;
    }

    // read Packets from the Format to get stream information
    // this function populates pFormatContext->streams
    // (of size equals to pFormatContext->nb_streams)
    // the arguments are:
    // the AVFormatContext
    // and options contains options for codec corresponding to i-th stream.
    // On return each dictionary will be filled with options that were not found.
    // https://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#gad42172e27cddafb81096939783b157bb
    if (avformat_find_stream_info(formatContext_, NULL) < 0)
    {
        error("Failed to find stream info");
        goto find_stream_info_failed;
    }

    videoStreamIdx = -1;
    audioStreamIdx = -1;
    for (int i = 0; i < formatContext_->nb_streams; ++i)
    {
        AVStream* stream = formatContext_->streams[i];
        AVCodecParameters* codecParams = stream->codecpar;
        AVCodec* codec = avcodec_find_decoder(codecParams->codec_id);
        if (codec == nullptr)
        {
            error("Unsupported codec");
            continue;
        }

        switch (codecParams->codec_type)
        {
            case AVMEDIA_TYPE_VIDEO: {
                videoStreamIdx = i;
                videoCodec = codec;
            } break;

            case AVMEDIA_TYPE_AUDIO: {
                audioStreamIdx = i;
                audioCodec = codec;
            } break;

            default: break;
        }
    }
    if (videoStreamIdx == -1)
    {
        error("Input does not contain a video stream");
        goto video_stream_not_found;
    }
    if (audioStreamIdx == -1)
        info("Input does not contain an audio stream");

    // https://ffmpeg.org/doxygen/trunk/structAVCodecContext.html
    videoCodecContext = avcodec_alloc_context3(videoCodec);
    if (videoCodecContext == nullptr)
    {
        error("Failed to allocate video codec context");
        goto alloc_video_codec_context_failed;
    }

    // Fill the codec context based on the values from the supplied codec parameters
    // https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#gac7b282f51540ca7a99416a3ba6ee0d16
    if (avcodec_parameters_to_context(videoCodecContext, formatContext_->streams[videoStreamIdx]->codecpar) < 0)
    {
        error("Failed to copy video codec params to video codec context");
        goto copy_video_codec_params_failed;
    }

    // Initialize the AVCodecContext to use the given AVCodec.
    // https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#ga11f785a188d7d9df71621001465b0f1d
    if (avcodec_open2(videoCodecContext, videoCodec, NULL) < 0)
    {
        error("Failed to open video codec");
        goto open_video_codec_failed;
    }

    // https://ffmpeg.org/doxygen/trunk/structAVFrame.html
    videoFrame = av_frame_alloc();
    if (videoFrame == nullptr)
    {
        error("Failed to allocate video frame");
        goto alloc_video_frame_failed;
    }

    // https://ffmpeg.org/doxygen/trunk/structAVPacket.html
    packet = av_packet_alloc();
    if (packet == nullptr)
    {
        error("failed to allocate packet");
        goto alloc_video_packet_failed;
    }

    readFrames = 1;
    while (av_read_frame(formatContext_, packet) >= 0)
    {
        if (packet->stream_index == videoStreamIdx)
        {
            int response = avcodec_send_packet(videoCodecContext, packet);
            if (response < 0)
            {
                error("Decoder error");
                goto exit;
            }

            while (response >= 0)
            {
                response = avcodec_receive_frame(videoCodecContext, videoFrame);
                if (response == AVERROR(EAGAIN))
                    goto need_next_packet;
                else if (response == AVERROR_EOF)
                    goto exit;
                else if (response < 0)
                {
                    error("Decoder error");
                    goto exit;
                }

                info(QString("Decoded frame: %1x%2").arg(videoCodecContext->width).arg(videoCodecContext->height));

                if (readFrames-- <= 0)
                    goto exit;
            }
        }

        need_next_packet : av_packet_unref(packet);
        continue;

        exit : av_packet_unref(packet);
        break;
    }

    av_packet_free(&packet);
    av_frame_free(&videoFrame);
    avcodec_close(videoCodecContext);
    avcodec_free_context(&videoCodecContext);
    avformat_close_input(&formatContext_);
    avformat_free_context(formatContext_);

    return true;

    alloc_video_packet_failed        : av_frame_free(&videoFrame);
    alloc_video_frame_failed         : avcodec_close(videoCodecContext);
    open_video_codec_failed          :
    copy_video_codec_params_failed   : avcodec_free_context(&videoCodecContext);
    alloc_video_codec_context_failed :
    video_stream_not_found           :
    find_stream_info_failed          : avformat_close_input(&formatContext_);
    open_input_failed                : avformat_free_context(formatContext_);
    alloc_context_failed             : return false;
}

// ----------------------------------------------------------------------------
void VideoPlayer::info(const QString& msg)
{
    logWidget_->textCursor().insertText("[INFO] " + msg + "\n");
}

// ----------------------------------------------------------------------------
void VideoPlayer::error(const QString& msg)
{
    logWidget_->textCursor().insertText("[ERROR] " + msg + "\n");
}
