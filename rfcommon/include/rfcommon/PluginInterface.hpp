#pragma once

#include "rfcommon/PluginType.hpp"
#include <cstdint>

namespace rfcommon {
    class Hash40Strings;
    class Log;
    class Plugin;
    class UserMotionLabels;
    class PluginContext;
}

class QWidget;

extern "C" {

struct RFPluginFactoryInfo
{
    const char* name;
    const char* category;
    const char* author;
    const char* contact;
    const char* description;
};

struct RFPluginFactory
{
    rfcommon::Plugin* (*create)(RFPluginFactory* factory, rfcommon::PluginContext* pluginCtx, rfcommon::Log* log, rfcommon::UserMotionLabels* userLabels, rfcommon::Hash40Strings* hash40Strings);
    void (*destroy)(rfcommon::Plugin* plugin);

    RFPluginType type;
    RFPluginFactoryInfo info;
};

struct RFPluginInterface
{
    uint32_t version;
    RFPluginFactory* factories;
    int (*start)(uint32_t version, const char** error);
    void (*stop)(void);
};

}
