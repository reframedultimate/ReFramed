#pragma once

#include <QWidget>

struct VideoPlayerData;

class VideoPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit VideoPlayer(QWidget* parent=nullptr);
    ~VideoPlayer();

private:
    VideoPlayerData* d;
};
