#pragma once

#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/MetadataListener.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/Reference.hpp"

namespace rfcommon {
    class MappingInfo;
    class Metadata;
}

namespace rfapp {

class MetadataEditListener;

class MetadataEditModel : public rfcommon::MetadataListener
{
public:
    typedef rfcommon::SmallVector<rfcommon::Reference<rfcommon::MappingInfo>, 1> MappingInfoList;
    typedef rfcommon::SmallVector<rfcommon::Reference<rfcommon::Metadata>, 1> MetadataList;

    MetadataEditModel();
    ~MetadataEditModel();

    /*!
     * The listening widgets will take the data stored in map and mdata and
     * load it into their UIs. The metadata will stay in tact.
     */
    void setAndAdopt(MappingInfoList&& map, MetadataList&& mdata);

    /*!
     * The listening widgets will copy the data they have in their UIs into the
     * metadata. The current metadata will be overwritten with the values in the
     * UIs.
     */
    void setAndOverwrite(MappingInfoList&& map, MetadataList&& mdata);

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
     * The widgets will use prevMetadata() as a point of reference/comparison
     * to derive the new game/score count. Note that if more than session's
     * metadata is set, this will do nothing.
     *
     * Widgets can assume that prevMetadata() will always return a valid
     * object, and metadata()/mappingInfo() will always contain exactly 1
     * instance.
     */
    void startNextGame();

    /*!
     * If the user edits values in the UI in between games, then the widgets
     * should call this to prevent the effects of startNextGame().
     */
    void setPendingChanges() { pendingChanges_ = true; }

    rfcommon::Metadata* prevMetadata() const { return prevMdata_; }
    const MappingInfoList& mappingInfo() const { return map_; }
    const MetadataList& metadata() const { return mdata_; }

    rfcommon::ListenerDispatcher<MetadataEditListener> dispatcher;

private:
    void onMetadataTimeChanged(rfcommon::TimeStamp timeStarted, rfcommon::TimeStamp timeEnded) override;
    void onMetadataTournamentDetailsChanged() override;
    void onMetadataEventDetailsChanged() override;
    void onMetadataCommentatorsChanged() override;
    void onMetadataGameDetailsChanged() override;
    void onMetadataPlayerDetailsChanged() override;
    void onMetadataWinnerChanged(int winnerPlayerIdx) override;
    void onMetadataTrainingSessionNumberChanged(rfcommon::SessionNumber number) override;

private:
    rfcommon::Reference<rfcommon::Metadata> prevMdata_;
    MappingInfoList map_;
    MetadataList mdata_;
    bool pendingChanges_ = false;
};

}
