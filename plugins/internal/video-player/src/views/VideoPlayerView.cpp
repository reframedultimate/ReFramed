#include "rfcommon/Profiler.hpp"
#include "video-player/views/VideoPlayerView.hpp"
#include "video-player/models/VideoPlayerModel.hpp"
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
    , logWidget_(new QPlainTextEdit)
    , videoSurface_(new VideoSurface)
{
    setLayout(new QVBoxLayout);

    logWidget_->setReadOnly(true);
    logWidget_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    layout()->addWidget(logWidget_);
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
}

// ----------------------------------------------------------------------------
void VideoPlayerView::onFileClosed()
{
}

// ----------------------------------------------------------------------------
void VideoPlayerView::onPresentImage(const QImage& image)
{
    PROFILE(VideoPlayerView, onPresentCurrentFrame);

    videoSurface_->image = image;
    videoSurface_->update();
}
