#pragma once

#include <QWidget>
#include "application/listeners/SavedGameSessionManagerListener.hpp"
#include "application/listeners/SavedGameSessionGroupListener.hpp"
#include "application/listeners/DataSetBackgroundLoaderListener.hpp"
#include "uh/DataSetFilterListener.hpp"
#include "uh/Reference.hpp"
#include <unordered_map>
#include <memory>

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
class SavedGameSessionManager;

class DataSetFilterView : public QWidget
                        , public SavedGameSessionManagerListener
                        , public SavedGameSessionGroupListener
                        , public DataSetBackgroundLoaderListener
                        , public uh::DataSetFilterListener
{
    Q_OBJECT

public:
    explicit DataSetFilterView(SavedGameSessionManager* manager, QWidget* parent=nullptr);
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
    void addGroupToInputRecordingsList(SavedGameSessionGroup* group);
    void removeGroupFromInputRecordingsList(SavedGameSessionGroup* group);
    void removeGroupFromInputDataSet(SavedGameSessionGroup* group);
    void moveFilterWidgetInLayout(DataSetFilterWidget* widget, int layoutIndex);
    void dirtyDataSetFilters();

private:
    void onSavedGameSessionGroupFileAdded(SavedGameSessionGroup* group, const QFileInfo& absPathToFile) override;
    void onSavedGameSessionGroupFileRemoved(SavedGameSessionGroup* group, const QFileInfo& absPathToFile) override;

    void onSavedGameSessionManagerGroupAdded(SavedGameSessionGroup* group) override;
    void onSavedGameSessionManagerGroupNameChanged(SavedGameSessionGroup* group, const QString& oldName, const QString& newName) override;
    void onSavedGameSessionManagerGroupRemoved(SavedGameSessionGroup* group) override;

    void onDataSetBackgroundLoaderDataSetLoaded(SavedGameSessionGroup* group, uh::DataSet* dataSet) override;

    void onDataSetFilterDirtied(uh::DataSetFilter* filter);

    void onSavedGameSessionManagerDefaultGameSessionSaveLocationChanged(const QDir& path) override { (void)path; }
    void onSavedGameSessionManagerGameSessionSourceAdded(const QString& name, const QDir& path) override { (void)name; (void)path; }
    void onSavedGameSessionManagerGameSessionSourceNameChanged(const QString& oldName, const QString& newName) override { (void)oldName; (void)newName; }
    void onSavedGameSessionManagerGameSessionSourceRemoved(const QString& name) override { (void)name; }
    void onSavedGameSessionManagerVideoSourceAdded(const QString& name, const QDir& path) override { (void)name; (void)path; }
    void onSavedGameSessionManagerVideoSourceNameChanged(const QString& oldName, const QString& newName) override { (void)oldName; (void)newName; }
    void onSavedGameSessionManagerVideoSourceRemoved(const QString& name) override { (void)name; }

private:
    Ui::DataSetFilterView* ui_;
    QVBoxLayout* filterWidgetsLayout_;
    SavedGameSessionManager* savedGameSessionManager_;
    DataSetBackgroundLoader* dataSetBackgroundLoader_;
    std::unique_ptr<uh::DataSetFilterChain> dataSetFilterChain_;
    std::unique_ptr<uh::DataSet> inputDataSetMerged_;
    uh::Reference<uh::DataSet> outputDataSet_;
    std::unordered_map<const SavedGameSessionGroup*, uh::Reference<uh::DataSet>> inputDataSets_;
    bool dataSetFiltersDirty_ = true;
};

}
