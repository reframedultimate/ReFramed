#pragma once

#include "rfplot/config.hpp"
#include "rfplot/DeltaPlotPicker.hpp"

class QWidget;

namespace rfplot {

/*!
 * Provides a method for zooming in and out in real-time on both X/Y axes by
 * dragging the mouse.
 */
class Zoomer : public DeltaPlotPicker
{
    Q_OBJECT

public:
    explicit Zoomer(QWidget* canvas);
    ~Zoomer();

protected slots:
    void doZoom(const QPointF& origin, const QPointF& current);

private slots:
    void onActivated(bool activated);

private:
    bool userJustClicked_;
    QRectF originalPlotDimension_;
};

}
