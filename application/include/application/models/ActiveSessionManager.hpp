#pragma once

#include "application/listeners/ReplayManagerListener.hpp"
#include "application/models/Protocol.hpp"  // MOC requires this because of smart pointers
#include "rfcommon/Reference.hpp"
#include "rfcommon/SetFormat.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/ProtocolListener.hpp"
#include "rfcommon/SessionMetaDataListener.hpp"
#include <QObject>
#include <QDir>
#include <vector>

namespace rfcommon {
    class Session;
    class GameSessionMetaData;
    class TrainingSessionMetaData;
}

namespace rfapp {

class Protocol;
class Settings;
class ActiveSessionManagerListener;
class ReplayManager;

/*!
 * \brief Central class that manages set information ("set" is the term used
 * for a set of smash games, e.g. a Bo5 is a set of 3-5 games) and saving
 * sessions to files as they come in.
 */
class ActiveSessionManager : public QObject
                           , public ReplayManagerListener
                           , public rfcommon::ProtocolListener
                           , public rfcommon::SessionMetaDataListener
{
    Q_OBJECT

public:
    ActiveSessionManager(Protocol* protocol, rfapp::ReplayManager* manager, QObject* parent=nullptr);
    ~ActiveSessionManager();

    void setSetFormat(const rfcommon::SetFormat& format);
    void setP1Name(const QString& name);
    void setP2Name(const QString& name);
    void setGameNumber(rfcommon::GameNumber number);
    void setTrainingSessionNumber(rfcommon::GameNumber number);

    rfcommon::ListenerDispatcher<ActiveSessionManagerListener> dispatcher;

private:
    void findUniqueGameAndSetNumbers(rfcommon::MappingInfo* map, rfcommon::GameSessionMetaData* meta);
    void findUniqueTrainingSessionNumber(rfcommon::MappingInfo* map, rfcommon::TrainingSessionMetaData* meta);
    bool shouldStartNewSet(const rfcommon::GameSessionMetaData* meta);

private:
    void onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) override {}
    void onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) override {}
    void onProtocolConnectedToServer(const char* ipAddress, uint16_t port) override {}
    void onProtocolDisconnectedFromServer() override {}

    void onProtocolTrainingStarted(rfcommon::Session* training) override {}
    void onProtocolTrainingResumed(rfcommon::Session* training) override {}
    void onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining) override {}
    void onProtocolTrainingEnded(rfcommon::Session* training) override {}
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
    void onSessionMetaDataTimeStartedChanged(rfcommon::TimeStamp timeStarted) override;
    void onSessionMetaDataTimeEndedChanged(rfcommon::TimeStamp timeEnded) override;

    void onSessionMetaDataPlayerNameChanged(int player, const rfcommon::SmallString<15>& name) override;
    void onSessionMetaDataSetNumberChanged(rfcommon::SetNumber number) override;
    void onSessionMetaDataGameNumberChanged(rfcommon::GameNumber number) override;
    void onSessionMetaDataSetFormatChanged(const rfcommon::SetFormat& format) override;
    void onSessionMetaDataWinnerChanged(int winner) override;

    void onSessionMetaDataTrainingSessionNumberChanged(rfcommon::GameNumber number) override;

private:
    Protocol* protocol_;
    ReplayManager* replayManager_;
    std::vector<rfcommon::Reference<rfcommon::GameSessionMetaData>> pastGameMetaData_;
    rfcommon::Reference<rfcommon::SessionMetaData> activeMetaData_;
    rfcommon::Reference<rfcommon::MappingInfo> activeMappingInfo_;
    QString gameSaveFormat_;
    QString trainingSaveFormat_;
    QString p1Name_;
    QString p2Name_;
    rfcommon::SetFormat format_;
    rfcommon::GameNumber gameNumber_;
    rfcommon::SetNumber setNumber_;
};

}