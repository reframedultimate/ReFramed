#include "application/views/VisualizerView.hpp"
#include "application/models/PluginManager.hpp"
#include "rfcommon/Plugin.hpp"

#include <QVBoxLayout>
#include <QMdiArea>

namespace rfapp {

class PluginWidget : public QWidget
{
public:
    PluginWidget(PluginManager* manager, const char* name)
        : manager_(manager)
    {
        setLayout(new QVBoxLayout);
        plugin_ = manager_->create(name);
        if (plugin_)
        {
            if (auto i = plugin_->uiInterface())
                view_ = i->createView();
            if (view_)
                layout()->addWidget(view_);
        }
    }

    ~PluginWidget()
    {
        if (plugin_)
        {
            if (view_)
                plugin_->uiInterface()->destroyView(view_);

            manager_->destroy(plugin_);
        }
    }

private:
    PluginManager* manager_;
    rfcommon::Plugin* plugin_ = nullptr;
    QWidget* view_ = nullptr;
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
