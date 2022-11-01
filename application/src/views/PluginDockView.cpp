#include "application/views/PluginDockView.hpp"
#include "application/models/PluginManager.hpp"
#include "application/models/Protocol.hpp"
#include "application/models/ReplayManager.hpp"

#include "rfcommon/Plugin.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/MetaData.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/VisualizerContext.hpp"

#include "ads/DockAreaWidget.h"
#include "ads/DockAreaTitleBar.h"

#include <QTabBar>
#include <QMenu>
#include <QAction>
#include <QToolButton>
#include <QStyle>
#include <QApplication>

namespace rfapp {

// ----------------------------------------------------------------------------
PluginDockView::PluginDockView(ReplayManager* replayManager, PluginManager* pluginManager, QWidget* parent)
    : CDockManager(parent)
    , protocol_(nullptr)
    , replayManager_(replayManager)
    , pluginManager_(pluginManager)
    , visCtx_(new rfcommon::VisualizerContext)
    , sessionState_(DISCONNECTED)
    , activeSessionState_(NO_ACTIVE_SESSION)
    , replayState_(NONE_LOADED)
{
    ads::CDockWidget* pluginHomePage = new ads::CDockWidget("Home");
    pluginHomePage->setWidget(new QWidget);
    pluginHomePage->setFeature(ads::CDockWidget::DockWidgetClosable, false);
    pluginHomePage->setFeature(ads::CDockWidget::DockWidgetFloatable, false);
    ads::CDockAreaWidget* dockArea = addDockWidget(ads::CenterDockWidgetArea, pluginHomePage);
    PluginDockView::onDockAreaCreated(dockArea);

    connect(this, &CDockManager::dockAreaCreated, this, &PluginDockView::onDockAreaCreated);
}

// ----------------------------------------------------------------------------
PluginDockView::PluginDockView(Protocol* protocol, PluginManager* pluginManager, QWidget* parent)
    : CDockManager(parent)
    , protocol_(protocol)
    , replayManager_(nullptr)
    , pluginManager_(pluginManager)
    , visCtx_(new rfcommon::VisualizerContext)
    , sessionState_(DISCONNECTED)
    , activeSessionState_(NO_ACTIVE_SESSION)
    , replayState_(NONE_LOADED)
{
    ads::CDockWidget* pluginHomePage = new ads::CDockWidget("Home");
    pluginHomePage->setWidget(new QWidget);
    pluginHomePage->setFeature(ads::CDockWidget::DockWidgetClosable, false);
    pluginHomePage->setFeature(ads::CDockWidget::DockWidgetFloatable, false);
    ads::CDockAreaWidget* dockArea = addDockWidget(ads::CenterDockWidgetArea, pluginHomePage);
    PluginDockView::onDockAreaCreated(dockArea);

    protocol_->dispatcher.addListener(this);

    connect(this, &CDockManager::dockAreaCreated, this, &PluginDockView::onDockAreaCreated);
}

// ----------------------------------------------------------------------------
PluginDockView::~PluginDockView()
{
    if (protocol_)
        protocol_->dispatcher.removeListener(this);

    clearActiveSession();
    clearReplays();

    for (const auto& data : plugins_)
    {
        data.view->setParent(nullptr);
        data.plugin->uiInterface()->destroyView(data.view);
        pluginManager_->destroy(data.plugin);
    }
}

// ----------------------------------------------------------------------------
void PluginDockView::loadGameReplays(const QStringList& fileNames)
{
    PROFILE(PluginDockView, loadGameReplays);

    clearReplays();

    QStringList loadedFileNames;
    activeReplays_.clear();
    for (auto fileName : fileNames)
    {
        assert(QDir(fileName).isAbsolute());
        auto session = rfcommon::Session::load(replayManager_, fileName.toLocal8Bit().constData());
        if (session == nullptr)
        {
            activeReplays_.clear();
            return;
        }

        activeReplays_.push_back(session);
    }

    if (activeReplays_.size() == 1)
    {
        for (const auto& data : plugins_)
            if (auto i = data.plugin->replayInterface())
                i->onGameSessionLoaded(activeReplays_[0]);
    }
    else if (activeReplays_.size() > 1)
    {
        rfcommon::SmallVector<rfcommon::Session*, 16> replayArray;
        for (const auto& r : activeReplays_)
            replayArray.emplace(r);

        for (const auto& data : plugins_)
            if (auto i = data.plugin->replayInterface())
                i->onGameSessionSetLoaded(replayArray.data(), replayArray.count());
    }

    // Only switch states if we successfully loaded a replay
    if (activeReplays_.size() > 0)
        replayState_ = GAME_LOADED;
}

// ----------------------------------------------------------------------------
void PluginDockView::clearReplays()
{
    PROFILE(PluginDockView, clearReplays);

    if (activeReplays_.size() == 1)
    {
        for (const auto& data : plugins_)
            if (auto i = data.plugin->replayInterface())
                i->onGameSessionUnloaded(activeReplays_[0]);
    }
    else if (activeReplays_.size() > 1)
    {
        rfcommon::SmallVector<rfcommon::Session*, 16> replayArray;
        for (const auto& r : activeReplays_)
            replayArray.emplace(r);

        for (const auto& data : plugins_)
            if (auto i = data.plugin->replayInterface())
                i->onGameSessionSetUnloaded(replayArray.data(), replayArray.count());
    }

    activeReplays_.clear();

    replayState_ = NONE_LOADED;
}

// ----------------------------------------------------------------------------
void PluginDockView::clearActiveSession()
{
    PROFILE(PluginDockView, clearActiveSession);

    if (activeSessionState_ != NO_ACTIVE_SESSION
        && activeSessionState_ != TRAINING_STARTED_ENDED
        && activeSessionState_ != TRAINING_RESUMED_ENDED
        && activeSessionState_ != GAME_STARTED_ENDED
        && activeSessionState_ != GAME_RESUMED_ENDED)
    {
        assert(activeSession_.notNull());
        assert(activeSession_->tryGetMetaData());

        switch (activeSession_->tryGetMetaData()->type())
        {
            case rfcommon::MetaData::GAME:
                for (const auto& data : plugins_)
                    if (auto i = data.plugin->realtimeInterface())
                        i->onProtocolGameEnded(activeSession_);
                break;

            case rfcommon::MetaData::TRAINING:
                for (const auto& data : plugins_)
                    if (auto i = data.plugin->realtimeInterface())
                        i->onProtocolTrainingEnded(activeSession_);
                break;
        }

        activeSession_.drop();
        activeSessionState_ = NO_ACTIVE_SESSION;
    }
}

// ----------------------------------------------------------------------------
void PluginDockView::onAddNewPluginRequested(ads::CDockAreaWidget* dockArea)
{
    PROFILE(PluginDockView, onTabBarClicked);

    auto pluginLoaded = [this](const QString& name) -> bool {
        for (const auto& data : plugins_)
            if (data.name == name)
                return true;
        return false;
    };

    // Create a list of all available plugins that can be loaded. There are
    // two modes here. Either this class was created with a valid protocol
    // object, in which case we look for plugins that implement the realtime
    // API, or we were not given a protocol (nullptr), in which case we
    // assume we're in replay mode and look for plugins that implement the
    // replay API
    auto pluginNames = protocol_ ?
            pluginManager_->availableFactoryNames(RFPluginType::UI | RFPluginType::REALTIME) :
            pluginManager_->availableFactoryNames(RFPluginType::UI | RFPluginType::REPLAY);
    std::sort(pluginNames.begin(), pluginNames.end());

    // Open popup menu with all of the plugins that aren't loaded yet
    QMenu popup;
    for (const auto& name : pluginNames)
    {
        if (pluginLoaded(name))
            continue;
        popup.addAction(name);
    }

    QAction* action = popup.exec(QCursor::pos());
    if (action == nullptr)
        return;

    // Create plugin
    PluginData data;
    data.name = action->text();
    data.plugin = pluginManager_->create(data.name, visCtx_);
    if (data.plugin == nullptr)
        return;

    data.view = nullptr;
    if (auto i = data.plugin->uiInterface())
        data.view = i->createView();
    if (data.view == nullptr)
    {
        pluginManager_->destroy(data.plugin);
        return;
    }
    plugins_.push_back(data);

    // Create tab and place view from plugin inside it
    ads::CDockWidget* dockWidget = new ads::CDockWidget(data.name);
    dockWidget->setWidget(data.view);
    dockWidget->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, true);
    dockWidget->setFeature(ads::CDockWidget::CustomCloseHandling, true);
    addDockWidget(ads::CenterDockWidgetArea, dockWidget, dockArea);
    connect(dockWidget, &ads::CDockWidget::closeRequested, [this, dockWidget] {
        onClosePluginRequested(dockWidget);
    });

