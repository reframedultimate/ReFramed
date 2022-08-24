#include "stats/listeners/PlayerMetaListener.hpp"
#include "stats/models/PlayerMeta.hpp"
#include "rfcommon/Hash40Strings.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/MetaData.hpp"
#include "rfcommon/UserMotionLabels.hpp"

// ----------------------------------------------------------------------------
PlayerMeta::PlayerMeta(rfcommon::UserMotionLabels* userLabels, rfcommon::Hash40Strings* hash40Strings)
    : userLabels_(userLabels)
    , hash40Strings_(hash40Strings)
{
    userLabels_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
PlayerMeta::~PlayerMeta()
{
    if (metaData_)
        metaData_->dispatcher.removeListener(this);

    userLabels_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void PlayerMeta::setMetaData(rfcommon::MappingInfo* map, rfcommon::MetaData* mdata)
{
    clearMetaData();

    players_.resize(mdata->fighterCount());
    for (int i = 0; i != mdata->fighterCount(); ++i)
    {
        players_[i].name = mdata->name(i).cStr();
        players_[i].character = map->fighter.toName(mdata->fighterID(i));
    }

    metaData_ = mdata;
    metaData_->dispatcher.addListener(this);

    dispatcher.dispatch(&PlayerMetaListener::onPlayerMetaChanged);
}

// ----------------------------------------------------------------------------
void PlayerMeta::clearMetaData()
{
    if (metaData_)
        metaData_->dispatcher.removeListener(this);

    players_.clear();
    dispatcher.dispatch(&PlayerMetaListener::onPlayerMetaChanged);
}

// ----------------------------------------------------------------------------
QString PlayerMeta::name(int fighterIdx) const
{
    return players_.count() ?
        players_[fighterIdx].name :
        QString("Player ") + QString::number(fighterIdx + 1);
}

// ----------------------------------------------------------------------------
QString PlayerMeta::character(int fighterIdx) const
{
    return players_.count() ?
        players_[fighterIdx].character :
        "";
}

// ----------------------------------------------------------------------------
QString PlayerMeta::moveName(int fighterIdx, rfcommon::FighterMotion motion) const
{
    const char* label = nullptr;

    if (motion.isValid() == false)
        return "None";

    if (metaData_.notNull())
    {
        const auto fighterID = metaData_->fighterID(fighterIdx);
        label = userLabels_->toStringHighestLayer(fighterID, motion, nullptr);
    }

    if (label == nullptr)
        label = hash40Strings_->toString(motion, "(unknown move)");

    return label;
}

// ----------------------------------------------------------------------------
void PlayerMeta::onMetaDataPlayerNameChanged(int fighterIdx, const rfcommon::String& name)
{
    players_[fighterIdx].name = name.cStr();
    dispatcher.dispatch(&PlayerMetaListener::onPlayerMetaChanged);
}

// ----------------------------------------------------------------------------
void PlayerMeta::onMetaDataTimeStartedChanged(rfcommon::TimeStamp timeStarted)  {}
void PlayerMeta::onMetaDataTimeEndedChanged(rfcommon::TimeStamp timeEnded) {}
void PlayerMeta::onMetaDataSetNumberChanged(rfcommon::SetNumber number) {}
void PlayerMeta::onMetaDataGameNumberChanged(rfcommon::GameNumber number) {}
void PlayerMeta::onMetaDataSetFormatChanged(const rfcommon::SetFormat& format) {}
void PlayerMeta::onMetaDataWinnerChanged(int winnerPlayerIdx) {}
void PlayerMeta::onMetaDataTrainingSessionNumberChanged(rfcommon::GameNumber number) {}

// ----------------------------------------------------------------------------
void PlayerMeta::onUserMotionLabelsLayerAdded(int layerIdx, const char* name) { dispatcher.dispatch(&PlayerMetaListener::onPlayerMetaChanged); }
void PlayerMeta::onUserMotionLabelsLayerRemoved(int layerIdx, const char* name) { dispatcher.dispatch(&PlayerMetaListener::onPlayerMetaChanged); }
void PlayerMeta::onUserMotionLabelsNewEntry(rfcommon::FighterID fighterID, int entryIdx) { dispatcher.dispatch(&PlayerMetaListener::onPlayerMetaChanged); }
void PlayerMeta::onUserMotionLabelsUserLabelChanged(rfcommon::FighterID fighterID, int entryIdx, const char* oldLabel, const char* newLabel) { dispatcher.dispatch(&PlayerMetaListener::onPlayerMetaChanged); }
void PlayerMeta::onUserMotionLabelsCategoryChanged(rfcommon::FighterID fighterID, int entryIdx, rfcommon::UserMotionLabelsCategory oldCategory, rfcommon::UserMotionLabelsCategory newCategory) { dispatcher.dispatch(&PlayerMetaListener::onPlayerMetaChanged); }
