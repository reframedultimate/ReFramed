#pragma once

#include "rfcommon/ListenerDispatcher.hpp"
#include "stats/StatType.hpp"

#include <QString>
#include <QDir>

class SettingsListener;

/*!
 * \brief Holds the state of all of the user settings in the
 * settings view. The settings are also saved to JSON so they
 * persist accross runs.
 */
class SettingsModel
{
public:
    enum ResetBehavior
    {
        RESET_EACH_GAME,
        RESET_EACH_SET
    };

    /*!
     * \brief Construct and load settings from a file
     */
    SettingsModel(const QString& settingsFile);

    /*! Load settings from the file specified in the constructor. Return false if it fails. */
    bool load();

    /*! Save settings to the file specified in the constructor. Return false if it fails. */
    bool save();

    /*! 
     * \brief Enable or disable a particular statistic. This controls
     * which types of statistics are displayed in the UI and also
     * exported.
     */
    void setStatEnabled(StatType type, bool enable);

    /*! \brief Check to see if a particular statistic is enabled or not. */
    bool statEnabled(StatType type) const;

    /*!
     * \brief Moves the specified statistic to a particular index.
     * You can use this to set the order in which the stats appear in the
     * UI and in the exported files.
     */
    void setStatAtIndex(int idx, StatType type);

    /*!
     * \brief Gets the statistic type at the index.
     */
    StatType statAtIndex(int idx) const;

    /*! 
     * \brief Number of stats that are enabled. Use this combined with
     * statAtIndex() to loop over all enabled statistics in the order they
     * were arranged in by the user.
     */
    int numStatsEnabled() const;

    ResetBehavior resetBehavior() const;
    void setResetBehavior(ResetBehavior behavior);

    /*! \brief Return true if exporting to OBS is enabled */
    bool obsEnabled() const;

    /*! \brief Enable or disable exporting to OBS */
    void obsSetEnabled(bool enable);

    /*!
     * \brief Configure how many additional newlines to insert in between
     * the exported statistics. 0 Means no additional newlines.
     */
    void obsSetAdditionalNewlines(int lines);

    /*!
     * \brief How many additional newlines to insert in between the 
     * exported statistics. 0 Means no additional newlines.
     */
    int obsAdditionalNewlines() const;

    /*! \brief Set the directory to export OBS files to */
    void obsSetDestinationFolder(const QDir& dir);

    /*! \brief The directory to export OBS files to */
    const QDir& obsDestinationFolder() const;

    /*! 
     * \brief Returns in seconds how often to generate exports. A value
     * of 0 Means do it after each game only.
     */
    int obsExportInterval() const;

    /*!
     * \brief Set in seconds how often to generate exports. A value
     * of 0 Means do it after each game only.
     */
    void obsSetExportInterval(int seconds);

    bool wsEnabled() const;
    void wsSetEnabled(bool enable);

    bool wsAutoStart() const;
    void wsSetAutoStart(bool enable);

    bool wsSecureMode() const;
    void wsSetSecureMode(bool enable);

    QString wsHostName() const;
    uint16_t wsPort() const;
    void wsSetHostNameAndPort(const QString& hostName, uint16_t port);

    rfcommon::ListenerDispatcher<SettingsListener> dispatcher;

private:
    bool statEnabled_[STAT_COUNT] = {
#define X(type, str, colorcode) true,
        STAT_TYPES_LIST
#undef X
    };

    rfcommon::SmallVector<StatType, STAT_COUNT> statAtIndex_ = {
#define X(type, str, colorcode) STAT_##type,
        STAT_TYPES_LIST
#undef X
    };

    QString settingsFile_;
    QDir obsDestinationFolder_;
    QString wsHostName_;
    int obsAdditionalNewlines_ = 0;
    int obsExportInterval_ = 0;
    uint16_t wsPort_ = 1337;
    ResetBehavior resetBehavior_ = RESET_EACH_GAME;
    bool obsEnabled_ = false;
    bool wsEnabled_ = false;
    bool wsAutoStart_ = false;
    bool wsSecureMode_ = false;
};
