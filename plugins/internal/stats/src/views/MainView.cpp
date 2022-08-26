#include "stats/views/ExportView.hpp"
#include "stats/views/MainView.hpp"
#include "stats/views/StatsView.hpp"
#include "stats/views/StatsArrangeView.hpp"

#include <QVBoxLayout>
#include <QTabWidget>

// ----------------------------------------------------------------------------
MainView::MainView(PlayerMeta* playerMeta, StatsCalculator* statsModel, SettingsModel* settingsModel, QWidget* parent)
{
    setLayout(new QVBoxLayout);

    QTabWidget* tabWidget = new QTabWidget;
    tabWidget->addTab(new StatsView(playerMeta, statsModel, settingsModel), "Statistics");
    tabWidget->addTab(new StatsArrangeView(settingsModel), "Enable and Disable Stats");
    tabWidget->addTab(new ExportView(settingsModel), "Settings and Export");
    layout()->addWidget(tabWidget);
}
