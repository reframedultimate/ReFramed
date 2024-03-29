#pragma once

#include "rfcommon/Reference.hpp"
#include "rfcommon/ProtocolListener.hpp"

#include "ads/DockManager.h"

#include <QHash>
#include <QVector>
#include <QString>

namespace rfcommon {
    class Session;
    class Plugin;
    class PluginContext;
}

namespace rfapp {

class PluginManager;
class Protocol;
class ReplayManager;

/*!
 * Handles loading/unloading of plugins (user can select which ones
 * he wants to load), and provides those plugins with data from single
 * or multiple replay files. This view can also act as a live session
 * viewer by using setActiveSession()
 */
class PluginDockView
        : public ads::CDockManager
        , public rfcommon::ProtocolListener
{
    Q_OBJECT

public:
    explicit PluginDockView(ReplayManager* replayManager, PluginManager* pluginManager, QWidget* parent=nullptr);
    explicit PluginDockView(Protocol* protocol, PluginManager* pluginManager, QWidget* parent=nullptr);
    ~PluginDockView();

    /*!
     * \brief Loads the specified set of sessions from files and
     * passes it to the plugins.
     *
     * If a session is active and running, then this will cause all
     * plugins to receive onProtocolGameEnded()/onProtocolTrainingEnded()
     * before receiving onGameSessionLoaded()/onTrainingSessionLoaded().
     *
     * If a session is already loaded, then this will cause all plugins
     * to receive onGameSessionUnloaded/onTrainingSessionUnloaded()
     * before receiving onGameSessionLoaded()/onTrainingSessionLoaded().
     *
     * If a set of sessions is already loaded, then this will cause all
     * plugins to receive onGameSessionSetUnloaded() before receiving
     * onGameSessionSetLoaded().
     */
    void loadGameReplays(const QStringList& filePaths);

    /*!
     * \brief Clears any sessions loaded from files. This does not affect
     * active sessions. This is used when the user de-selects, moves, or
     * deletes replays and causes plugins to remove any data they have
     * loaded.
     */
    void clearReplays();

    /*!
     * \brief Causes all plugins to receive the onProtocolGameEnded()
     * or onProtocolTrainingEnded() event, even if this isn't technically
     * the case. Causes plugins to remove any data they have loaded from
     * an active session.
     */
    void clearActiveSession();

private slots:
    void onAddNewPluginRequested(ads::CDockAreaWidget* dockArea);
    void onClosePluginRequested(ads::CDockWidget* dockWidget);
    void onDockAreaCreated(ads::CDockAreaWidget* dockArea);

private:
    void onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) override;
    void onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) override;
    void onProtocolConnectedToServer(const char* ipAddress, uint16_t port) override;
    void onProtocolDisconnectedFromServer() override;

    void onProtocolTrainingStarted(rfcommon::Session* training) override;
    void onProtocolTrainingResumed(rfcommon::Session* training) override;
    void onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining) override;
    void onProtocolTrainingEnded(rfcommon::Session* training) override;
    void onProtocolGameStarted(rfcommon::Session* game) override;
    void onProtocolGameResumed(rfcommon::Session* game) override;
    void onProtocolGameEnded(rfcommon::Session* game) override;

private:
    struct PluginData
    {
        rfcommon::Plugin* plugin;
        QWidget* view;
        QString name;
    };

    enum SessionState
    {
        DISCONNECTED,
        ATTEMPT_CONNECT,
        CONNECTED,
    };
    enum ActiveSessionState
    {
        NO_ACTIVE_SESSION,
        TRAINING_STARTED,
        TRAINING_RESUMED,
        TRAINING_STARTED_ENDED,
        TRAINING_RESUMED_ENDED,
        GAME_STARTED,
        GAME_RESUMED,
        GAME_STARTED_ENDED,
        GAME_RESUMED_ENDED
    };
    enum ReplayState
    {
        NONE_LOADED,
        GAME_LOADED,
        TRAINING_LOADED
    };

    Protocol* protocol_;
    ReplayManager* replayManager_;
    PluginManager* pluginManager_;

    rfcommon::Reference<rfcommon::PluginContext> pluginCtx_;
    QString ipAddress_;
    uint16_t port_;
    SessionState sessionState_;
    ActiveSessionState activeSessionState_;

    ReplayState replayState_;
    rfcommon::Reference<rfcommon::Session> activeSession_;
    QVector<rfcommon::Reference<rfcommon::Session>> activeReplays_;
    QVector<PluginData> plugins_;
};

}
