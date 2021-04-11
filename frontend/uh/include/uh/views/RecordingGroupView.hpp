#pragma once

#include "uh/listeners/RecordingGroupListener.hpp"
#include <QWidget>

class QListWidgetItem;

namespace Ui {
    class RecordingGroupView;
}

namespace uh {

class RecordingGroup;
class RecordingView;

class RecordingGroupView : public QWidget
                         , public RecordingGroupListener
{
    Q_OBJECT

public:
    explicit RecordingGroupView(QWidget* parent=nullptr);
    ~RecordingGroupView();

public slots:
    void setRecordingGroup(RecordingGroup* group);
    void clear();

private slots:
    void onCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* previous);

private:
    void onRecordingGroupNameChanged(const QString& name) override;
    void onRecordingGroupFileAdded(const QFileInfo& absPathToFile) override;
    void onRecordingGroupFileRemoved(const QFileInfo& absPathToFile) override;

private:
    RecordingGroup* currentGroup_ = nullptr;
    RecordingView* recordingView_;
    Ui::RecordingGroupView* ui_;
};

}
