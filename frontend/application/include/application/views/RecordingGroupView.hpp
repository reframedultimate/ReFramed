#pragma once

#include "application/listeners/RecordingGroupListener.hpp"
#include <QWidget>

class QListWidgetItem;
class QStringListModel;

namespace Ui {
    class RecordingGroupView;
}

namespace uhapp {

class RecordingGroup;
class RecordingView;
class RecordingNameCompleter;

class RecordingGroupView : public QWidget
                         , public RecordingGroupListener
{
    Q_OBJECT

public:
    explicit RecordingGroupView(QWidget* parent=nullptr);
    ~RecordingGroupView();

public slots:
    void setRecordingGroupWeakRef(RecordingGroup* group);
    void recordingGroupExpired();

private slots:
    void onCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* previous);
    void onFiltersTextChanged(const QString& text);

private:
    void onRecordingGroupNameChanged(const QString& name) override;
    void onRecordingGroupFileAdded(const QFileInfo& absPathToFile) override;
    void onRecordingGroupFileRemoved(const QFileInfo& absPathToFile) override;

private:
    Ui::RecordingGroupView* ui_;
    RecordingGroup* currentGroup_ = nullptr;
    RecordingView* recordingView_;
    RecordingNameCompleter* filterCompleter_;
};

}
