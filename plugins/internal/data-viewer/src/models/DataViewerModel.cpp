#include "data-viewer/models/BaseStatusIDModel.hpp"
#include "data-viewer/models/FighterIDModel.hpp"
#include "data-viewer/models/FighterStatesModel.hpp"
#include "data-viewer/models/DataViewerModel.hpp"
#include "data-viewer/models/HitStatusIDModel.hpp"
#include "data-viewer/models/MetadataModel.hpp"
#include "data-viewer/models/SpecificStatusIDModel.hpp"
#include "data-viewer/models/StageIDModel.hpp"
#include "data-viewer/models/VideoMetadataModel.hpp"
#include "data-viewer/listeners/DataViewerListener.hpp"

#include "rfcommon/Frame.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/Hash40Strings.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/Metadata.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/UserMotionLabels.hpp"
#include "rfcommon/VideoMeta.hpp"

// ----------------------------------------------------------------------------
DataViewerModel::DataViewerModel(rfcommon::UserMotionLabels* userLabels, rfcommon::Hash40Strings* hash40Strings)
    : userLabels_(userLabels)
    , hash40Strings_(hash40Strings)
    , baseStatusIDModel_(new BaseStatusIDModel)
    , fighterIDModel_(new FighterIDModel)
    , hitStatusIDModel_(new HitStatusIDModel)
    , metadataModel_(new MetadataModel)
    , videoMetadataModel_(new VideoMetadataModel)
    , specificStatusIDModel_(new SpecificStatusIDModel)
    , stageIDModel_(new StageIDModel)
{
}

// ----------------------------------------------------------------------------
DataViewerModel::~DataViewerModel()
{
    clearSession();
}

// ----------------------------------------------------------------------------
void DataViewerModel::setSession(rfcommon::Session* session)
{
    PROFILE(DataViewerModel, setSession);

    clearSession();

    mappingInfo_ = session->tryGetMappingInfo();
    metadata_ = session->tryGetMetadata();
    videoMetadata_ = session->tryGetVideoMeta();
    frameData_ = session->tryGetFrameData();

    baseStatusIDModel_->setMappingInfo(mappingInfo_);
    fighterIDModel_->setMappingInfo(mappingInfo_);
    hitStatusIDModel_->setMappingInfo(mappingInfo_);
    metadataModel_->setMetadata(mappingInfo_, metadata_);
    videoMetadataModel_->setVideoMetadata(videoMetadata_);
    specificStatusIDModel_->setMappingInfo(mappingInfo_);
    stageIDModel_->setMappingInfo(mappingInfo_);

    fighterStatesModels_.clear();
    if (frameData_)
    {
        for (int i = 0; i != frameData_->fighterCount(); ++i)
            fighterStatesModels_.emplace_back(new FighterStatesModel(
                    frameData_,
                    mappingInfo_,
                    i,
                    metadata_ ? metadata_->playerFighterID(i) : rfcommon::FighterID::makeInvalid(),
                    userLabels_,
                    hash40Strings_));
        frameData_->dispatcher.addListener(this);
    }

    dispatcher.dispatch(&DataViewerListener::onNewData, mappingInfo_, metadata_, videoMetadata_, frameData_);
}

// ----------------------------------------------------------------------------
void DataViewerModel::clearSession()
{
    PROFILE(DataViewerModel, clearSession);

    dispatcher.dispatch(&DataViewerListener::onClear);

    baseStatusIDModel_->setMappingInfo(nullptr);
    fighterIDModel_->setMappingInfo(nullptr);
    hitStatusIDModel_->setMappingInfo(nullptr);
    metadataModel_->setMetadata(nullptr, nullptr);
    specificStatusIDModel_->setMappingInfo(nullptr);
    stageIDModel_->setMappingInfo(nullptr);

    if (frameData_)
        frameData_->dispatcher.removeListener(this);

    mappingInfo_.drop();
    metadata_.drop();
    frameData_.drop();
}

// ----------------------------------------------------------------------------
rfcommon::MappingInfo* DataViewerModel::mappingInfo() const { return mappingInfo_; }
rfcommon::Metadata* DataViewerModel::metadata() const { return metadata_; }
rfcommon::VideoMeta* DataViewerModel::videoMetadata() const { return videoMetadata_; }
rfcommon::FrameData* DataViewerModel::frameData() const { return frameData_; }

// ----------------------------------------------------------------------------
QAbstractTableModel* DataViewerModel::baseStatusIDModel() const { return baseStatusIDModel_.get(); }
QAbstractTableModel* DataViewerModel::fighterIDModel() const { return fighterIDModel_.get(); }
QAbstractTableModel* DataViewerModel::hitStatusIDModel() const { return hitStatusIDModel_.get(); }
QAbstractItemModel* DataViewerModel::metadataModel() const { return metadataModel_.get(); }
QAbstractTableModel* DataViewerModel::videoMetadataModel() const { return videoMetadataModel_.get(); }
QAbstractTableModel* DataViewerModel::specificStatusIDModel() const { return specificStatusIDModel_.get(); }
QAbstractTableModel* DataViewerModel::stageIDModel() const { return stageIDModel_.get(); }

// ----------------------------------------------------------------------------
int DataViewerModel::fighterStatesModelCount() const { return fighterStatesModels_.size(); }
QAbstractTableModel* DataViewerModel::fighterStatesModel(int idx) const { return fighterStatesModels_[idx].get(); }

// ----------------------------------------------------------------------------
void DataViewerModel::onFrameDataNewUniqueFrame(int frameIdx, const rfcommon::Frame<4>& frame) {}
void DataViewerModel::onFrameDataNewFrame(int frameIdx, const rfcommon::Frame<4>& frame) { dispatcher.dispatch(&DataViewerListener::onNewFrame); }
