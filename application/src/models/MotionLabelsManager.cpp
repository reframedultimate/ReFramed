#include "application/models/MotionLabelsManager.hpp"
#include "application/models/Protocol.hpp"

#include "rfcommon/Frame.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/FighterState.hpp"
#include "rfcommon/Metadata.hpp"
#include "rfcommon/MotionLabels.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/MotionLabels.hpp"

#include <QStandardPaths>
#include <QDir>

namespace rfapp {

// ----------------------------------------------------------------------------
MotionLabelsManager::MotionLabelsManager(Protocol* protocol, rfcommon::MotionLabels* labels)
    : protocol_(protocol)
    , motionLabels_(labels)
{
    loadAllLayers();

    motionLabels_->dispatcher.addListener(this);
    protocol_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
MotionLabelsManager::~MotionLabelsManager()
{
    clearActiveSession();

    protocol_->dispatcher.removeListener(this);
    motionLabels_->dispatcher.removeListener(this);

    saveAllLayers();
}

// ----------------------------------------------------------------------------
bool MotionLabelsManager::loadAllLayers()
{
    return true;
}

// ----------------------------------------------------------------------------
bool MotionLabelsManager::saveAllLayers()
{
    PROFILE(MotionLabelsManager, saveAllLayers);

    if (motionLabelsModified_ == false)
        return true;

    return motionLabels_->save();
}

// ----------------------------------------------------------------------------
rfcommon::MotionLabels* MotionLabelsManager::motionLabels() const
{
    NOPROFILE();
    return motionLabels_;
}

// ----------------------------------------------------------------------------
void MotionLabelsManager::onMotionLabelsLoaded() { NOPROFILE(); }
void MotionLabelsManager::onMotionLabelsHash40sUpdated() { NOPROFILE(); motionLabelsModified_ = true; }

void MotionLabelsManager::onMotionLabelsLayerInserted(int layerIdx) { NOPROFILE(); motionLabelsModified_ = true; }
void MotionLabelsManager::onMotionLabelsLayerRemoved(int layerIdx) { NOPROFILE(); motionLabelsModified_ = true; }
void MotionLabelsManager::onMotionLabelsLayerNameChanged(int layerIdx) { NOPROFILE(); motionLabelsModified_ = true; }
void MotionLabelsManager::onMotionLabelsLayerUsageChanged(int layerIdx, int oldUsage) { NOPROFILE(); motionLabelsModified_ = true; }
void MotionLabelsManager::onMotionLabelsLayerMoved(int fromIdx, int toIdx) { NOPROFILE(); motionLabelsModified_ = true; }
void MotionLabelsManager::onMotionLabelsLayerMerged(int layerIdx) { NOPROFILE(); motionLabelsModified_ = true; }

void MotionLabelsManager::onMotionLabelsRowInserted(rfcommon::FighterID fighterID, int row) { NOPROFILE(); motionLabelsModified_ = true; }
void MotionLabelsManager::onMotionLabelsLabelChanged(rfcommon::FighterID fighterID, int row, int layerIdx) { NOPROFILE(); motionLabelsModified_ = true; }
void MotionLabelsManager::onMotionLabelsCategoryChanged(rfcommon::FighterID fighterID, int row, int oldCategory) { NOPROFILE(); motionLabelsModified_ = true; }

// ----------------------------------------------------------------------------
void MotionLabelsManager::setActiveSession(rfcommon::Session* session)
{
    PROFILE(MotionLabelsManager, setActiveSession);

    clearActiveSession();

    activeSession_ = session;
    if (auto fdata = activeSession_->tryGetFrameData())
        fdata->dispatcher.addListener(this);
}
void MotionLabelsManager::clearActiveSession()
{
    PROFILE(MotionLabelsManager, clearActiveSession);

    if (activeSession_)
        if (auto fdata = activeSession_->tryGetFrameData())
            fdata->dispatcher.removeListener(this);

    activeSession_.drop();
}

// ----------------------------------------------------------------------------
void MotionLabelsManager::onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) { NOPROFILE(); }
void MotionLabelsManager::onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) { NOPROFILE(); }
void MotionLabelsManager::onProtocolConnectedToServer(const char* ipAddress, uint16_t port) { NOPROFILE(); }
void MotionLabelsManager::onProtocolDisconnectedFromServer() { NOPROFILE(); }

void MotionLabelsManager::onProtocolTrainingStarted(rfcommon::Session* training) { NOPROFILE(); setActiveSession(training); }
void MotionLabelsManager::onProtocolTrainingResumed(rfcommon::Session* training) { NOPROFILE(); setActiveSession(training); }
void MotionLabelsManager::onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining) { NOPROFILE(); setActiveSession(newTraining); }
void MotionLabelsManager::onProtocolTrainingEnded(rfcommon::Session* training) { NOPROFILE(); clearActiveSession(); }

void MotionLabelsManager::onProtocolGameStarted(rfcommon::Session* game) { NOPROFILE(); setActiveSession(game); }
void MotionLabelsManager::onProtocolGameResumed(rfcommon::Session* game) { NOPROFILE(); setActiveSession(game); }
void MotionLabelsManager::onProtocolGameEnded(rfcommon::Session* game) { NOPROFILE(); clearActiveSession(); }

// ----------------------------------------------------------------------------
void MotionLabelsManager::onFrameDataNewUniqueFrame(int frameIdx, const rfcommon::Frame<4>& frame)
{
    PROFILE(MotionLabelsManager, onFrameDataNewUniqueFrame);

    auto mdata = activeSession_->tryGetMetadata();
    if (!mdata)
        return;

    for (int fighterIdx = 0; fighterIdx != frame.count(); ++fighterIdx)
    {
        auto fighterID = mdata->playerFighterID(fighterIdx);
        auto motion = frame[fighterIdx].motion();
        motionLabels_->addUnknownMotion(fighterID, motion);
    }
}
void MotionLabelsManager::onFrameDataNewFrame(int frameIdx, const rfcommon::Frame<4>& frame) { NOPROFILE(); }

}