    switch (sessionState_)
    {
    case ATTEMPT_CONNECT:
        if (auto i = data.plugin->realtimeInterface())
            i->onProtocolAttemptConnectToServer(ipAddress_.toUtf8().constData(), port_);
        break;
    case CONNECTED:
        if (auto i = data.plugin->realtimeInterface())
        {
            i->onProtocolAttemptConnectToServer(ipAddress_.toUtf8().constData(), port_);
            i->onProtocolConnectedToServer(ipAddress_.toUtf8().constData(), port_);
        }
        break;
    case DISCONNECTED:
        break;
    }

    // See if there is an active session we can give to the new plugin
    switch (activeSessionState_)
    {
    case TRAINING_STARTED:
        assert(activeSession_.notNull());
        if (auto i = data.plugin->realtimeInterface())
            i->onProtocolTrainingStarted(activeSession_);
        break;

    case TRAINING_RESUMED:
        assert(activeSession_.notNull());
        if (auto i = data.plugin->realtimeInterface())
            i->onProtocolTrainingResumed(activeSession_);
        break;

    case TRAINING_STARTED_ENDED:
        assert(activeSession_.notNull());
        if (auto i = data.plugin->realtimeInterface())
        {
            switch (sessionState_)
            {
            case DISCONNECTED:
                i->onProtocolAttemptConnectToServer(ipAddress_.toUtf8().constData(), port_);
                i->onProtocolConnectedToServer(ipAddress_.toUtf8().constData(), port_);
                i->onProtocolTrainingStarted(activeSession_);
                i->onProtocolTrainingEnded(activeSession_);
                i->onProtocolDisconnectedFromServer();
                break;
            case ATTEMPT_CONNECT:
                i->onProtocolConnectedToServer(ipAddress_.toUtf8().constData(), port_);
                i->onProtocolTrainingStarted(activeSession_);
                i->onProtocolTrainingEnded(activeSession_);
                i->onProtocolDisconnectedFromServer();
                i->onProtocolAttemptConnectToServer(ipAddress_.toUtf8().constData(), port_);
                break;
            case CONNECTED:
                i->onProtocolTrainingStarted(activeSession_);
                i->onProtocolTrainingEnded(activeSession_);
                break;
            }
        }
        break;

    case TRAINING_RESUMED_ENDED:
        assert(activeSession_.notNull());
        if (auto i = data.plugin->realtimeInterface())
        {
            switch (sessionState_)
            {
            case DISCONNECTED:
                i->onProtocolAttemptConnectToServer(ipAddress_.toUtf8().constData(), port_);
                i->onProtocolConnectedToServer(ipAddress_.toUtf8().constData(), port_);
                i->onProtocolTrainingResumed(activeSession_);
                i->onProtocolTrainingEnded(activeSession_);
                i->onProtocolDisconnectedFromServer();
                break;
            case ATTEMPT_CONNECT:
                i->onProtocolConnectedToServer(ipAddress_.toUtf8().constData(), port_);
                i->onProtocolTrainingResumed(activeSession_);
                i->onProtocolTrainingEnded(activeSession_);
                i->onProtocolDisconnectedFromServer();
                i->onProtocolAttemptConnectToServer(ipAddress_.toUtf8().constData(), port_);
                break;
            case CONNECTED:
                i->onProtocolTrainingResumed(activeSession_);
                i->onProtocolTrainingEnded(activeSession_);
                break;
            }
        }
        break;

    case GAME_STARTED:
        assert(activeSession_.notNull());
        if (auto i = data.plugin->realtimeInterface())
            i->onProtocolGameStarted(activeSession_);
        break;

    case GAME_RESUMED:
        assert(activeSession_.notNull());
        if (auto i = data.plugin->realtimeInterface())
            i->onProtocolGameResumed(activeSession_);
        break;

    case GAME_STARTED_ENDED:
        assert(activeSession_.notNull());
        if (auto i = data.plugin->realtimeInterface())
        {
            switch (sessionState_)
            {
            case DISCONNECTED:
                i->onProtocolAttemptConnectToServer(ipAddress_.toUtf8().constData(), port_);
                i->onProtocolConnectedToServer(ipAddress_.toUtf8().constData(), port_);
                i->onProtocolGameStarted(activeSession_);
                i->onProtocolGameEnded(activeSession_);
                i->onProtocolDisconnectedFromServer();
                break;
            case ATTEMPT_CONNECT:
                i->onProtocolConnectedToServer(ipAddress_.toUtf8().constData(), port_);
                i->onProtocolGameStarted(activeSession_);
                i->onProtocolGameEnded(activeSession_);
                i->onProtocolDisconnectedFromServer();
                i->onProtocolAttemptConnectToServer(ipAddress_.toUtf8().constData(), port_);
                break;
            case CONNECTED:
                i->onProtocolGameStarted(activeSession_);
                i->onProtocolGameEnded(activeSession_);
                break;
            }
        }
        break;

    case GAME_RESUMED_ENDED:
        assert(activeSession_.notNull());
        if (auto i = data.plugin->realtimeInterface())
        {
            switch (sessionState_)
            {
            case DISCONNECTED:
                i->onProtocolAttemptConnectToServer(ipAddress_.toUtf8().constData(), port_);
                i->onProtocolConnectedToServer(ipAddress_.toUtf8().constData(), port_);
                i->onProtocolGameResumed(activeSession_);
                i->onProtocolGameEnded(activeSession_);
                i->onProtocolDisconnectedFromServer();
                break;
            case ATTEMPT_CONNECT:
                i->onProtocolConnectedToServer(ipAddress_.toUtf8().constData(), port_);
                i->onProtocolGameResumed(activeSession_);
                i->onProtocolGameEnded(activeSession_);
                i->onProtocolDisconnectedFromServer();
                i->onProtocolAttemptConnectToServer(ipAddress_.toUtf8().constData(), port_);
                break;
            case CONNECTED:
                i->onProtocolGameResumed(activeSession_);
                i->onProtocolGameEnded(activeSession_);
                break;
            }
        }
        break;

    case NO_ACTIVE_SESSION:
        switch (replayState_)
        {
        case GAME_LOADED:
            assert(activeReplays_.size() > 0);
            if (activeReplays_.size() == 1)
            {
                if (auto i = data.plugin->replayInterface())
                    i->onGameSessionLoaded(activeReplays_[0]);
            }
            else
            {
                rfcommon::SmallVector<rfcommon::Session*, 16> replayArray;
                for (const auto& r : activeReplays_)
                    replayArray.emplace(r);

                if (auto i = data.plugin->replayInterface())
                    i->onGameSessionSetLoaded(replayArray.data(), replayArray.count());
            }
            break;

        case TRAINING_LOADED:
            assert(activeReplays_.size() == 1);
            if (auto i = data.plugin->replayInterface())
                i->onTrainingSessionLoaded(activeReplays_[0]);
            break;

        case NONE_LOADED: break;
        }
    }
}

