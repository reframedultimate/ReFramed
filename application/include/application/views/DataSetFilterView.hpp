#pragma once

#include <QWidget>
#include "application/listeners/ReplayManagerListener.hpp"
#include "application/listeners/ReplayGroupListener.hpp"
#include "application/listeners/DataSetBackgroundLoaderListener.hpp"
#include "rfcommon/DataSetFilterListener.hpp"
#include "rfcommon/Reference.hpp"
#include <unordered_map>
#include <memory>

class QVBoxLayout;
class QListWidgetItem;

namespace rfcommon {
    class DataSetFilterChain;
    class DataSet;
}

namespace Ui {
    class DataSetFilterView;
}

namespace rfapp {

class DataSetFilterWidget;
class DataSetBackgroundLoader;
class ReplayManager;

class DataSetFilterView : public QWidget
                        , public ReplayManagerListener
                        , public ReplayGroupListener
                        , public DataSetBackgroundLoaderListener
                        , public rfcommon::DataSetFilterListener
{
    Q_OBJECT

public:
    explicit DataSetFilterView(ReplayManager* manager, QWidget* parent=nullptr);
    ~DataSetFilterView();

protected:
    virtual bool eventFilter(QObject* target, QEvent* e) override;

private slots:
    void onToolButtonAddFilterTriggered(QAction* action);
    void onInputGroupItemChanged(QListWidgetItem* item);
    void onFilterEnabled(DataSetFilterWidget* filter, bool enable);
    void onFilterInverted(DataSetFilterWidget* filter, bool inverted);
    void onFilterMoveUp(DataSetFilterWidget* filter);
    void onFilterMoveDown(DataSetFilterWidget* filter);
    void onRemoveFilterRequested(DataSetFilterWidget* filter);
    void reprocessInputDataSet();

private:
    void addNewFilterWidget(DataSetFilterWidget* widget);
    void recursivelyInstallEventFilter(QObject* obj);
    void addGroupToInputRecordingsList(ReplayGroup* group);
    void removeGroupFromInputRecordingsList(ReplayGroup* group);
    void removeGroupFromInputDataSet(ReplayGroup* group);
    void moveFilterWidgetInLayout(DataSetFilterWidget* widget, int layoutIndex);
    void dirtyDataSetFilters();

private:
    void onReplayGroupFileAdded(ReplayGroup* group, const QFileInfo& absPathToFile) override;
    void onReplayGroupFileRemoved(ReplayGroup* group, const QFileInfo& absPathToFile) override;

    void onReplayManagerGroupAdded(ReplayGroup* group) override;
    void onReplayManagerGroupNameChanged(ReplayGroup* group, const QString& oldName, const QString& newName) override;
    void onReplayManagerGroupRemoved(ReplayGroup* group) override;

    void onDataSetBackgroundLoaderDataSetLoaded(ReplayGroup* group, rfcommon::DataSet* dataSet) override;

    void onDataSetFilterDirtied(rfcommon::DataSetFilter* filter);

    void onReplayManagerDefaultReplaySaveLocationChanged(const QDir& path) override { (void)path; }
    void onReplayManagerReplaySourceAdded(const QString& name, const QDir& path) override { (void)name; (void)path; }
    void onReplayManagerReplaySourceNameChanged(const QString& oldName, const QString& newName) override { (void)oldName; (void)newName; }
    void onReplayManagerReplaySourcePathChanged(const QString& name, const QDir& oldPath, const QDir& newPath) override { (void)name; (void)oldPath; (void)newPath; }
    void onReplayManagerReplaySourceRemoved(const QString& name) override { (void)name; }
    void onReplayManagerVideoSourceAdded(const QString& name, const QDir& path) override { (void)name; (void)path; }
    void onReplayManagerVideoSourceNameChanged(const QString& oldName, const QString& newName) override { (void)oldName; (void)newName; }
    void onReplayManagerVideoSourcePathChanged(const QString& name, const QDir& oldPath, const QDir& newPath) override { (void)name; (void)oldPath; (void)newPath; }
    void onReplayManagerVideoSourceRemoved(const QString& name) override { (void)name; }

private:
    Ui::DataSetFilterView* ui_;
    QVBoxLayout* filterWidgetsLayout_;
    ReplayManager* savedGameSessionManager_;
    DataSetBackgroundLoader* dataSetBackgroundLoader_;
    std::unique_ptr<rfcommon::DataSetFilterChain> dataSetFilterChain_;
    std::unique_ptr<rfcommon::DataSet> inputDataSetMerged_;
    rfcommon::Reference<rfcommon::DataSet> outputDataSet_;
    std::unordered_map<const ReplayGroup*, rfcommon::Reference<rfcommon::DataSet>> inputDataSets_;
    bool dataSetFiltersDirty_ = true;
};

}
