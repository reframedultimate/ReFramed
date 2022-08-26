#pragma once

#include "stats/StatType.hpp"
#include <QWidget>

class SettingsModel;
class SettingsDragWidget;

namespace Ui {
    class ExportView;
}

class ExportView : public QWidget
{
    Q_OBJECT

public:
    explicit ExportView(SettingsModel* model, QWidget* parent=nullptr);
    ~ExportView();

private slots:
    void onResetEachGameToggled(bool eachGame);

    void onOBSCheckBoxToggled(bool checked);
    void onOBSInsertNewLinesCheckBoxToggled(bool enable);
    void onOBSSpinBoxNewLinesChanged(int value);
    void onOBSBrowseFolderButtonReleased();
    void onOBSExportAfterEachGameToggled(bool checked);
    void onOBSExportIntervalValueChanged(int value);

private:
    Ui::ExportView* ui_;
    SettingsModel* settings_;
};
