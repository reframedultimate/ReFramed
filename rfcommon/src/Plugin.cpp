#include "rfcommon/Plugin.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/VisualizerContext.hpp"
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
Plugin::VisualizerInterface::VisualizerInterface(VisualizerContext* visCtx, const RFPluginFactory* factory)
    : visCtx_(visCtx)
    , name_(factory->info.name)
{
    visCtx_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
Plugin::VisualizerInterface::~VisualizerInterface()
{
    visCtx_->dispatcher.removeListener(this);
    clearVisualizerData();
}

// ----------------------------------------------------------------------------
int Plugin::VisualizerInterface::visualizerSourceCount() const
{
    return visCtx_->sources.count();
}

// ----------------------------------------------------------------------------
const char* Plugin::VisualizerInterface::visualizerName(int sourceIdx) const
{
    return (visCtx_->sources.begin() + sourceIdx)->key().cStr();
}

// ----------------------------------------------------------------------------
const VisualizerData& Plugin::VisualizerInterface::visualizerData(int sourceIdx) const
{
    return (visCtx_->sources.begin() + sourceIdx)->value();
}

// ----------------------------------------------------------------------------
void Plugin::VisualizerInterface::setVisualizerData(VisualizerData&& data)
{
    visCtx_->sources.insertAlways(name_, std::move(data));
    visCtx_->dispatcher.dispatch(&VisualizerInterface::onVisualizerDataChanged);
}

// ----------------------------------------------------------------------------
void Plugin::VisualizerInterface::clearVisualizerData()
{
    if (visCtx_->sources.erase(name_))
        visCtx_->dispatcher.dispatch(&VisualizerInterface::onVisualizerDataChanged);
}

}
