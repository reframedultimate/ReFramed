#pragma once

#include "rfcommon/Vector.hpp"
#include "rfcommon/FighterID.hpp"
#include <QDialog>

class QAbstractTableModel;
class QTableView;
class QComboBox;

namespace rfcommon {
    class Hash40Strings;
    class MappingInfo;
    class Session;
    class UserMotionLabels;
}

namespace rfapp {

class UserLabelsEditor : public QDialog
{
    Q_OBJECT

public:
    explicit UserLabelsEditor(rfcommon::UserMotionLabels* userMotionLabels, rfcommon::Hash40Strings* hash40Strings, QWidget* parent=nullptr);
    ~UserLabelsEditor();

    void populateFromGlobalData(rfcommon::MappingInfo* globalMappingInfo);
    void populateFromSessions(rfcommon::Session** loadedSessions, int sessionCount);

protected:
    void closeEvent(QCloseEvent* event);

private slots:
    void onFighterSelected(int index);

private:
    rfcommon::UserMotionLabels* userMotionLabels_;
    rfcommon::Hash40Strings* hash40Strings_;
    QComboBox* comboBox_fighters;
    rfcommon::Vector<rfcommon::FighterID> indexToFighterID_;
    rfcommon::Vector<QAbstractTableModel*> tableModels_;
    rfcommon::Vector<QTableView*> tableViews_;
};

}
