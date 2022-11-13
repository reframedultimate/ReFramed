#pragma once

#include "rfcommon/FrameDataListener.hpp"
#include "rfcommon/FramesLeft.hpp"
#include "rfcommon/Reference.hpp"
#include "rfcommon/Vector.hpp"
#include "qwt_series_data.h"

class DamageTimePlotModel;

namespace rfcommon {
    class FrameData;
    class MetaData;
}

class DamageTimeCurveData
    : public QwtSeriesData<QPointF>
    , public rfcommon::FrameDataListener
{
public:
    DamageTimeCurveData(DamageTimePlotModel* model, rfcommon::MetaData* metaData, rfcommon::FrameData* frameData, int fighterIdx);
    ~DamageTimeCurveData();

    size_t size() const override { return points_.count(); }
    QPointF sample(size_t i) const override { return points_[i]; }
    QRectF boundingRect() const override { return cachedBoundingRect; }
    inline void updateBoundingRect() { cachedBoundingRect = qwtBoundingRect(*this); }

    bool hasThisFrameData(const rfcommon::FrameData* frameData) const;
    void appendDataPoint(rfcommon::FramesLeft framesLeft, float damage);

private:
    void onFrameDataNewUniqueFrame(int frameIdx, const rfcommon::Frame<4>& frame) override;
    void onFrameDataNewFrame(int frameIdx, const rfcommon::Frame<4>& frame) override;

private:
    DamageTimePlotModel* model_;
    rfcommon::Reference<rfcommon::MetaData> metaData_;
    rfcommon::Reference<rfcommon::FrameData> frameData_;
    rfcommon::Vector<QPointF> points_;
    const int fighterIdx_;
};
