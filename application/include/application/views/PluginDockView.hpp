#pragma once

#include "ads/DockManager.h"

namespace rfapp {

class PluginDockView : public ads::CDockManager
{
    Q_OBJECT

public:
    explicit PluginDockView(QWidget* parent=nullptr);
    ~PluginDockView();

private:
};

}
