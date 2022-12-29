#include "application/config.hpp"
#include "application/models/PluginManager.hpp"
#include "application/models/Protocol.hpp"
#include "rfcommon/Hash40Strings.hpp"
#include "rfcommon/LastWindowsError.hpp"
#include "rfcommon/Log.hpp"
#include "rfcommon/Metadata.hpp"
#include "rfcommon/Plugin.hpp"
#include "rfcommon/PluginInterface.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/UserMotionLabels.hpp"
#include <cassert>
#include <QDebug>
#include <QWidget>
#include <QDir>

#if defined(RFCOMMON_PLATFORM_WINDOWS)
#   include <Windows.h>
#endif

namespace rfapp {

PluginManager::LoadedPlugin::LoadedPlugin(const char* fileName)
    : library(fileName)
{}
PluginManager::LoadedPlugin::~LoadedPlugin()
{}
PluginManager::LoadedPlugin::LoadedPlugin(LoadedPlugin&& other)
    : library(std::move(other.library))
    , iface(other.iface)
    , started(other.started)
{}

// ----------------------------------------------------------------------------
PluginManager::PluginManager(rfcommon::UserMotionLabels* userLabels, rfcommon::Hash40Strings* hash40Strings)
    : userLabels_(userLabels)
    , hash40Strings_(hash40Strings)
{
    // On Windows, add a directory to the search path so plugins
    // that depend on additional DLLs can load those from this "deps" directory
#if defined(RFCOMMON_PLATFORM_WINDOWS)
    auto log = rfcommon::Log::root();
    CHAR buf[MAX_PATH];
    strcpy(buf, APP_PLUGINDEPSDIR);
    for (CHAR* p = buf; *p; ++p)
        if (*p == '/')
            *p = '\\';
    log->info("Adding DLL search path: %s", buf);
    if (!SetDllDirectoryA(buf))
        log->error("Failed to add DLL search path: %s: %s", buf, rfcommon::LastWindowsError().cStr());
#endif

    scanForPlugins();
}

// ----------------------------------------------------------------------------
PluginManager::~PluginManager()
{
    auto log = rfcommon::Log::root();

    log->beginDropdown("Stopping plugins");
    for (const auto& plugin : plugins_)
        if (plugin.started)
            if (plugin.iface->stop)
            {
                log->info("Calling stop() for plugin factory \"%s\"", plugin.iface->factories[0].info.name ? plugin.iface->factories[0].info.name : "");
                plugin.iface->stop();
            }
    log->endDropdown();

    // Destructors take care of unloading libs
}

// ----------------------------------------------------------------------------
void PluginManager::scanForPlugins()
{
    PROFILE(PluginManager, scanForPlugins);

    auto log = rfcommon::Log::root();

    log->beginDropdown("Loading plugin interfaces");
        QDir pluginDir(APP_PLUGINDIR);
        const auto files = pluginDir.entryList(QStringList() << "*.so" << "*.dll", QDir::Files);
        for (const auto& pluginFile : files)
            loadInterface(pluginDir.absoluteFilePath(pluginFile));
    log->endDropdown();
}

// ----------------------------------------------------------------------------
bool PluginManager::loadInterface(const QString& fileName)
{
    PROFILE(PluginManager, loadInterface);

    auto log = rfcommon::Log::root();

    QByteArray fileNameBA = fileName.toUtf8();
    log->info("Loading plugin %s", fileNameBA.constData());
    LoadedPlugin& plugin = plugins_.emplace(fileNameBA.constData());
    if (plugin.library.isOpen() == false)
        goto open_failed;

    plugin.iface = static_cast<RFPluginInterface*>(plugin.library.lookupSymbolAddress("plugin_interface"));
    if (plugin.iface == nullptr)
    {
        log->error("Failed to lookup symbol 'plugin_interface' in plugin %s", fileNameBA.constData());
        goto init_plugin_failed;
    }
    if (plugin.iface->factories == nullptr)
    {
        log->error("Plugin %s did not register any factories", fileNameBA.constData());
        goto init_plugin_failed;
    }

    for (RFPluginFactory* factory = plugin.iface->factories; factory->info.name != nullptr; ++factory)
    {
        if (factoryNames_.contains(factory->info.name))
        {
            log->warning("Plugin is trying to register factory \"%s\", but a factory with the same name was already registered by a previously loaded plugin. Skipping...", factory->info.name);
            continue;
        }
        factoryNames_.insert(factory->info.name);
        log->info("  - Successfully registered plugin factory \"%s\"", factory->info.name);
    }

    return true;

    init_plugin_failed :
    open_failed :
        plugins_.pop();
        return false;
}

// ----------------------------------------------------------------------------
QVector<QString> PluginManager::availableFactoryNames(RFPluginType type) const
{
    PROFILE(PluginManager, availableFactoryNames);

    QVector<QString> list;
    for (const auto& plugin : plugins_)
        for (RFPluginFactory* factory = plugin.iface->factories; factory->info.name != nullptr; ++factory)
            if ((factory->type & type) == type)
                list.push_back(factory->info.name);

    return list;
}

// ----------------------------------------------------------------------------
QVector<QString> PluginManager::availableFactoryNamesExact(RFPluginType type) const
{
    PROFILE(PluginManager, availableFactoryNamesExact);

    QVector<QString> list;
    for (const auto& plugin : plugins_)
        for (RFPluginFactory* factory = plugin.iface->factories; factory->info.name != nullptr; ++factory)
            if (factory->type == type)
                list.push_back(factory->info.name);

    return list;
}

// ----------------------------------------------------------------------------
const RFPluginFactoryInfo* PluginManager::getFactoryInfo(const QString& name) const
{
    PROFILE(PluginManager, getFactoryInfo);

    for (const auto& plugin : plugins_)
        for (RFPluginFactory* factory = plugin.iface->factories; factory->info.name != nullptr; ++factory)
            if (name == factory->info.name)
                return &factory->info;

    return nullptr;
}

// ----------------------------------------------------------------------------
rfcommon::Plugin* PluginManager::create(const QString& name, rfcommon::VisualizerContext* visCtx)
{
    PROFILE(PluginManager, create);

    auto log = rfcommon::Log::root();

    for (auto pluginIt = plugins_.begin(); pluginIt != plugins_.end(); ++pluginIt)
        for (RFPluginFactory* factory = pluginIt->iface->factories; factory->info.name != nullptr; ++factory)
            if (name == factory->info.name)
            {
                // See if start() needs to be called
                if (pluginIt->started == false && pluginIt->iface->start)
                {
                    const char* startError = nullptr;
                    log->info("Calling start() for plugin factory \"%s\"", factory->info.name);
                    if (pluginIt->iface->start(APP_VERSION, &startError) != 0)
                    {
                        log->error("Call to start() failed for plugin factory \"%s\": %s", factory->info.name, startError ? startError : "(Plugin reported no error message)");
                        for (factory = pluginIt->iface->factories; factory->info.name != nullptr; ++factory)
                            factoryNames_.remove(factory->info.name);
                        plugins_.erase(pluginIt);
                        return nullptr;
                    }
                    pluginIt->started = true;
                }

                // Instantiate object
                rfcommon::Log::root()->info("Creating plugin \"%s\"", factory->info.name);
                rfcommon::Log* pluginLog = rfcommon::Log::root()->child(factory->info.name);
                rfcommon::Plugin* plugin = factory->create(factory, visCtx, pluginLog, userLabels_, hash40Strings_);
                if (plugin == nullptr)
                    log->error("Call to create() failed for plugin factory \"%s\"", factory->info.name);
                return plugin;
            }

    return nullptr;
}

// ----------------------------------------------------------------------------
void PluginManager::destroy(rfcommon::Plugin* plugin)
{
    PROFILE(PluginManager, destroy);

    const RFPluginFactory* factory = plugin->factory();

    rfcommon::Log::root()->info("Destroying plugin \"%s\"", plugin->factory()->info.name);
    factory->destroy(plugin);
}

}
