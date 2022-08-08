#pragma once

#include "rfcommon/FrameDataListener.hpp"
#include "rfcommon/FramesLeft.hpp"
#include "rfcommon/TimeStamp.hpp"
#include "rfcommon/Reference.hpp"
#include "rfcommon/Vector.hpp"
#include "qwt_series_data.h"

class XYPositionsPlotModel;

namespace rfcommon {
    class FrameData;
    class MetaData;
    class Vec2;
}

class XYPositionsPlotCurveData
    : public QwtSeriesData<QPointF>
    , public rfcommon::FrameDataListener
{
public:
    XYPositionsPlotCurveData(XYPositionsPlotModel* model, rfcommon::FrameData* frameData, int fighterIdx);
    ~XYPositionsPlotCurveData();

    size_t size() const override { return points_.count(); }
    QPointF sample(size_t i) const override { return points_[i]; }
    QRectF boundingRect() const override { return cachedBoundingRect; }
    inline void updateBoundingRect() { cachedBoundingRect = qwtBoundingRect(*this); }

    bool hasThisFrameData(const rfcommon::FrameData* frameData) const;
    void appendDataPoint(const rfcommon::Vec2& pos);

private:
    void onFrameDataNewUniqueFrame(int frameIdx, const rfcommon::Frame<4>& frame) override;
    void onFrameDataNewFrame(int frameIdx, const rfcommon::Frame<4>& frame) override;

private:
    XYPositionsPlotModel* model_;
    rfcommon::Reference<rfcommon::FrameData> frameData_;
    rfcommon::Vector<QPointF> points_;
    const int fighterIdx_;
};
