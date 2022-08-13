#include "video-player/views/VideoPlayerView.hpp"
#include "video-player/models/VideoDecoder.hpp"
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
VideoPlayerView::VideoPlayerView(VideoPlayerModel* model, QWidget *parent)
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

    connect(&timer_, &QTimer::timeout, this, &VideoPlayerView::drawNextFrame);
    connect(decoder_, &VideoDecoder::info, this, &VideoPlayerView::info);
    connect(decoder_, &VideoDecoder::error, this, &VideoPlayerView::error);

    timer_.setInterval(16);
    timer_.start();
}

// ----------------------------------------------------------------------------
VideoPlayerView::~VideoPlayerView()
{
}

// ----------------------------------------------------------------------------
bool VideoPlayerView::openFile(const QString& fileName)
{
    return decoder_->openFile(fileName);
}

// ----------------------------------------------------------------------------
void VideoPlayerView::drawNextFrame()
{
    videoSurface_->image = decoder_->currentFrameAsImage();
    videoSurface_->update();
    decoder_->nextFrame();
}

// ----------------------------------------------------------------------------
void VideoPlayerView::info(const QString& msg)
{
    logWidget_->textCursor().insertText("[INFO] " + msg + "\n");
    logWidget_->verticalScrollBar()->setValue(logWidget_->verticalScrollBar()->maximum());
}

// ----------------------------------------------------------------------------
void VideoPlayerView::error(const QString& msg)
{
    logWidget_->textCursor().insertText("[ERROR] " + msg + "\n");
    logWidget_->verticalScrollBar()->setValue(logWidget_->verticalScrollBar()->maximum());
}
