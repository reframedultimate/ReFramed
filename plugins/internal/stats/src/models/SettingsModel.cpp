#include "rfcommon/Profiler.hpp"
#include "stats/models/SettingsModel.hpp"
#include "stats/listeners/SettingsListener.hpp"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>

// ----------------------------------------------------------------------------
SettingsModel::SettingsModel(const QString& settingsFile)
    : settingsFile_(settingsFile)
{
    load();
}

// ----------------------------------------------------------------------------
bool SettingsModel::load()
{
    PROFILE(SettingsModel, load);

    QFile file(settingsFile_);
    if (file.open(QIODevice::ReadOnly) == false)
        return false;

    QJsonObject data = QJsonDocument::fromJson(file.readAll()).object();
    
    const QJsonObject obs = data["obs"].toObject();
    obsEnabled_ = obs["enabled"].toBool(false);
    obsAdditionalNewlines_ = obs["extranewlines"].toInt(0);
    obsDestinationFolder_.setPath(obs["dir"].toString(""));
    obsExportInterval_ = obs["exportinterval"].toInt(0);

    const QJsonObject ws = data["websockets"].toObject();
    wsEnabled_ = ws["enabled"].toBool(false);
    wsAutoStart_ = ws["autostart"].toBool(false);
    wsSecureMode_ = ws["secure"].toBool(false);
    wsHostName_ = ws["hostname"].toString("");
    wsPort_ = ws["port"].toInt(0);

    const QJsonObject stats = data["stats"].toObject();
    QString resetBehaviorStr = stats["reseteach"].toString("");

    if (resetBehaviorStr == "set")
        resetBehavior_ = RESET_EACH_SET;
    else
        resetBehavior_ = RESET_EACH_GAME;

    if (stats.contains("enabled"))
    {
        // Reset arrays
        statAtIndex_.clear();
        for (int i = 0; i != STAT_COUNT; ++i)
            statEnabled_[i] = false;

        const QJsonArray enabledStats = stats["enabled"].toArray();
        for (const auto& statNameRef : enabledStats)
        {
            const QString statName = statNameRef.toString("");
            if (statName == "")
                continue;

            QByteArray ba = statName.toLatin1();
            StatType type = stringToStatType(ba.constData());
            if (type == STAT_COUNT)
                continue;

            statEnabled_[type] = true;
            statAtIndex_.push(type);
        }
    }
    else
    {
        // Enable everything by default
        statAtIndex_.clear();
        for (int i = 0; i != STAT_COUNT; ++i)
        {
            statEnabled_[i] = true;
            statAtIndex_.push(static_cast<StatType>(i));
        }
    }

    return true;
}

// ----------------------------------------------------------------------------
bool SettingsModel::save()
{
    PROFILE(SettingsModel, save);

    QJsonArray enabledStats;
    for (int i = 0; i != numStatsEnabled(); ++i)
    {
        StatType type = statAtIndex(i);
        enabledStats.append(statTypeToString(type));
    }

    auto resetBehaviorStr = [this]() -> const char* {
        switch (resetBehavior_) {
            case RESET_EACH_GAME: return "game";
            case RESET_EACH_SET: return "set";
        }
        return "";
    };

    QJsonObject stats = {
        {"enabled", enabledStats},
        {"reseteach", resetBehaviorStr()}
    };

    QJsonObject obs = {
        {"enabled", obsEnabled()},
        {"extranewlines", obsAdditionalNewlines()},
        {"dir", obsDestinationFolder().absolutePath()},
        {"exportinterval", obsExportInterval_}
    };

    QJsonObject ws = {
        {"enabled", wsEnabled()},
        {"autostart", wsAutoStart()},
        {"secure", wsSecureMode()},
        {"hostname", wsHostName()},
        {"port", wsPort()}
    };

    QJsonObject data = {
        {"version", "1.0"},
        {"stats", stats},
        {"obs", obs},
        {"websockets", ws},
    };

    QFile file(settingsFile_);
    if (file.open(QIODevice::WriteOnly) == false)
        return false;
    file.write(QJsonDocument(data).toJson());
    return true;
}

// ----------------------------------------------------------------------------
void SettingsModel::setStatEnabled(StatType type, bool enable)
{
    PROFILE(SettingsModel, setStatEnabled);

    if (statEnabled_[type] == enable)
        return;

    if (enable)
        statAtIndex_.push(type);
    else
    {
        for (auto it = statAtIndex_.begin(); it != statAtIndex_.end(); ++it)
            if (*it == type)
            {
                statAtIndex_.erase(it);
                break;
            }
    }

    statEnabled_[type] = enable;

    save();
    dispatcher.dispatch(&SettingsListener::onSettingsStatsChanged);
}

// ----------------------------------------------------------------------------
bool SettingsModel::statEnabled(StatType type) const
{
    PROFILE(SettingsModel, statEnabled);

    return statEnabled_[type];
}

// ----------------------------------------------------------------------------
void SettingsModel::setStatAtIndex(int idx, StatType type)
{
    PROFILE(SettingsModel, setStatAtIndex);

    if (statAtIndex_[idx] == type)
        return;

    int currentIdx = 0;
    for (; statAtIndex_[currentIdx] != type; ++currentIdx)
        if (currentIdx == statAtIndex_.count())
            return;  // This stat wasn't enabled

    statAtIndex_.take(currentIdx);
    statAtIndex_.insert(idx, type);

    save();
    dispatcher.dispatch(&SettingsListener::onSettingsStatsChanged);
}

