#pragma once

#include "rfcommon/FrameDataListener.hpp"
#include "rfcommon/FighterID.hpp"
#include "rfcommon/Reference.hpp"
#include "rfcommon/MotionLabelsListener.hpp"
#include <QAbstractTableModel>

namespace rfcommon {
    class FrameData;
    class Hash40Strings;
    class MappingInfo;
    class MotionLabels;
}

class FighterStatesModel
        : public QAbstractTableModel
        , public rfcommon::FrameDataListener
        , public rfcommon::MotionLabelsListener
{
public:
    FighterStatesModel(
        rfcommon::FrameData* frameData,
        rfcommon::MappingInfo* mappingInfo,
        int fighterIdx,
        rfcommon::FighterID fighterID,
        rfcommon::MotionLabels* labels);
    ~FighterStatesModel();

    int rowCount(const QModelIndex& parent=QModelIndex()) const override;
    int columnCount(const QModelIndex& parent=QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const override;
    QVariant data(const QModelIndex& index, int role=Qt::DisplayRole) const override;

private:
    void onFrameDataNewUniqueFrame(int frameIdx, const rfcommon::Frame<4>& frame) override;
    void onFrameDataNewFrame(int frameIdx, const rfcommon::Frame<4>& frame) override;

private:
    // Labels changing means we need to update the labels column
    void updateMotionLabelsColumn();
    void updateHash40StringsColumn();

    void onMotionLabelsLoaded() override;
    void onMotionLabelsHash40sUpdated() override;

    void onMotionLabelsLayerInserted(int layerIdx) override;
    void onMotionLabelsLayerRemoved(int layerIdx) override;
    void onMotionLabelsLayerNameChanged(int layerIdx) override;
    void onMotionLabelsLayerUsageChanged(int layerIdx, int oldUsage) override;
    void onMotionLabelsLayerMoved(int fromIdx, int toIdx) override;
    void onMotionLabelsLayerMerged(int layerIdx) override;

    void onMotionLabelsRowInserted(rfcommon::FighterID fighterID, int row) override;
    void onMotionLabelsLabelChanged(rfcommon::FighterID fighterID, int row, int layerIdx) override;
    void onMotionLabelsCategoryChanged(rfcommon::FighterID fighterID, int row, int oldCategory) override;

private:
    rfcommon::Reference<rfcommon::FrameData> frameData_;
    rfcommon::Reference<rfcommon::MappingInfo> mappingInfo_;
    rfcommon::Reference<rfcommon::MotionLabels> labels_;
    const int fighterIdx_;
    const rfcommon::FighterID fighterID_;
};
