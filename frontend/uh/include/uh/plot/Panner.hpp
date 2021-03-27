#pragma once

#include "uh/plot/DeltaPlotPicker.hpp"


class QWidget;

namespace uh {

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

} // namespace uh
