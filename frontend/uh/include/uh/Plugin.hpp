#pragma once

#include "uh/config.hpp"

class QWidget;
class UHPluginFactory;

namespace uh {

class UH_PUBLIC_API Plugin
{
public:
    Plugin(UHPluginFactory* factory);
    virtual ~Plugin();

    virtual QWidget* createView() = 0;
    virtual void destroyView(QWidget* view) = 0;

    UHPluginFactory* factory() const
        { return factory_; }

private:
    UHPluginFactory* const factory_;
};

}
