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

namespace Ui {
    class MotionLabelsEditor;
}

namespace rfapp {

class MainWindow;
class Protocol;
class MotionLabelsManager;

class MotionLabelsEditor
        : public QDialog
        , public rfcommon::MotionLabelsListener
{
    Q_OBJECT

public:
    explicit MotionLabelsEditor(
            MainWindow* mainWindow,
            MotionLabelsManager* manager,
            Protocol* protocol,
            rfcommon::MappingInfo* globalMappingInfo=nullptr);
    ~MotionLabelsEditor();

    void populateFromGlobalData(rfcommon::MappingInfo* globalMappingInfo);
    void populateFromSessions(rfcommon::Session** loadedSessions, int sessionCount);

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onFighterSelected(int index);
    void onCustomContextMenuRequested(int tabIdx, const QPoint& globalPos);

private:
    void updateFightersDropdown(rfcommon::FighterID fighterID);
    bool highlightNextConflict(int direction);

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
    Ui::MotionLabelsEditor* ui_;
    MainWindow* mainWindow_;
    MotionLabelsManager* manager_;
    rfcommon::Reference<rfcommon::MappingInfo> globalMappingInfo_;
    rfcommon::Vector<rfcommon::FighterID> indexToFighterID_;
    rfcommon::Vector<QAbstractTableModel*> tableModels_;
    rfcommon::Vector<QTableView*> tableViews_;

    int currentConflictTableIdx_ = -1;
};

}
