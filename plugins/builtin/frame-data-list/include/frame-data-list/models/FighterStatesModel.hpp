#pragma once

#include <QAbstractTableModel>
#include "rfcommon/Reference.hpp"
#include "rfcommon/FrameDataListener.hpp"
#include "rfcommon/FighterID.hpp"

namespace rfcommon {
    class FrameData;
    class MappingInfo;
}

class FighterStatesModel
        : public QAbstractTableModel
        , public rfcommon::FrameDataListener
{
public:
    FighterStatesModel(rfcommon::FrameData* frameData, rfcommon::MappingInfo* mappingInfo, int fighterIdx, rfcommon::FighterID fighterID);
    ~FighterStatesModel();

    int rowCount(const QModelIndex& parent=QModelIndex()) const override;
    int columnCount(const QModelIndex& parent=QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const override;
    QVariant data(const QModelIndex& index, int role=Qt::DisplayRole) const override;

private:
    void onFrameDataNewUniqueFrame(int frameIdx, const rfcommon::Frame& frame) override;
    void onFrameDataNewFrame(int frameIdx, const rfcommon::Frame& frame) override;

private:
    rfcommon::Reference<rfcommon::FrameData> frameData_;
    rfcommon::Reference<rfcommon::MappingInfo> mappingInfo_;
    const int fighterIdx_;
    const rfcommon::FighterID fighterID_;
};
