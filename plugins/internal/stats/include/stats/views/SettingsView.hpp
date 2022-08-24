#pragma once

#include "stats/StatType.hpp"
#include <QWidget>

class SettingsModel;
class SettingsDragWidget;

namespace Ui {
    class SettingsView;
}

class SettingsView : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsView(SettingsModel* model, QWidget* parent=nullptr);
    ~SettingsView();

private slots:
    void onResetEachGameToggled(bool eachGame);
    void onStatEnabled(int insertIndex, StatType type);
    void onStatDisabled(int insertIndex, StatType type);

    void onOBSCheckBoxToggled(bool checked);
    void onOBSInsertNewLinesCheckBoxToggled(bool enable);
    void onOBSSpinBoxNewLinesChanged(int value);
    void onOBSBrowseFolderButtonReleased();
    void onOBSExportAfterEachGameToggled(bool checked);
    void onOBSExportIntervalValueChanged(int value);

private:
    Ui::SettingsView* ui_;
    SettingsModel* settings_;
    SettingsDragWidget* enabledStats_;
    SettingsDragWidget* disabledStats_;
};
