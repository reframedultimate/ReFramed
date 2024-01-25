#pragma once

#include "rfcommon/FighterMotion.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/MetadataListener.hpp"
#include "rfcommon/Reference.hpp"
#include "rfcommon/MotionLabelsListener.hpp"
#include <QString>

class PlayerMetaListener;

namespace rfcommon {
    class Hash40Strings;
    class MappingInfo;
    class Metadata;
    class GameMetadata;
    class MotionLabels;
}

class PlayerMeta
        : public rfcommon::MetadataListener
        , public rfcommon::MotionLabelsListener
{
public:
    PlayerMeta(rfcommon::MotionLabels* labels);
    ~PlayerMeta();

    void setMetadata(rfcommon::MappingInfo* map, rfcommon::Metadata* mdata);
    void clearMetadata();

    rfcommon::MappingInfo* latestMappingInfo() const;
    rfcommon::GameMetadata* latestMetadata() const;

    int playerCount() const;
    QString name(int fighterIdx) const;
    QString tag(int fighterIdx) const;
    QString character(int fighterIdx) const;
    QString moveName(int fighterIdx, rfcommon::FighterMotion motion) const;

    rfcommon::ListenerDispatcher<PlayerMetaListener> dispatcher;

private:
    void onMetadataTimeChanged(rfcommon::TimeStamp timeStarted, rfcommon::TimeStamp timeEnded) override;
    void onMetadataTournamentDetailsChanged() override;
    void onMetadataEventDetailsChanged() override;
    void onMetadataCommentatorsChanged() override;
    void onMetadataGameDetailsChanged() override;
    void onMetadataPlayerDetailsChanged() override;
    void onMetadataWinnerChanged(int winnerPlayerIdx) override;
    void onMetadataTrainingSessionNumberChanged(rfcommon::SessionNumber number) override;

private:
    // If motion labels are edited, we need to update the table (and possibly re-export data)
    void onMotionLabelsLoaded() override;
    void onMotionLabelsHash40sUpdated() override;

    void onMotionLabelsPreferredLayerChanged(int usage) override;

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
    rfcommon::Reference<rfcommon::Metadata> mdata_;
    rfcommon::Reference<rfcommon::MappingInfo> map_;
    rfcommon::Reference<rfcommon::MotionLabels> labels_;
};
