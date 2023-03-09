#pragma once

#include "rfcommon/FighterID.hpp"

namespace rfcommon {

class MotionLabelsListener
{
public:
    virtual void onMotionLabelsLoaded() = 0;
    virtual void onMotionLabelsHash40sUpdated() = 0;

    virtual void onMotionLabelsLayerInserted(int layerIdx) = 0;
    virtual void onMotionLabelsLayerRemoved(int layerIdx) = 0;
    virtual void onMotionLabelsLayerNameChanged(int layerIdx) = 0;
    virtual void onMotionLabelsLayerUsageChanged(int layerIdx, int oldUsage) = 0;
    virtual void onMotionLabelsLayerMoved(int fromIdx, int toIdx) = 0;
    virtual void onMotionLabelsLayerMerged(int layerIdx) = 0;

    virtual void onMotionLabelsRowInserted(FighterID fighterID, int row) = 0;
    virtual void onMotionLabelsLabelChanged(FighterID fighterID, int row, int layerIdx) = 0;
    virtual void onMotionLabelsCategoryChanged(FighterID fighterID, int row, int oldCategory) = 0;
};

}
