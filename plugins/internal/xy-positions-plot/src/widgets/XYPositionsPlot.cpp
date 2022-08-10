#include "xy-positions-plot/widgets/XYPositionsPlot.hpp"
#include "xy-positions-plot/models/XYPositionsPlotModel.hpp"
#include "xy-positions-plot/models/XYPositionsPlotCurveData.hpp"

#include "rfplot/ColorPalette.hpp"

#include "qwt_plot_curve.h"

#include <QActionGroup>
#include <QMenu>
#include <QPen>

// ----------------------------------------------------------------------------
XYPositionsPlot::XYPositionsPlot(XYPositionsPlotModel* model, QWidget* parent)
    : RealtimePlot(parent)
    , model_(model)
    , curveTypeActionGroup_(new QActionGroup(this))
{
    QAction* dotted = curveTypeActionGroup_->addAction("Dotted");
    dotted->setCheckable(true);
    QAction* lines = curveTypeActionGroup_->addAction("Lines");
    lines->setCheckable(true);
    lines->setChecked(true);

    model_->dispatcher.addListener(this);

    connect(dotted, &QAction::triggered, this, &XYPositionsPlot::onDottedAction);
    connect(lines, &QAction::triggered, this, &XYPositionsPlot::onLinesAction);
}

// ----------------------------------------------------------------------------
XYPositionsPlot::~XYPositionsPlot()
{
    model_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void XYPositionsPlot::clearCurves()
{
    for (auto& curve : curves_)
    {
        curve->detach();
        delete curve;
    }
    curves_.clear();
}

// ----------------------------------------------------------------------------
void XYPositionsPlot::buildCurves()
{
    for (int sessionIdx = 0; sessionIdx != model_->sessionCount(); ++sessionIdx)
        for (int fighterIdx = 0; fighterIdx != model_->fighterCount(sessionIdx); ++fighterIdx)
        {
            const QList<QAction*>& actions = curveTypeActionGroup_->actions();

            QwtPlotCurve* curve = new QwtPlotCurve;
            curve->setPen(QPen(rfplot::ColorPalette::getColor(fighterIdx), 2.0));
            curve->setStyle(actions[0]->isChecked() ? QwtPlotCurve::Dots : QwtPlotCurve::Lines);
            curve->setData(model_->newCurveData(sessionIdx, fighterIdx));
            //curve->setTitle(model_->name(fighterIdx).cStr());
            curve->attach(this);
            curves_.push(curve);
        }
}

// ----------------------------------------------------------------------------
void XYPositionsPlot::prependContextMenuActions(QMenu* menu)
{
    for (const auto& action : curveTypeActionGroup_->actions())
        menu->addAction(action);
    menu->addSeparator();

    int curveIdx = 0;
    for (int s = 0; s != model_->sessionCount(); ++s)
        for (int f = 0; f != model_->fighterCount(s); ++f)
        {
            QAction* a = menu->addAction(QString::number(s) + ", " + QString::number(f));
            a->setCheckable(true);
            a->setChecked(curves_[curveIdx]->isVisible());
            connect(a, &QAction::triggered, [=](bool checked) {
                setCurveVisible(s, f, checked);
            });
            curveIdx++;
        }

    menu->addSeparator();
}

// ----------------------------------------------------------------------------
void XYPositionsPlot::onDottedAction(bool enable)
{
    if (!enable)
        return;

    for (auto& curve : curves_)
        curve->setStyle(QwtPlotCurve::Dots);

    replot();
}

// ----------------------------------------------------------------------------
void XYPositionsPlot::onLinesAction(bool enable)
{
    if (!enable)
        return;

    for (auto& curve : curves_)
        curve->setStyle(QwtPlotCurve::Lines);

    replot();
}

// ----------------------------------------------------------------------------
void XYPositionsPlot::setCurveVisible(int sessionIdx, int fighterIdx, bool visible)
{
    int curveIdx = 0;
    for (int s = 0; s != model_->sessionCount(); ++s)
        for (int f = 0; f != model_->fighterCount(sessionIdx); ++f)
        {
            if (s == sessionIdx && f == fighterIdx)
                break;
            curveIdx++;
        }
    assert(curveIdx < curves_.count());

    curves_[curveIdx]->setVisible(visible);
    replot();
}

// ----------------------------------------------------------------------------
void XYPositionsPlot::onDataSetChanged()
{
    clearCurves();
    buildCurves();
    forceAutoScale();
}

// ----------------------------------------------------------------------------
void XYPositionsPlot::onDataChanged()
{
    conditionalAutoScale();
}

// ----------------------------------------------------------------------------
void XYPositionsPlot::onNamesChanged()
{
    /*curves_[fighterIdx]->setTitle(model_->name(fighterIdx).cStr());*/
    conditionalAutoScale();
}
