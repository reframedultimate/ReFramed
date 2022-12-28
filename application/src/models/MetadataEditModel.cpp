#include "application/models/MetadataEditModel.hpp"
#include "application/listeners/MetadataEditListener.hpp"

#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/Metadata.hpp"

namespace rfapp {

// ----------------------------------------------------------------------------
MetadataEditModel::MetadataEditModel()
{}

// ----------------------------------------------------------------------------
MetadataEditModel::~MetadataEditModel()
{
    if (mdata_.count())
        for (auto& m : mdata_)
            m->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void MetadataEditModel::setAndAdopt(MappingInfoList&& map, MetadataList&& mdata)
{
    clear();

    map_ = map;
    mdata_ = mdata;
    for (auto& m : mdata_)
        m->dispatcher.addListener(this);

    dispatcher.dispatch(&MetadataEditListener::onAdoptMetadata, map, mdata);
}

// ----------------------------------------------------------------------------
void MetadataEditModel::setAndOverwrite(MappingInfoList&& map, MetadataList&& mdata)
{
    clear();

    map_ = map;
    mdata_ = mdata;
    for (auto& m : mdata_)
        m->dispatcher.addListener(this);

    dispatcher.dispatch(&MetadataEditListener::onOverwriteMetadata, map, mdata);
}

// ----------------------------------------------------------------------------
void MetadataEditModel::clear()
{
    if (mdata_.count() == 0)
        return;

    dispatcher.dispatch(&MetadataEditListener::onMetadataCleared, map_, mdata_);

    if (mdata_.count() == 1)
        prevMdata_ = mdata_[0];
    else
        prevMdata_.drop();

    for (auto& m : mdata_)
        m->dispatcher.removeListener(this);
    mdata_.clear();
    map_.clear();
}

// ----------------------------------------------------------------------------
void MetadataEditModel::startNextGame()
{
    if (prevMdata_.isNull())
        return;
    if (mdata_.count() != 1)
        return;

    if (pendingChanges_)
    {
        pendingChanges_ = false;
        return;
    }

    dispatcher.dispatch(&MetadataEditListener::onNextGameStarted);
}

// ----------------------------------------------------------------------------
void MetadataEditModel::onMetadataTimeChanged(rfcommon::TimeStamp timeStarted, rfcommon::TimeStamp timeEnded)
{
    dispatcher.dispatch(&MetadataEditListener::onMetadataTimeChanged, timeStarted, timeEnded);
}

// ----------------------------------------------------------------------------
void MetadataEditModel::onMetadataTournamentDetailsChanged()
{
    dispatcher.dispatch(&MetadataEditListener::onMetadataTournamentDetailsChanged);
}

// ----------------------------------------------------------------------------
void MetadataEditModel::onMetadataEventDetailsChanged()
{
    dispatcher.dispatch(&MetadataEditListener::onMetadataEventDetailsChanged);
}

// ----------------------------------------------------------------------------
void MetadataEditModel::onMetadataCommentatorsChanged()
{
    dispatcher.dispatch(&MetadataEditListener::onMetadataCommentatorsChanged);
}

// ----------------------------------------------------------------------------
void MetadataEditModel::onMetadataGameDetailsChanged()
{
    dispatcher.dispatch(&MetadataEditListener::onMetadataGameDetailsChanged);
}

// ----------------------------------------------------------------------------
void MetadataEditModel::onMetadataPlayerDetailsChanged()
{
    dispatcher.dispatch(&MetadataEditListener::onMetadataPlayerDetailsChanged);
}

// ----------------------------------------------------------------------------
void MetadataEditModel::onMetadataWinnerChanged(int winnerPlayerIdx)
{
    dispatcher.dispatch(&MetadataEditListener::onMetadataWinnerChanged, winnerPlayerIdx);
}

// ----------------------------------------------------------------------------
void MetadataEditModel::onMetadataTrainingSessionNumberChanged(rfcommon::SessionNumber number)
{
    dispatcher.dispatch(&MetadataEditListener::onMetadataTrainingSessionNumberChanged ,number);
}

}
