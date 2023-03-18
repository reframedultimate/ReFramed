#include "stats/listeners/PlayerMetaListener.hpp"
#include "stats/models/PlayerMeta.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/GameMetadata.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/MotionLabels.hpp"

// ----------------------------------------------------------------------------
PlayerMeta::PlayerMeta(rfcommon::MotionLabels* labels)
    : labels_(labels)
{
    labels_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
PlayerMeta::~PlayerMeta()
{
    if (mdata_)
        mdata_->dispatcher.removeListener(this);

    labels_->dispatcher.removeListener(this);
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
        label = labels_->toPreferredReadable(fighterID, motion);
    }

    // fallback to hash40 string
    if (label == nullptr)
        label = labels_->lookupHash40(motion);
    if (label == nullptr)
        return motion.toHex().cStr();

    return QString::fromUtf8(label);
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
void PlayerMeta::onMotionLabelsLoaded() { dispatcher.dispatch(&PlayerMetaListener::onPlayerMetaChanged); }
void PlayerMeta::onMotionLabelsHash40sUpdated() { dispatcher.dispatch(&PlayerMetaListener::onPlayerMetaChanged); }

void PlayerMeta::onMotionLabelsPreferredLayerChanged(int usage) { dispatcher.dispatch(&PlayerMetaListener::onPlayerMetaChanged); }

void PlayerMeta::onMotionLabelsLayerInserted(int layerIdx) { dispatcher.dispatch(&PlayerMetaListener::onPlayerMetaChanged); }
void PlayerMeta::onMotionLabelsLayerRemoved(int layerIdx) { dispatcher.dispatch(&PlayerMetaListener::onPlayerMetaChanged); }
void PlayerMeta::onMotionLabelsLayerNameChanged(int layerIdx) {}
void PlayerMeta::onMotionLabelsLayerUsageChanged(int layerIdx, int oldUsage) {}
void PlayerMeta::onMotionLabelsLayerMoved(int fromIdx, int toIdx) {}
void PlayerMeta::onMotionLabelsLayerMerged(int layerIdx) { dispatcher.dispatch(&PlayerMetaListener::onPlayerMetaChanged); }

void PlayerMeta::onMotionLabelsRowInserted(rfcommon::FighterID fighterID, int row) { dispatcher.dispatch(&PlayerMetaListener::onPlayerMetaChanged); }
void PlayerMeta::onMotionLabelsLabelChanged(rfcommon::FighterID fighterID, int row, int layerIdx) { dispatcher.dispatch(&PlayerMetaListener::onPlayerMetaChanged); }
void PlayerMeta::onMotionLabelsCategoryChanged(rfcommon::FighterID fighterID, int row, int oldCategory) {}
