#pragma once

#include "stats/StatType.hpp"
#include <QWidget>

class SettingsModel;
class SettingsDragWidget;

namespace Ui {
    class StatsArrangeView;
}

class StatsArrangeView : public QWidget
{
    Q_OBJECT

public:
    explicit StatsArrangeView(SettingsModel* model, QWidget* parent=nullptr);
    ~StatsArrangeView();

private slots:
    void onStatEnabled(int insertIndex, StatType type);
    void onStatDisabled(int insertIndex, StatType type);

private:
    Ui::StatsArrangeView* ui_;
    SettingsModel* settings_;
    SettingsDragWidget* enabledStats_;
    SettingsDragWidget* disabledStats_;
};
