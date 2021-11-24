#pragma once

#include "uh/config.hpp"

class QWidget;

namespace uh {

class UH_PUBLIC_API Plugin
{
public:
    virtual ~Plugin() {}
    virtual QWidget* createView() = 0;
    virtual void destroyView(QWidget* widget) = 0;
};

}
