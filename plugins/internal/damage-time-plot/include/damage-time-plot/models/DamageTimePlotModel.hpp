#pragma once

#include "rfcommon/RealtimePlugin.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/MetaDataListener.hpp"
#include "rfcommon/FrameDataListener.hpp"
#include "rfcommon/FramesLeft.hpp"
#include "rfcommon/Reference.hpp"
#include <memory>

namespace rfcommon {
    class MetaData;
    class FrameData;
}

class DamageTimePlotListener;
class DamageTimeCurveData;

class DamageTimePlotModel
{
public:
    DamageTimePlotModel();
    ~DamageTimePlotModel();

    void addSession(rfcommon::Session* session);
    void clearAll();

    int sessionCount() const;
    int fighterCount(int sessionIdx);

    DamageTimeCurveData* newCurveData(int sessionIdx, int fighterIdx);

    rfcommon::ListenerDispatcher<DamageTimePlotListener> dispatcher;

private:
    friend class DamageTimeCurveData;
    void onCurveDataChanged();

private:
    rfcommon::SmallVector<rfcommon::Reference<rfcommon::Session>, 4> sessions_;
};
