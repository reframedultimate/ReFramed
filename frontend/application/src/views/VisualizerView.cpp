#include "application/views/VisualizerView.hpp"
#include "application/models/PluginManager.hpp"
#include "uh/VisualizerPlugin.hpp"

#include <QVBoxLayout>
#include <QMdiArea>

namespace uhapp {

class PluginWidget : public QWidget
{
public:
    PluginWidget(PluginManager* manager, const char* name)
        : manager_(manager)
    {
        setLayout(new QVBoxLayout);
        plugin_ = manager_->createVisualizer(name);
        if (plugin_)
        {
            widget_ = plugin_->createView();
            layout()->addWidget(widget_);
        }
    }

    ~PluginWidget()
    {
        if (plugin_)
        {
            widget_->setParent(nullptr);
            plugin_->destroyView(widget_);
            manager_->destroy(plugin_);
        }
    }

private:
    PluginManager* manager_;
    uh::VisualizerPlugin* plugin_ = nullptr;
    QWidget* widget_ = nullptr;
};

// ----------------------------------------------------------------------------
VisualizerView::VisualizerView(PluginManager* pluginManager, QWidget* parent)
    : QWidget(parent)
    , pluginManager_(pluginManager)
    , mdiArea_(new QMdiArea)
{
    setLayout(new QVBoxLayout);

    layout()->addWidget(mdiArea_);

    QWidget* video = new PluginWidget(pluginManager_, "Video Player");
    mdiArea_->addSubWindow(video);
    video->showMaximized();
}

// ----------------------------------------------------------------------------
VisualizerView::~VisualizerView()
{
}

}
