#pragma once

#include <QWidget>

namespace Ui {
    class RecordingView;
}

namespace uh {

class RecordingView : public QWidget
{
    Q_OBJECT
public:
    explicit RecordingView(QWidget* parent=nullptr);
    ~RecordingView();

private:
    Ui::RecordingView* ui_;
};

}
