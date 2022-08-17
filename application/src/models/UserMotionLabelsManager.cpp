#include "application/models/UserMotionLabelsManager.hpp"
#include "application/models/Protocol.hpp"

#include "rfcommon/Frame.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/FighterState.hpp"
#include "rfcommon/MappedFile.hpp"
#include "rfcommon/MetaData.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/UserMotionLabels.hpp"

#include <QStandardPaths>
#include <QDir>

namespace rfapp {

// ----------------------------------------------------------------------------
UserMotionLabelsManager::UserMotionLabelsManager(Protocol* protocol)
    : protocol_(protocol)
    , userMotionLabels_(new rfcommon::UserMotionLabels)
{
    loadAllLayers();

    userMotionLabels_->dispatcher.addListener(this);
    protocol_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
UserMotionLabelsManager::~UserMotionLabelsManager()
{
    clearActiveSession();

    protocol_->dispatcher.removeListener(this);
    userMotionLabels_->dispatcher.removeListener(this);

    saveAllLayers();
}

// ----------------------------------------------------------------------------
bool UserMotionLabelsManager::loadAllLayers()
{
    bool success = true;

    QDir dir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    if (dir.exists("motion") == false)
        return true;

    dir.cd("motion");

    for (const auto& file : dir.entryList({ "*.json" }, QDir::Files | QDir::NoDot | QDir::NoDotDot, QDir::Name))
    {
        if (file == "unlabeled.json")
            continue;

        QByteArray ba = dir.absoluteFilePath(file).toUtf8();
        rfcommon::MappedFile f;
        if (f.open(ba.constData()) == false)
        {
            success = false;
            continue;
        }

        userMotionLabels_->loadLayer(f.address(), f.size());
    }

    QString fileName = dir.absoluteFilePath("unlabeled.json");
    QByteArray ba = fileName.toUtf8();
    rfcommon::MappedFile f;
    if (f.open(ba.constData()) == false)
        return false;
    userMotionLabels_->loadUnlabeled(f.address(), f.size());

    return success;
}

// ----------------------------------------------------------------------------
bool UserMotionLabelsManager::saveAllLayers()
{
    if (motionLabelsModified_ == false)
        return true;

    bool success = true;

    QDir dir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    if (dir.exists("motion") == false)
        dir.mkdir("motion");

    dir.cd("motion");
    
    // Temporary backup of previous files
    QStringList previousFiles = dir.entryList({ "*.json" }, QDir::Files | QDir::NoDot | QDir::NoDotDot);
    for (const auto& f : previousFiles)
        dir.rename(f, "." + f);

    for (int layerIdx = 0; layerIdx != userMotionLabels_->layerCount(); ++layerIdx)
    {
        QString fileName = dir.absoluteFilePath(QString::number(layerIdx + 1) + "_" + userMotionLabels_->layerName(layerIdx) + ".json");
        QByteArray ba = fileName.toUtf8();
        FILE* fp = fopen(ba.constData(), "wb");
        if (fp == nullptr)
        {
            success = false;
            continue;
        }

        int result = userMotionLabels_->saveLayer(fp, layerIdx);
        fclose(fp);
        if (result == 0)
        {
            success = false;
            continue;
        }
    }

    QString fileName = dir.absoluteFilePath("unlabeled.json");
    QByteArray ba = fileName.toUtf8();
    FILE* fp = fopen(ba.constData(), "wb");
    if (fp == nullptr)
        success = false;
    else
    {
        if (userMotionLabels_->saveUnlabeled(fp) == 0)
            success = false;
        fclose(fp);
    }

    // Remove previous files
    for (const auto& f : previousFiles)
        dir.remove("." + f);

    return success;
}

// ----------------------------------------------------------------------------
rfcommon::UserMotionLabels* UserMotionLabelsManager::userMotionLabels() const
{
    return userMotionLabels_;
}

// ----------------------------------------------------------------------------
void UserMotionLabelsManager::onUserMotionLabelsLayerAdded(int layerIdx, const char* name) { motionLabelsModified_ = true; }
void UserMotionLabelsManager::onUserMotionLabelsLayerRemoved(int layerIdx, const char* name) { motionLabelsModified_ = true; }

void UserMotionLabelsManager::onUserMotionLabelsNewEntry(rfcommon::FighterID fighterID, int entryIdx) { motionLabelsModified_ = true; }
void UserMotionLabelsManager::onUserMotionLabelsUserLabelChanged(rfcommon::FighterID fighterID, int entryIdx, const char* oldLabel, const char* newLabel) { motionLabelsModified_ = true; }
void UserMotionLabelsManager::onUserMotionLabelsCategoryChanged(rfcommon::FighterID fighterID, int entryIdx, rfcommon::UserMotionLabelsCategory oldCategory, rfcommon::UserMotionLabelsCategory newCategory) { motionLabelsModified_ = true; }

// ----------------------------------------------------------------------------
void UserMotionLabelsManager::setActiveSession(rfcommon::Session* session)
{
    clearActiveSession();

    activeSession_ = session;
    if (auto fdata = activeSession_->tryGetFrameData())
        fdata->dispatcher.addListener(this);
}
void UserMotionLabelsManager::clearActiveSession()
{
    if (activeSession_)
        if (auto fdata = activeSession_->tryGetFrameData())
            fdata->dispatcher.removeListener(this);

    activeSession_.drop();
}

// ----------------------------------------------------------------------------
void UserMotionLabelsManager::onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) {}
void UserMotionLabelsManager::onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) {}
void UserMotionLabelsManager::onProtocolConnectedToServer(const char* ipAddress, uint16_t port) {}
void UserMotionLabelsManager::onProtocolDisconnectedFromServer() {}

// ----------------------------------------------------------------------------
void UserMotionLabelsManager::onProtocolTrainingStarted(rfcommon::Session* training)
{
    setActiveSession(training);
}
void UserMotionLabelsManager::onProtocolTrainingResumed(rfcommon::Session* training) 
{
    setActiveSession(training);
}
void UserMotionLabelsManager::onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining) 
{
    setActiveSession(newTraining);
}
void UserMotionLabelsManager::onProtocolTrainingEnded(rfcommon::Session* training) 
{
    clearActiveSession();
}
void UserMotionLabelsManager::onProtocolGameStarted(rfcommon::Session* game) 
{
    setActiveSession(game);
}
void UserMotionLabelsManager::onProtocolGameResumed(rfcommon::Session* game) 
{
    setActiveSession(game);
}
void UserMotionLabelsManager::onProtocolGameEnded(rfcommon::Session* game) 
{
    clearActiveSession();
}

// ----------------------------------------------------------------------------
void UserMotionLabelsManager::onFrameDataNewUniqueFrame(int frameIdx, const rfcommon::Frame<4>& frame) 
{
    auto mdata = activeSession_->tryGetMetaData();
    if (!mdata)
        return;

    for (int fighterIdx = 0; fighterIdx != frame.count(); ++fighterIdx)
    {
        auto fighterID = mdata->fighterID(fighterIdx);
        auto motion = frame[fighterIdx].motion();
        userMotionLabels_->addUnknownMotion(fighterID, motion);
    }
}
void UserMotionLabelsManager::onFrameDataNewFrame(int frameIdx, const rfcommon::Frame<4>& frame) {}

}
