#pragma once

#include <QWidget>

namespace Ui {
    class RecordingGroupView;
}

namespace uh {

class RecordingGroup;

class RecordingGroupView : public QWidget
{
    Q_OBJECT

public:
    explicit RecordingGroupView(QWidget* parent=nullptr);
    ~RecordingGroupView();

public slots:
    void setRecordingGroup(RecordingGroup* group);

private:
    Ui::RecordingGroupView* ui_;
};

}
