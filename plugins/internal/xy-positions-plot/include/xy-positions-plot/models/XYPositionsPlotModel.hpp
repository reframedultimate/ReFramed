#pragma once

#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/Reference.hpp"
#include "rfcommon/Vector.hpp"
#include <memory>

namespace rfcommon {
    class Session;
}

class XYPositionsPlotListener;
class XYPositionsPlotCurveData;

class XYPositionsPlotModel
{
public:
    XYPositionsPlotModel();
    ~XYPositionsPlotModel();

    void addSession(rfcommon::Session* session);
    void clearAll();

    int sessionCount() const;
    int fighterCount(int sessionIdx);

    XYPositionsPlotCurveData* newCurveData(int sessionIdx, int fighterIdx);

    rfcommon::ListenerDispatcher<XYPositionsPlotListener> dispatcher;

private:
    friend class XYPositionsPlotCurveData;
    void onCurveDataChanged();

private:
    rfcommon::SmallVector<rfcommon::Reference<rfcommon::Session>, 4> sessions_;
};
