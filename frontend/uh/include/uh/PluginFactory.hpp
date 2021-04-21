#pragma once

namespace uh {

class Plugin;

class PluginFactory
{
public:
    enum Type
    {
        VISUALIZER,
        ANALYZER
    };

    virtual Plugin* create() = 0;
    virtual void destroy(Plugin* widget) = 0;
    virtual Type type() const = 0;
    virtual const char* author() const = 0;
    virtual const char* contact() const = 0;
    virtual const char* description() const = 0;
};

}
