#include "uh/plot/Panner.hpp"
#include "uh/plot/MousePlotPickerMachine.hpp"

#include "qwt_plot.h"
#include "qwt_scale_div.h"


namespace uh {

// ----------------------------------------------------------------------------
Panner::Panner(QWidget* canvas) :
    DeltaPlotPicker(canvas)
{
    setStateMachine(new MousePlotPickerMachine);

    connect(this, SIGNAL(delta(QPointF)), this, SLOT(doPan(QPointF)));
}

// ----------------------------------------------------------------------------
Panner::~Panner()
{
}

// ----------------------------------------------------------------------------
void Panner::doPan(const QPointF& delta)
{
    const QwtScaleDiv& xScaleDiv = plot()->axisScaleDiv(xAxis());
    const QwtScaleDiv& yScaleDiv = plot()->axisScaleDiv(yAxis());

    plot()->setAxisScale(xAxis(),
                         xScaleDiv.lowerBound() + delta.x(),
                         xScaleDiv.upperBound() + delta.x());
    plot()->setAxisScale(yAxis(),
                         yScaleDiv.lowerBound() + delta.y(),
                         yScaleDiv.upperBound() + delta.y());
    plot()->replot();
}

}
