#include "stats/listeners/PlayerMetaListener.hpp"
#include "stats/models/PlayerMeta.hpp"
#include "rfcommon/Hash40Strings.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/GameMetadata.hpp"
#include "rfcommon/Profiler.hpp"
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
    if (mdata_)
        mdata_->dispatcher.removeListener(this);

    userLabels_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void PlayerMeta::setMetadata(rfcommon::MappingInfo* map, rfcommon::Metadata* mdata)
{
    PROFILE(PlayerMeta, setMetadata);

    clearMetadata();

    map_ = map;
    mdata_ = mdata;
    mdata_->dispatcher.addListener(this);

    dispatcher.dispatch(&PlayerMetaListener::onPlayerMetaChanged);
}

// ----------------------------------------------------------------------------
void PlayerMeta::clearMetadata()
{
    PROFILE(PlayerMeta, clearMetadata);

    if (mdata_)
        mdata_->dispatcher.removeListener(this);
    mdata_.drop();

    dispatcher.dispatch(&PlayerMetaListener::onPlayerMetaChanged);
}

// ----------------------------------------------------------------------------
rfcommon::GameMetadata* PlayerMeta::latestMetadata() const
{
    PROFILE(PlayerMeta, latestMetadata);

    if (mdata_.isNull())
        return nullptr;
    if (mdata_->type() != rfcommon::Metadata::GAME)
        return nullptr;
    return mdata_->asGame();
}

// ----------------------------------------------------------------------------
int PlayerMeta::playerCount() const
{
    PROFILE(PlayerMeta, playerCount);

    return mdata_ ? mdata_->fighterCount() : 0;
}

// ----------------------------------------------------------------------------
QString PlayerMeta::name(int fighterIdx) const
{
    PROFILE(PlayerMeta, name);

    return mdata_ ?
        mdata_->asGame()->playerName(fighterIdx).cStr() :
        QString("Player ") + QString::number(fighterIdx + 1);
}

// ----------------------------------------------------------------------------
QString PlayerMeta::tag(int fighterIdx) const
{
    PROFILE(PlayerMeta, tag);

    return mdata_ ?
        mdata_->playerTag(fighterIdx).cStr() :
        QString("Player ") + QString::number(fighterIdx + 1);
}

// ----------------------------------------------------------------------------
QString PlayerMeta::character(int fighterIdx) const
{
    PROFILE(PlayerMeta, character);

    return mdata_ ?
        map_->fighter.toName(mdata_->playerFighterID(fighterIdx)) :
        QString("Player ") + QString::number(fighterIdx + 1);
}

// ----------------------------------------------------------------------------
QString PlayerMeta::moveName(int fighterIdx, rfcommon::FighterMotion motion) const
{
    PROFILE(PlayerMeta, moveName);

    const char* label = nullptr;

    if (motion.isValid() == false)
        return "None";

    if (mdata_.notNull())
    {
        const auto fighterID = mdata_->playerFighterID(fighterIdx);
        label = userLabels_->toStringHighestLayer(fighterID, motion, nullptr);
    }

    if (label == nullptr)
        label = hash40Strings_->toString(motion, "(unknown move)");

    return label;
}

// ----------------------------------------------------------------------------
void PlayerMeta::onMetadataTimeChanged(rfcommon::TimeStamp timeStarted, rfcommon::TimeStamp timeEnded) {}
void PlayerMeta::onMetadataTournamentDetailsChanged() {}
void PlayerMeta::onMetadataEventDetailsChanged() {}
void PlayerMeta::onMetadataCommentatorsChanged() {}
void PlayerMeta::onMetadataGameDetailsChanged() {}
void PlayerMeta::onMetadataPlayerDetailsChanged()
{
    PROFILE(PlayerMeta, onMetadataPlayerDetailsChanged);

    dispatcher.dispatch(&PlayerMetaListener::onPlayerMetaChanged);
}
void PlayerMeta::onMetadataWinnerChanged(int winnerPlayerIdx) {}
void PlayerMeta::onMetadataTrainingSessionNumberChanged(rfcommon::SessionNumber number) {}

// ----------------------------------------------------------------------------
void PlayerMeta::onUserMotionLabelsLayerAdded(int layerIdx, const char* name) { dispatcher.dispatch(&PlayerMetaListener::onPlayerMetaChanged); }
void PlayerMeta::onUserMotionLabelsLayerRemoved(int layerIdx, const char* name) { dispatcher.dispatch(&PlayerMetaListener::onPlayerMetaChanged); }
void PlayerMeta::onUserMotionLabelsNewEntry(rfcommon::FighterID fighterID, int entryIdx) { dispatcher.dispatch(&PlayerMetaListener::onPlayerMetaChanged); }
void PlayerMeta::onUserMotionLabelsUserLabelChanged(rfcommon::FighterID fighterID, int entryIdx, const char* oldLabel, const char* newLabel) { dispatcher.dispatch(&PlayerMetaListener::onPlayerMetaChanged); }
void PlayerMeta::onUserMotionLabelsCategoryChanged(rfcommon::FighterID fighterID, int entryIdx, rfcommon::UserMotionLabelsCategory oldCategory, rfcommon::UserMotionLabelsCategory newCategory) { dispatcher.dispatch(&PlayerMetaListener::onPlayerMetaChanged); }
