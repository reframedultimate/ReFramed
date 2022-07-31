#include "application/config.hpp"
#include "application/models/PluginManager.hpp"
#include "application/models/Protocol.hpp"
#include "rfcommon/AnalyzerPlugin.hpp"
#include "rfcommon/PluginInterface.hpp"
#include "rfcommon/RealtimePlugin.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/SessionMetaData.hpp"
#include "rfcommon/StandalonePlugin.hpp"
#include "rfcommon/VisualizerPlugin.hpp"
#include "rfcommon/dynlib.h"
#include <cassert>
#include <QDebug>
#include <QWidget>
#include <QDir>

namespace rfapp {

// ----------------------------------------------------------------------------
PluginManager::PluginManager(Protocol* protocol)
    : protocol_(protocol)
{
    QDir pluginDir("share/reframed/plugins");
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
    if (i->start(APP_VERSION, &pluginError) != 0)
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
    QVector<QString> list;
    for (auto factory = factories_.begin(); factory != factories_.end(); ++factory)
    {
        if (!!(factory.value()->type & type))
            list.push_back(factory.value()->info.name);
    }
    return list;
}

// ----------------------------------------------------------------------------
const RFPluginFactoryInfo* PluginManager::getFactoryInfo(const QString &name) const
{
    auto it = factories_.find(name);
    if (it == factories_.end())
        return nullptr;

    RFPluginFactory* factory = it.value();
    return &factory->info;
}

// ----------------------------------------------------------------------------
rfcommon::AnalyzerPlugin* PluginManager::createAnalyzerModel(const QString& name)
{
    // XXX is dynamic_cast required here due to some of these having multiple inheritance?
    return static_cast<rfcommon::AnalyzerPlugin*>(createModel(name, RFPluginType::ANALYZER));
}

// ----------------------------------------------------------------------------
rfcommon::VisualizerPlugin* PluginManager::createVisualizerModel(const QString& name)
{
    return static_cast<rfcommon::VisualizerPlugin*>(createModel(name, RFPluginType::VISUALIZER));
}

// ----------------------------------------------------------------------------
rfcommon::RealtimePlugin* PluginManager::createRealtimeModel(const QString& name)
{
    return static_cast<rfcommon::RealtimePlugin*>(createModel(name, RFPluginType::REALTIME));
}

// ----------------------------------------------------------------------------
rfcommon::StandalonePlugin* PluginManager::createStandaloneModel(const QString& name)
{
    return static_cast<rfcommon::StandalonePlugin*>(createModel(name, RFPluginType::STANDALONE));
}

// ----------------------------------------------------------------------------
rfcommon::Plugin* PluginManager::createModel(const QString& name, RFPluginType type)
{
    auto it = factories_.find(name);
    if (it == factories_.end())
        return nullptr;

    RFPluginFactory* factory = it.value();
    if (!(factory->type & type))
        return nullptr;

    rfcommon::Plugin* model = factory->createModel(factory);
    if (model == nullptr)
        return nullptr;

    rfcommon::RealtimePlugin* realtime = dynamic_cast<rfcommon::RealtimePlugin*>(model);
    if (realtime)
    {
        // Realtime plugins listen to protocol events
        protocol_->dispatcher.addListener(realtime);

        // It's possible the plugin was created during an active session. Some plugins
        // might be interested in hookin in to the middle of a match/training mode session
        if (rfcommon::Session* session = protocol_->activeSession())
        {
            rfcommon::SessionMetaData* meta = session->tryGetMetaData();
            assert(meta);

            switch (meta->type())
            {
                case rfcommon::SessionMetaData::GAME:
                    realtime->onProtocolGameResumed(protocol_->activeSession());
                    break;
                case rfcommon::SessionMetaData::TRAINING:
                    realtime->onProtocolTrainingResumed(protocol_->activeSession());
                    break;
            }
        }
    }

    return model;
}

// ----------------------------------------------------------------------------
void PluginManager::destroyModel(rfcommon::Plugin* model)
{
    RFPluginFactory* factory = model->factory();

    rfcommon::RealtimePlugin* realtime = dynamic_cast<rfcommon::RealtimePlugin*>(model);
    if (realtime)
        protocol_->dispatcher.removeListener(realtime);

    factory->destroyModel(model);
}

}
