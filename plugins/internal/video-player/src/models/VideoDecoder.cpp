#include "video-player/models/VideoDecoder.hpp"
#include "rfcommon/Deserializer.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Log.hpp"

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
};

// ----------------------------------------------------------------------------
VideoDecoder::VideoDecoder(const void* address, uint64_t size, rfcommon::Log* log, QObject* parent)
    : QThread(parent)
    , log_(log)
    , currentGameFrame_(rfcommon::FrameIndex::fromValue(-1))
    , bufSize_(64)
    , frameFreeList_(bufSize_)
    , currentFrame_(nullptr)
{
    // We maintain a double ended queue of AVFrames that are filled asynchronously
    FrameDequeEntry* entries = frameFreeList_.entries();
    for (int i = 0; i != bufSize_; ++i)
    {
        FrameDequeEntry* entry = &entries[i];
        entry->frame = av_frame_alloc();
    }

    // Open file from memory and start decoder thread
    isOpen_ = openFile(address, size);
    start();
}

// ----------------------------------------------------------------------------
VideoDecoder::~VideoDecoder()
{
    // Joins decoding thread and cleans up everything
    if (isOpen_)
        closeFile();

    // Free AVFrames in double ended queue
    FrameDequeEntry* entries = frameFreeList_.entries();
    for (int i = 0; i != bufSize_; ++i)
    {
        av_frame_free(&entries[i].frame);
    }
}

// ----------------------------------------------------------------------------
QImage VideoDecoder::currentFrameAsImage()
{
    PROFILE(VideoDecoder, currentFrameAsImage);

    if (currentFrame_ == nullptr)
        return QImage();

    AVFrame* frame = currentFrame_->frame;
    log_->debug("pts: %d", frame->pts);

    videoScaleCtx_ = sws_getCachedContext(videoScaleCtx_,
        sourceWidth_, sourceHeight_, static_cast<AVPixelFormat>(frame->format),
        sourceWidth_, sourceHeight_, AV_PIX_FMT_RGB24,
        SWS_BILINEAR, nullptr, nullptr, nullptr);

    sws_scale(videoScaleCtx_,
        frame->data, frame->linesize, 0, sourceHeight_,
        rgbFrame_->data, rgbFrame_->linesize);

    return QImage(rgbFrame_->data[0], sourceWidth_, sourceHeight_, rgbFrame_->linesize[0], QImage::Format_RGB888);
}

static int read_callback(void* opaque, uint8_t* buf, int buf_size)
{
    PROFILE(VideoDecoderGlobal, read_callback);

    auto ioChunkReader = reinterpret_cast<rfcommon::Deserializer*>(opaque);
    return ioChunkReader->read(buf, buf_size);
}

static int64_t seek_callback(void* opaque, int64_t offset, int whence)
{
    PROFILE(VideoDecoderGlobal, seek_callback);

    auto ioChunkReader = reinterpret_cast<rfcommon::Deserializer*>(opaque);
    switch (whence)
    {
        case SEEK_SET: ioChunkReader->seekSet(offset); return ioChunkReader->bytesRead();
        case SEEK_CUR: ioChunkReader->seekCur(offset); return ioChunkReader->bytesRead();
        case SEEK_END: ioChunkReader->seekEnd(offset); return ioChunkReader->bytesRead();
        case AVSEEK_SIZE: return ioChunkReader->bytesTotal();
    }

    return AVERROR(EIO); // unexpected seek request, treat it as error
}

