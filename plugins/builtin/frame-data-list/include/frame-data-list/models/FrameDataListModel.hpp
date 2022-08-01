#pragma once

#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/Reference.hpp"
#include <memory>

namespace rfcommon {
    class FrameData;
    class MappingInfo;
    class Session;
    class SessionMetaData;
}

class QAbstractTableModel;

class FrameDataListListener;
class MetaDataModel;

class FrameDataListModel
{
public:
    FrameDataListModel();
    ~FrameDataListModel();

    void setSession(rfcommon::Session* session);
    void clearSession(rfcommon::Session* session);

    QAbstractTableModel* metaDataModel() const;

    rfcommon::ListenerDispatcher<FrameDataListListener> dispatcher;

private:
    std::unique_ptr<MetaDataModel> metaDataModel_;
    rfcommon::Reference<rfcommon::MappingInfo> mappingInfo_;
    rfcommon::Reference<rfcommon::SessionMetaData> metaData_;
    rfcommon::Reference<rfcommon::FrameData> frameData_;
};
