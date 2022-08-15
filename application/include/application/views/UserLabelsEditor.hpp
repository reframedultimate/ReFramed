#pragma once

#include "rfcommon/Vector.hpp"
#include <QDialog>

class QTableView;
class QComboBox;

namespace rfcommon {
    class MappingInfo;
    class Session;
    class UserLabels;
}

namespace rfapp {

class UserLabelsEditor : public QDialog
{
    Q_OBJECT

public:
    explicit UserLabelsEditor(rfcommon::UserLabels* userLabels, QWidget* parent=nullptr);
    ~UserLabelsEditor();

    void populateFromGlobalData(rfcommon::MappingInfo* globalMappingInfo);
    void populateFromSessions(rfcommon::Session** loadedSessions, int sessionCount);

protected:
    void closeEvent(QCloseEvent* event);

private slots:

private:
    rfcommon::UserLabels* userLabels_;
    QComboBox* comboBox_fighters;
    rfcommon::Vector<QTableView*> tables_;
};

}
