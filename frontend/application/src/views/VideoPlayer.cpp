#include "application/views/VideoPlayer.hpp"

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

struct VideoPlayerData
{
    AVFormatContext* formatContext = nullptr;
    AVCodec* codec = nullptr;
};

// ----------------------------------------------------------------------------
VideoPlayer::VideoPlayer(QWidget *parent)
    : QWidget(parent)
    , d(new VideoPlayerData)
{
}

// ----------------------------------------------------------------------------
VideoPlayer::~VideoPlayer()
{
    delete d;
}

// ----------------------------------------------------------------------------
bool VideoPlayer::openFile(const QString& fileName)
{
    // AVFormatContext holds the header information from the format (Container)
    // Allocating memory for this component
    // http://ffmpeg.org/doxygen/trunk/structAVFormatContext.html
    d->formatContext = avformat_alloc_context();
    if (d->formatContext == nullptr)
        goto alloc_context_failed;

    // Open the file and read its header. The codecs are not opened.
    // The function arguments are:
    // AVFormatContext (the component we allocated memory for),
    // url (filename),
    // AVInputFormat (if you pass NULL it'll do the auto detect)
    // and AVDictionary (which are options to the demuxer)
    // http://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#ga31d601155e9035d5b0e7efedc894ee49
    if (avformat_open_input(&d->formatContext, fileName.toStdString().c_str(), NULL, NULL) != 0)
          goto open_input_failed;

    // read Packets from the Format to get stream information
    // this function populates pFormatContext->streams
    // (of size equals to pFormatContext->nb_streams)
    // the arguments are:
    // the AVFormatContext
    // and options contains options for codec corresponding to i-th stream.
    // On return each dictionary will be filled with options that were not found.
    // https://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#gad42172e27cddafb81096939783b157bb
    if (avformat_find_stream_info(d->formatContext, NULL) < 0)
        goto find_stream_info_failed;

    for (int i = 0; i < d->formatContext->nb_streams; ++i)
    {

    }

    return true;

    find_stream_info_failed : avformat_close_input(&d->formatContext);
    open_input_failed       : avformat_free_context(d->formatContext);
    alloc_context_failed    : return false;
}
