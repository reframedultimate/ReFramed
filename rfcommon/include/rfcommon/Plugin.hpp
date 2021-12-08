#pragma once

#include "rfcommon/config.hpp"

class QWidget;
struct RFPluginFactory;

namespace rfcommon {

class RFCOMMON_PUBLIC_API Plugin
{
public:
    Plugin(RFPluginFactory* factory);
    virtual ~Plugin();

    virtual QWidget* createView() = 0;
    virtual void destroyView(QWidget* view) = 0;

    RFPluginFactory* factory() const
        { return factory_; }

private:
    RFPluginFactory* const factory_;
};

}
