#pragma once

#include "rfcommon/FighterID.hpp"
#include "rfcommon/UserMotionLabelsCategory.hpp"

namespace rfcommon {

class UserMotionLabelsListener
{
public:
    virtual void onUserMotionLabelsLayerAdded(int layerIdx, const char* name) = 0;
    virtual void onUserMotionLabelsLayerRemoved(int layerIdx, const char* name) = 0;

    virtual void onUserMotionLabelsNewEntry(FighterID fighterID, int entryIdx) = 0;
    virtual void onUserMotionLabelsUserLabelChanged(FighterID fighterID, int entryIdx, const char* oldLabel, const char* newLabel) = 0;
    virtual void onUserMotionLabelsCategoryChanged(FighterID fighterID, int entryIdx, UserMotionLabelsCategory oldCategory, UserMotionLabelsCategory newCategory) = 0;
};

}
