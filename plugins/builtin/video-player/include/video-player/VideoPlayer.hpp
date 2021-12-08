#pragma once

#include "rfcommon/VisualizerPlugin.hpp"
#include "rfcommon/String.hpp"
#include <QWidget>

extern "C" {
typedef struct AVFormatContext AVFormatContext;
typedef struct AVCodec AVCodec;
typedef struct AVCodecParameters AVCodecParameters;
typedef struct AVStream AVStream;
}

class QPlainTextEdit;

class VideoPlayer : public QWidget
                  , public rfcommon::VisualizerPlugin
{
    Q_OBJECT

public:
    explicit VideoPlayer(RFPluginFactory* factory, QWidget* parent=nullptr);
    ~VideoPlayer();

    bool openFile(const QString& fileName);

    QWidget* createView() override { return this; }
    void destroyView(QWidget* view) { (void)view; }

private:
    void info(const QString& msg);
    void error(const QString& msg);

private:
    QPlainTextEdit* logWidget_;
    AVFormatContext* formatContext_ = nullptr;
};
