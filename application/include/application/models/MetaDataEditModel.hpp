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

    /*!
     * The listening widgets will take the data stored in map and mdata and
     * load it into their UIs. The metadata will stay in tact.
     */
    void setAndAdopt(MappingInfoList&& map, MetaDataList&& mdata);

    /*!
     * The listening widgets will copy the data they have in their UIs into the
     * metadata. The current metadata will be overwritten with the values in the
     * UIs.
     */
    void setAndOverwrite(MappingInfoList&& map, MetaDataList&& mdata);

    /*!
     * All references to mapping info and metadata are released. The widgets
     * will retain the last known state of data in their UIs.
     */
    void clear();

    /*!
     * Some widgets need to know when the next game starts (Game widget) in
     * order to e.g. increment the score or game counter. Call this after
     * setting the metadata (setAndAdopt() or setAndOverwrite).
     *
     * The widgets will use prevMetaData() as a point of reference/comparison
     * to derive the new game/score count. Note that if more than session's
     * metadata is set, this will do nothing.
     *
     * Widgets can assume that prevMetaData() will always return a valid
     * object, and metaData()/mappingInfo() will always contain exactly 1
     * instance.
     */
    void startNextGame();

    rfcommon::MetaData* prevMetaData() const { return prevMdata_; }
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
    rfcommon::Reference<rfcommon::MetaData> prevMdata_;
    MappingInfoList map_;
    MetaDataList mdata_;
};

}