// ----------------------------------------------------------------------------
bool VideoDecoder::openFile(const void* address, uint64_t size)
{
    PROFILE(VideoDecoder, openFile);

    log_->info("Opening file from memory, address: 0x%" PRIXPTR ", size: 0x%" PRIX64, address, size);

    int result;
    const AVCodec* videoCodec;
    const AVCodec* audioCodec;
    rfcommon::Deserializer* ioChunkReader;
    unsigned char* ioBuffer;
    int bufSize;

    ioChunkReader = new rfcommon::Deserializer(address, size);

    ioBuffer = static_cast<unsigned char*>(av_malloc(8192));
    ioCtx_ = avio_alloc_context(
        ioBuffer,
        8192,
        0,     // 0 for reading, 1 for writing. We're only reading.
        reinterpret_cast<void*>(ioChunkReader),  // opaque pointer, will be passed to read callback
        &read_callback,
        NULL,  // write callback
        &seek_callback   // seek callback
    );

    // AVFormatContext holds the header information from the format (Container)
    // Allocating memory for this component
    // http://ffmpeg.org/doxygen/trunk/structAVFormatContext.html
    inputCtx_ = avformat_alloc_context();
    if (inputCtx_ == nullptr)
    {
        log_->error("Failed to allocate avformat context");
        goto alloc_context_failed;
    }
    inputCtx_->pb = ioCtx_;

    // tell our input context that we're using custom i/o and there's no backing file
    inputCtx_->flags |= AVFMT_FLAG_CUSTOM_IO | AVFMT_NOFILE;

    // Open the file and read its header. The codecs are not opened.
    // The function arguments are:
    // AVFormatContext (the component we allocated memory for),
    // url (filename), or some non-empty placeholder for when we use a custom IO
    // AVInputFormat (if you pass NULL it'll do the auto detect)
    // and AVDictionary (which are options to the demuxer)
    // http://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#ga31d601155e9035d5b0e7efedc894ee49
    if ((result = avformat_open_input(&inputCtx_, "ReFramed Video", NULL, NULL)) != 0)
    {
        char buf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(result, buf, AV_ERROR_MAX_STRING_SIZE);
        log_->error(buf);
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
    if (avformat_find_stream_info(inputCtx_, NULL) < 0)
    {
        log_->error("Failed to find stream info");
        goto find_stream_info_failed;
    }

    videoCodec = nullptr;
    audioCodec = nullptr;
    for (int i = 0; i < inputCtx_->nb_streams; ++i)
    {
        AVStream* stream = inputCtx_->streams[i];
        AVCodecParameters* codecParams = stream->codecpar;
        const AVCodec* codec = avcodec_find_decoder(codecParams->codec_id);
        if (codec == nullptr)
        {
            log_->error("Unsupported codec");
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
        log_->error("Input does not contain a video stream");
        goto video_stream_not_found;
    }
    if (audioStreamIdx_ == -1)
        log_->info("Input does not contain an audio stream");

    // https://ffmpeg.org/doxygen/trunk/structAVCodecContext.html
    videoCodecCtx_ = avcodec_alloc_context3(videoCodec);
    if (videoCodecCtx_ == nullptr)
    {
        log_->error("Failed to allocate video codec context");
        goto alloc_video_codec_context_failed;
    }

    // Fill the codec context based on the values from the supplied codec parameters
    // https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#gac7b282f51540ca7a99416a3ba6ee0d16
    if (avcodec_parameters_to_context(videoCodecCtx_, inputCtx_->streams[videoStreamIdx_]->codecpar) < 0)
    {
        log_->error("Failed to copy video codec params to video codec context");
        goto copy_video_codec_params_failed;
    }

    // Initialize the AVCodecContext to use the given AVCodec.
    // https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#ga11f785a188d7d9df71621001465b0f1d
    if (avcodec_open2(videoCodecCtx_, videoCodec, NULL) < 0)
    {
        log_->error("Failed to open video codec");
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

    log_->info("Video stream initialized. Decoding first frame...");

    // Decode the very first frame before starting the decoder thread,
    // so the calling code has a valid frame to show
    FrameDequeEntry* entry = frameFreeList_.take();
    if (decodeNextFrame(entry))
    {
        currentFrame_ = entry;
        currentGameFrame_ = 0;
    }
    else
    {
        frameFreeList_.put(entry);
        closeFile();
        return false;
    }

    return true;

    open_video_codec_failed          :
    copy_video_codec_params_failed   : avcodec_free_context(&videoCodecCtx_);
    alloc_video_codec_context_failed :
    video_stream_not_found           :
    find_stream_info_failed          : avformat_close_input(&inputCtx_);
    open_input_failed                : avformat_free_context(inputCtx_);
    alloc_context_failed             : return false;
}

// ----------------------------------------------------------------------------
void VideoDecoder::closeFile()
{
    PROFILE(VideoDecoder, closeFile);

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
    avformat_close_input(&inputCtx_);
    avformat_free_context(inputCtx_);
    delete reinterpret_cast<rfcommon::Deserializer*>(ioCtx_->opaque);
    av_free(ioCtx_->buffer);
    avio_context_free(&ioCtx_);
}

// ----------------------------------------------------------------------------
bool VideoDecoder::nextVideoFrame()
{
    PROFILE(VideoDecoder, nextFrame);

    QMutexLocker guard(&mutex_);

    if (frontQueue_.count() == 0)
        return false;

    if (currentFrame_)
        backQueue_.putFront(currentFrame_);
    currentFrame_ = frontQueue_.takeBack();
    currentFrameIdx_++;

    cond_.wakeOne();

    return true;
}

// ----------------------------------------------------------------------------
bool VideoDecoder::prevVideoFrame()
{
    PROFILE(VideoDecoder, prevFrame);

    QMutexLocker guard(&mutex_);

    if (backQueue_.count() == 0)
        return false;

    if (currentFrame_)
        frontQueue_.putBack(currentFrame_);
    currentFrame_ = backQueue_.takeFront();
    currentFrameIdx_--;

    cond_.wakeOne();

    return true;
}

// ----------------------------------------------------------------------------
void VideoDecoder::seekToGameFrame(rfcommon::FrameIndex frame)
{
    PROFILE(VideoDecoder, seekToMs);

    QMutexLocker guard(&mutex_);

    // Convert game frame index into video frame index
    /*
    AVRational game_time_base = av_make_q(1, 60);  // Game runs at 60 fps
    AVRational r_frame_rate = inputCtx_->streams[videoStreamIdx_]->r_frame_rate;
    AVRational video_frame_time_base = av_make_q(r_frame_rate.den, r_frame_rate.num);
    currentFrameIdx_ = av_rescale_q(frame.index(), game_time_base, video_frame_time_base);*/
    currentGameFrame_ = frame.index();

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
    }
}

// ----------------------------------------------------------------------------
bool VideoDecoder::decodeNextFrame(FrameDequeEntry* entry)
{
    PROFILE(VideoDecoder, decodeNextFrame);

    AVPacket packet;

    // Returns <0 if an error occurred, or if end of file was reached
    int response;
    while (true)
    {
        response = av_read_frame(inputCtx_, &packet);
        if (response < 0) 
        {
            if ((response == AVERROR_EOF || avio_feof(inputCtx_->pb)))
                break;
            if (inputCtx_->pb && inputCtx_->pb->error)
                break;
            continue;
        }

        if (packet.stream_index == videoStreamIdx_)
        {
            // Send packet to video decoder
            response = avcodec_send_packet(videoCodecCtx_, &packet);
            if (response < 0)
            {
                if ((response == AVERROR_EOF || avio_feof(inputCtx_->pb)))
                    goto exit;
                if (inputCtx_->pb && inputCtx_->pb->error)
                    goto exit;
                goto need_next_packet;
            }

            response = avcodec_receive_frame(videoCodecCtx_, entry->frame);
            if (response == AVERROR(EAGAIN))
                goto need_next_packet;
            else if (response == AVERROR_EOF)
                goto exit;
            else if (response < 0)
                log_->error("Decoder error");

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
    PROFILE(VideoDecoder, run);

    mutex_.lock();
    while (requestShutdown_ == false)
    {
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

        // AVSEEK_FLAG_BACKWARD should seek to the first keyframe that occurs
        // before the specific time_stamp, however, people online have said that
        // sometimes it will seek to a keyframe after the timestamp specified,
        // if the timestamp is one frame before the keyframe. To fix this, we
        // just subtract 1 more frame
        // https://stackoverflow.com/questions/20734814/ffmpeg-av-seek-frame-with-avseek-flag-any-causes-grey-screen

        // If the current frame is NULL, this indicates that we may have to
        // seek to the current frame index.
        if (currentFrame_ == nullptr)
        {
            int64_t seek_target = currentFrameIdx_;
            FrameDequeEntry* entry = frameFreeList_.take();
            assert(entry);
            mutex_.unlock();
                // Convert from frame index to timestamp in the stream's time base
                AVRational r_frame_rate = inputCtx_->streams[videoStreamIdx_]->r_frame_rate;
                AVRational video_frame_time_base = av_make_q(r_frame_rate.den, r_frame_rate.num);
                AVRational stream_time_base = inputCtx_->streams[videoStreamIdx_]->time_base;
                seek_target = seek_target > 0 ? seek_target - 1 : seek_target;  // See comment above
                seek_target = av_rescale_q(seek_target, video_frame_time_base, stream_time_base);

                // Seek and decode frame
                int seek_result = av_seek_frame(inputCtx_, videoStreamIdx_, seek_target, AVSEEK_FLAG_BACKWARD);
                bool decode_result = decodeNextFrame(entry);
            mutex_.lock();

            if (decode_result)
                currentFrame_ = entry;
            else
            {
                frameFreeList_.put(entry);
                continue;
            }
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

            continue;
        }

        if (backQueue_.count() < expectedBackQueueCount)
        {
            int64_t backQueueStartFrame = currentFrameIdx_ - bufSize_ / 2;
            FrameDequeEntry* entry = frameFreeList_.take();
            assert(entry);
            mutex_.unlock();
                // Convert from frame index to timestamp in the stream's time base
                AVRational r_frame_rate = inputCtx_->streams[videoStreamIdx_]->r_frame_rate;
                AVRational video_frame_time_base = av_make_q(r_frame_rate.den, r_frame_rate.num);
                AVRational stream_time_base = inputCtx_->streams[videoStreamIdx_]->time_base;
                int64_t seek_target = backQueueStartFrame > 0 ? backQueueStartFrame - 1 : backQueueStartFrame;
                seek_target = seek_target < 0 ? 0 : seek_target;
                seek_target = av_rescale_q(seek_target, video_frame_time_base, stream_time_base);

                // Seek and decode frame
                int seek_result = av_seek_frame(inputCtx_, videoStreamIdx_, seek_target, AVSEEK_FLAG_BACKWARD);
                bool decode_result = decodeNextFrame(entry);
            mutex_.lock();

            continue;
        }

        if (requestShutdown_ == false)
            cond_.wait(&mutex_);
    }
    mutex_.unlock();
}
