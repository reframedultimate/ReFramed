#include "video-player/models/VideoDecoder.hpp"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

struct FrameDequeEntry
{
    FrameDequeEntry* next;
    FrameDequeEntry* prev;
    AVFrame* frame;
    int number;
};

// ----------------------------------------------------------------------------
VideoDecoder::VideoDecoder(QObject* parent)
    : QThread(parent)
    , currentFrameIndex_(-1)
    , bufSize_(64)
    , frameFreeList_(bufSize_)
    , currentFrame_(nullptr)
{
    FrameDequeEntry* entries = frameFreeList_.entries();
    for (int i = 0; i != bufSize_; ++i)
    {
        FrameDequeEntry* entry = &entries[i];
        entry->frame = av_frame_alloc();
        entry->number = -1;
    }
}

// ----------------------------------------------------------------------------
VideoDecoder::~VideoDecoder()
{
    closeFile();

    FrameDequeEntry* entries = frameFreeList_.entries();
    for (int i = 0; i != bufSize_; ++i)
    {
        av_frame_free(&entries[i].frame);
    }
}

// ----------------------------------------------------------------------------
QImage VideoDecoder::currentFrameAsImage()
{
    if (currentFrame_ == nullptr)
        return QImage();

    AVFrame* frame = currentFrame_->frame;

    videoScaleCtx_ = sws_getCachedContext(videoScaleCtx_,
        sourceWidth_, sourceHeight_, static_cast<AVPixelFormat>(frame->format),
        sourceWidth_, sourceHeight_, AV_PIX_FMT_RGB24,
        SWS_BILINEAR, nullptr, nullptr, nullptr);

    sws_scale(videoScaleCtx_,
        frame->data, frame->linesize, 0, sourceHeight_,
        rgbFrame_->data, rgbFrame_->linesize);

    return QImage(rgbFrame_->data[0], sourceWidth_, sourceHeight_, rgbFrame_->linesize[0], QImage::Format_RGB888);
}

