#include "frame-data-list/models/FrameDataListModel.hpp"
#include "frame-data-list/models/MetaDataModel.hpp"
#include "frame-data-list/views/FrameDataListView.hpp"
#include "frame-data-list/listeners/FrameDataListListener.hpp"
#include "rfcommon/Frame.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/SessionMetaData.hpp"

// ----------------------------------------------------------------------------
FrameDataListModel::FrameDataListModel()
    : metaDataModel_(new MetaDataModel)
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

    metaDataModel_->setMetaData(mappingInfo_, metaData_);
    dispatcher.dispatch(&FrameDataListListener::onNewData, mappingInfo_, metaData_, frameData_);
}

// ----------------------------------------------------------------------------
void FrameDataListModel::clearSession(rfcommon::Session* session)
{
    metaDataModel_->clearMetaData(mappingInfo_, metaData_);
    dispatcher.dispatch(&FrameDataListListener::onDataFinalized, mappingInfo_, metaData_, frameData_);

    mappingInfo_.drop();
    metaData_.drop();
    frameData_.drop();
}

// ----------------------------------------------------------------------------
QAbstractTableModel* FrameDataListModel::metaDataModel() const
{
    return metaDataModel_.get();
}
