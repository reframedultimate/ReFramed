#pragma once

#include "uhplot/config.hpp"
#include "uhplot/DeltaPlotPicker.hpp"

class QWidget;

namespace uhplot {

/*!
 * Provides a method for panning in real-time in both X/Y directions using the
 * mouse.
 */
class Panner : public DeltaPlotPicker
{
    Q_OBJECT

public:
    explicit Panner(QWidget* canvas);
    ~Panner();

protected slots:
    void doPan(const QPointF& delta);
};

}
