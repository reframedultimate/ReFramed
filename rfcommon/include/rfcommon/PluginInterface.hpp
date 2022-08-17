#pragma once

#include "rfcommon/PluginType.hpp"
#include <cstdint>

namespace rfcommon {
    class Hash40Strings;
    class Plugin;
    class UserMotionLabels;
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
    rfcommon::Plugin* (*create)(RFPluginFactory* factory, rfcommon::UserMotionLabels* userLabels, rfcommon::Hash40Strings* hash40Strings);
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
