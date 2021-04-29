#pragma once

#include "uh/VisualizerPlugin.hpp"
#include <QWidget>

struct VideoPlayerData;

class VideoPlayer : public QWidget
                  , public uh::VisualizerPlugin
{
    Q_OBJECT

public:
    explicit VideoPlayer(QWidget* parent=nullptr);
    ~VideoPlayer();

    bool openFile(const std::string& fileName);

    QWidget* takeWidget() override { return this; }
    void giveWidget(QWidget* widget) override {}

private:
    VideoPlayerData* d;
};
