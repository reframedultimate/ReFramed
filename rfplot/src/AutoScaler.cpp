#include "rfcommon/Profiler.hpp"
#include "rfplot/AutoScaler.hpp"
#include "qwt_plot.h"
#include "qwt_plot_canvas.h"
#include <QMouseEvent>

namespace rfplot {

// ----------------------------------------------------------------------------
AutoScaler::AutoScaler(QWidget* canvas) :
    DeltaPlotPicker(canvas)
{
    connect(this, SIGNAL(activated(bool)), this, SLOT(onActivated(bool)));
}

// ----------------------------------------------------------------------------
AutoScaler::~AutoScaler()
{
}

// ----------------------------------------------------------------------------
void AutoScaler::widgetMouseDoubleClickEvent(QMouseEvent* me)
{
    PROFILE(AutoScaler, widgetMouseDoubleClickEvent);

    if(me->button() & btn_)
        autoScale();
}

// ----------------------------------------------------------------------------
void AutoScaler::autoScale()
{
    PROFILE(AutoScaler, autoScale);

    bool storeX = plot()->axisAutoScale(xAxis());
    bool storeYLeft = plot()->axisAutoScale(QwtPlot::yLeft);
    bool storeYRight = plot()->axisAutoScale(QwtPlot::yRight);

    plot()->setAxisAutoScale(xAxis(), true);
    plot()->setAxisAutoScale(QwtPlot::yLeft, true);
    plot()->setAxisAutoScale(QwtPlot::yRight, true);
    plot()->replot();

    plot()->setAxisAutoScale(xAxis(), storeX);
    plot()->setAxisAutoScale(QwtPlot::yLeft, storeYLeft);
    plot()->setAxisAutoScale(QwtPlot::yRight, storeYRight);
}

}
