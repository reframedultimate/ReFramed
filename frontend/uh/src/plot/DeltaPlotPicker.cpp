#include "uh/plot/DeltaPlotPicker.hpp"
#include "uh/plot/MousePlotPickerMachine.hpp"

#include "qwt_plot.h"

namespace uh {


// ----------------------------------------------------------------------------
DeltaPlotPicker::DeltaPlotPicker(QWidget* canvas) :
    QwtPlotPicker(canvas),
    userJustClicked_(false)
{
    setStateMachine(new MousePlotPickerMachine);

    connect(this, SIGNAL(activated(bool)), this, SLOT(onActivated(bool)));
    connect(this, SIGNAL(moved(QPoint)), this, SLOT(onMoved(QPoint)));
}

// ----------------------------------------------------------------------------
DeltaPlotPicker::~DeltaPlotPicker()
{
}

// ----------------------------------------------------------------------------
QPointF DeltaPlotPicker::originalInvTransform(const QPoint& point) const
{
    return QPointF(originalCanvasMapX_.invTransform(point.x()),
                   originalCanvasMapY_.invTransform(point.y()));
}

// ----------------------------------------------------------------------------
void DeltaPlotPicker::onActivated(bool activated)
{
    userJustClicked_ = activated;

    if(activated)
    {
        originalCanvasMapX_ = plot()->canvasMap(xAxis());
        originalCanvasMapY_ = plot()->canvasMap(yAxis());
        originalClickPosition_ = QPointF(0, 0);

        emit this->activated(activated, originalInvTransform(trackerPosition()));
    }
    else
    {
        emit released(originalClickPosition_, originalInvTransform(lastPoint_));
    }
}

// ----------------------------------------------------------------------------
void DeltaPlotPicker::onMoved(const QPoint& point)
{
    if(userJustClicked_)
    {
        userJustClicked_ = false;
        lastPoint_ = point;
        originalClickPosition_ = originalInvTransform(point);
    }

    QPointF transformedPoint = originalInvTransform(point);
    QPointF d = originalInvTransform(lastPoint_) - transformedPoint;
    lastPoint_ = point;
    emit delta(d);

    // The base class emits a similar signal with 1 argument. Don't confuse it!
    emit moved(originalClickPosition_, transformedPoint);
}

// ----------------------------------------------------------------------------
void DeltaPlotPicker::move(const QPoint& point)
{
    onMoved(point);
    QwtPicker::move(point);
}

} // namespace uh
