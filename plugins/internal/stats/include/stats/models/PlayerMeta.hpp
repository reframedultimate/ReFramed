#pragma once

#include "rfcommon/FighterMotion.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/MetadataListener.hpp"
#include "rfcommon/Reference.hpp"
#include "rfcommon/UserMotionLabelsListener.hpp"
#include <QString>

class PlayerMetaListener;

namespace rfcommon {
    class Hash40Strings;
    class MappingInfo;
    class Metadata;
    class GameMetadata;
    class UserMotionLabels;
}

class PlayerMeta
        : public rfcommon::MetadataListener
        , public rfcommon::UserMotionLabelsListener
{
public:
    PlayerMeta(rfcommon::UserMotionLabels* userLabels, rfcommon::Hash40Strings* hash40Strings);
    ~PlayerMeta();

    void setMetadata(rfcommon::MappingInfo* map, rfcommon::Metadata* mdata);
    void clearMetadata();

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
    void onUserMotionLabelsLayerAdded(int layerIdx, const char* name) override;
    void onUserMotionLabelsLayerRemoved(int layerIdx, const char* name) override;
    void onUserMotionLabelsNewEntry(rfcommon::FighterID fighterID, int entryIdx) override;
    void onUserMotionLabelsUserLabelChanged(rfcommon::FighterID fighterID, int entryIdx, const char* oldLabel, const char* newLabel) override;
    void onUserMotionLabelsCategoryChanged(rfcommon::FighterID fighterID, int entryIdx, rfcommon::UserMotionLabelsCategory oldCategory, rfcommon::UserMotionLabelsCategory newCategory) override;

private:
    rfcommon::Reference<rfcommon::Metadata> mdata_;
    rfcommon::Reference<rfcommon::MappingInfo> map_;
    rfcommon::Reference<rfcommon::UserMotionLabels> userLabels_;
    rfcommon::Reference<rfcommon::Hash40Strings> hash40Strings_;
};
