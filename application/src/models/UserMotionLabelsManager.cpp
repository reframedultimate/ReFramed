#include "application/models/UserMotionLabelsManager.hpp"
#include "application/models/Protocol.hpp"

#include "rfcommon/Frame.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/FighterState.hpp"
#include "rfcommon/MappedFile.hpp"
#include "rfcommon/Metadata.hpp"
#include "rfcommon/MotionLabels.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/UserMotionLabels.hpp"
#include "rfcommon/Utf8.hpp"

#include <QStandardPaths>
#include <QDir>

namespace rfapp {

// ----------------------------------------------------------------------------
UserMotionLabelsManager::UserMotionLabelsManager(rfcommon::MotionLabels* motionLabels, Protocol* protocol)
    : motionLabels_(motionLabels)
    , protocol_(protocol)
    , userMotionLabels_(new rfcommon::UserMotionLabels)
{
    loadAllLayers();

    motionLabels_->dispatcher.addListener(this);
    protocol_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
UserMotionLabelsManager::~UserMotionLabelsManager()
{
    clearActiveSession();

    protocol_->dispatcher.removeListener(this);
    motionLabels_->dispatcher.removeListener(this);

    saveAllLayers();
}

// ----------------------------------------------------------------------------
bool UserMotionLabelsManager::loadAllLayers()
{
    return true;
}

// ----------------------------------------------------------------------------
bool UserMotionLabelsManager::saveAllLayers()
{
    PROFILE(UserMotionLabelsManager, saveAllLayers);

    if (motionLabelsModified_ == false)
        return true;

    return motionLabels_->save();
}

// ----------------------------------------------------------------------------
rfcommon::UserMotionLabels* UserMotionLabelsManager::userMotionLabels() const
{
    NOPROFILE();
    return userMotionLabels_;
}

// ----------------------------------------------------------------------------
rfcommon::MotionLabels* UserMotionLabelsManager::motionLabels() const
{
    NOPROFILE();
    return motionLabels_;
}

// ----------------------------------------------------------------------------
void UserMotionLabelsManager::onMotionLabelsLoaded() { NOPROFILE(); }
void UserMotionLabelsManager::onMotionLabelsHash40sUpdated() { NOPROFILE(); motionLabelsModified_ = true; }

void UserMotionLabelsManager::onMotionLabelsLayerInserted(int layerIdx) { NOPROFILE(); motionLabelsModified_ = true; }
void UserMotionLabelsManager::onMotionLabelsLayerRemoved(int layerIdx) { NOPROFILE(); motionLabelsModified_ = true; }
void UserMotionLabelsManager::onMotionLabelsLayerNameChanged(int layerIdx) { NOPROFILE(); motionLabelsModified_ = true; }
void UserMotionLabelsManager::onMotionLabelsLayerUsageChanged(int layerIdx, int oldUsage) { NOPROFILE(); motionLabelsModified_ = true; }
void UserMotionLabelsManager::onMotionLabelsLayerMoved(int fromIdx, int toIdx) { NOPROFILE(); motionLabelsModified_ = true; }
void UserMotionLabelsManager::onMotionLabelsLayerMerged(int layerIdx) { NOPROFILE(); motionLabelsModified_ = true; }

void UserMotionLabelsManager::onMotionLabelsRowInserted(rfcommon::FighterID fighterID, int row) { NOPROFILE(); motionLabelsModified_ = true; }
void UserMotionLabelsManager::onMotionLabelsLabelChanged(rfcommon::FighterID fighterID, int row, int layerIdx) { NOPROFILE(); motionLabelsModified_ = true; }
void UserMotionLabelsManager::onMotionLabelsCategoryChanged(rfcommon::FighterID fighterID, int row, int oldCategory) { NOPROFILE(); motionLabelsModified_ = true; }

// ----------------------------------------------------------------------------
void UserMotionLabelsManager::setActiveSession(rfcommon::Session* session)
{
    PROFILE(UserMotionLabelsManager, setActiveSession);

    clearActiveSession();

    activeSession_ = session;
    if (auto fdata = activeSession_->tryGetFrameData())
        fdata->dispatcher.addListener(this);
}
void UserMotionLabelsManager::clearActiveSession()
{
    PROFILE(UserMotionLabelsManager, clearActiveSession);

    if (activeSession_)
        if (auto fdata = activeSession_->tryGetFrameData())
            fdata->dispatcher.removeListener(this);

    activeSession_.drop();
}

// ----------------------------------------------------------------------------
void UserMotionLabelsManager::onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) { NOPROFILE(); }
void UserMotionLabelsManager::onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) { NOPROFILE(); }
void UserMotionLabelsManager::onProtocolConnectedToServer(const char* ipAddress, uint16_t port) { NOPROFILE(); }
void UserMotionLabelsManager::onProtocolDisconnectedFromServer() { NOPROFILE(); }

void UserMotionLabelsManager::onProtocolTrainingStarted(rfcommon::Session* training) { NOPROFILE(); setActiveSession(training); }
void UserMotionLabelsManager::onProtocolTrainingResumed(rfcommon::Session* training) { NOPROFILE(); setActiveSession(training); }
void UserMotionLabelsManager::onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining) { NOPROFILE(); setActiveSession(newTraining); }
void UserMotionLabelsManager::onProtocolTrainingEnded(rfcommon::Session* training) { NOPROFILE(); clearActiveSession(); }

void UserMotionLabelsManager::onProtocolGameStarted(rfcommon::Session* game) { NOPROFILE(); setActiveSession(game); }
void UserMotionLabelsManager::onProtocolGameResumed(rfcommon::Session* game) { NOPROFILE(); setActiveSession(game); }
void UserMotionLabelsManager::onProtocolGameEnded(rfcommon::Session* game) { NOPROFILE(); clearActiveSession(); }

// ----------------------------------------------------------------------------
void UserMotionLabelsManager::onFrameDataNewUniqueFrame(int frameIdx, const rfcommon::Frame<4>& frame)
{
    PROFILE(UserMotionLabelsManager, onFrameDataNewUniqueFrame);

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
void UserMotionLabelsManager::onFrameDataNewFrame(int frameIdx, const rfcommon::Frame<4>& frame) { NOPROFILE(); }

}