// ----------------------------------------------------------------------------
void PluginDockView::onClosePluginRequested(ads::CDockWidget* dockWidget)
{
    PROFILE(PluginDockView, closeTabWithView);

    for (auto it = plugins_.begin(); it != plugins_.end(); ++it)
    {
        if (it->view != dockWidget->widget())
            continue;

        switch (activeSessionState_)
        {
            case TRAINING_STARTED:
            case TRAINING_RESUMED:
                assert(activeSession_.notNull());
                assert(activeSession_->tryGetMetaData());
                assert(activeSession_->tryGetMetaData()->type() == rfcommon::MetaData::TRAINING);
                if (auto i = it->plugin->realtimeInterface())
                    i->onProtocolTrainingEnded(activeSession_);
                break;

            case GAME_STARTED:
            case GAME_RESUMED:
                assert(activeSession_.notNull());
                assert(activeSession_->tryGetMetaData());
                assert(activeSession_->tryGetMetaData()->type() == rfcommon::MetaData::GAME);
                if (auto i = it->plugin->realtimeInterface())
                    i->onProtocolGameEnded(activeSession_);
                break;

            case NO_ACTIVE_SESSION:
            case TRAINING_STARTED_ENDED:
            case TRAINING_RESUMED_ENDED:
            case GAME_STARTED_ENDED:
            case GAME_RESUMED_ENDED:
                break;
        }

        if (activeReplays_.size() == 1)
        {
            if (auto i = it->plugin->replayInterface())
                i->onGameSessionUnloaded(activeReplays_[0]);
        }
        else if (activeReplays_.size() > 1)
        {
            rfcommon::SmallVector<rfcommon::Session*, 16> replayArray;
            for (const auto& r : activeReplays_)
                replayArray.emplace(r);

            if (auto i = it->plugin->replayInterface())
                i->onGameSessionSetUnloaded(replayArray.data(), replayArray.count());
        }

        switch (sessionState_)
        {
        case ATTEMPT_CONNECT:
            if (auto i = it->plugin->realtimeInterface())
                i->onProtocolFailedToConnectToServer("Plugin is being destroyed", ipAddress_.toUtf8().constData(), port_);
            break;
        case CONNECTED:
            if (auto i = it->plugin->realtimeInterface())
                i->onProtocolDisconnectedFromServer();
        case DISCONNECTED:
            break;
        }

        it->view->setParent(nullptr);
        it->plugin->uiInterface()->destroyView(it->view);
        pluginManager_->destroy(it->plugin);
        plugins_.erase(it);

        dockWidget->closeDockWidget();

        break;
    }
}

