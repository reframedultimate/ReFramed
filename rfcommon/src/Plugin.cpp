#include "rfcommon/Plugin.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/PluginContext.hpp"
#include "rfcommon/PluginInterface.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
Plugin::Plugin(const RFPluginFactory* factory)
    : factory_(factory)
{
}

// ----------------------------------------------------------------------------
Plugin::~Plugin()
{
}

// ----------------------------------------------------------------------------
const RFPluginFactory* Plugin::factory() const
{
    NOPROFILE();

    return factory_;
}

// ----------------------------------------------------------------------------
Plugin::SharedDataInterface::SharedDataInterface(PluginContext* pluginCtx, const RFPluginFactory* factory)
    : pluginCtx_(pluginCtx)
    , name_(factory->info.name)
{
    pluginCtx_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
Plugin::SharedDataInterface::~SharedDataInterface()
{
    pluginCtx_->dispatcher.removeListener(this);
    clearSharedData();
}

// ----------------------------------------------------------------------------
int Plugin::SharedDataInterface::sharedDataSourceCount() const
{
    return pluginCtx_->sources.count();
}

// ----------------------------------------------------------------------------
const char* Plugin::SharedDataInterface::sharedDataName(int sourceIdx) const
{
    return (pluginCtx_->sources.begin() + sourceIdx)->key().cStr();
}

// ----------------------------------------------------------------------------
const PluginSharedData& Plugin::SharedDataInterface::sharedData(int sourceIdx) const
{
    return (pluginCtx_->sources.begin() + sourceIdx)->value();
}

// ----------------------------------------------------------------------------
void Plugin::SharedDataInterface::setSharedData(PluginSharedData&& data)
{
    pluginCtx_->sources.insertAlways(name_, std::move(data));
    pluginCtx_->dispatcher.dispatch(&SharedDataInterface::onSharedDataChanged);
}

// ----------------------------------------------------------------------------
void Plugin::SharedDataInterface::clearSharedData()
{
    if (pluginCtx_->sources.erase(name_))
        pluginCtx_->dispatcher.dispatch(&SharedDataInterface::onSharedDataChanged);
}

}
