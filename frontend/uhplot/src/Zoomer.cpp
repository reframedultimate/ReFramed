#include "uhplot/Zoomer.hpp"
#include "uhplot/MousePlotPickerMachine.hpp"

#include "qwt_plot.h"
#include "qwt_scale_div.h"

#include <QtCore/qmath.h>

static QPointF qPow(qreal base, const QPointF& exponent)
{
    return QPointF(
        qPow(base, exponent.x()),
        qPow(base, exponent.y())
    );
}

namespace uhplot {

// ----------------------------------------------------------------------------
Zoomer::Zoomer(QWidget* canvas) :
    DeltaPlotPicker(canvas)
{
    setStateMachine(new MousePlotPickerMachine);

    connect(this, SIGNAL(activated(bool)), this, SLOT(onActivated(bool)));
    connect(this, SIGNAL(moved(QPointF,QPointF)), this, SLOT(doZoom(QPointF,QPointF)));
}

// ----------------------------------------------------------------------------
Zoomer::~Zoomer()
{
}

// ----------------------------------------------------------------------------
void Zoomer::onActivated(bool activated)
{
    // Store the original distances from the point where the user clicked
    // (origin) to the plot's upper and lower points so we have a reference
    // when zooming
    if(activated)
    {
        const QwtScaleDiv& xScaleDiv = plot()->axisScaleDiv(xAxis());
        const QwtScaleDiv& yScaleDiv = plot()->axisScaleDiv(yAxis());

        // left, top, width, height
        originalPlotDimension_ = QRectF(xScaleDiv.lowerBound(),
                                        yScaleDiv.lowerBound(),
                                        xScaleDiv.upperBound() - xScaleDiv.lowerBound(),
                                        yScaleDiv.upperBound() - yScaleDiv.lowerBound());
    }
}

// ----------------------------------------------------------------------------
void Zoomer::doZoom(const QPointF& origin, const QPointF& current)
{
    // The mouse position is mapped to a normalized coordinate system where
    // [-1,-1] is the top left of the canvas boundaries and [1,1] is bottom
    // right. The coordinate system is offset such that its centre is placed
    // where the user initially clicked in the plot (the "origin").
    QPointF mouseDistanceToOrigin = origin - current;
    QPointF mouse = QPointF(mouseDistanceToOrigin.x() * 2.0 / originalPlotDimension_.width(),
                            mouseDistanceToOrigin.y() * 2.0 / originalPlotDimension_.height());

    // Convert the normalized mouse coordinate to a logarithmic scale. It's
    // more intuitive for zooming.
    const qreal magic = 4.0;  // Chosen by what "felt right" when using the zoom
    QPointF scaleFactor = qPow(magic, mouse);

    // The mouse coordinate can now be used as a factor with the original
    // plot dimensions to calculate a new width and height.
    QPointF newDimensions(originalPlotDimension_.width()  * scaleFactor.x(),
                          originalPlotDimension_.height() * scaleFactor.y());

    // The new width and height have to be split into two halves each so they
    // can be used as an offset around the origin (where the user clicked).
    qreal leftInfluence = (origin.x() - originalPlotDimension_.x()) / originalPlotDimension_.width();
    qreal rightInfluence = 1.0 - leftInfluence;
    qreal upperInfluence = (origin.y() - originalPlotDimension_.y()) / originalPlotDimension_.height();
    qreal lowerInfluence = 1.0 - upperInfluence;

    qreal leftWidth   = newDimensions.x() * leftInfluence;
    qreal rightWidth  = newDimensions.x() * rightInfluence;
    qreal upperHeight = newDimensions.y() * upperInfluence;
    qreal lowerHeight = newDimensions.y() * lowerInfluence;

    // Update axes and replot
    plot()->setAxisScale(xAxis(), origin.x() - leftWidth, origin.x() + rightWidth);
    plot()->setAxisScale(yAxis(), origin.y() - upperHeight, origin.y() + lowerHeight);
    plot()->replot();
}

}
