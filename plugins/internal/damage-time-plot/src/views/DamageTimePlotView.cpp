#include "damage-time-plot/views/DamageTimePlotView.hpp"
#include "damage-time-plot/models/DamageTimeCurveData.hpp"
#include "damage-time-plot/models/DamageTimePlotModel.hpp"
#include "rfplot/ColorPalette.hpp"
#include "rfplot/RealtimePlot.hpp"
#include "qwt_plot_curve.h"
#include "qwt_date_scale_draw.h"
#include "qwt_scale_engine.h"
#include "qwt_text.h"
#include <QPen>
#include <QListView>
#include <QSplitter>
#include <QVBoxLayout>

namespace {

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
    : QWidget(parent)
    , model_(model)
    , plot_(new rfplot::RealtimePlot)
    , sessionsList_(new QListView)
{
    plot_->setTitle("Damage over Time");
    plot_->setAxisScaleDraw(QwtPlot::xBottom, new TimeScaleDraw);
    plot_->axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Inverted, true);
    buildCurves();

    QSplitter* splitter = new QSplitter;
    splitter->addWidget(sessionsList_);
    splitter->addWidget(plot_);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({440, 1});

    setLayout(new QVBoxLayout);
    layout()->addWidget(splitter);

    model_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
DamageTimePlotView::~DamageTimePlotView()
{
    model_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void DamageTimePlotView::clearCurves()
{
    for (auto& curve : curves_)
        delete curve;
    curves_.clear();
}

// ----------------------------------------------------------------------------
void DamageTimePlotView::buildCurves()
{

    for (int sessionIdx = 0; sessionIdx != model_->sessionCount(); ++sessionIdx)
        for (int fighterIdx = 0; fighterIdx != model_->fighterCount(sessionIdx); ++fighterIdx)
        {
            QwtPlotCurve* curve = new QwtPlotCurve;
            curve->setPen(QPen(rfplot::ColorPalette::getColor(fighterIdx), 2.0));
            curve->setData(model_->newCurveData(sessionIdx, fighterIdx));
            //curve->setTitle(model_->name(fighterIdx).cStr());
            curve->attach(plot_);
            curves_.push(curve);
        }
}

// ----------------------------------------------------------------------------
void DamageTimePlotView::onDataSetChanged()
{
    clearCurves();
    buildCurves();
    plot_->forceAutoScale();
}

// ----------------------------------------------------------------------------
void DamageTimePlotView::onDataChanged()
{
    plot_->conditionalAutoScale();
}

// ----------------------------------------------------------------------------
void DamageTimePlotView::onNamesChanged()
{
    /*curves_[fighterIdx]->setTitle(model_->name(fighterIdx).cStr());*/
    plot_->conditionalAutoScale();
}
