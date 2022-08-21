#include "application/config.hpp"
#include "application/models/PluginManager.hpp"
#include "application/models/Protocol.hpp"
#include "rfcommon/dynlib.h"
#include "rfcommon/Hash40Strings.hpp"
#include "rfcommon/MetaData.hpp"
#include "rfcommon/Plugin.hpp"
#include "rfcommon/PluginInterface.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/UserMotionLabels.hpp"
#include <cassert>
#include <QDebug>
#include <QWidget>
#include <QDir>

#if defined(_WIN32)
#include "Windows.h"
#endif

namespace rfapp {

// ----------------------------------------------------------------------------
PluginManager::PluginManager(rfcommon::UserMotionLabels* userLabels, rfcommon::Hash40Strings* hash40Strings)
    : userLabels_(userLabels)
    , hash40Strings_(hash40Strings)
{
    // On Windows, add a directory to the search path so plugins
    // that depend on additional DLLs can load those from this "deps" directory
#if defined(_WIN32)
    CHAR buf[MAX_PATH];
    strcpy(buf, APP_PLUGINDEPSDIR);
    for (CHAR* p = buf; *p; ++p)
        if (*p == '/')
            *p = '\\';
    SetDllDirectoryA(buf);
#endif

    QDir pluginDir(APP_PLUGINDIR);
    for (const auto& pluginFile : pluginDir.entryList(QStringList() << "*.so" << "*.dll", QDir::Files))
        loadPlugin(pluginDir.absoluteFilePath(pluginFile));
}

// ----------------------------------------------------------------------------
PluginManager::~PluginManager()
{
    for (const auto& dl : libraries_)
    {
        RFPluginInterface* i = static_cast<RFPluginInterface*>(rfcommon_dynlib_lookup_symbol_address(dl, "plugin_interface"));
        assert(i != nullptr);
        i->stop();
        rfcommon_dynlib_close(dl);
    }
}

// ----------------------------------------------------------------------------
bool PluginManager::loadPlugin(const QString& fileName)
{
    PROFILE(PluginManager, loadPlugin);

    RFPluginInterface* i;
    const char* pluginError = nullptr;
    qDebug() << "Loading " << fileName;
    rfcommon_dynlib* dl = rfcommon_dynlib_open(fileName.toStdString().c_str());
    if (dl == nullptr)
    {
        qDebug() << "Failed to load " << fileName;
        goto open_failed;
    }

    i = static_cast<RFPluginInterface*>(rfcommon_dynlib_lookup_symbol_address(dl, "plugin_interface"));
    if (i == nullptr)
    {
        qDebug() << "Failed to lookup symbol 'plugin_interface' in " << fileName;
        goto init_plugin_failed;
    }
    if (i->start == nullptr || i->start(APP_VERSION, &pluginError) != 0)
    {
        qDebug() << "Call to start() failed in " << fileName;
        if (pluginError)
            qDebug() << pluginError;
        goto init_plugin_failed;
    }

    for (RFPluginFactory* factory = i->factories; factory->info.name != nullptr; ++factory)
    {
        if (factories_.contains(factory->info.name))
            goto duplicate_factory_name;
        factories_.insert(factory->info.name, factory);
        qDebug() << "  - Successfully registered plugin factory \"" << factory->info.name << "\"";
    }

    libraries_.push_back(dl);

    return true;

    duplicate_factory_name :
        for (RFPluginFactory* factory = i->factories; factory->info.name != nullptr; ++factory)
        {
            auto it = factories_.find(factory->info.name);
            if (it != factories_.end())
                factories_.erase(it);
        }
    init_plugin_failed :
        rfcommon_dynlib_close(dl);
    open_failed :
        return false;
}

// ----------------------------------------------------------------------------
QVector<QString> PluginManager::availableFactoryNames(RFPluginType type) const
{
    PROFILE(PluginManager, availableFactoryNames);

    QVector<QString> list;
    for (auto factory = factories_.begin(); factory != factories_.end(); ++factory)
    {
        if ((factory.value()->type & type) == type)
            list.push_back(factory.value()->info.name);
    }
    return list;
}

// ----------------------------------------------------------------------------
const RFPluginFactoryInfo* PluginManager::getFactoryInfo(const QString &name) const
{
    PROFILE(PluginManager, getFactoryInfo);

    auto it = factories_.find(name);
    if (it == factories_.end())
        return nullptr;

    RFPluginFactory* factory = it.value();
    return &factory->info;
}

// ----------------------------------------------------------------------------
rfcommon::Plugin* PluginManager::create(const QString& name)
{
    PROFILE(PluginManager, create);

    auto it = factories_.find(name);
    if (it == factories_.end())
        return nullptr;

    RFPluginFactory* factory = it.value();
    rfcommon::Plugin* model = factory->create(factory, userLabels_, hash40Strings_);
    if (model == nullptr)
        return nullptr;

    return model;
}

// ----------------------------------------------------------------------------
void PluginManager::destroy(rfcommon::Plugin* model)
{
    PROFILE(PluginManager, destroy);

    const RFPluginFactory* factory = model->factory();
    factory->destroy(model);
}

}
