#include "application/plot/ColorPalette.hpp"
#include "application/views/DamageTimePlot.hpp"
#include "uh/Recording.hpp"
#include "uh/PlayerState.hpp"
#include "qwt_plot_curve.h"
#include "qwt_date_scale_draw.h"

namespace uhapp {

namespace {

class CurveData : public QwtArraySeriesData<QPointF>
{
public:
    QRectF boundingRect() const override
    {
        if (d_boundingRect.width() < 0.0)
            d_boundingRect = qwtBoundingRect(*this);

        return d_boundingRect;
    }

    inline void append(const QPointF& point)
    {
        d_samples += point;
        d_boundingRect = QRectF(0.0, 0.0, -1.0, -1.0);
    }

    inline void setSample(int idx, const QPointF& point)
    {
        d_samples[idx] = point;
        d_boundingRect = QRectF(0.0, 0.0, -1.0, -1.0);
    }

    void clear()
    {
        d_samples.clear();
        d_samples.squeeze();
        d_boundingRect = QRectF(0.0, 0.0, -1.0, -1.0);
    }
};

class TimeScaleDraw : public QwtScaleDraw
{
public:
    QwtText label(double v) const override
    {
        return QTime(0, 0).addSecs(static_cast<int>(v)).toString();
    }

private:
    QTime baseTime_;
};

}

// ----------------------------------------------------------------------------
DamageTimePlot::DamageTimePlot(QWidget* parent)
    : RealtimePlot(parent)
{
    setTitle("Damage over Time");
    setAxisScaleDraw(QwtPlot::xBottom, new TimeScaleDraw);
}

// ----------------------------------------------------------------------------
DamageTimePlot::~DamageTimePlot()
{
    clear();
}

// ----------------------------------------------------------------------------
static void appendDataPoint(CurveData* data, float frame, float damage, float* largestTimeSeen)
{
    float time = static_cast<float>(frame) / 60.0;
    if (time > *largestTimeSeen)
    {
        *largestTimeSeen = time;
        float lastLargestSeen = 0.0;
        for (int i = 0; i < (int)data->size(); ++i)
        {
            float contender = data->sample(i).x();
            if (lastLargestSeen < contender)
                lastLargestSeen = contender;
        }
        for (int i = 0; i < (int)data->size(); ++i)
        {
            QPointF current = data->sample(i);
            data->setSample(i, QPointF(current.x() - lastLargestSeen + *largestTimeSeen, current.y()));
        }
    }

    data->append(QPointF(*largestTimeSeen - time, damage));
}

// ----------------------------------------------------------------------------
void DamageTimePlot::setRecording(uh::Recording* recording)
{
    clear();
    recording_ = recording;
    recording_->dispatcher.addListener(this);

    for (int player = 0; player != recording_->playerCount(); ++player)
    {
        CurveData* data = new CurveData;
        QwtPlotCurve* curve = new QwtPlotCurve;
        curve->setPen(QPen(ColorPalette::getColor(player), 2.0));
        curve->setData(data);
        curve->setTitle(recording_->playerName(player).cStr());
        curve->attach(this);
        curves_.push(curve);

        prevFrames_.push(0);
        prevDamageValues_.push(0.0);
        for (int i = 0; i < recording_->playerStateCount(player); ++i)
        {
            const auto& state = recording_->playerStateAt(player, i);

            if (i == 0 || i == recording_->playerStateCount(player) - 1)
            {
                prevDamageValues_[player] = state.damage();
                appendDataPoint(data, state.frame(), state.damage(), &largestTimeSeen_);
            }
            else if (state.damage() != prevDamageValues_[player])
            {
                appendDataPoint(data, prevFrames_[player], prevDamageValues_[player], &largestTimeSeen_);
                appendDataPoint(data, state.frame(), state.damage(), &largestTimeSeen_);
                prevDamageValues_[player] = state.damage();
            }
            prevFrames_[player] = state.frame();
        }
    }

    forceAutoScale();
}

// ----------------------------------------------------------------------------
void DamageTimePlot::clear()
{
    if (recording_)
        recording_->dispatcher.removeListener(this);
    recording_ = nullptr;

    for (auto& curve : curves_)
        delete curve;
    curves_.clear();
    prevFrames_.clear();
    prevDamageValues_.clear();
    largestTimeSeen_ = 0.0;
    replot();
}

// ----------------------------------------------------------------------------
void DamageTimePlot::onActiveRecordingPlayerNameChanged(int player, const uh::SmallString<15>& name)
{
    curves_[player]->setTitle(name.cStr());
    conditionalAutoScale();
}

// ----------------------------------------------------------------------------
void DamageTimePlot::onActiveRecordingNewUniquePlayerState(int player, const uh::PlayerState& state)
{
    CurveData* data = static_cast<CurveData*>(curves_[player]->data());

    if (prevFrames_[player] == 0)
    {
        appendDataPoint(data, state.frame(), state.damage(), &largestTimeSeen_);
    }
    else if (state.damage() != prevDamageValues_[player])
    {
        appendDataPoint(data, prevFrames_[player], prevDamageValues_[player], &largestTimeSeen_);
        appendDataPoint(data, state.frame(), state.damage(), &largestTimeSeen_);
        prevDamageValues_[player] = state.damage();
    }
    prevFrames_[player] = state.frame();

    conditionalAutoScale();
}

}
