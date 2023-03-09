#pragma once

#include "rfcommon/Vector.hpp"
#include "rfcommon/FighterID.hpp"
#include "rfcommon/Reference.hpp"
#include "rfcommon/MotionLabelsListener.hpp"
#include <QDialog>

class QAbstractTableModel;
class QTableView;
class QTabWidget;
class QComboBox;

namespace rfcommon {
    class Hash40Strings;
    class MappingInfo;
    class Session;
}

namespace rfapp {

class MainWindow;
class UserMotionLabelsManager;

class UserMotionLabelsEditor
        : public QDialog
        , public rfcommon::MotionLabelsListener
{
    Q_OBJECT

public:
    explicit UserMotionLabelsEditor(
            MainWindow* mainWindow,
            UserMotionLabelsManager* manager,
            rfcommon::Hash40Strings* hash40Strings,
            rfcommon::MappingInfo* globalMappingInfo=nullptr);
    ~UserMotionLabelsEditor();

    void populateFromGlobalData(rfcommon::MappingInfo* globalMappingInfo);
    void populateFromSessions(rfcommon::Session** loadedSessions, int sessionCount);

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onFighterSelected(int index);
    void onCustomContextMenuRequested(int tabIdx, const QPoint& globalPos);

private:
    void updateFightersDropdown(rfcommon::FighterID fighterID);

private:
    void onMotionLabelsLoaded() override;
    void onMotionLabelsHash40sUpdated() override;

    void onMotionLabelsLayerInserted(int layerIdx) override;
    void onMotionLabelsLayerRemoved(int layerIdx) override;
    void onMotionLabelsLayerNameChanged(int layerIdx) override;
    void onMotionLabelsLayerUsageChanged(int layerIdx, int oldUsage) override;
    void onMotionLabelsLayerMoved(int fromIdx, int toIdx) override;
    void onMotionLabelsLayerMerged(int layerIdx) override;

    void onMotionLabelsRowInserted(rfcommon::FighterID fighterID, int row) override;
    void onMotionLabelsLabelChanged(rfcommon::FighterID fighterID, int row, int layerIdx) override;
    void onMotionLabelsCategoryChanged(rfcommon::FighterID fighterID, int row, int oldCategory) override;

private:
    MainWindow* mainWindow_;
    UserMotionLabelsManager* manager_;
    rfcommon::Reference<rfcommon::Hash40Strings> hash40Strings_;
    rfcommon::Reference<rfcommon::MappingInfo> globalMappingInfo_;
    QComboBox* comboBox_fighters;
    QTabWidget* tabWidget_categories;
    rfcommon::Vector<rfcommon::FighterID> indexToFighterID_;
    rfcommon::Vector<QAbstractTableModel*> tableModels_;
    rfcommon::Vector<QTableView*> tableViews_;
};

}
