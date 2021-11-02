#pragma once

#include "uh/PluginType.hpp"
#include <cstdint>

namespace uh {
    class Plugin;
}

extern "C" {

struct UHPluginInfo
{
    const char* name;
    const char* author;
    const char* contact;
    const char* description;
};

struct UHPluginFactory
{
    uh::Plugin* (*create)(void);
    void (*destroy)(uh::Plugin* plugin);
    UHPluginType type;
    UHPluginInfo info;
};

struct UHPluginInterface
{
    uint32_t version;
    UHPluginFactory* factories;
    int (*start)(uint32_t version);
    void (*stop)(void);
};

}
