#pragma once

#include "rfcommon/FrameDataListener.hpp"
#include "rfcommon/FighterID.hpp"
#include "rfcommon/Reference.hpp"
#include "rfcommon/UserMotionLabelsListener.hpp"
#include <QAbstractTableModel>

namespace rfcommon {
    class FrameData;
    class Hash40Strings;
    class MappingInfo;
    class UserMotionLabels;
}

class FighterStatesModel
        : public QAbstractTableModel
        , public rfcommon::FrameDataListener
        , public rfcommon::UserMotionLabelsListener
{
public:
    FighterStatesModel(
        rfcommon::FrameData* frameData,
        rfcommon::MappingInfo* mappingInfo,
        int fighterIdx, 
        rfcommon::FighterID fighterID, 
        rfcommon::UserMotionLabels* userLabels,
        rfcommon::Hash40Strings* hash40Strings);
    ~FighterStatesModel();

    int rowCount(const QModelIndex& parent=QModelIndex()) const override;
    int columnCount(const QModelIndex& parent=QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const override;
    QVariant data(const QModelIndex& index, int role=Qt::DisplayRole) const override;

private:
    void onFrameDataNewUniqueFrame(int frameIdx, const rfcommon::Frame<4>& frame) override;
    void onFrameDataNewFrame(int frameIdx, const rfcommon::Frame<4>& frame) override;

private:
    // user labels changing means we need to update the user labels column
    void updateMotionUserLabelsColumn();
    void onUserMotionLabelsLayerAdded(int layerIdx, const char* name) override;
    void onUserMotionLabelsLayerRemoved(int layerIdx, const char* name) override;
    void onUserMotionLabelsNewEntry(rfcommon::FighterID fighterID, int entryIdx) override;
    void onUserMotionLabelsUserLabelChanged(rfcommon::FighterID fighterID, int entryIdx, const char* oldLabel, const char* newLabel) override;
    void onUserMotionLabelsCategoryChanged(rfcommon::FighterID fighterID, int entryIdx, rfcommon::UserMotionLabelsCategory oldCategory, rfcommon::UserMotionLabelsCategory newCategory) override;

private:
    rfcommon::Reference<rfcommon::FrameData> frameData_;
    rfcommon::Reference<rfcommon::MappingInfo> mappingInfo_;
    rfcommon::Reference<rfcommon::UserMotionLabels> userLabels_;
    rfcommon::Reference<rfcommon::Hash40Strings> hash40Strings_;
    const int fighterIdx_;
    const rfcommon::FighterID fighterID_;
};
