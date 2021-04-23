#pragma once

#include <QWidget>
#include "application/listeners/RecordingManagerListener.hpp"
#include "application/listeners/RecordingGroupListener.hpp"
#include "application/listeners/DataSetBackgroundLoaderListener.hpp"

class QVBoxLayout;
class QListWidgetItem;

namespace uh {
    class DataSetFilterChain;
    class DataSet;
}

namespace Ui {
    class DataSetFilterView;
}

namespace uhapp {

class DataSetFilterWidget;
class DataSetBackgroundLoader;
class RecordingManager;

class DataSetFilterView : public QWidget
                        , public RecordingManagerListener
                        , public RecordingGroupListener
                        , public DataSetBackgroundLoaderListener
{
    Q_OBJECT

public:
    explicit DataSetFilterView(RecordingManager* recordingManager, QWidget* parent=nullptr);
    ~DataSetFilterView();

protected:
    virtual bool eventFilter(QObject* target, QEvent* e) override;

private slots:
    void onToolButtonAddFilterTriggered(QAction* action);
    void onInputGroupItemChanged(QListWidgetItem* item);

private:
    void addNewFilterWidget(DataSetFilterWidget* widget);
    void recursivelyInstallEventFilter(QObject* obj);
    void addGroupToInputRecordingsList(RecordingGroup* group);
    void removeGroupFromInputRecordingsList(RecordingGroup* group);
    void removeGroupFromInputDataSet(RecordingGroup* group);

private:
    void onRecordingGroupNameChanged(RecordingGroup* group, const QString& oldName, const QString& newName) override;
    void onRecordingGroupFileAdded(RecordingGroup* group, const QFileInfo& absPathToFile) override;
    void onRecordingGroupFileRemoved(RecordingGroup* group, const QFileInfo& absPathToFile) override;

    void onRecordingManagerGroupAdded(RecordingGroup* group) override;
    void onRecordingManagerGroupRemoved(RecordingGroup* group) override;

    void onDataSetBackgroundLoaderDataSetLoaded(RecordingGroup* group, uh::DataSet* dataSet) override;

    void onRecordingManagerDefaultRecordingLocationChanged(const QDir& path) override { (void)path; }
    void onRecordingManagerRecordingSourceAdded(const QString& name, const QDir& path) override { (void)name; (void)path; }
    void onRecordingManagerRecordingSourceNameChanged(const QString& oldName, const QString& newName) override { (void)oldName; (void)newName; }
    void onRecordingManagerRecordingSourceRemoved(const QString& name) override { (void)name; }
    void onRecordingManagerVideoSourceAdded(const QString& name, const QDir& path) override { (void)name; (void)path; }
    void onRecordingManagerVideoSourceNameChanged(const QString& oldName, const QString& newName) override { (void)oldName; (void)newName; }
    void onRecordingManagerVideoSourceRemoved(const QString& name) override { (void)name; }

private:
    Ui::DataSetFilterView* ui_;
    QVBoxLayout* filterWidgetsLayout_;
    RecordingManager* recordingManager_;
    DataSetBackgroundLoader* dataSetBackgroundLoader_;
    std::unique_ptr<uh::DataSetFilterChain> dataSetFilterChain_;
    std::unique_ptr<uh::DataSet> inputDataSet_;
};

}
