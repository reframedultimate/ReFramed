#pragma once

class QImage;

class VideoPlayerListener
{
public:
    virtual void onFileOpened() = 0;
    virtual void onFileClosed() = 0;
    virtual void onPresentImage(const QImage& image) = 0;
};
