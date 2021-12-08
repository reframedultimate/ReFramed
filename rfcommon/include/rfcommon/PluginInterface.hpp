#pragma once

#include "rfcommon/PluginType.hpp"
#include <cstdint>

namespace rfcommon {
    class Plugin;
}

class QWidget;

extern "C" {

struct RFPluginFactoryInfo
{
    const char* name;
    const char* author;
    const char* contact;
    const char* description;
};

struct RFPluginFactory
{
    rfcommon::Plugin* (*createModel)(RFPluginFactory*);
    void (*destroyModel)(rfcommon::Plugin* plugin);

    QWidget* (*createView)(rfcommon::Plugin* model);
    void (*destroyView)(rfcommon::Plugin* model, QWidget* view);

    RFPluginType type;
    RFPluginFactoryInfo info;
};

struct RFPluginInterface
{
    uint32_t version;
    RFPluginFactory* factories;
    int (*start)(uint32_t version);
    void (*stop)(void);
};

}
