#pragma once

#include <QString>
#include <QVector>

class PlayerMeta;
class StatsCalculator;
class SettingsModel;
class WebSocketServer;

class QDir;

class WSExporter
{
public:
    WSExporter(
        const PlayerMeta* playerMeta,
        const StatsCalculator* stats,
        const SettingsModel* settings,
        const WebSocketServer* server);
    ~WSExporter();

    void writeJSON(bool gameStarted, bool gameEnded) const;

private:
    const PlayerMeta* playerMeta_;
    const StatsCalculator* stats_;
    const SettingsModel* settings_;
    const WebSocketServer* server_;
};
