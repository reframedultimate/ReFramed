#pragma once

#include <QString>

class VideoPlayerListener
{
public:
    virtual void onPresentCurrentFrame() = 0;
    virtual void onInfo(const QString& msg) = 0;
    virtual void onError(const QString& msg) = 0;
};
