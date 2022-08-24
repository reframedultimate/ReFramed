#pragma once

#include <QString>
#include <QVector>

class PlayerMeta;
class StatsCalculator;
class SettingsModel;

class QDir;

class OBSExporter
{
public:
    OBSExporter(
        const PlayerMeta* playerMeta,
        const StatsCalculator* stats,
        const SettingsModel* settings);
    bool exportEmptyValues() const;
    bool exportStatistics() const;

    void setPlayerTag(int idx, const QString& tag);
    void setPlayerCharacter(int idx, const QString& character);

private:
    bool writeNames() const;
    bool writeScene() const;
    bool writePlayerCharacters() const;
    bool writePlayerTags() const;
    bool writePlayerStats() const;
    bool writePlayerStatsEmpty() const;

private:
    const PlayerMeta* playerMeta_;
    const StatsCalculator* stats_;
    const SettingsModel* settings_;
    QVector<QString> tags_;
    QVector<QString> chars_;
};
