#pragma once

#include <QWidget>

class PlayerMeta;
class StatsCalculator;
class SettingsModel;
class WebSocketServer;

class MainView : public QWidget
{
public:
    explicit MainView(PlayerMeta* playerMeta, StatsCalculator* statsModel, SettingsModel* settingsModel, WebSocketServer* wsServer, QWidget* parent=nullptr);
};
