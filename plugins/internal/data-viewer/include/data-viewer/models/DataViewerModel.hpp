#pragma once

#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/Reference.hpp"
#include "rfcommon/FrameDataListener.hpp"
#include <memory>
#include <vector>

namespace rfcommon {
    class FrameData;
    class Hash40Strings;
    class MappingInfo;
    class MetaData;
    class Session;
    class UserMotionLabels;
}

class QAbstractTableModel;

class BaseStatusIDModel;
class FighterIDModel;
class FighterStatesModel;
class DataViewerListener;
class HitStatusIDModel;
class MetaDataModel;
class SpecificStatusIDModel;
class StageIDModel;

class DataViewerModel : public rfcommon::FrameDataListener
{
public:
    DataViewerModel(rfcommon::UserMotionLabels* motionLabels, rfcommon::Hash40Strings* hash40Strings);
    ~DataViewerModel();

    void setSession(rfcommon::Session* session);
    void clearSession();

    rfcommon::MappingInfo* mappingInfo() const;
    rfcommon::MetaData* metaData() const;
    rfcommon::FrameData* frameData() const;

    QAbstractTableModel* baseStatusIDModel() const;
    QAbstractTableModel* fighterIDModel() const;
    QAbstractTableModel* hitStatusIDModel() const;
    QAbstractTableModel* metaDataModel() const;
    QAbstractTableModel* specificStatusIDModel() const;
    QAbstractTableModel* stageIDModel() const;

    int fighterStatesModelCount() const;
    QAbstractTableModel* fighterStatesModel(int idx) const;

    rfcommon::ListenerDispatcher<DataViewerListener> dispatcher;

private:
    void onFrameDataNewUniqueFrame(int frameIdx, const rfcommon::Frame<4>& frame) override;
    void onFrameDataNewFrame(int frameIdx, const rfcommon::Frame<4>& frame) override;

private:
    rfcommon::Reference<rfcommon::UserMotionLabels> userLabels_;
    rfcommon::Reference<rfcommon::Hash40Strings> hash40Strings_;

    std::unique_ptr<BaseStatusIDModel> baseStatusIDModel_;
    std::unique_ptr<FighterIDModel> fighterIDModel_;
    std::unique_ptr<HitStatusIDModel> hitStatusIDModel_;
    std::unique_ptr<MetaDataModel> metaDataModel_;
    std::unique_ptr<SpecificStatusIDModel> specificStatusIDModel_;
    std::unique_ptr<StageIDModel> stageIDModel_;

    std::vector<std::unique_ptr<FighterStatesModel>> fighterStatesModels_;

    rfcommon::Reference<rfcommon::MappingInfo> mappingInfo_;
    rfcommon::Reference<rfcommon::MetaData> metaData_;
    rfcommon::Reference<rfcommon::FrameData> frameData_;
};