// ----------------------------------------------------------------------------
void PluginDockView::onDockAreaCreated(ads::CDockAreaWidget* dockArea)
{
    QToolButton* launchPluginButton = new QToolButton;
    launchPluginButton->setText("+");

    ads::CDockAreaTitleBar* titleBar = dockArea->titleBar();
    int i = titleBar->indexOf(titleBar->button(ads::TitleBarButtonTabsMenu));
    dockArea->titleBar()->insertWidget(i - 1, launchPluginButton);

    connect(launchPluginButton, &QToolButton::released, [this, dockArea] {
        onAddNewPluginRequested(dockArea);
    });
}

// ----------------------------------------------------------------------------
void PluginDockView::onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port)
{
    PROFILE(PluginDockView, onProtocolAttemptConnectToServer);

    sessionState_ = ATTEMPT_CONNECT;
    ipAddress_ = ipAddress;
    port_ = port;

    for (const auto& data : plugins_)
        if (auto i = data.plugin->realtimeInterface())
            i->onProtocolAttemptConnectToServer(ipAddress, port);
}
void PluginDockView::onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port)
{
    PROFILE(PluginDockView, onProtocolFailedToConnectToServer);

    sessionState_ = DISCONNECTED;

    for (const auto& data : plugins_)
        if (auto i = data.plugin->realtimeInterface())
            i->onProtocolFailedToConnectToServer(errormsg, ipAddress, port);
}
void PluginDockView::onProtocolConnectedToServer(const char* ipAddress, uint16_t port)
{
    PROFILE(PluginDockView, onProtocolConnectedToServer);

    sessionState_ = CONNECTED;

    for (const auto& data : plugins_)
        if (auto i = data.plugin->realtimeInterface())
            i->onProtocolConnectedToServer(ipAddress, port);
}
void PluginDockView::onProtocolDisconnectedFromServer()
{
    PROFILE(PluginDockView, onProtocolDisconnectedFromServer);

    sessionState_ = DISCONNECTED;

    for (const auto& data : plugins_)
        if (auto i = data.plugin->realtimeInterface())
            i->onProtocolDisconnectedFromServer();
}

