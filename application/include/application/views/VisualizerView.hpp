#pragma once

#include "rfcommon/Vector.hpp"
#include <QWidget>

class QMdiArea;

namespace rfcommon {
    class VisualizerPlugin;
}

namespace rfapp {

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
