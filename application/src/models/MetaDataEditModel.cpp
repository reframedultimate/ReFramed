#include "application/models/MetaDataEditModel.hpp"
#include "application/listeners/MetaDataEditListener.hpp"

#include "rfcommon/MetaData.hpp"

namespace rfapp {

// ----------------------------------------------------------------------------
MetaDataEditModel::MetaDataEditModel()
{}

// ----------------------------------------------------------------------------
MetaDataEditModel::~MetaDataEditModel()
{
    if (mdata_)
        mdata_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void MetaDataEditModel::setAndAdopt(rfcommon::MetaData* mdata)
{
    clear();

    mdata_ = mdata;
    mdata_->dispatcher.addListener(this);

    dispatcher.dispatch(&MetaDataEditListener::onAdoptMetaData, mdata);
}

// ----------------------------------------------------------------------------
void MetaDataEditModel::setAndOverwrite(rfcommon::MetaData* mdata)
{
    clear();

    mdata_ = mdata;
    mdata_->dispatcher.addListener(this);

    dispatcher.dispatch(&MetaDataEditListener::onOverwriteMetaData, mdata);
}

// ----------------------------------------------------------------------------
void MetaDataEditModel::clear()
{
    if (mdata_)
    {
        dispatcher.dispatch(&MetaDataEditListener::onMetaDataCleared, mdata_);
        mdata_->dispatcher.removeListener(this);
        mdata_.drop();
    }
}

// ----------------------------------------------------------------------------
void MetaDataEditModel::notifyBracketTypeChanged()
{
    dispatcher.dispatch(&MetaDataEditListener::onMetaDataEventDetailsChanged);
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