// ----------------------------------------------------------------------------
bool VideoDecoder::openFile(const void* address, uint64_t size)
{
    emit info("Opening file from memory");

    const AVCodec* videoCodec;
    const AVCodec* audioCodec;
    int bufSize;

    ioCtx_ = avio_alloc_context(address, size, )

    // AVFormatContext holds the header information from the format (Container)
    // Allocating memory for this component
    // http://ffmpeg.org/doxygen/trunk/structAVFormatContext.html
    ctx_ = avformat_alloc_context();
    if (ctx_ == nullptr)
    {
        emit error("Failed to allocate avformat context");
        goto alloc_context_failed;
    }

    // Open the file and read its header. The codecs are not opened.
    // The function arguments are:
    // AVFormatContext (the component we allocated memory for),
    // url (filename),
    // AVInputFormat (if you pass NULL it'll do the auto detect)
    // and AVDictionary (which are options to the demuxer)
    // http://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#ga31d601155e9035d5b0e7efedc894ee49
    if (avformat_open_input(&ctx_, fileName.toStdString().c_str(), NULL, NULL) != 0)
    {
        emit error("Failed to open input file \"" + fileName + "\"");
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
    if (avformat_find_stream_info(ctx_, NULL) < 0)
    {
        emit error("Failed to find stream info");
        goto find_stream_info_failed;
    }

    videoCodec = nullptr;
    audioCodec = nullptr;
    for (int i = 0; i < ctx_->nb_streams; ++i)
    {
        AVStream* stream = ctx_->streams[i];
        AVCodecParameters* codecParams = stream->codecpar;
        const AVCodec* codec = avcodec_find_decoder(codecParams->codec_id);
        if (codec == nullptr)
        {
            emit error("Unsupported codec");
            continue;
        }

        switch (codecParams->codec_type)
        {
            case AVMEDIA_TYPE_VIDEO: {
                videoStreamIdx_ = i;
                videoCodec = codec;
            } break;

            case AVMEDIA_TYPE_AUDIO: {
                audioStreamIdx_ = i;
                audioCodec = codec;
            } break;

            default: break;
        }
    }
    if (videoStreamIdx_ == -1)
    {
        emit error("Input does not contain a video stream");
        goto video_stream_not_found;
    }
    if (audioStreamIdx_ == -1)
        info("Input does not contain an audio stream");

    // https://ffmpeg.org/doxygen/trunk/structAVCodecContext.html
    videoCodecCtx_ = avcodec_alloc_context3(videoCodec);
    if (videoCodecCtx_ == nullptr)
    {
        emit error("Failed to allocate video codec context");
        goto alloc_video_codec_context_failed;
    }

    // Fill the codec context based on the values from the supplied codec parameters
    // https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#gac7b282f51540ca7a99416a3ba6ee0d16
    if (avcodec_parameters_to_context(videoCodecCtx_, ctx_->streams[videoStreamIdx_]->codecpar) < 0)
    {
        emit error("Failed to copy video codec params to video codec context");
        goto copy_video_codec_params_failed;
    }

    // Initialize the AVCodecContext to use the given AVCodec.
    // https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#ga11f785a188d7d9df71621001465b0f1d
    if (avcodec_open2(videoCodecCtx_, videoCodec, NULL) < 0)
    {
        emit error("Failed to open video codec");
        goto open_video_codec_failed;
    }

    rgbFrame_ = av_frame_alloc();

    bufSize = av_image_fill_arrays(
        rgbFrame_->data,           /* Data pointers to be filled in */
        rgbFrame_->linesize,       /* linesizes for the image in dst_data to be filled in */
        nullptr,                   /* Source buffer - nullptr means calculate and return the required buffer size in bytes */
        AV_PIX_FMT_RGB24,          /* Destination format */
        videoCodecCtx_->width,     /* Destination width */
        videoCodecCtx_->height,    /* Destination height */
        1                          /* The value used in source buffer for linesize alignment (? no idea what that means but people on SO are using 1) */
    );
    /* if (bufSize < 0) ... */
    rgbFrameBuffer_ = (uint8_t*)av_malloc(bufSize);
    av_image_fill_arrays(
        rgbFrame_->data,           /* Data pointers to be filled in */
        rgbFrame_->linesize,       /* linesizes for the image in dst_data to be filled in */
        rgbFrameBuffer_,           /* Source buffer */
        AV_PIX_FMT_RGB24,          /* Destination format */
        videoCodecCtx_->width,     /* Destination width */
        videoCodecCtx_->height,    /* Destination height */
        1                          /* The value used in source buffer for linesize alignment (? no idea what that means but people on SO are using 1) */
    );

    sourceWidth_ = videoCodecCtx_->width;
    sourceHeight_ = videoCodecCtx_->height;
    requestShutdown_ = false;
    currentFrameIndex_ = 0;
    start();

    return true;

    open_video_codec_failed          :
    copy_video_codec_params_failed   : avcodec_free_context(&videoCodecCtx_);
    alloc_video_codec_context_failed :
    video_stream_not_found           :
    find_stream_info_failed          : avformat_close_input(&ctx_);
    open_input_failed                : avformat_free_context(ctx_);
    alloc_context_failed             : return false;
}

// ----------------------------------------------------------------------------
bool VideoDecoder::closeFile()
{
    mutex_.lock();
        requestShutdown_ = true;
        cond_.wakeOne();
    mutex_.unlock();
    wait();

    while (frontQueue_.count())
    {
        FrameDequeEntry* entry = frontQueue_.takeFront();
        av_frame_unref(entry->frame);
        frameFreeList_.put(entry);
    }
    while (backQueue_.count())
    {
        FrameDequeEntry* entry = backQueue_.takeFront();
        av_frame_unref(entry->frame);
        frameFreeList_.put(entry);
    }
    if (currentFrame_)
    {
        av_frame_unref(currentFrame_->frame);
        frameFreeList_.put(currentFrame_);
        currentFrame_ = nullptr;
    }

    if (videoScaleCtx_)
    {
        sws_freeContext(videoScaleCtx_);
        videoScaleCtx_ = nullptr;
    }

    av_free(rgbFrameBuffer_);
    av_frame_free(&rgbFrame_);
    avcodec_close(videoCodecCtx_);
    avcodec_free_context(&videoCodecCtx_);
    avformat_close_input(&ctx_);
    avformat_free_context(ctx_);

    return true;
}

// ----------------------------------------------------------------------------
void VideoDecoder::nextFrame()
{
    QMutexLocker guard(&mutex_);

    if (frontQueue_.count() == 0)
        return;

    if (currentFrame_)
        backQueue_.putFront(currentFrame_);
    currentFrame_ = frontQueue_.takeBack();
    cond_.wakeOne();
}

// ----------------------------------------------------------------------------
void VideoDecoder::prevFrame()
{
    QMutexLocker guard(&mutex_);

    if (backQueue_.count() == 0)
        return;

    if (currentFrame_)
        frontQueue_.putBack(currentFrame_);
    currentFrame_ = backQueue_.takeFront();
    cond_.wakeOne();
}

// ----------------------------------------------------------------------------
void VideoDecoder::seekToMs(uint64_t offsetFromStart)
{
    QMutexLocker guard(&mutex_);

    /*
    // Clear all cached frames
    while (frontQueue_.count())
    {
        FrameDequeEntry* entry = frontQueue_.takeBack();
        av_frame_unref(entry->frame);
        frameFreeList_.put(entry);
    }
    while (backQueue_.count())
    {
        FrameDequeEntry* entry = frontQueue_.takeBack();
        av_frame_unref(entry->frame);
        frameFreeList_.put(entry);
    }
    if (currentFrame_ != nullptr)
    {
        av_frame_unref(currentFrame_->frame);
        frameFreeList_.put(currentFrame_);
        currentFrame_ = nullptr;
    }*/
}

// ----------------------------------------------------------------------------
bool VideoDecoder::decodeNextFrame(FrameDequeEntry* entry)
{
    AVPacket packet;

    // Returns <0 if an error occurred, or if end of file was reached
    int response;
    while ((response = av_read_frame(ctx_, &packet)) >= 0)
    {
        if (packet.stream_index == videoStreamIdx_)
        {
            // Send packet to video decoder
            response = avcodec_send_packet(videoCodecCtx_, &packet);
            if (response < 0)
            {
                emit error("Failed to send packet to video decoder");
                goto exit;
            }

            response = avcodec_receive_frame(videoCodecCtx_, entry->frame);
            if (response == AVERROR(EAGAIN))
                goto need_next_packet;
            else if (response == AVERROR_EOF)
                goto exit;
            else if (response < 0)
                error("Decoder error");

            // Frame is successfully decoded here

            goto exit;
        }

        need_next_packet : av_packet_unref(&packet);
        continue;

        exit : av_packet_unref(&packet);
        break;
    }

    return (response >= 0);
}

// ----------------------------------------------------------------------------
void VideoDecoder::run()
{
    mutex_.lock();
    while (true)
    {
        if (requestShutdown_)
            break;

        int expectedBackQueueCount = bufSize_ / 2;
        int expectedFrontQueueCount = (bufSize_ - 1) / 2;

        // Remove any excess entries from front and back queues and return them
        // to the freelist. This occurs when we move to the next or previous
        // frame.
        while (backQueue_.count() > expectedBackQueueCount)
        {
            FrameDequeEntry* entry = backQueue_.takeBack();
            av_frame_unref(entry->frame);
            frameFreeList_.put(entry);
        }
        while (frontQueue_.count() > expectedFrontQueueCount)
        {
            FrameDequeEntry* entry = frontQueue_.takeFront();
            av_frame_unref(entry->frame);
            frameFreeList_.put(entry);
        }

        // If both queues are full then there's nothing left to do. Otherwise
        // try to decode as many frames as possible until both queues are full.
        if (frontQueue_.count() < expectedFrontQueueCount)
        {
            FrameDequeEntry* entry = frameFreeList_.take();
            assert(entry);
            mutex_.unlock();
                bool result = decodeNextFrame(entry);
            mutex_.lock();

            if (result)
                frontQueue_.putFront(entry);
            else
                frameFreeList_.put(entry);
        }

        if (frontQueue_.count() == expectedFrontQueueCount && requestShutdown_ == false)
            cond_.wait(&mutex_);
    }
    mutex_.unlock();
}
