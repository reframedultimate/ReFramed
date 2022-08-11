#pragma once

#include "rfcommon/Reference.hpp"
#include "rfcommon/ProtocolListener.hpp"
#include <QTabWidget>
#include <QHash>
#include <QVector>
#include <QString>

namespace rfcommon {
    class Session;
    class Plugin;
}

namespace rfapp {

class PluginManager;
class Protocol;

/*!
 * Handles loading/unloading of plugins (user can select which ones
 * he wants to load), and provides those plugins with data from single
 * or multiple replay files. This view can also act as a live session
 * viewer by using setActiveSession()
 */
class ReplayViewer 
    : public QTabWidget
    , public rfcommon::ProtocolListener
{
    Q_OBJECT
public:
    explicit ReplayViewer(PluginManager* pluginManager, QWidget* parent=nullptr);
    explicit ReplayViewer(Protocol* protocol, PluginManager* pluginManager, QWidget* parent=nullptr);
    ~ReplayViewer();

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
    void loadGameReplays(const QStringList& fileNames);

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
    void onGameReplaysLoaded(const QStringList& fileNames, const QVector<rfcommon::Session*>& sessions);
    void onTabBarClicked(int index);
    void onCurrentTabChanged(int index);

private:
    void closeTabWithView(QWidget* view);

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

    struct ReplayData
    {
        QString fileName;
        rfcommon::Reference<rfcommon::Session> session;
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

    int findInCache(const QString& fileName) const;

    Protocol* protocol_;
    PluginManager* pluginManager_;

    rfcommon::Reference<rfcommon::Session> activeSession_;
    QString ipAddress_;
    uint16_t port_;
    SessionState sessionState_;
    ActiveSessionState activeSessionState_;

    ReplayState replayState_;
    QVector<rfcommon::Session*> activeReplays_;
    QStringList pendingReplays_;
    QVector<ReplayData> replayCache_;
    QVector<PluginData> plugins_;
    int previousTab_;
};

}
