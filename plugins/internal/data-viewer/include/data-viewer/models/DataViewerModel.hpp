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
    class Metadata;
    class MotionLabels;
    class Session;
    class VideoMeta;
}

class QAbstractTableModel;
class QAbstractItemModel;

class BaseStatusIDModel;
class DataViewerListener;
class FighterIDModel;
class FighterStatesModel;
class HitStatusIDModel;
class MetadataModel;
class SpecificStatusIDModel;
class StageIDModel;
class VideoMetadataModel;

class DataViewerModel : public rfcommon::FrameDataListener
{
public:
    DataViewerModel(rfcommon::MotionLabels* labels);
    ~DataViewerModel();

    void setSession(rfcommon::Session* session);
    void clearSession();

    rfcommon::MappingInfo* mappingInfo() const;
    rfcommon::Metadata* metadata() const;
    rfcommon::VideoMeta* videoMetadata() const;
    rfcommon::FrameData* frameData() const;

    QAbstractTableModel* baseStatusIDModel() const;
    QAbstractTableModel* fighterIDModel() const;
    QAbstractTableModel* hitStatusIDModel() const;
    QAbstractItemModel* metadataModel() const;
    QAbstractTableModel* videoMetadataModel() const;
    QAbstractTableModel* specificStatusIDModel() const;
    QAbstractTableModel* stageIDModel() const;

    int fighterStatesModelCount() const;
    QAbstractTableModel* fighterStatesModel(int idx) const;

    rfcommon::ListenerDispatcher<DataViewerListener> dispatcher;

private:
    void onFrameDataNewUniqueFrame(int frameIdx, const rfcommon::Frame<4>& frame) override;
    void onFrameDataNewFrame(int frameIdx, const rfcommon::Frame<4>& frame) override;

private:
    rfcommon::Reference<rfcommon::MotionLabels> labels_;

    std::unique_ptr<BaseStatusIDModel> baseStatusIDModel_;
    std::unique_ptr<FighterIDModel> fighterIDModel_;
    std::unique_ptr<HitStatusIDModel> hitStatusIDModel_;
    std::unique_ptr<MetadataModel> metadataModel_;
    std::unique_ptr<VideoMetadataModel> videoMetadataModel_;
    std::unique_ptr<SpecificStatusIDModel> specificStatusIDModel_;
    std::unique_ptr<StageIDModel> stageIDModel_;

    std::vector<std::unique_ptr<FighterStatesModel>> fighterStatesModels_;

    rfcommon::Reference<rfcommon::MappingInfo> mappingInfo_;
    rfcommon::Reference<rfcommon::Metadata> metadata_;
    rfcommon::Reference<rfcommon::VideoMeta> videoMetadata_;
    rfcommon::Reference<rfcommon::FrameData> frameData_;
};
