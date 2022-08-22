#include "rfcommon/Profiler.hpp"
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
    , splitter_(new QSplitter)
    , plot_(new rfplot::RealtimePlot)
    , sessionsList_(new QListView)
    , lastSessionCount_(0)
{
    plot_->setTitle("Damage over Time");
    plot_->setAxisScaleDraw(QwtPlot::xBottom, new TimeScaleDraw);
    plot_->axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Inverted, true);

    splitter_->addWidget(plot_);

    setLayout(new QVBoxLayout);
    layout()->addWidget(splitter_);

    model_->dispatcher.addListener(this);

    buildCurves();
}

// ----------------------------------------------------------------------------
DamageTimePlotView::~DamageTimePlotView()
{
    model_->dispatcher.removeListener(this);

    delete sessionsList_;
}

// ----------------------------------------------------------------------------
void DamageTimePlotView::clearCurves()
{
    PROFILE(DamageTimePlotView, clearCurves);

    for (auto& curve : curves_)
    {
        curve->detach();
        delete curve;
    }
    curves_.clear();
}

// ----------------------------------------------------------------------------
void DamageTimePlotView::buildCurves()
{
    PROFILE(DamageTimePlotView, buildCurves);

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
    PROFILE(DamageTimePlotView, onDataSetChanged);

    if (model_->sessionCount() == 1)
    {
        plot_->setParent(nullptr);
        sessionsList_->setParent(nullptr);
        splitter_->addWidget(plot_);
    }
    else if (lastSessionCount_ == 1 && model_->sessionCount() > 1)
    {
        plot_->setParent(nullptr);
        sessionsList_->setParent(nullptr);

        splitter_->addWidget(sessionsList_);
        splitter_->addWidget(plot_);
        splitter_->setStretchFactor(0, 0);
        splitter_->setStretchFactor(1, 1);
        splitter_->setSizes({ 440, 1 });
    }
    lastSessionCount_ = model_->sessionCount();

    clearCurves();
    buildCurves();
    plot_->forceAutoScale();
}

// ----------------------------------------------------------------------------
void DamageTimePlotView::onDataChanged()
{
    PROFILE(DamageTimePlotView, onDataChanged);

    plot_->conditionalAutoScale();
}

// ----------------------------------------------------------------------------
void DamageTimePlotView::onNamesChanged()
{
    PROFILE(DamageTimePlotView, onNamesChanged);

    /*curves_[fighterIdx]->setTitle(model_->name(fighterIdx).cStr());*/
    plot_->conditionalAutoScale();
}
