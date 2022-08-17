#pragma once

#include "rfcommon/Reference.hpp"
#include "rfcommon/ProtocolListener.hpp"
#include "rfcommon/UserMotionLabelsListener.hpp"

namespace rfcommon {
    class Hash40Strings;
    class UserMotionLabels;
}

namespace rfapp {

class Protocol;

class UserMotionLabelsManager 
    : public rfcommon::UserMotionLabelsListener
    , public rfcommon::ProtocolListener
{
public:
    UserMotionLabelsManager(Protocol* protocol);
    ~UserMotionLabelsManager();

    bool loadAllLayers();
    bool saveAllLayers();

    rfcommon::UserMotionLabels* userMotionLabels() const;

private:
    void onUserMotionLabelsLayerAdded(int layerIdx, const char* name) override;
    void onUserMotionLabelsLayerRemoved(int layerIdx, const char* name) override;

    void onUserMotionLabelsNewEntry(rfcommon::FighterID fighterID, int entryIdx) override;
    void onUserMotionLabelsEntryChanged(rfcommon::FighterID fighterID, int entryIdx) override;
    void onUserMotionLabelsEntryRemoved(rfcommon::FighterID fighterID, int entryIdx) override;

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
    Protocol* protocol_;
    rfcommon::Reference<rfcommon::UserMotionLabels> userMotionLabels_;
};

}
