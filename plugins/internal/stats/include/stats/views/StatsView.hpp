#pragma once

#include "stats/listeners/SettingsListener.hpp"
#include "stats/listeners/StatsCalculatorListener.hpp"
#include "stats/listeners/PlayerMetaListener.hpp"
#include <QWidget>

// Forward declare the class created by Qt designer
namespace Ui {
    class StatsView;
}

class PlayerMeta;
class QGridLayout;
class QLabel;
class StatsCalculator;
class SettingsModel;

class StatsView 
    : public QWidget
    , public StatsCalculatorListener
    , public SettingsListener
    , public PlayerMetaListener
{
    Q_OBJECT

public:
    explicit StatsView(
        PlayerMeta* playerMeta,
        StatsCalculator* stats,
        SettingsModel* settings,
        QWidget* parent=nullptr);
    ~StatsView();

private:
    void updateStatsLabels();
    void recreateLayout();

private:
    void onStatsUpdated() override;
    void onSettingsStatsChanged() override;
    void onSettingsOBSChanged() override;
    void onSettingsWSChanged() override;

private:
    void onPlayerMetaChanged() override;

private:
    // Holds info on the player's name, tag, and character
    PlayerMeta* playerMeta_;

    // We hold a weak reference to the calculator so we can update
    // the UI when stats are updated
    StatsCalculator* stats_;

    // We also listen to changes to the settings
    SettingsModel* settings_;

    // Holds all of the UI text labels with statistics from the calculator
    QGridLayout* layout_;

    QLabel* p1Label_;
    QLabel* p2Label_;
};
