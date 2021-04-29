#include "video-player/VideoPlayer.hpp"
#include <QPlainTextEdit>
#include <QVBoxLayout>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

// ----------------------------------------------------------------------------
VideoPlayer::VideoPlayer(QWidget *parent)
    : QWidget(parent)
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

    for (int i = 0; i < formatContext_->nb_streams; ++i)
    {
        AVStream* stream = formatContext_->streams[i];
        AVCodecParameters* codecParams = stream->codecpar;
        info(QString("time_base %1/%2").arg(stream->time_base.num).arg(stream->time_base.den));
        info(QString("r_frame_rate %1/%2").arg(stream->r_frame_rate.num).arg(stream->r_frame_rate.den));
        info(QString("start_time %1").arg(stream->start_time));
        info(QString("duration %1").arg(stream->duration));

        AVCodec* codec = avcodec_find_decoder(codecParams->codec_id);
        if (codec == nullptr)
        {
            error("Unsupported codec");
            continue;
        }
    }

    avformat_close_input(&formatContext_);
    avformat_free_context(formatContext_);

    return true;

    find_stream_info_failed : avformat_close_input(&formatContext_);
    open_input_failed       : avformat_free_context(formatContext_);
    alloc_context_failed    : return false;
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
