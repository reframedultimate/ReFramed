#pragma once

#include "uhplot/config.hpp"
#include "uhplot/DeltaPlotPicker.hpp"

class QWidget;

namespace uhplot {

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