// ----------------------------------------------------------------------------
void PluginDockView::onProtocolTrainingStarted(rfcommon::Session* training)
{
    PROFILE(PluginDockView, onProtocolTrainingStarted);

    activeSessionState_ = TRAINING_STARTED;
    activeSession_ = training;

    // If there are replays being displayed currently, override it with
    // the active session. Seems to make sense from a user perspective
    clearReplays();
    for (const auto& data : plugins_)
        if (auto i = data.plugin->realtimeInterface())
            i->onProtocolTrainingStarted(training);
}
void PluginDockView::onProtocolTrainingResumed(rfcommon::Session* training)
{
    PROFILE(PluginDockView, onProtocolTrainingResumed);

    activeSessionState_ = TRAINING_RESUMED;
    activeSession_ = training;

    // If there are replays being displayed currently, override it with
    // the active session. Seems to make sense from a user perspective
    clearReplays();
    for (const auto& data : plugins_)
        if (auto i = data.plugin->realtimeInterface())
            i->onProtocolTrainingResumed(training);
}
void PluginDockView::onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining)
{
    PROFILE(PluginDockView, onProtocolTrainingReset);

    assert(activeSession_ == oldTraining);
    activeSession_ = newTraining;

    // If there are replays being displayed currently, then those have
    // precedence. Store the active session for when the user deselects
    // replays in the UI, or until a session start event is received
    if (replayState_ == NONE_LOADED)
    {
        for (const auto& data : plugins_)
            if (auto i = data.plugin->realtimeInterface())
                i->onProtocolTrainingReset(oldTraining, newTraining);
    }
}
void PluginDockView::onProtocolTrainingEnded(rfcommon::Session* training)
{
    PROFILE(PluginDockView, onProtocolTrainingEnded);

    assert(activeSession_ == training);
    activeSessionState_ = activeSessionState_ == TRAINING_STARTED ? TRAINING_STARTED_ENDED : TRAINING_RESUMED_ENDED;

    // If there are replays being displayed currently, then those
    // have precendence and we don't need to propagate the training
    // ended event
    if (replayState_ == NONE_LOADED)
    {
        for (const auto& data : plugins_)
            if (auto i = data.plugin->realtimeInterface())
                i->onProtocolTrainingEnded(training);
    }
}
void PluginDockView::onProtocolGameStarted(rfcommon::Session* game)
{
    PROFILE(PluginDockView, onProtocolGameStarted);

    activeSessionState_ = GAME_STARTED;
    activeSession_ = game;

    // If there are replays being displayed currently, override it with
    // the active session. Seems to make sense from a user perspective
    clearReplays();
    for (const auto& data : plugins_)
        if (auto i = data.plugin->realtimeInterface())
            i->onProtocolGameStarted(game);
}
void PluginDockView::onProtocolGameResumed(rfcommon::Session* game)
{
    PROFILE(PluginDockView, onProtocolGameResumed);

    activeSessionState_ = GAME_RESUMED;
    activeSession_ = game;

    // If there are replays being displayed currently, override it with
    // the active session. Seems to make sense from a user perspective
    clearReplays();
    for (const auto& data : plugins_)
        if (auto i = data.plugin->realtimeInterface())
            i->onProtocolGameResumed(game);
}
void PluginDockView::onProtocolGameEnded(rfcommon::Session* game)
{
    PROFILE(PluginDockView, onProtocolGameEnded);

    assert(activeSession_ == game);
    activeSessionState_ = activeSessionState_ == GAME_STARTED ? GAME_STARTED_ENDED : GAME_RESUMED_ENDED;

    // If there are replays being displayed currently, then those
    // have precendence and we don't need to propagate the training
    // ended event
    if (replayState_ == NONE_LOADED)
    {
        for (const auto& data : plugins_)
            if (auto i = data.plugin->realtimeInterface())
                i->onProtocolGameEnded(game);
    }
}

}
