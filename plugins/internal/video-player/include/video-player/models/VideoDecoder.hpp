#pragma once

#include "rfcommon/FrameIndex.hpp"

#include <QThread>
#include <QImage>
#include <QMutex>
#include <QWaitCondition>

extern "C" {
typedef struct AVIOContext AVIOContext;
typedef struct AVFormatContext AVFormatContext;
typedef struct AVCodec AVCodec;
typedef struct AVCodecContext AVCodecContext;
typedef struct AVCodecParameters AVCodecParameters;
typedef struct AVStream AVStream;
typedef struct AVPacket AVPacket;
typedef struct AVFrame AVFrame;
typedef struct SwsContext SwsContext;
}

namespace rfcommon {
    class Log;
}

template <typename Entry>
class Queue
{
public:
    void putFront(Entry* entry)
    {
        entry->next = nullptr;

        if (front_)
            front_->next = entry;
        else
            back_ = entry;

        count_++;
        front_ = entry;
    }

    Entry* takeBack()
    {
        Entry* entry = back_;

        if (back_)
        {
            back_ = back_->next;
            count_--;
        }
        if (back_ == nullptr)
            front_ = nullptr;

        return entry;
    }

    Entry* peekFront() const { return front_; }
    Entry* peekback() const { return back_; }
    int count() const { return count_; }

private:
    Entry* front_ = nullptr;
    Entry* back_ = nullptr;
    int count_ = 0;
};

template <typename Entry>
class FreeList
{
public:
    FreeList()
        : first_(nullptr)
    {}

    FreeList(const FreeList&) = delete;
    FreeList(FreeList&&) = delete;
    FreeList& operator=(const FreeList&) = delete;
    FreeList& operator=(FreeList&&) = delete;

    void put(Entry* entry)
    {
        entry->next = first_;
        first_ = entry;
    }

    Entry* take()
    {
        Entry* entry = first_;
        if (first_)
            first_ = entry->next;
        return entry;
    }

private:
    Entry* first_;
};

template <typename Entry, int N>
class FlatFreeList : public FreeList<Entry>
{
public:
    FlatFreeList()
    {
        for (int i = N - 1; i >= 0; --i)
            put(&entries_[i]);
    }

    Entry* entries() const { return entries_; }
    int count() const { return N; }

private:
    Entry entries_[N];
};

struct FrameEntry
{
    FrameEntry* next;
    AVFrame* frame;
};

class AVDecoder
{
public:
    AVDecoder(rfcommon::Log* log);
    ~AVDecoder();

    bool openFile(const void* address, uint64_t size);
    void closeFile();

    bool isOpen() const { return isOpen_; }

    /*!
     * \brief Decodes the next frame of video and converts it into RGB24.
     * \param[out] bufPtr The pointer to the internally allocated buffer containing
     * the image data is written to this output variable. The buffer remains
     * valid up until the next call to nextVideoFrame().
     * \param[out] width The width in pixels of the image
     * \param[out] height The height in pixels of the image
     * \param[out] bytesPerLine Number of bytes for each picture line.
     * \return The video timestamp (PTS) of the decoded frame is returned. If
     * PTS is not available, 0 is returned (some codecs do this). If an error
     * in decoding occurred, -1 is returned.
     */
    int64_t nextVideoFrame(void** bufPtr, int* width, int* height, int* bytesPerLine);

    /*!
     * \brief Decodes the next chunk of audio.
     * \param buf
     * \param size
     * \return
     */
    int64_t nextAudioFrame(void** bufPtr, int* size);

    bool seekToGameFrame(rfcommon::FrameIndex frame);

private:
    bool decodeNextPacket();

private:
    rfcommon::Log* log_;

    // libav state
    int sourceWidth_ = 0;
    int sourceHeight_ = 0;
    int videoStreamIdx_ = -1;
    int audioStreamIdx_ = -1;
    AVIOContext* ioCtx_ = nullptr;
    AVFormatContext* inputCtx_ = nullptr;
    AVCodecContext* videoCodecCtx_ = nullptr;
    SwsContext* videoScaleCtx_ = nullptr;

    // queues
    FreeList<FrameEntry> frameFreeList_;
    FlatFreeList<FrameEntry, 64> pictureFreeList_;
    Queue<FrameEntry> audioQueue_;
    Queue<FrameEntry> videoQueue_;

    bool isOpen_ = false;
};
