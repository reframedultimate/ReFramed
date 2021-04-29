#pragma once

#include "uh/config.hpp"

class QWidget;

namespace uh {

class UH_PUBLIC_API Plugin
{
public:
    virtual ~Plugin() {}
    virtual QWidget* takeWidget() = 0;
    virtual void giveWidget(QWidget* widget) = 0;
};

}
