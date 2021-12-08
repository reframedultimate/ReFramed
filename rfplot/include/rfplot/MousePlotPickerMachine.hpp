#pragma once

#include "rfplot/config.hpp"
#include "qwt_picker_machine.h"

namespace rfplot {

class MousePlotPickerMachine : public QwtPickerMachine
{
public:
    MousePlotPickerMachine();
    virtual ~MousePlotPickerMachine();

    virtual QList<Command> transition(const QwtEventPattern&, const QEvent*);
};

} // namespace uh
