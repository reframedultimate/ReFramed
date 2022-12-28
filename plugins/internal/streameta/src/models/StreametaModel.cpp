#include "rfcommon/Profiler.hpp"
#include "streameta/models/StreametaModel.hpp"

// ----------------------------------------------------------------------------
FrameDataListModel::FrameDataListModel()
    : baseStatusIDModel_(new BaseStatusIDModel)
    , fighterIDModel_(new FighterIDModel)
    , hitStatusIDModel_(new HitStatusIDModel)
    , metadataModel_(new MetadataModel)
    , specificStatusIDModel_(new SpecificStatusIDModel)
    , stageIDModel_(new StageIDModel)
{
}

// ----------------------------------------------------------------------------
FrameDataListModel::~FrameDataListModel()
{
}

// ----------------------------------------------------------------------------
void FrameDataListModel::setSession(rfcommon::Session* session)
{
    PROFILE(FrameDataListModel, setSession);

    mappingInfo_ = session->tryGetMappingInfo();
    metadata_ = session->tryGetMetadata();
    frameData_ = session->tryGetFrameData();

    baseStatusIDModel_->setMappingInfo(mappingInfo_);
    fighterIDModel_->setMappingInfo(mappingInfo_);
    hitStatusIDModel_->setMappingInfo(mappingInfo_);
    metadataModel_->setMetadata(mappingInfo_, metadata_);
    specificStatusIDModel_->setMappingInfo(mappingInfo_);
    stageIDModel_->setMappingInfo(mappingInfo_);

    fighterStatesModels_.clear();
    if (frameData_)
    {
        for (int i = 0; i != frameData_->fighterCount(); ++i)
            fighterStatesModels_.emplace_back(new FighterStatesModel(
                    frameData_, mappingInfo_, i, metadata_ ? metadata_->fighterID(i) : rfcommon::FighterID::makeInvalid()));
        frameData_->dispatcher.addListener(this);
    }

    dispatcher.dispatch(&FrameDataListListener::onNewData, mappingInfo_, metadata_, frameData_);
}

// ----------------------------------------------------------------------------
void FrameDataListModel::finalizeSession(rfcommon::Session* session)
{
    PROFILE(FrameDataListModel, finalizeSession);

    dispatcher.dispatch(&FrameDataListListener::onDataFinalized, mappingInfo_, metadata_, frameData_);

    if (frameData_)
        frameData_->dispatcher.removeListener(this);

    mappingInfo_.drop();
    metadata_.drop();
    frameData_.drop();
}

// ----------------------------------------------------------------------------
rfcommon::MappingInfo* FrameDataListModel::mappingInfo() const { return mappingInfo_; }
rfcommon::Metadata* FrameDataListModel::metadata() const { return metadata_; }
rfcommon::FrameData* FrameDataListModel::frameData() const { return frameData_; }

// ----------------------------------------------------------------------------
QAbstractTableModel* FrameDataListModel::baseStatusIDModel() const { return baseStatusIDModel_.get(); }
QAbstractTableModel* FrameDataListModel::fighterIDModel() const { return fighterIDModel_.get(); }
QAbstractTableModel* FrameDataListModel::hitStatusIDModel() const { return hitStatusIDModel_.get(); }
QAbstractTableModel* FrameDataListModel::metadataModel() const { return metadataModel_.get(); }
QAbstractTableModel* FrameDataListModel::specificStatusIDModel() const { return specificStatusIDModel_.get(); }
QAbstractTableModel* FrameDataListModel::stageIDModel() const { return stageIDModel_.get(); }

// ----------------------------------------------------------------------------
int FrameDataListModel::fighterStatesModelCount() const { return fighterStatesModels_.size(); }
QAbstractTableModel* FrameDataListModel::fighterStatesModel(int idx) const { return fighterStatesModels_[idx].get(); }

// ----------------------------------------------------------------------------
void FrameDataListModel::onFrameDataNewUniqueFrame(int frameIdx, const rfcommon::Frame<4>& frame) {}
void FrameDataListModel::onFrameDataNewFrame(int frameIdx, const rfcommon::Frame<4>& frame) { dispatcher.dispatch(&FrameDataListListener::onNewFrame); }
