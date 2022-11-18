#pragma once

#include "rfcommon/MetaDataListener.hpp"
#include "rfcommon/BracketType.hpp"

namespace rfcommon {
    class MappingInfo;
    class MetaData;
}

namespace rfapp {

class MetaDataEditListener : public rfcommon::MetaDataListener
{
public:
    virtual void onAdoptMetaData(rfcommon::MappingInfo* map, rfcommon::MetaData* mdata) = 0;
    virtual void onOverwriteMetaData(rfcommon::MappingInfo* map, rfcommon::MetaData* mdata) = 0;
    virtual void onMetaDataCleared(rfcommon::MappingInfo* map, rfcommon::MetaData* mdata) = 0;
    virtual void onBracketTypeChangedUI(rfcommon::BracketType bracketType) = 0;
};

}
