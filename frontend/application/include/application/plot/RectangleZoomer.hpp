#pragma once

#include "application/plot/DeltaPlotPicker.hpp"


class QWidget;

namespace uh {

/*!
 * Provides a method for zooming in and out in real-time on both X/Y axes by
 * dragging the mouse.
 */
class RectangleZoomer : public DeltaPlotPicker
{
    Q_OBJECT

public:
    explicit RectangleZoomer(QWidget* canvas);
    ~RectangleZoomer();

public slots:
    void doZoom(const QPointF& origin, const QPointF& current);

private:
    bool userJustClicked_;
    QPointF initialClickPosition_;
};

} // namespace uh
