#include "rfcommon/Profiler.hpp"
#include "rfplot/RectangleZoomer.hpp"

#include <qwt_plot.h>
#include <qwt_picker_machine.h>
#include <qwt_scale_div.h>
#include <qwt_scale_engine.h>

namespace rfplot {

// ----------------------------------------------------------------------------
RectangleZoomer::RectangleZoomer(QWidget* canvas) :
    DeltaPlotPicker(canvas)
{
    setRubberBand(RectRubberBand);
    setStateMachine(new QwtPickerDragRectMachine);

    connect(this, SIGNAL(released(QPointF,QPointF)), this, SLOT(doZoom(QPointF,QPointF)));
}

// ----------------------------------------------------------------------------
RectangleZoomer::~RectangleZoomer()
{
}

// ----------------------------------------------------------------------------
void RectangleZoomer::doZoom(const QPointF& origin, const QPointF& current)
{
    PROFILE(RectangleZoomer, doZoom);

    if(origin.x() == 0 || origin.y() == 0)
        return;

    double left = std::min(origin.x(), current.x());
    double right = std::max(origin.x(), current.x());
    double bottom = std::min(origin.y(), current.y());
    double top = std::max(origin.y(), current.y());

    if (plot()->axisScaleEngine(xAxis())->testAttribute(QwtScaleEngine::Inverted))
        plot()->setAxisScale(xAxis(), right, left);
    else
        plot()->setAxisScale(xAxis(), left, right);

    plot()->setAxisScale(yAxis(), bottom, top);
    plot()->replot();
}

}
