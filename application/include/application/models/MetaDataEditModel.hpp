#pragma once

#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/MetaDataListener.hpp"
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
    MetaDataEditModel();
    ~MetaDataEditModel();

    void setAndAdopt(rfcommon::MappingInfo* map, rfcommon::MetaData* mdata);
    void setAndOverwrite(rfcommon::MappingInfo* map, rfcommon::MetaData* mdata);
    void clear();

    void notifyBracketTypeChanged();

    rfcommon::MappingInfo* mappingInfo() { return map_; }
    rfcommon::MetaData* metaData() { return mdata_; }

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
    rfcommon::Reference<rfcommon::MappingInfo> map_;
    rfcommon::Reference<rfcommon::MetaData> mdata_;
};

}
