#include "uh/plot/ColorPalette.hpp"
#include "uh/views/XYPositionPlot.hpp"
#include "uh/models/Recording.hpp"
#include "uh/models/PlayerState.hpp"
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
XYPositionPlot::XYPositionPlot(QWidget* parent)
    : RealtimePlot(parent)
{
    setTitle("XY Positions");
}

// ----------------------------------------------------------------------------
XYPositionPlot::~XYPositionPlot()
{
    clear();
}

// ----------------------------------------------------------------------------
void XYPositionPlot::clear()
{
    if (recording_)
        recording_->dispatcher.removeListener(this);
    recording_ = nullptr;

    for (auto& curve : curves_)
        delete curve;
    curves_.clear();
}

// ----------------------------------------------------------------------------
void XYPositionPlot::setRecording(Recording* recording)
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
        curve->setTitle(recording_->playerName(player));
        curve->setStyle(QwtPlotCurve::Dots);
        curve->attach(this);
        curves_.push_back(curve);

        for (const auto& state : recording_->playerStates(player))
        {
            data->append(QPointF(state.posx(), state.posy()));
        }
    }

    autoScale();
}

// ----------------------------------------------------------------------------
void XYPositionPlot::replotAndAutoScale()
{
    if (lastScaleWasAutomatic())
        autoScale();
    else
        replot();
}

// ----------------------------------------------------------------------------
void XYPositionPlot::onActiveRecordingPlayerNameChanged(int player, const QString& name)
{
    curves_[player]->setTitle(name);
}

// ----------------------------------------------------------------------------
void XYPositionPlot::onActiveRecordingNewUniquePlayerState(int player, const PlayerState& state)
{
    CurveData* data = static_cast<CurveData*>(curves_[player]->data());
    data->append(QPointF(state.posx(), state.posy()));
    replotAndAutoScale();
}

}
