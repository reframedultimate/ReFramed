#pragma once

#include "uh/Vector.hpp"
#include <QWidget>

class QMdiArea;

namespace uh {
    class VisualizerPlugin;
}

namespace uhapp {

class PluginManager;

class VisualizerView : public QWidget
{
    Q_OBJECT

public:
    explicit VisualizerView(PluginManager* pluginManager, QWidget* parent=nullptr);
    ~VisualizerView();

private:
    PluginManager* pluginManager_;
    QMdiArea* mdiArea_;
};

}
