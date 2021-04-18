#pragma once

#include "qwt_picker_machine.h"

namespace uh {

class MousePlotPickerMachine : public QwtPickerMachine
{
public:
    MousePlotPickerMachine();
    virtual ~MousePlotPickerMachine();

    virtual QList<Command> transition(const QwtEventPattern&, const QEvent*);
};

} // namespace uh
