#include "frame-data-list/models/BaseStatusIDModel.hpp"
#include "frame-data-list/models/FighterIDModel.hpp"
#include "frame-data-list/models/FighterStatesModel.hpp"
#include "frame-data-list/models/FrameDataListModel.hpp"
#include "frame-data-list/models/HitStatusIDModel.hpp"
#include "frame-data-list/models/MetaDataModel.hpp"
#include "frame-data-list/models/SpecificStatusIDModel.hpp"
#include "frame-data-list/models/StageIDModel.hpp"
#include "frame-data-list/views/FrameDataListView.hpp"
#include "frame-data-list/listeners/FrameDataListListener.hpp"
#include "rfcommon/Frame.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/MetaData.hpp"

// ----------------------------------------------------------------------------
FrameDataListModel::FrameDataListModel()
    : baseStatusIDModel_(new BaseStatusIDModel)
    , fighterIDModel_(new FighterIDModel)
    , hitStatusIDModel_(new HitStatusIDModel)
    , metaDataModel_(new MetaDataModel)
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
    mappingInfo_ = session->tryGetMappingInfo();
    metaData_ = session->tryGetMetaData();
    frameData_ = session->tryGetFrameData();

    baseStatusIDModel_->setMappingInfo(mappingInfo_);
    fighterIDModel_->setMappingInfo(mappingInfo_);
    hitStatusIDModel_->setMappingInfo(mappingInfo_);
    metaDataModel_->setMetaData(mappingInfo_, metaData_);
    specificStatusIDModel_->setMappingInfo(mappingInfo_);
    stageIDModel_->setMappingInfo(mappingInfo_);

    fighterStatesModels_.clear();
    if (frameData_)
    {
        for (int i = 0; i != frameData_->fighterCount(); ++i)
            fighterStatesModels_.emplace_back(new FighterStatesModel(
                    frameData_, mappingInfo_, i, metaData_ ? metaData_->fighterID(i) : rfcommon::FighterID::makeInvalid()));
        frameData_->dispatcher.addListener(this);
    }

    dispatcher.dispatch(&FrameDataListListener::onNewData, mappingInfo_, metaData_, frameData_);
}

// ----------------------------------------------------------------------------
void FrameDataListModel::finalizeSession(rfcommon::Session* session)
{
    dispatcher.dispatch(&FrameDataListListener::onDataFinalized, mappingInfo_, metaData_, frameData_);

    if (frameData_)
        frameData_->dispatcher.removeListener(this);

    mappingInfo_.drop();
    metaData_.drop();
    frameData_.drop();
}

// ----------------------------------------------------------------------------
rfcommon::MappingInfo* FrameDataListModel::mappingInfo() const { return mappingInfo_; }
rfcommon::MetaData* FrameDataListModel::metaData() const { return metaData_; }
rfcommon::FrameData* FrameDataListModel::frameData() const { return frameData_; }

// ----------------------------------------------------------------------------
QAbstractTableModel* FrameDataListModel::baseStatusIDModel() const { return baseStatusIDModel_.get(); }
QAbstractTableModel* FrameDataListModel::fighterIDModel() const { return fighterIDModel_.get(); }
QAbstractTableModel* FrameDataListModel::hitStatusIDModel() const { return hitStatusIDModel_.get(); }
QAbstractTableModel* FrameDataListModel::metaDataModel() const { return metaDataModel_.get(); }
QAbstractTableModel* FrameDataListModel::specificStatusIDModel() const { return specificStatusIDModel_.get(); }
QAbstractTableModel* FrameDataListModel::stageIDModel() const { return stageIDModel_.get(); }

// ----------------------------------------------------------------------------
int FrameDataListModel::fighterStatesModelCount() const { return fighterStatesModels_.size(); }
QAbstractTableModel* FrameDataListModel::fighterStatesModel(int idx) const { return fighterStatesModels_[idx].get(); }

// ----------------------------------------------------------------------------
void FrameDataListModel::onFrameDataNewUniqueFrame(int frameIdx, const rfcommon::Frame<4>& frame) {}
void FrameDataListModel::onFrameDataNewFrame(int frameIdx, const rfcommon::Frame<4>& frame) { dispatcher.dispatch(&FrameDataListListener::onNewFrame); }
