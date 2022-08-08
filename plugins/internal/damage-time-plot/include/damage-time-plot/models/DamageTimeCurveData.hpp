#pragma once

#include "rfcommon/FrameDataListener.hpp"
#include "rfcommon/FramesLeft.hpp"
#include "rfcommon/MetaDataListener.hpp"
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
    , public rfcommon::MetaDataListener
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
    void onMetaDataTimeStartedChanged(rfcommon::TimeStamp timeStarted) override;
    void onMetaDataTimeEndedChanged(rfcommon::TimeStamp timeEnded) override;

    // Game related events
    void onMetaDataPlayerNameChanged(int fighterIdx, const rfcommon::String& name) override;
    void onMetaDataSetNumberChanged(rfcommon::SetNumber number) override;
    void onMetaDataGameNumberChanged(rfcommon::GameNumber number) override;
    void onMetaDataSetFormatChanged(const rfcommon::SetFormat& format) override;
    void onMetaDataWinnerChanged(int winnerPlayerIdx) override;

    // In training mode this increments every time a new training room is loaded
    void onMetaDataTrainingSessionNumberChanged(rfcommon::GameNumber number) override;

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
