#include "video-player/VideoPlayer.hpp"
#include "video-player/VideoDecoder.hpp"
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QPainter>
#include <QResizeEvent>
#include <QScrollBar>
#include <QOpenGLWidget>

class VideoSurface : public QOpenGLWidget
{
public:
    explicit VideoSurface(QWidget* parent=nullptr)
        : QOpenGLWidget(parent)
    {
        setMinimumSize(QSize(320, 200));
        setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    }

    ~VideoSurface()
    {}

    void paintEvent(QPaintEvent* event) override
    {
        QPainter painter(this);
        painter.fillRect(rect(), Qt::black);

        if (image.isNull())
            return;

        int width = size().width();
        int height = size().height();
        if (width < image.width() * height / image.height())
            height = image.height() * width / image.width();
        else
            width = image.width() * height / image.height();

        QRect scaledRect(
            (size().width() - width) / 2,
            (size().height() - height) / 2,
            width,
            height
        );

        painter.drawImage(scaledRect, image);
    }

    QImage image;
};

// ----------------------------------------------------------------------------
VideoPlayer::VideoPlayer(QWidget *parent)
    : QWidget(parent)
    , logWidget_(new QPlainTextEdit)
    , videoSurface_(new VideoSurface)
    , decoder_(new VideoDecoder(this))
{
    setLayout(new QVBoxLayout);

    logWidget_->setReadOnly(true);
    logWidget_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    layout()->addWidget(logWidget_);
    layout()->addWidget(videoSurface_);

    //const char* videoFile = "/media/ssbu/Weekly 2021-04-05/2021-04-05_18-42-51.mkv";
    //const char* videoFile = "/media/ssbu/Weekly 2021-04-09/Weekly 2021-04-09.mp4";
    const char* videoFile = "/media/ssbu/Weekly 2021-04-18/Weekly 2021-04-18 - Bo5 - TheComet (Pika) vs NullSpace (Wolf) Game 1.mp4";

    connect(&timer_, &QTimer::timeout, this, &VideoPlayer::drawNextFrame);
    connect(decoder_, &VideoDecoder::info, this, &VideoPlayer::info);
    connect(decoder_, &VideoDecoder::error, this, &VideoPlayer::error);

    openFile(videoFile);
    timer_.setInterval(16);
    timer_.start();
}

// ----------------------------------------------------------------------------
VideoPlayer::~VideoPlayer()
{
}

// ----------------------------------------------------------------------------
bool VideoPlayer::openFile(const QString& fileName)
{
    return decoder_->openFile(fileName);
}

// ----------------------------------------------------------------------------
void VideoPlayer::drawNextFrame()
{
    videoSurface_->image = decoder_->currentFrameAsImage();
    videoSurface_->update();
    decoder_->nextFrame();
}

// ----------------------------------------------------------------------------
void VideoPlayer::info(const QString& msg)
{
    logWidget_->textCursor().insertText("[INFO] " + msg + "\n");
    logWidget_->verticalScrollBar()->setValue(logWidget_->verticalScrollBar()->maximum());
}

// ----------------------------------------------------------------------------
void VideoPlayer::error(const QString& msg)
{
    logWidget_->textCursor().insertText("[ERROR] " + msg + "\n");
    logWidget_->verticalScrollBar()->setValue(logWidget_->verticalScrollBar()->maximum());
}
