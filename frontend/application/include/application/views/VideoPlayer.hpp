#pragma once

#include <QWidget>

namespace uhapp {

class VideoPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit VideoPlayer(QWidget* parent=nullptr);
    ~VideoPlayer();

private:
};

}
