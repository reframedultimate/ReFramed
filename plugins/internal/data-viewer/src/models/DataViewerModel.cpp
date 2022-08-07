#include "data-viewer/models/BaseStatusIDModel.hpp"
#include "data-viewer/models/FighterIDModel.hpp"
#include "data-viewer/models/FighterStatesModel.hpp"
#include "data-viewer/models/DataViewerModel.hpp"
#include "data-viewer/models/HitStatusIDModel.hpp"
#include "data-viewer/models/MetaDataModel.hpp"
#include "data-viewer/models/SpecificStatusIDModel.hpp"
#include "data-viewer/models/StageIDModel.hpp"
#include "data-viewer/views/DataViewerView.hpp"
#include "data-viewer/listeners/DataViewerListener.hpp"
#include "rfcommon/Frame.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/MetaData.hpp"

// ----------------------------------------------------------------------------
DataViewerModel::DataViewerModel()
    : baseStatusIDModel_(new BaseStatusIDModel)
    , fighterIDModel_(new FighterIDModel)
    , hitStatusIDModel_(new HitStatusIDModel)
    , metaDataModel_(new MetaDataModel)
    , specificStatusIDModel_(new SpecificStatusIDModel)
    , stageIDModel_(new StageIDModel)
{
}

// ----------------------------------------------------------------------------
DataViewerModel::~DataViewerModel()
{
}

// ----------------------------------------------------------------------------
void DataViewerModel::setSession(rfcommon::Session* session)
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

    dispatcher.dispatch(&DataViewerListener::onNewData, mappingInfo_, metaData_, frameData_);
}

// ----------------------------------------------------------------------------
void DataViewerModel::clearSession()
{
    dispatcher.dispatch(&DataViewerListener::onClear);

    baseStatusIDModel_->setMappingInfo(nullptr);
    fighterIDModel_->setMappingInfo(nullptr);
    hitStatusIDModel_->setMappingInfo(nullptr);
    metaDataModel_->setMetaData(nullptr, nullptr);
    specificStatusIDModel_->setMappingInfo(nullptr);
    stageIDModel_->setMappingInfo(nullptr);

    if (frameData_)
        frameData_->dispatcher.removeListener(this);

    mappingInfo_.drop();
    metaData_.drop();
    frameData_.drop();
}

// ----------------------------------------------------------------------------
rfcommon::MappingInfo* DataViewerModel::mappingInfo() const { return mappingInfo_; }
rfcommon::MetaData* DataViewerModel::metaData() const { return metaData_; }
rfcommon::FrameData* DataViewerModel::frameData() const { return frameData_; }

// ----------------------------------------------------------------------------
QAbstractTableModel* DataViewerModel::baseStatusIDModel() const { return baseStatusIDModel_.get(); }
QAbstractTableModel* DataViewerModel::fighterIDModel() const { return fighterIDModel_.get(); }
QAbstractTableModel* DataViewerModel::hitStatusIDModel() const { return hitStatusIDModel_.get(); }
QAbstractTableModel* DataViewerModel::metaDataModel() const { return metaDataModel_.get(); }
QAbstractTableModel* DataViewerModel::specificStatusIDModel() const { return specificStatusIDModel_.get(); }
QAbstractTableModel* DataViewerModel::stageIDModel() const { return stageIDModel_.get(); }

// ----------------------------------------------------------------------------
int DataViewerModel::fighterStatesModelCount() const { return fighterStatesModels_.size(); }
QAbstractTableModel* DataViewerModel::fighterStatesModel(int idx) const { return fighterStatesModels_[idx].get(); }

// ----------------------------------------------------------------------------
void DataViewerModel::onFrameDataNewUniqueFrame(int frameIdx, const rfcommon::Frame<4>& frame) {}
void DataViewerModel::onFrameDataNewFrame(int frameIdx, const rfcommon::Frame<4>& frame) { dispatcher.dispatch(&DataViewerListener::onNewFrame); }
