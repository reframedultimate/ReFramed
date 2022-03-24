#include "damage-time-plot/views/DamageTimePlotView.hpp"
#include "damage-time-plot/models/DamageTimePlotModel.hpp"
#include "rfplot/ColorPalette.hpp"
#include "qwt_plot_curve.h"
#include "qwt_date_scale_draw.h"
#include "qwt_scale_engine.h"

namespace {

class DamageTimePlotCurveDataView : public QwtSeriesData<QPointF>
{
public:
    DamageTimePlotCurveDataView(DamageTimePlotModel* model, int fighterIdx)
        : model_(model)
        , fighterIdx_(fighterIdx)
    {
        updateBoundingRect();
    }

    size_t size() const override
        { return model_->dataCount(fighterIdx_); }

    QPointF sample(size_t i) const override
        { return QPointF(model_->secondsLeft(fighterIdx_, i), model_->damage(fighterIdx_, i)); }

    QRectF boundingRect() const override
    {
        return d_boundingRect;
    }

    inline void updateBoundingRect()
    {
        d_boundingRect = qwtBoundingRect(*this);
    }

private:
    const DamageTimePlotModel* model_;
    const int fighterIdx_;
};

class TimeScaleDraw : public QwtScaleDraw
{
public:
    QwtText label(double v) const override
    {
        return QTime(0, 0).addSecs(static_cast<int>(v)).toString();
    }

private:
};

}

// ----------------------------------------------------------------------------
DamageTimePlotView::DamageTimePlotView(DamageTimePlotModel* model, QWidget* parent)
    : RealtimePlot(parent)
    , model_(model)
{
    setTitle("Damage over Time");
    setAxisScaleDraw(QwtPlot::xBottom, new TimeScaleDraw);
    axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Inverted, true);

    DamageTimePlotView::onDamageTimePlotStartNew();

    model_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
DamageTimePlotView::~DamageTimePlotView()
{
    model_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void DamageTimePlotView::onDamageTimePlotStartNew()
{
    for (auto& curve : curves_)
        delete curve;
    curves_.clear();

    for (int fighterIdx = 0; fighterIdx != model_->fighterCount(); ++fighterIdx)
    {
        DamageTimePlotCurveDataView* data = new DamageTimePlotCurveDataView(model_, fighterIdx);
        QwtPlotCurve* curve = new QwtPlotCurve;
        curve->setPen(QPen(rfplot::ColorPalette::getColor(fighterIdx), 2.0));
        curve->setData(data);
        curve->setTitle(model_->name(fighterIdx).cStr());
        curve->attach(this);
        curves_.push(curve);
    }

    forceAutoScale();
}

// ----------------------------------------------------------------------------
void DamageTimePlotView::onDamageTimePlotDataChanged()
{
    for (auto& curve : curves_)
        static_cast<DamageTimePlotCurveDataView*>(curve->data())->updateBoundingRect();

    conditionalAutoScale();
}

// ----------------------------------------------------------------------------
void DamageTimePlotView::onDamageTimePlotNameChanged(int fighterIdx)
{
    curves_[fighterIdx]->setTitle(model_->name(fighterIdx).cStr());
    conditionalAutoScale();
}
