#pragma once

#include "application/listeners/ReplayManagerListener.hpp"
#include "application/models/Protocol.hpp"  // MOC requires this because of smart pointers
#include "rfcommon/Reference.hpp"
#include "rfcommon/SetFormat.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/ProtocolListener.hpp"
#include "rfcommon/SessionMetaDataListener.hpp"
#include "rfcommon/FrameDataListener.hpp"
#include <QObject>
#include <QDir>
#include <vector>

namespace rfcommon {
    class Session;
    class GameSessionMetaData;
}

namespace rfapp {

class Protocol;
class Settings;
class ActiveGameSessionManagerListener;
class ReplayManager;

/*!
 * \brief Central class that controls the connection + protocol with the Switch
 * and manages set information and saving recordings to files as they come in.
 */
class ActiveGameSessionManager : public QObject
                               , public ReplayManagerListener
                               , public rfcommon::ProtocolListener
                               , public rfcommon::SessionMetaDataListener
                               , public rfcommon::FrameDataListener
{
    Q_OBJECT

public:
    ActiveGameSessionManager(Protocol* protocol, rfapp::ReplayManager* manager, QObject* parent = nullptr);
    ~ActiveGameSessionManager();

    void setFormat(const rfcommon::SetFormat& format);
    void setP1Name(const QString& name);
    void setP2Name(const QString& name);
    void setGameNumber(rfcommon::GameNumber number);

    bool isSessionRunning() const;

    rfcommon::ListenerDispatcher<ActiveGameSessionManagerListener> dispatcher;

private:
    void findUniqueGameAndSetNumbers(rfcommon::Session* session);
    bool shouldStartNewSet(const rfcommon::Session* session);

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
    void onReplayManagerDefaultReplaySaveLocationChanged(const QDir& path) override;

    void onReplayManagerGroupAdded(ReplayGroup* group) override { (void)group; }
    void onReplayManagerGroupNameChanged(ReplayGroup* group, const QString& oldName, const QString& newName) override { (void)group; (void)oldName; (void)newName; }
    void onReplayManagerGroupRemoved(ReplayGroup* group) override { (void)group; }

    void onReplayManagerReplaySourceAdded(const QString& name, const QDir& path) override { (void)name; (void)path; }
    void onReplayManagerReplaySourceNameChanged(const QString& oldName, const QString& newName) override { (void)oldName; (void)newName; }
    void onReplayManagerReplaySourcePathChanged(const QString& name, const QDir& oldPath, const QDir& newPath) override { (void)name; (void)oldPath; (void)newPath; }
    void onReplayManagerReplaySourceRemoved(const QString& name) override { (void)name; }

    void onReplayManagerVideoSourceAdded(const QString& name, const QDir& path) override { (void)name; (void)path; }
    void onReplayManagerVideoSourceNameChanged(const QString& oldName, const QString& newName) override { (void)oldName; (void)newName; }
    void onReplayManagerVideoSourcePathChanged(const QString& name, const QDir& oldPath, const QDir& newPath) override { (void)name; (void)oldPath; (void)newPath; }
    void onReplayManagerVideoSourceRemoved(const QString& name) override { (void)name; }

private:
    void onSessionMetaDataPlayerNameChanged(int player, const rfcommon::SmallString<15>& name) override;
    void onSessionMetaDataSetNumberChanged(rfcommon::SetNumber number) override;
    void onSessionMetaDataGameNumberChanged(rfcommon::GameNumber number) override;
    void onSessionMetaDataSetFormatChanged(const rfcommon::SetFormat& format) override;
    void onSessionMetaDataWinnerChanged(int winner) override;

    void onFrameDataNewUniqueFrame(int frameIdx, const rfcommon::Frame& frame) override;
    void onFrameDataNewFrame(int frameIdx, const rfcommon::Frame& frame) override;

private:
    Protocol* protocol_;
    std::vector<rfcommon::Reference<rfcommon::Session>> pastSessions_;
    rfcommon::Reference<rfcommon::Session> activeSession_;
    ReplayManager* replayManager_;
    QString gameSaveFormat_;
    QString trainingSaveFormat_;
    QString p1Name_;
    QString p2Name_;
    rfcommon::SetFormat format_;
    rfcommon::GameNumber gameNumber_;
    rfcommon::SetNumber setNumber_;
};

}
