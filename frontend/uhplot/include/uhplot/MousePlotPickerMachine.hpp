#pragma once

#include "uhplot/config.hpp"
#include "qwt_picker_machine.h"

namespace uhplot {

class MousePlotPickerMachine : public QwtPickerMachine
{
public:
    MousePlotPickerMachine();
    virtual ~MousePlotPickerMachine();

    virtual QList<Command> transition(const QwtEventPattern&, const QEvent*);
};

} // namespace uh
