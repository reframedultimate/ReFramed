#include "application/views/VisualizerView.hpp"
#include "application/models/PluginManager.hpp"
#include "rfcommon/VisualizerPlugin.hpp"

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
        model_ = manager_->createVisualizerModel(name);
        if (model_)
        {
            view_ = model_->createView();
            if (view_)
                layout()->addWidget(view_);
        }
    }

    ~PluginWidget()
    {
        if (model_)
        {
            if (view_)
                model_->destroyView(view_);

            manager_->destroyModel(model_);
        }
    }

private:
    PluginManager* manager_;
    rfcommon::VisualizerPlugin* model_ = nullptr;
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
