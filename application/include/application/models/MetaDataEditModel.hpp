#pragma once

#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/MetaDataListener.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/Reference.hpp"

namespace rfcommon {
    class MappingInfo;
    class MetaData;
}

namespace rfapp {

class MetaDataEditListener;

class MetaDataEditModel : public rfcommon::MetaDataListener
{
public:
    typedef rfcommon::SmallVector<rfcommon::Reference<rfcommon::MappingInfo>, 1> MappingInfoList;
    typedef rfcommon::SmallVector<rfcommon::Reference<rfcommon::MetaData>, 1> MetaDataList;

    MetaDataEditModel();
    ~MetaDataEditModel();

    void setAndAdopt(MappingInfoList&& map, MetaDataList&& mdata);
    void setAndOverwrite(MappingInfoList&& map, MetaDataList&& mdata);
    void clear();

    const MappingInfoList& mappingInfo() const { return map_; }
    const MetaDataList& metaData() const { return mdata_; }

    rfcommon::ListenerDispatcher<MetaDataEditListener> dispatcher;

private:
    void onMetaDataTimeChanged(rfcommon::TimeStamp timeStarted, rfcommon::TimeStamp timeEnded) override;
    void onMetaDataTournamentDetailsChanged() override;
    void onMetaDataEventDetailsChanged() override;
    void onMetaDataCommentatorsChanged() override;
    void onMetaDataGameDetailsChanged() override;
    void onMetaDataPlayerDetailsChanged() override;
    void onMetaDataWinnerChanged(int winnerPlayerIdx) override;
    void onMetaDataTrainingSessionNumberChanged(rfcommon::SessionNumber number) override;

private:
    MappingInfoList map_;
    MetaDataList mdata_;
};

}
