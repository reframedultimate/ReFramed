#include "application/ui_SessionView.h"
#include "application/views/SessionView.hpp"
#include "application/models/PluginManager.hpp"
#include "rfcommon/RealtimePlugin.hpp"
#include "rfcommon/Session.hpp"
#include <QTabBar>
#include <QMenu>
#include <QAction>

namespace rfapp {

// ----------------------------------------------------------------------------
SessionView::SessionView(PluginManager* pluginManager, QWidget* parent)
    : QTabWidget(parent)
    , pluginManager_(pluginManager)
    , previousTab_(0)
{
    addTab(new QWidget, "+");
    connect(this, &QTabWidget::tabBarClicked, this, &SessionView::onTabBarClicked);
    connect(this, &QTabWidget::currentChanged, this, &SessionView::onCurrentTabChanged);
}

// ----------------------------------------------------------------------------
SessionView::~SessionView()
{
    for (const auto& data : plugins_)
    {
        data.view->setParent(nullptr);
        data.plugin->destroyView(data.view);
        pluginManager_->destroyModel(data.plugin);
    }
}

// ----------------------------------------------------------------------------
void SessionView::setSavedGameSession(rfcommon::Session* session)
{
    for (const auto& data : plugins_)
        data.plugin->onGameSessionLoaded(session);
}

// ----------------------------------------------------------------------------
void SessionView::clearSavedGameSession(rfcommon::Session* session)
{
    for (const auto& data : plugins_)
        data.plugin->onGameSessionUnloaded(session);
}

// ----------------------------------------------------------------------------
void SessionView::setSavedGameSessionSet(rfcommon::Session** sessions, int count)
{
    for (const auto& data : plugins_)
        data.plugin->onGameSessionSetLoaded(sessions, count);
}

// ----------------------------------------------------------------------------
void SessionView::clearSavedGameSessionSet(rfcommon::Session** sessions, int count)
{
    for (const auto& data : plugins_)
        data.plugin->onGameSessionSetUnloaded(sessions, count);
}

// ----------------------------------------------------------------------------
void SessionView::onTabBarClicked(int index)
{
    if (index < count() - 1 || index < 0)
        return;

    auto pluginLoaded = [this](const QString& name) -> bool {
        for (const auto& data : plugins_)
            if (data.name == name)
                return true;
        return false;
    };

    QMenu popup;
    for (const auto& name : pluginManager_->availableFactoryNames(RFPluginType::REALTIME))
    {
        if (pluginLoaded(name))
            continue;
        popup.addAction(name);
    }

    QAction* action = popup.exec(QCursor::pos());
    if (action == nullptr)
        return;

    PluginData data;
    data.name = action->text();
    data.plugin = pluginManager_->createRealtimeModel(data.name);
    if (data.plugin == nullptr)
        return;
    data.view = data.plugin->createView();
    if (data.view == nullptr)
    {
        pluginManager_->destroyModel(data.plugin);
        return;
    }
    plugins_.push_back(data);

    insertTab(index, data.view, data.name);
    previousTab_ = index;
}

// ----------------------------------------------------------------------------
void SessionView::onCurrentTabChanged(int index)
{
    if (index > 0 && index < count() - 1)
        previousTab_ = index;
    else
        setCurrentIndex(previousTab_);
}

}
