#pragma once

#include <QWidget>

class PlayerMeta;
class StatsCalculator;
class SettingsModel;

class MainView : public QWidget
{
public:
    explicit MainView(PlayerMeta* playerMeta, StatsCalculator* statsModel, SettingsModel* settingsModel, QWidget* parent=nullptr);
};
