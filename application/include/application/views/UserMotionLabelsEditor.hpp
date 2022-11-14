#pragma once

#include "rfcommon/Vector.hpp"
#include "rfcommon/FighterID.hpp"
#include "rfcommon/Reference.hpp"
#include "rfcommon/UserMotionLabelsListener.hpp"
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
        , public rfcommon::UserMotionLabelsListener
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
    void onUserMotionLabelsLayerAdded(int layerIdx, const char* name) override;
    void onUserMotionLabelsLayerRemoved(int layerIdx, const char* name) override;

    void onUserMotionLabelsNewEntry(rfcommon::FighterID fighterID, int entryIdx) override;
    void onUserMotionLabelsUserLabelChanged(rfcommon::FighterID fighterID, int entryIdx, const char* oldLabel, const char* newLabel) override;
    void onUserMotionLabelsCategoryChanged(rfcommon::FighterID fighterID, int entryIdx, rfcommon::UserMotionLabelsCategory oldCategory, rfcommon::UserMotionLabelsCategory newCategory) override;

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
