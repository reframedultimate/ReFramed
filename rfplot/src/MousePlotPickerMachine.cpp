#include "rfplot/MousePlotPickerMachine.hpp"

#include <QEvent>
#include <QMouseEvent>
#include "qwt_event_pattern.h"

namespace rfplot {

// ----------------------------------------------------------------------------
MousePlotPickerMachine::MousePlotPickerMachine():
    QwtPickerMachine(PointSelection)
{
}

// ----------------------------------------------------------------------------
MousePlotPickerMachine::~MousePlotPickerMachine()
{
}

// ----------------------------------------------------------------------------
QList<QwtPickerMachine::Command> MousePlotPickerMachine::transition(
    const QwtEventPattern& eventPattern, const QEvent* event )
{
    QList<QwtPickerMachine::Command> cmdList;

    switch(event->type())
    {
        case QEvent::MouseButtonPress:
            if(eventPattern.mouseMatch(QwtEventPattern::MouseSelect1,
                static_cast<const QMouseEvent*>(event)))
            {
                if(state() == 0)
                {
                    cmdList += Begin;
                    setState(1);
                }
            }
            break;

        case QEvent::MouseMove:
        case QEvent::Wheel:
            if(state() != 0)
                cmdList += Move;
            break;

        case QEvent::MouseButtonRelease:
            if(state() != 0)
            {
                cmdList += End;
                setState(0);
            }
            break;

        default:
            break;
    }

    return cmdList;
}

}
