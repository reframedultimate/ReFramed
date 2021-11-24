#pragma once

#include "uh/VisualizerPlugin.hpp"
#include "uh/String.hpp"
#include <QWidget>

extern "C" {
typedef struct AVFormatContext AVFormatContext;
typedef struct AVCodec AVCodec;
typedef struct AVCodecParameters AVCodecParameters;
typedef struct AVStream AVStream;
}

class QPlainTextEdit;

class VideoPlayer : public QWidget
                  , public uh::VisualizerPlugin
{
    Q_OBJECT

public:
    explicit VideoPlayer(QWidget* parent=nullptr);
    ~VideoPlayer();

    bool openFile(const QString& fileName);

    QWidget* createView() override { return this; }
    void destroyView(QWidget* widget) override {}

private:
    void info(const QString& msg);
    void error(const QString& msg);

private:
    QPlainTextEdit* logWidget_;
    AVFormatContext* formatContext_ = nullptr;
};
