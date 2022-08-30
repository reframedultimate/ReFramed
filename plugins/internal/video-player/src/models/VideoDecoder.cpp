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

// ----------------------------------------------------------------------------
AVDecoder::AVDecoder(rfcommon::Log* log)
    : log_(log)
{
}

// ----------------------------------------------------------------------------
AVDecoder::~AVDecoder()
{
    if (isOpen_)
        closeFile();
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
bool AVDecoder::openFile(const void* address, uint64_t size)
{
    PROFILE(VideoDecoder, openFile);

    log_->info("Opening file from memory, address: 0x%" PRIxPTR ", size: 0x%" PRIx64, (intptr_t)address, size);

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
        log_->error("%s", buf);
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

    currentPacket_ = av_packet_alloc();

    sourceWidth_ = videoCodecCtx_->width;
    sourceHeight_ = videoCodecCtx_->height;

    log_->info("Video stream initialized");

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
void AVDecoder::closeFile()
{
    PROFILE(VideoDecoder, closeFile);

    // Free AVFrames left in queues
    while (videoQueue_.count())
    {
        FrameEntry* e = videoQueue_.takeBack();
        av_free(e->frame->data[0]);  // This is the picture buffer we manually allocated
        av_frame_free(&e->frame);
        freeFrameEntries_.put(e);
    }
    while (audioQueue_.count())
    {
        FrameEntry* e = audioQueue_.takeBack();
        av_frame_free(&e->frame);
        freeFrameEntries_.put(e);
    }

    if (videoScaleCtx_)
    {
        sws_freeContext(videoScaleCtx_);
        videoScaleCtx_ = nullptr;
    }

    av_packet_free(&currentPacket_);
    avcodec_close(videoCodecCtx_);
    avcodec_free_context(&videoCodecCtx_);
    avformat_close_input(&inputCtx_);
    avformat_free_context(inputCtx_);
    delete reinterpret_cast<rfcommon::Deserializer*>(ioCtx_->opaque);
    av_free(ioCtx_->buffer);
    avio_context_free(&ioCtx_);
}

// ----------------------------------------------------------------------------
AVFrame* AVDecoder::takeNextVideoFrame()
{
    while (videoQueue_.count() == 0)
    {
        if (decodeNextPacket() == false)
            return nullptr;
    }

    FrameEntry* e = videoQueue_.takeBack();
    AVFrame* frame = e->frame;
    freeFrameEntries_.put(e);
    return frame;
}

// ----------------------------------------------------------------------------
void AVDecoder::giveVideoFrame(AVFrame* frame)
{
    FrameEntry* e = freeFrameEntries_.take();
    if (e == nullptr)
    {
        // Should never happen but just in case if it does, clean up
        log_->error("Used up all free frame entries!");
        av_free(frame->data[0]);
        av_frame_free(&frame);
    }
    else
    {
        e->frame = frame;
        picturePool_.put(e);
    }
}

// ----------------------------------------------------------------------------
AVFrame* AVDecoder::takeNextAudioFrame()
{
    while (audioQueue_.count() == 0)
    {
        if (decodeNextPacket() == false)
            return nullptr;
    }

    FrameEntry* e = audioQueue_.takeBack();
    AVFrame* frame = e->frame;
    freeFrameEntries_.put(e);
    return frame;
}

// ----------------------------------------------------------------------------
void AVDecoder::giveAudioFrame(AVFrame* frame)
{
    FrameEntry* e = freeFrameEntries_.take();
    if (e == nullptr)
    {
        // Should never happen but just in case if it does, clean up
        log_->error("Used up all free frame entries!");
        av_frame_free(&frame);
    }
    else
    {
        e->frame = frame;
        framePool_.put(e);
    }
}

// ----------------------------------------------------------------------------
bool AVDecoder::seekToTimeStamp(int64_t target_ts)
{
    PROFILE(VideoDecoder, seekToMs);

    // Clear all buffers before seeking
    while (videoQueue_.count())
        picturePool_.put(videoQueue_.takeBack());
    while (audioQueue_.count())
        framePool_.put(audioQueue_.takeBack());

    // AVSEEK_FLAG_BACKWARD should seek to the first keyframe that occurs
    // before the specific time_stamp, however, people online have said that
    // sometimes it will seek to a keyframe after the timestamp specified,
    // if the timestamp is one frame before the keyframe. To fix this, we
    // just subtract 1 more frame
    // https://stackoverflow.com/questions/20734814/ffmpeg-av-seek-frame-with-avseek-flag-any-causes-grey-screen

    // Seek and decode frame
    log_->info("Seeking to %ld", target_ts);
    int seek_result = av_seek_frame(inputCtx_, videoStreamIdx_, target_ts, AVSEEK_FLAG_BACKWARD);
    if (seek_result < 0)
    {
        char buf[AV_ERROR_MAX_STRING_SIZE];
        log_->error("av_seek_frame failed: %s", av_make_error_string(buf, AV_ERROR_MAX_STRING_SIZE, seek_result));
        return false;
    }

    return true;
}

// ----------------------------------------------------------------------------
bool AVDecoder::seekToTimeStamp(int64_t target_ts, int num, int den)
{
    AVRational from = av_make_q(num, den);
    AVRational to = inputCtx_->streams[videoStreamIdx_]->time_base;
    return seekToTimeStamp(av_rescale_q(target_ts, from, to));
}

// ----------------------------------------------------------------------------
bool AVDecoder::decodeNextPacket()
{
    PROFILE(VideoDecoder, decodeNextFrame);

    int response;
    while (true)
    {
        response = av_read_frame(inputCtx_, currentPacket_);
        if (response < 0)
        {
            if ((response == AVERROR_EOF || avio_feof(inputCtx_->pb)))
                return false;
            if (inputCtx_->pb && inputCtx_->pb->error)
                return false;
            continue;
        }

        if (currentPacket_->stream_index == videoStreamIdx_)
        {
            // Send packet to video decoder
            response = avcodec_send_packet(videoCodecCtx_, currentPacket_);
            if (response < 0)
            {
                if ((response == AVERROR_EOF || avio_feof(inputCtx_->pb)))
                    goto exit_failure;
                if (inputCtx_->pb && inputCtx_->pb->error)
                    goto exit_failure;
                goto need_next_packet;
            }

            FrameEntry* frameEntry = framePool_.take();
            if (frameEntry == nullptr)
            {
                frameEntry = freeFrameEntries_.take();
                if (frameEntry == nullptr)
                {
                    log_->error("Used up all free frame entries!");
                    goto exit_failure;
                }
                frameEntry->frame = av_frame_alloc();
            }
            AVFrame* frame = frameEntry->frame;

            response = avcodec_receive_frame(videoCodecCtx_, frame);
            if (response == AVERROR(EAGAIN))
                goto need_next_packet;
            else if (response == AVERROR_EOF)
                goto exit_failure;
            else if (response < 0)
            {
                log_->error("Decoder error");
                goto need_next_packet;
            }

            // Frame is successfully decoded here

            // Get a picture we can re-use from the freelist
            FrameEntry* picEntry = picturePool_.take();
            if (picEntry == nullptr)
            {
                // Have to allocate a new entry
                picEntry = freeFrameEntries_.take();
                if (picEntry == nullptr)
                {
                    log_->error("Used up all free frame entries!");
                    goto exit_failure;
                }
                picEntry->frame = av_frame_alloc();

                // Allocates the raw image buffer and fills in the frame's data
                // pointers and line sizes.
                //
                // NOTE: The image buffer has to be manually freed with
                //       av_free(qEntry->frame.data[0]), as av_frame_unref()
                //       does not take care of this.
                av_image_alloc(
                    picEntry->frame->data,      /* Data pointers to be filled in */
                    picEntry->frame->linesize,  /* linesizes for the image in dst_data to be filled in */
                    sourceWidth_,
                    sourceHeight_,
                    AV_PIX_FMT_RGB24,
                    1                           /* Alignment */
                );
            }

            // Convert frame to RGB24 format
            videoScaleCtx_ = sws_getCachedContext(videoScaleCtx_,
                sourceWidth_, sourceHeight_, static_cast<AVPixelFormat>(frame->format),
                sourceWidth_, sourceHeight_, AV_PIX_FMT_RGB24,
                SWS_BILINEAR, nullptr, nullptr, nullptr);

            sws_scale(videoScaleCtx_,
                frame->data, frame->linesize, 0, sourceHeight_,
                picEntry->frame->data, picEntry->frame->linesize);

            videoQueue_.putFront(picEntry);
            av_frame_unref(frame);
            framePool_.put(frameEntry);

            goto exit_success;
        }

        if (currentPacket_->stream_index == audioStreamIdx_)
        {

        }

        need_next_packet : av_packet_unref(currentPacket_);
        continue;

        exit_failure : av_packet_unref(currentPacket_);
        return false;

        exit_success: av_packet_unref(currentPacket_);
        break;
    }

    return true;
}

/*
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

        out:
        if (requestShutdown_ == false)
            cond_.wait(&mutex_);
    }
    mutex_.unlock();
}*/
