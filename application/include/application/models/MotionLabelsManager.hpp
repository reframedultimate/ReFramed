#pragma once

#include "rfcommon/FrameDataListener.hpp"
#include "rfcommon/MotionLabelsListener.hpp"
#include "rfcommon/ProtocolListener.hpp"
#include "rfcommon/Reference.hpp"

namespace rfcommon {
    class Hash40Strings;
    class Metadata;
    class MotionLabels;
    class Session;
}

namespace rfapp {

class Protocol;

/*!
 * \brief This class is responsible for loading and saving user layers
 * to and from disk, as well as updating the "rfcommon::MotionLabels"
 * structure with new motion values from live sessions as the data comes in.
 */
class MotionLabelsManager
        : public rfcommon::MotionLabelsListener
        , public rfcommon::ProtocolListener
        , public rfcommon::FrameDataListener
{
public:
    MotionLabelsManager(Protocol* protocol, rfcommon::MotionLabels* motionLabels);
    ~MotionLabelsManager();

    void discardChanges();
    bool saveChanges();

    rfcommon::MotionLabels* motionLabels() const;

private:
    void onMotionLabelsLoaded() override;
    void onMotionLabelsHash40sUpdated() override;

    void onMotionLabelsPreferredLayerChanged(int usage) override;

    void onMotionLabelsLayerInserted(int layerIdx) override;
    void onMotionLabelsLayerRemoved(int layerIdx) override;
    void onMotionLabelsLayerNameChanged(int layerIdx) override;
    void onMotionLabelsLayerUsageChanged(int layerIdx, int oldUsage) override;
    void onMotionLabelsLayerMoved(int fromIdx, int toIdx) override;
    void onMotionLabelsLayerMerged(int layerIdx) override;

    void onMotionLabelsRowInserted(rfcommon::FighterID fighterID, int row) override;
    void onMotionLabelsLabelChanged(rfcommon::FighterID fighterID, int row, int layerIdx) override;
    void onMotionLabelsCategoryChanged(rfcommon::FighterID fighterID, int row, int oldCategory) override;

private:
    void setActiveSession(rfcommon::Session* session);
    void clearActiveSession();
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
    void onFrameDataNewUniqueFrame(int frameIdx, const rfcommon::Frame<4>& frame) override;
    void onFrameDataNewFrame(int frameIdx, const rfcommon::Frame<4>& frame) override;

private:
    Protocol* protocol_;
    rfcommon::Reference<rfcommon::MotionLabels> motionLabels_;
    rfcommon::Reference<rfcommon::Session> activeSession_;

    bool pendingChanges_ = false;
};

}
