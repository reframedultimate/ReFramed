#pragma once

#include "uh/PluginType.hpp"
#include <cstdint>

namespace uh {
    class Plugin;
}

struct PluginFactory
{
    uh::Plugin* (*create)(void);
    void (*destroy)(uh::Plugin* plugin);
    uh::PluginType type;
    const char* name;
    const char* author;
    const char* contact;
    const char* description;
};

struct PluginInterface
{
    int (*start)(uint32_t version);
    void (*stop)(void);
    PluginFactory* factories;
};
