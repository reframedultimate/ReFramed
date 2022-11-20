#include "application/models/MetaDataEditModel.hpp"
#include "application/listeners/MetaDataEditListener.hpp"

#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/MetaData.hpp"

namespace rfapp {

// ----------------------------------------------------------------------------
MetaDataEditModel::MetaDataEditModel()
{}

// ----------------------------------------------------------------------------
MetaDataEditModel::~MetaDataEditModel()
{
    if (mdata_.count())
        for (auto& m : mdata_)
            m->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void MetaDataEditModel::setAndAdopt(MappingInfoList&& map, MetaDataList&& mdata)
{
    clear();

    map_ = map;
    mdata_ = mdata;
    for (auto& m : mdata_)
        m->dispatcher.addListener(this);

    dispatcher.dispatch(&MetaDataEditListener::onAdoptMetaData, map, mdata);
}

// ----------------------------------------------------------------------------
void MetaDataEditModel::setAndOverwrite(MappingInfoList&& map, MetaDataList&& mdata)
{
    clear();

    map_ = map;
    mdata_ = mdata;
    for (auto& m : mdata_)
        m->dispatcher.addListener(this);

    dispatcher.dispatch(&MetaDataEditListener::onOverwriteMetaData, map, mdata);
}

// ----------------------------------------------------------------------------
void MetaDataEditModel::clear()
{
    if (mdata_.count())
    {
        dispatcher.dispatch(&MetaDataEditListener::onMetaDataCleared, map_, mdata_);
        for (auto& m : mdata_)
            m->dispatcher.removeListener(this);
        mdata_.clear();
        map_.clear();
    }
}

// ----------------------------------------------------------------------------
void MetaDataEditModel::onMetaDataTimeChanged(rfcommon::TimeStamp timeStarted, rfcommon::TimeStamp timeEnded)
{
    dispatcher.dispatch(&MetaDataEditListener::onMetaDataTimeChanged, timeStarted, timeEnded);
}

// ----------------------------------------------------------------------------
void MetaDataEditModel::onMetaDataTournamentDetailsChanged()
{
    dispatcher.dispatch(&MetaDataEditListener::onMetaDataTournamentDetailsChanged);
}

// ----------------------------------------------------------------------------
void MetaDataEditModel::onMetaDataEventDetailsChanged()
{
    dispatcher.dispatch(&MetaDataEditListener::onMetaDataEventDetailsChanged);
}

// ----------------------------------------------------------------------------
void MetaDataEditModel::onMetaDataCommentatorsChanged()
{
    dispatcher.dispatch(&MetaDataEditListener::onMetaDataCommentatorsChanged);
}

// ----------------------------------------------------------------------------
void MetaDataEditModel::onMetaDataGameDetailsChanged()
{
    dispatcher.dispatch(&MetaDataEditListener::onMetaDataGameDetailsChanged);
}

// ----------------------------------------------------------------------------
void MetaDataEditModel::onMetaDataPlayerDetailsChanged()
{
    dispatcher.dispatch(&MetaDataEditListener::onMetaDataPlayerDetailsChanged);
}

// ----------------------------------------------------------------------------
void MetaDataEditModel::onMetaDataWinnerChanged(int winnerPlayerIdx)
{
    dispatcher.dispatch(&MetaDataEditListener::onMetaDataWinnerChanged, winnerPlayerIdx);
}

// ----------------------------------------------------------------------------
void MetaDataEditModel::onMetaDataTrainingSessionNumberChanged(rfcommon::SessionNumber number)
{
    dispatcher.dispatch(&MetaDataEditListener::onMetaDataTrainingSessionNumberChanged ,number);
}

}
