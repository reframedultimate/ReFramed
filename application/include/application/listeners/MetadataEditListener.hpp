#pragma once

#include "rfcommon/MetadataListener.hpp"
#include "rfcommon/BracketType.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/Reference.hpp"

namespace rfcommon {
    class MappingInfo;
    class Metadata;
}

namespace rfapp {

class MetadataEditListener : public rfcommon::MetadataListener
{
public:
    typedef rfcommon::SmallVector<rfcommon::Reference<rfcommon::MappingInfo>, 1> MappingInfoList;
    typedef rfcommon::SmallVector<rfcommon::Reference<rfcommon::Metadata>, 1> MetadataList;

    virtual void onAdoptMetadata(const MappingInfoList& map, const MetadataList& mdata) = 0;
    virtual void onOverwriteMetadata(const MappingInfoList& map, const MetadataList& mdata) = 0;
    virtual void onMetadataCleared(const MappingInfoList& map, const MetadataList& mdata) = 0;
    virtual void onNextGameStarted() = 0;
    virtual void onBracketTypeChangedUI(rfcommon::BracketType bracketType) = 0;
};

}
