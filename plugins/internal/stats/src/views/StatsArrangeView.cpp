#include "rfcommon/Profiler.hpp"
#include "ui_StatsArrangeView.h"
#include "stats/models/SettingsModel.hpp"
#include "stats/views/StatsArrangeView.hpp"
#include "stats/widgets/SettingsDragWidget.hpp"

#include <QVBoxLayout>

// ----------------------------------------------------------------------------
StatsArrangeView::StatsArrangeView(SettingsModel* model, QWidget* parent)
    : QWidget(parent)
    , ui_(new Ui::StatsArrangeView)
    , settings_(model)
    , enabledStats_(new SettingsDragWidget)
    , disabledStats_(new SettingsDragWidget)
{
    ui_->setupUi(this);

    for (int i = 0; i != settings_->numStatsEnabled(); ++i)
    {
        StatType type = settings_->statAtIndex(i);
        enabledStats_->addStat(type);
    }

    for (int i = 0; i != STAT_COUNT; ++i)
    {
        StatType type = static_cast<StatType>(i);
        if (settings_->statEnabled(type) == false)
            disabledStats_->addStat(type);
    }

    ui_->groupBox_enabledStats->setLayout(new QVBoxLayout);
    ui_->groupBox_disabledStats->setLayout(new QVBoxLayout);
    ui_->groupBox_enabledStats->layout()->addWidget(enabledStats_);
    ui_->groupBox_disabledStats->layout()->addWidget(disabledStats_);

    connect(enabledStats_, &SettingsDragWidget::statAdded, this, &StatsArrangeView::onStatEnabled);
    connect(disabledStats_, &SettingsDragWidget::statAdded, this, &StatsArrangeView::onStatDisabled);
}

// ----------------------------------------------------------------------------
StatsArrangeView::~StatsArrangeView()
{
    delete ui_;
}

// ----------------------------------------------------------------------------
void StatsArrangeView::onStatEnabled(int insertIndex, StatType type)
{
    PROFILE(StatsArrangeView, onStatEnabled);

    settings_->setStatEnabled(type, true);
    settings_->setStatAtIndex(insertIndex, type);
}

// ----------------------------------------------------------------------------
void StatsArrangeView::onStatDisabled(int insertIndex, StatType type)
{
    PROFILE(StatsArrangeView, onStatDisabled);

    settings_->setStatEnabled(type, false);
}
