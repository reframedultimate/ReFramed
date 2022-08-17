#pragma once

#include "rfcommon/FighterID.hpp"

namespace rfcommon {

class UserMotionLabelsListener
{
public:
    virtual void onUserMotionLabelsLayerAdded(int layerIdx, const char* name) = 0;
    virtual void onUserMotionLabelsLayerRemoved(int layerIdx, const char* name) = 0;

    virtual void onUserMotionLabelsNewEntry(FighterID fighterID, int entryIdx) = 0;
    virtual void onUserMotionLabelsEntryChanged(FighterID fighterID, int entryIdx) = 0;
    virtual void onUserMotionLabelsEntryRemoved(FighterID fighterID, int entryIdx) = 0;
};

}
