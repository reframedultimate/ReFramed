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
    auto it = visCtx_->sources.insertIfNew(name_, VisualizerData());
    assert(it != visCtx_->sources.end());
    visCtx_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
Plugin::VisualizerInterface::~VisualizerInterface()
{
    visCtx_->dispatcher.removeListener(this);
    bool found = visCtx_->sources.erase(name_);
    assert(found);

    visCtx_->dispatcher.dispatch(&VisualizerInterface::onVisualizerDataChanged);
}

// ----------------------------------------------------------------------------
int Plugin::VisualizerInterface::visualizerSourceCount() const
{
    return visCtx_->sources.count();
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

}
