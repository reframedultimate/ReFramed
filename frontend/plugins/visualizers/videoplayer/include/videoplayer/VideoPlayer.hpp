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

    QWidget* takeWidget() override { return this; }
    void giveWidget() override { setParent(nullptr); }

private:
    VideoPlayerData* d;
};