// ----------------------------------------------------------------------------
StatType SettingsModel::statAtIndex(int idx) const
{
    PROFILE(SettingsModel, statAtIndex);

    return statAtIndex_[idx];
}

// ----------------------------------------------------------------------------
int SettingsModel::numStatsEnabled() const
{
    PROFILE(SettingsModel, numStatsEnabled);

    return statAtIndex_.count();
}

// ----------------------------------------------------------------------------
SettingsModel::ResetBehavior SettingsModel::resetBehavior() const
{
    PROFILE(SettingsModel, resetBehavior);

    return resetBehavior_;
}

// ----------------------------------------------------------------------------
void SettingsModel::setResetBehavior(ResetBehavior behavior)
{
    PROFILE(SettingsModel, setResetBehavior);

    resetBehavior_ = behavior;

    save();
    dispatcher.dispatch(&SettingsListener::onSettingsOBSChanged);
}

// ----------------------------------------------------------------------------
bool SettingsModel::obsEnabled() const
{
    PROFILE(SettingsModel, obsEnabled);

    return obsEnabled_;
}

// ----------------------------------------------------------------------------
void SettingsModel::obsSetEnabled(bool enable)
{
    PROFILE(SettingsModel, obsSetEnabled);

    obsEnabled_ = enable;

    save();
    dispatcher.dispatch(&SettingsListener::onSettingsOBSChanged);
}

// ----------------------------------------------------------------------------
void SettingsModel::obsSetAdditionalNewlines(int lines)
{
    PROFILE(SettingsModel, obsSetAdditionalNewlines);

    obsAdditionalNewlines_ = lines;

    save();
    dispatcher.dispatch(&SettingsListener::onSettingsOBSChanged);
}

// ----------------------------------------------------------------------------
int SettingsModel::obsAdditionalNewlines() const
{
    PROFILE(SettingsModel, obsAdditionalNewlines);

    return obsAdditionalNewlines_;
}

// ----------------------------------------------------------------------------
void SettingsModel::obsSetDestinationFolder(const QDir& dir)
{
    PROFILE(SettingsModel, obsSetDestinationFolder);

    obsDestinationFolder_ = dir;

    save();
    dispatcher.dispatch(&SettingsListener::onSettingsOBSChanged);
}

// ----------------------------------------------------------------------------
const QDir& SettingsModel::obsDestinationFolder() const
{
    PROFILE(SettingsModel, obsDestinationFolder);

    return obsDestinationFolder_;
}

// ----------------------------------------------------------------------------
int SettingsModel::obsExportInterval() const
{
    PROFILE(SettingsModel, obsExportInterval);

    return obsExportInterval_;
}

// ----------------------------------------------------------------------------
void SettingsModel::obsSetExportInterval(int seconds)
{
    PROFILE(SettingsModel, obsSetExportInterval);

    obsExportInterval_ = seconds;

    save();
    dispatcher.dispatch(&SettingsListener::onSettingsOBSChanged);
}

// ----------------------------------------------------------------------------
bool SettingsModel::wsEnabled() const
{
    PROFILE(SettingsModel, wsEnabled);

    return wsEnabled_;
}

// ----------------------------------------------------------------------------
void SettingsModel::wsSetEnabled(bool enable)
{
    PROFILE(SettingsModel, wsSetEnabled);

    wsEnabled_ = enable;

    save();
    dispatcher.dispatch(&SettingsListener::onSettingsWSChanged);
}

// ----------------------------------------------------------------------------
bool SettingsModel::wsAutoStart() const
{
    PROFILE(SettingsModel, wsAutoStart);

    return wsAutoStart_;
}

// ----------------------------------------------------------------------------
void SettingsModel::wsSetAutoStart(bool enable)
{
    PROFILE(SettingsModel, wsSetAutoStart);

    wsAutoStart_ = enable;

    save();
    dispatcher.dispatch(&SettingsListener::onSettingsWSChanged);
}

// ----------------------------------------------------------------------------
bool SettingsModel::wsSecureMode() const
{
    PROFILE(SettingsModel, wsSecureMode);

    return wsSecureMode_;
}

// ----------------------------------------------------------------------------
void SettingsModel::wsSetSecureMode(bool enable)
{
    PROFILE(SettingsModel, wsSetSecureMode);

    wsSecureMode_ = enable;

    save();
    dispatcher.dispatch(&SettingsListener::onSettingsWSChanged);
}

// ----------------------------------------------------------------------------
QString SettingsModel::wsHostName() const
{
    PROFILE(SettingsModel, wsHostName);

    return wsHostName_;
}

// ----------------------------------------------------------------------------
void SettingsModel::wsSetHostNameAndPort(const QString& hostName, uint16_t port)
{
    PROFILE(SettingsModel, wsSetHostNameAndPort);

    wsHostName_ = hostName;
    wsPort_ = port;

    save();
    dispatcher.dispatch(&SettingsListener::onSettingsWSChanged);
}

// ----------------------------------------------------------------------------
uint16_t SettingsModel::wsPort() const
{
    PROFILE(SettingsModel, wsPort);

    return wsPort_;
}
