#pragma once

#include <QThread>
#include <QImage>
#include <QMutex>
#include <QWaitCondition>

extern "C" {
typedef struct AVFormatContext AVFormatContext;
typedef struct AVCodec AVCodec;
typedef struct AVCodecContext AVCodecContext;
typedef struct AVCodecParameters AVCodecParameters;
typedef struct AVStream AVStream;
typedef struct AVPacket AVPacket;
typedef struct AVFrame AVFrame;
typedef struct SwsContext SwsContext;
}

struct FrameDequeEntry;

template <typename Entry>
class Deque
{
public:
    /*!
     * \brief Inserts an item into the back of the queue.
     */
    void putBack(Entry* entry)
    {
        entry->prev = nullptr;
        entry->next = back_;

        if (back_)
            back_->prev = entry;
        else
            front_ = entry;

        count_++;
        back_ = entry;
    }

    void putFront(Entry* entry)
    {
        entry->prev = front_;
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
            back_ = back_->next;
        if (back_ == nullptr)
            front_ = nullptr;

        count_--;
        return entry;
    }

    /*!
     * \brief Pops an item from the front of the queue.
     */
    Entry* takeFront()
    {
        Entry* entry = front_;

        if (front_)
            front_ = front_->prev;
        if (front_ == nullptr)
            back_ = nullptr;

        count_--;
        return entry;
    }

    Entry* peekFront() const
    {
        return front_;
    }

    Entry* peekback() const
    {
        return back_;
    }

    int count() const
    {
        return count_;
    }

private:
    Entry* front_ = nullptr;
    Entry* back_ = nullptr;
    int count_ = 0;
};

template <typename Entry>
class FlatFreeList
{
public:
    FlatFreeList(int size)
        : first_(nullptr)
        , entries_(nullptr)
    {
        resize(size);
    }

    ~FlatFreeList()
    {
        if (entries_)
            delete[] entries_;
    }

    FlatFreeList(const FlatFreeList&) = delete;
    FlatFreeList(FlatFreeList&&) = delete;
    FlatFreeList& operator=(const FlatFreeList&) = delete;
    FlatFreeList& operator=(FlatFreeList&&) = delete;

    void resize(int size)
    {
        assert(size > 0);

        if (entries_)
            delete[] entries_;

        entries_ = new Entry[size];

        first_ = nullptr;
        for (int i = size - 1; i >= 0; --i)
            put(&entries_[i]);
    }

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

    Entry* entries() const
    {
        return entries_;
    }

private:
    Entry* first_;
    Entry* entries_;
};

class VideoDecoder : public QThread
{
    Q_OBJECT

public:
    explicit VideoDecoder(QObject* parent=nullptr);
    ~VideoDecoder();

    QImage currentFrameAsImage();

public slots:
    bool openFile(const QString& fileName);
    bool closeFile();

    void nextFrame();
    void prevFrame();
    void seekToMs(uint64_t offsetFromStart);

signals:
    void info(const QString& msg);
    void error(const QString& msg);

private:
    bool decodeNextFrame(FrameDequeEntry* entry);
    void run() override;

private:
    int sourceWidth_ = 0;
    int sourceHeight_ = 0;
    int videoStreamIdx_ = -1;
    int audioStreamIdx_ = -1;
    AVFormatContext* ctx_ = nullptr;
    AVCodecContext* videoCodecCtx_ = nullptr;
    SwsContext* videoScaleCtx_ = nullptr;
    AVFrame* rgbFrame_ = nullptr;
    uint8_t* rgbFrameBuffer_ = nullptr;

    QMutex mutex_;
    QWaitCondition cond_;

    int currentFrameIndex_;
    int bufSize_;
    FlatFreeList<FrameDequeEntry> frameFreeList_;
    Deque<FrameDequeEntry> backQueue_;
    Deque<FrameDequeEntry> frontQueue_;
    FrameDequeEntry* currentFrame_;

    bool requestShutdown_ = false;
};
