#include "uh/plot/ColorPalette.hpp"
#include "uh/views/DamagePlot.hpp"
#include "qwt_plot_curve.h"
#include "qwt_date_scale_draw.h"

namespace uh {

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
DamagePlot::DamagePlot(QWidget* parent)
    : RealtimePlot(parent)
{
    setTitle("Damage over Time");
    setAxisScaleDraw(QwtPlot::xBottom, new TimeScaleDraw);
}

// ----------------------------------------------------------------------------
DamagePlot::~DamagePlot()
{
    for (auto& curve : curves_)
        delete curve;
}

// ----------------------------------------------------------------------------
void DamagePlot::resetPlot(int playerCount)
{
    for (auto& curve : curves_)
        delete curve;
    curves_.clear();

    for (int i = 0; i != playerCount; ++i)
    {
        QwtPlotCurve* curve = new QwtPlotCurve;
        curve->setPen(QPen(ColorPalette::getColor(i), 2.0));
        curve->setData(new CurveData);
        curve->attach(this);
        curves_.push_back(curve);
    }

    largestTimeSeen_ = 0.0;
    replot();
}

// ----------------------------------------------------------------------------
void DamagePlot::addPlayerDamageValue(int idx, uint32_t frame, float damage)
{
    QwtPlotCurve* curve = curves_[idx];
    CurveData* data = static_cast<CurveData*>(curve->data());

    float time = static_cast<float>(frame) / 60.0;
    if (time > largestTimeSeen_)
    {
        largestTimeSeen_ = time;
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
            data->setSample(i, QPointF(current.x() - lastLargestSeen + largestTimeSeen_, current.y()));
        }
    }

    data->append(QPointF(largestTimeSeen_ - time, damage));
}

// ----------------------------------------------------------------------------
void DamagePlot::setPlayerTag(int idx, const QString& tag)
{
    curves_[idx]->setTitle(tag);
}

// ----------------------------------------------------------------------------
void DamagePlot::replotAndAutoScale()
{
    if (lastScaleWasAutomatic())
        autoScale();
    else
        replot();
}

}
