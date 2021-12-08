#pragma once

#include "rfplot/config.hpp"
#include "rfplot/DeltaPlotPicker.hpp"

class QWidget;

namespace rfplot {

/*!
 * Provides a method for auto-scaling the data in a plot when double-clicking.
 */
class AutoScaler : public DeltaPlotPicker
{
public:
    explicit AutoScaler(QWidget* canvas);
    ~AutoScaler();

    void setMouseButton(QFlags<Qt::MouseButton> btn)
            { btn_ = btn;}

    void autoScale();

protected:
    virtual void widgetMouseDoubleClickEvent(QMouseEvent *me) override;

private:
    QFlags<Qt::MouseButton> btn_;
};

}
