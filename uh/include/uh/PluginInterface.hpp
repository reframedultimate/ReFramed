#pragma once

#include "uh/PluginType.hpp"
#include <cstdint>

namespace uh {
    class Plugin;
}

class QWidget;

extern "C" {

struct UHPluginFactoryInfo
{
    const char* name;
    const char* author;
    const char* contact;
    const char* description;
};

struct UHPluginFactory
{
    uh::Plugin* (*createModel)(UHPluginFactory*);
    void (*destroyModel)(uh::Plugin* plugin);

    QWidget* (*createView)(uh::Plugin* model);
    void (*destroyView)(uh::Plugin* model, QWidget* view);

    UHPluginType type;
    UHPluginFactoryInfo info;
};

struct UHPluginInterface
{
    uint32_t version;
    UHPluginFactory* factories;
    int (*start)(uint32_t version);
    void (*stop)(void);
};

}
