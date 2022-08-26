#pragma once

#include "rfcommon/FighterMotion.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/MetaDataListener.hpp"
#include "rfcommon/Reference.hpp"
#include "rfcommon/UserMotionLabelsListener.hpp"
#include <QString>

class PlayerMetaListener;

namespace rfcommon {
    class Hash40Strings;
    class MappingInfo;
    class MetaData;
    class GameMetaData;
    class UserMotionLabels;
}

class PlayerMeta 
        : public rfcommon::MetaDataListener
        , public rfcommon::UserMotionLabelsListener
{
public:
    PlayerMeta(rfcommon::UserMotionLabels* userLabels, rfcommon::Hash40Strings* hash40Strings);
    ~PlayerMeta();

    void setMetaData(rfcommon::MappingInfo* map, rfcommon::MetaData* mdata);
    void clearMetaData();

    rfcommon::GameMetaData* latestMetaData() const;

    int playerCount() const;
    QString name(int fighterIdx) const;
    QString tag(int fighterIdx) const;
    QString character(int fighterIdx) const;
    QString moveName(int fighterIdx, rfcommon::FighterMotion motion) const;

    rfcommon::ListenerDispatcher<PlayerMetaListener> dispatcher;

private:
    void onMetaDataTimeStartedChanged(rfcommon::TimeStamp timeStarted) override;
    void onMetaDataTimeEndedChanged(rfcommon::TimeStamp timeEnded) override;

    // Game related events
    void onMetaDataPlayerNameChanged(int fighterIdx, const rfcommon::String& name) override;
    void onMetaDataSetNumberChanged(rfcommon::SetNumber number) override;
    void onMetaDataGameNumberChanged(rfcommon::GameNumber number) override;
    void onMetaDataSetFormatChanged(const rfcommon::SetFormat& format) override;
    void onMetaDataWinnerChanged(int winnerPlayerIdx) override;

    // In training mode this increments every time a new training room is loaded
    void onMetaDataTrainingSessionNumberChanged(rfcommon::GameNumber number) override;

private:
    // If motion labels are edited, we need to update the table (and possibly re-export data)
    void onUserMotionLabelsLayerAdded(int layerIdx, const char* name) override;
    void onUserMotionLabelsLayerRemoved(int layerIdx, const char* name) override;
    void onUserMotionLabelsNewEntry(rfcommon::FighterID fighterID, int entryIdx) override;
    void onUserMotionLabelsUserLabelChanged(rfcommon::FighterID fighterID, int entryIdx, const char* oldLabel, const char* newLabel) override;
    void onUserMotionLabelsCategoryChanged(rfcommon::FighterID fighterID, int entryIdx, rfcommon::UserMotionLabelsCategory oldCategory, rfcommon::UserMotionLabelsCategory newCategory) override;

private:
    rfcommon::Reference<rfcommon::MetaData> mdata_;
    rfcommon::Reference<rfcommon::MappingInfo> map_;
    rfcommon::Reference<rfcommon::UserMotionLabels> userLabels_;
    rfcommon::Reference<rfcommon::Hash40Strings> hash40Strings_;
};
