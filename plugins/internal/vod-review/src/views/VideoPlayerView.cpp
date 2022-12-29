#include "rfcommon/Profiler.hpp"
#include "vod-review/views/VideoPlayerView.hpp"
#include "vod-review/models/VideoPlayerModel.hpp"
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QPainter>
#include <QResizeEvent>
#include <QScrollBar>
#include <QOpenGLWidget>
#include <QShortcut>

extern "C" {
#include <libavformat/avformat.h>
}

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
    , model_(model)
    , videoSurface_(new VideoSurface)
{
    setLayout(new QVBoxLayout);
    layout()->addWidget(videoSurface_);

    model_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
VideoPlayerView::~VideoPlayerView()
{
    model_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void VideoPlayerView::onFileOpened()
{
    PROFILE(VideoPlayerView, onFileOpened);

}

// ----------------------------------------------------------------------------
void VideoPlayerView::onFileClosed()
{
    PROFILE(VideoPlayerView, onFileClosed);

}

// ----------------------------------------------------------------------------
void VideoPlayerView::onPlayerPaused()
{
    PROFILE(VideoPlayerView, onPlayerPaused);

}

// ----------------------------------------------------------------------------
void VideoPlayerView::onPlayerResumed()
{
    PROFILE(VideoPlayerView, onPlayerResumed);

}

// ----------------------------------------------------------------------------
void VideoPlayerView::onPresentImage(const QImage& image)
{
    PROFILE(VideoPlayerView, onPresentImage);

    videoSurface_->image = image;
    videoSurface_->update();
}
