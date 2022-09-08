#pragma once

#include "application/listeners/ReplayManagerListener.hpp"
#include "application/models/Protocol.hpp"  // MOC requires this because of smart pointers
#include "rfcommon/Reference.hpp"
#include "rfcommon/SetFormat.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/ProtocolListener.hpp"
#include "rfcommon/MetaDataListener.hpp"
#include "rfcommon/FrameDataListener.hpp"
#include <QObject>
#include <QDir>
#include <vector>

namespace rfcommon {
    class FrameData;
    class GameMetaData;
    class Session;
    class TrainingMetaData;
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
class ActiveSessionManager
    : public QObject
    , public ReplayManagerListener
    , public rfcommon::ProtocolListener
    , public rfcommon::MetaDataListener
    , public rfcommon::FrameDataListener
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

    Protocol* protocol() const;

    rfcommon::ListenerDispatcher<ActiveSessionManagerListener> dispatcher;

private:
    bool shouldStartNewSet(const rfcommon::GameMetaData* meta);

private:
    void onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) override {}
    void onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) override {}
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
    void onMetaDataTimeStartedChanged(rfcommon::TimeStamp timeStarted) override;
    void onMetaDataTimeEndedChanged(rfcommon::TimeStamp timeEnded) override;

    void onMetaDataPlayerNameChanged(int player, const char* name) override;
    void onMetaDataSponsorChanged(int fighterIdx, const char* sponsor) override;
    void onMetaDataTournamentNameChanged(const char* name) override;
    void onMetaDataEventNameChanged(const char* name) override;
    void onMetaDataRoundNameChanged(const char* name) override;
    void onMetaDataCommentatorsChanged(const rfcommon::SmallVector<rfcommon::String, 2>& names) override;
    void onMetaDataSetNumberChanged(rfcommon::SetNumber number) override;
    void onMetaDataGameNumberChanged(rfcommon::GameNumber number) override;
    void onMetaDataSetFormatChanged(const rfcommon::SetFormat& format) override;
    void onMetaDataWinnerChanged(int winner) override;

    void onMetaDataTrainingSessionNumberChanged(rfcommon::GameNumber number) override;

private:
    void onFrameDataNewUniqueFrame(int frameIdx, const rfcommon::Frame<4>& frame) override;
    void onFrameDataNewFrame(int frameIdx, const rfcommon::Frame<4>& frame) override;

private:
    Protocol* protocol_;
    ReplayManager* replayManager_;
    std::vector<rfcommon::Reference<rfcommon::GameMetaData>> pastGameMetaData_;
    rfcommon::Reference<rfcommon::MetaData> activeMetaData_;
    rfcommon::Reference<rfcommon::MappingInfo> activeMappingInfo_;
    rfcommon::Reference<rfcommon::FrameData> activeFrameData_;
    QString p1Name_;
    QString p2Name_;
    rfcommon::SetFormat format_;
    rfcommon::GameNumber gameNumber_;
    rfcommon::SetNumber setNumber_;
};

}
