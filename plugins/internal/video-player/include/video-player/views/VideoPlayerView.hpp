#pragma once

#include "rfcommon/String.hpp"
#include <QWidget>
#include <QTimer>

class QPlainTextEdit;
class VideoSurface;
class VideoDecoder;

class VideoPlayerView : public QWidget
{
    Q_OBJECT

public:
    explicit VideoPlayerView(QWidget* parent=nullptr);
    ~VideoPlayerView();

    bool openFile(const QString& fileName);

private slots:
    void drawNextFrame();
    void info(const QString& msg);
    void error(const QString& msg);

private:
    QTimer timer_;
    QPlainTextEdit* logWidget_;
    VideoSurface* videoSurface_;
    VideoDecoder* decoder_;
};
