#pragma once

#include "rfcommon/LinearMap.hpp"
#include <QWidget>

class QTabWidget;
class QMenu;

namespace Ui {
    class AnalysisInputView;
}

namespace rfcommon {
    class Plugin;
}

namespace rfapp {

class PluginManager;

class AnalysisView : public QWidget
{
    Q_OBJECT

public:
    explicit AnalysisView(PluginManager* pluginManager, QWidget* parent=nullptr);
    ~AnalysisView();

private slots:
    void onTabBarClicked(int index);
    void onTabIndexChanged(int index);

private:
    void closeTab(QWidget* widget);

private:
    PluginManager* pluginManager_;
    Ui::AnalysisInputView* inputUi_;
    QTabWidget* tabWidget_;
    QMenu* addMenu_;

    struct LoadedPlugin
    {
        rfcommon::Plugin* plugin;
        QWidget* view;
    };

    QHash<QString, LoadedPlugin> loadedPlugins_;
};

}
