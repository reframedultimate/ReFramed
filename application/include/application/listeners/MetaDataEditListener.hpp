#pragma once

#include "rfcommon/MetaDataListener.hpp"
#include "rfcommon/BracketType.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/Reference.hpp"

namespace rfcommon {
    class MappingInfo;
    class MetaData;
}

namespace rfapp {

class MetaDataEditListener : public rfcommon::MetaDataListener
{
public:
    typedef rfcommon::SmallVector<rfcommon::Reference<rfcommon::MappingInfo>, 1> MappingInfoList;
    typedef rfcommon::SmallVector<rfcommon::Reference<rfcommon::MetaData>, 1> MetaDataList;

    virtual void onAdoptMetaData(const MappingInfoList& map, const MetaDataList& mdata) = 0;
    virtual void onOverwriteMetaData(const MappingInfoList& map, const MetaDataList& mdata) = 0;
    virtual void onMetaDataCleared(const MappingInfoList& map, const MetaDataList& mdata) = 0;
    virtual void onBracketTypeChangedUI(rfcommon::BracketType bracketType) = 0;
};

}
