#pragma once

#include "rfcommon/RealtimePlugin.hpp"
#include "rfcommon/String.hpp"
#include <QWidget>
#include <QTimer>

class QPlainTextEdit;
class VideoSurface;
class VideoDecoder;

class VideoPlayer
    : public QWidget
    , public rfcommon::RealtimePlugin
{
    Q_OBJECT

public:
    explicit VideoPlayer(QWidget* parent=nullptr);
    ~VideoPlayer();

    bool openFile(const QString& fileName);

    QWidget* takeWidget() override { return this; }
    void giveWidget(QWidget* widget) override {}

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

