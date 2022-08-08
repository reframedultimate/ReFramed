#include "xy-positions-plot/models/XYPositionsPlotCurveData.hpp"
#include "xy-positions-plot/models/XYPositionsPlotModel.hpp"
#include "xy-positions-plot/listeners/XYPositionsPlotListener.hpp"
#include "rfcommon/Frame.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/FrameData.hpp"

// ----------------------------------------------------------------------------
XYPositionsPlotModel::XYPositionsPlotModel()
{}

// ----------------------------------------------------------------------------
XYPositionsPlotModel::~XYPositionsPlotModel()
{
    clearAll();
}

// ----------------------------------------------------------------------------
void XYPositionsPlotModel::addSession(rfcommon::Session* session)
{
    rfcommon::FrameData* frameData = session->tryGetFrameData();
    if (frameData == nullptr)
        return;  // No frame data, no point

    sessions_.emplace(session);

    dispatcher.dispatch(&XYPositionsPlotListener::onDataSetChanged);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotModel::clearAll()
{
    sessions_.clearCompact();
    dispatcher.dispatch(&XYPositionsPlotListener::onDataSetChanged);
}

// ----------------------------------------------------------------------------
int XYPositionsPlotModel::sessionCount() const
{
    return sessions_.count();
}

// ----------------------------------------------------------------------------
int XYPositionsPlotModel::fighterCount(int sessionIdx)
{
    return sessions_[sessionIdx]->tryGetFrameData()->fighterCount();
}

// ----------------------------------------------------------------------------
XYPositionsPlotCurveData* XYPositionsPlotModel::newCurveData(int sessionIdx, int fighterIdx)
{
    return new XYPositionsPlotCurveData(this, sessions_[sessionIdx]->tryGetFrameData(), fighterIdx);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotModel::onCurveDataChanged()
{
    dispatcher.dispatch(&XYPositionsPlotListener::onDataChanged);
}
